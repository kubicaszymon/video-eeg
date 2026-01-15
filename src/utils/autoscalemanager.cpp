#include "autoscalemanager.h"
#include <QDebug>
#include <algorithm>
#include <cmath>

AutoScaleManager::AutoScaleManager(QObject *parent)
    : QObject{parent}
{
    m_recentRanges.reserve(TREND_BUFFER_SIZE);
}

void AutoScaleManager::reset()
{
    m_globalMin = std::numeric_limits<double>::infinity();
    m_globalMax = -std::numeric_limits<double>::infinity();
    m_globalSum = 0.0;
    m_globalSumSquares = 0.0;

    m_calibratedMin = 0.0;
    m_calibratedMax = 0.0;
    m_calibratedRange = 0.0;

    m_scaleFactor = 1.0;
    m_detectedUnit = DataUnit::Unknown;
    m_totalSamplesProcessed = 0;
    m_recentRanges.clear();

    CalibrationState oldState = m_calibrationState;
    m_calibrationState = CalibrationState::NotStarted;

    if (oldState != m_calibrationState)
    {
        emit calibrationStateChanged(m_calibrationState);
    }

    qDebug() << "[AutoScale] Reset - ready for new calibration";
}

void AutoScaleManager::processChunk(const std::vector<std::vector<float>>& chunk,
                                    const QVector<int>& selectedChannelIndices)
{
    if (chunk.empty() || chunk[0].empty() || selectedChannelIndices.isEmpty())
    {
        return;
    }

    // Poprzedni stan do detekcji zmian
    CalibrationState previousState = m_calibrationState;
    double previousScaleFactor = m_scaleFactor;
    DataUnit previousUnit = m_detectedUnit;

    // Aktualizuj statystyki
    updateStatistics(chunk, selectedChannelIndices);

    // Już przy pierwszej paczce danych ustalamy skalę - nie ma potrzeby długiej kalibracji
    if (m_calibrationState == CalibrationState::NotStarted)
    {
        // Ustaw skalę od razu na podstawie pierwszych danych
        m_calibratedMin = m_globalMin;
        m_calibratedMax = m_globalMax;
        m_calibratedRange = m_calibratedMax - m_calibratedMin;

        // Wykryj jednostkę
        m_detectedUnit = detectUnit(m_calibratedMin, m_calibratedMax);

        // Oblicz bazowy współczynnik skalowania
        m_scaleFactor = calculateOptimalScaleFactor(m_calibratedRange, m_targetSpacing);

        m_calibrationState = CalibrationState::Calibrated;

        qDebug() << "[AutoScale] Scale set from first data:"
                 << "range:" << m_calibratedMin << "to" << m_calibratedMax
                 << "(" << dataRangeInMicrovolts() << "μV)"
                 << "unit:" << unitString()
                 << "scaleFactor:" << m_scaleFactor;
    }
    // Po ustaleniu skali NIE zmieniamy jej automatycznie
    // Użytkownik kontroluje wzmocnienie przez suwak Gain

    // Emituj sygnały o zmianach
    if (previousState != m_calibrationState)
    {
        emit calibrationStateChanged(m_calibrationState);
    }

    if (!qFuzzyCompare(previousScaleFactor, m_scaleFactor))
    {
        emit scaleFactorChanged(m_scaleFactor);
    }

    if (previousUnit != m_detectedUnit)
    {
        emit detectedUnitChanged(m_detectedUnit);
    }
}

void AutoScaleManager::updateStatistics(const std::vector<std::vector<float>>& chunk,
                                        const QVector<int>& selectedChannelIndices)
{
    const int numSamples = static_cast<int>(chunk.size());
    const int totalChunkChannels = static_cast<int>(chunk[0].size());

    double chunkMin = std::numeric_limits<double>::infinity();
    double chunkMax = -std::numeric_limits<double>::infinity();

    // Przejdź przez WSZYSTKIE wybrane kanały i WSZYSTKIE próbki
    for (int sample = 0; sample < numSamples; ++sample)
    {
        for (int i = 0; i < selectedChannelIndices.size(); ++i)
        {
            int channelIndex = selectedChannelIndices[i];
            if (channelIndex < 0 || channelIndex >= totalChunkChannels)
            {
                continue;
            }

            double value = static_cast<double>(chunk[sample][channelIndex]);

            // Ignoruj wartości NaN i nieskończone
            if (!std::isfinite(value))
            {
                continue;
            }

            // Aktualizuj min/max chunk
            chunkMin = std::min(chunkMin, value);
            chunkMax = std::max(chunkMax, value);

            // Aktualizuj sumy do obliczenia średniej/wariancji
            m_globalSum += value;
            m_globalSumSquares += value * value;
        }

        m_totalSamplesProcessed++;
    }

    // Aktualizuj globalne min/max
    if (chunkMin < m_globalMin)
    {
        m_globalMin = chunkMin;
    }
    if (chunkMax > m_globalMax)
    {
        m_globalMax = chunkMax;
    }

    // Zapisz zakres chunk do bufora trendów
    double chunkRange = chunkMax - chunkMin;
    if (std::isfinite(chunkRange) && chunkRange > 0)
    {
        m_recentRanges.append(chunkRange);
        if (m_recentRanges.size() > TREND_BUFFER_SIZE)
        {
            m_recentRanges.removeFirst();
        }
    }
}

AutoScaleManager::DataUnit AutoScaleManager::detectUnit(double minVal, double maxVal) const
{
    // Zakres wartości
    double absMax = std::max(std::abs(minVal), std::abs(maxVal));

    // Wykrywanie na podstawie parametrów wzmacniacza i typowych zakresów EEG:
    //
    // Wzmacniacz ma:
    // - Zakres pomiarowy: ±341 mV
    // - Rozdzielczość: 24 bity (40 nV/bit)
    // - Typowy sygnał EEG: 10-100 μV = 0.01-0.1 mV
    //
    // Jeśli dane są w mV (zakres wzmacniacza):
    // - absMax będzie < 341 (ale typowo << 1 dla EEG)
    //
    // Jeśli dane są w μV:
    // - absMax będzie typowo 10-200 dla EEG
    //
    // Jeśli dane są w V:
    // - absMax będzie bardzo małe (< 0.001)

    if (absMax < 0.001)
    {
        // Bardzo małe wartości - prawdopodobnie wolty
        // (typowy EEG w V: 0.00001-0.0001)
        return DataUnit::Volts;
    }
    else if (absMax < 1.0)
    {
        // Wartości < 1 - prawdopodobnie miliwolty
        // (typowy EEG w mV: 0.01-0.1, zakres wzmacniacza: ±341 mV)
        return DataUnit::Millivolts;
    }
    else if (absMax < 1000)
    {
        // Wartości 1-1000 - prawdopodobnie mikrowolty
        // (typowy EEG w μV: 10-200)
        return DataUnit::Microvolts;
    }
    else if (absMax < 1000000)
    {
        // Duże wartości - prawdopodobnie raw ADC
        // (24-bit ADC: ±8388608)
        return DataUnit::RawADC;
    }
    else
    {
        // Bardzo duże wartości - nieznane
        return DataUnit::Unknown;
    }
}

QString AutoScaleManager::unitString() const
{
    switch (m_detectedUnit)
    {
    case DataUnit::Microvolts:
        return "μV";
    case DataUnit::Millivolts:
        return "mV";
    case DataUnit::Volts:
        return "V";
    case DataUnit::RawADC:
        return "raw";
    case DataUnit::Normalized:
        return "norm";
    case DataUnit::Unknown:
    default:
        return "?";
    }
}

QString AutoScaleManager::formatValue(double value) const
{
    switch (m_detectedUnit)
    {
    case DataUnit::Microvolts:
        return QString::number(value, 'f', 1) + " μV";
    case DataUnit::Millivolts:
        return QString::number(value * 1000, 'f', 2) + " μV";  // Konwertuj do μV
    case DataUnit::Volts:
        return QString::number(value * 1e6, 'f', 1) + " μV";   // Konwertuj do μV
    case DataUnit::RawADC:
        return QString::number(value, 'f', 0);
    default:
        return QString::number(value, 'g', 4);
    }
}

double AutoScaleManager::calculateOptimalScaleFactor(double dataRange, double targetSpacing) const
{
    if (dataRange <= 0 || !std::isfinite(dataRange))
    {
        return 1.0;
    }

    // Chcemy, żeby amplituda danych zajmowała targetAmplitudeRatio spacing
    double targetAmplitude = targetSpacing * m_targetAmplitudeRatio;

    // Współczynnik skalowania
    double scale = targetAmplitude / dataRange;

    // Ogranicz do rozsądnych wartości
    const double MIN_SCALE = 0.0001;
    const double MAX_SCALE = 100000.0;

    scale = std::clamp(scale, MIN_SCALE, MAX_SCALE);

    return scale;
}

double AutoScaleManager::dataRangeInMicrovolts() const
{
    double range = m_calibratedRange;
    if (range <= 0) range = m_globalMax - m_globalMin;
    if (!std::isfinite(range) || range <= 0) return 0.0;

    // Konwertuj do μV w zależności od wykrytej jednostki
    switch (m_detectedUnit)
    {
    case DataUnit::Millivolts:
        return range * 1000.0;  // mV -> μV
    case DataUnit::Volts:
        return range * 1000000.0;  // V -> μV
    case DataUnit::Microvolts:
        return range;  // już w μV
    default:
        return range;  // raw lub unknown - zwróć jak jest
    }
}

double AutoScaleManager::suggestedScaleBarValue() const
{
    // Sugerowana wartość scale bar powinna być "ładną" liczbą
    // np. 10, 20, 50, 100, 200, 500 μV

    double rangeUV = dataRangeInMicrovolts();
    if (rangeUV <= 0) return 50.0;  // domyślnie 50 μV

    // Chcemy żeby scale bar zajmował około 10-20% zakresu
    double targetValue = rangeUV * 0.15;

    // Znajdź najbliższą "ładną" wartość
    static const double niceValues[] = {1, 2, 5, 10, 20, 50, 100, 200, 500, 1000, 2000, 5000};
    static const int numNice = sizeof(niceValues) / sizeof(niceValues[0]);

    double bestValue = niceValues[0];
    double bestDiff = std::abs(targetValue - bestValue);

    for (int i = 1; i < numNice; ++i)
    {
        double diff = std::abs(targetValue - niceValues[i]);
        if (diff < bestDiff)
        {
            bestDiff = diff;
            bestValue = niceValues[i];
        }
    }

    return bestValue;
}

double AutoScaleManager::calculateScaleBarHeight(double valueInOriginalUnits, double gain) const
{
    // valueInOriginalUnits jest w μV
    // Musimy obliczyć jak wysoki będzie słupek na wykresie

    if (m_scaleFactor <= 0 || !std::isfinite(m_scaleFactor))
    {
        return 0.0;
    }

    // Konwertuj wartość z μV do jednostek danych (mV, V, etc.)
    double valueInDataUnits = valueInOriginalUnits;

    switch (m_detectedUnit)
    {
    case DataUnit::Millivolts:
        valueInDataUnits = valueInOriginalUnits / 1000.0;  // μV -> mV
        break;
    case DataUnit::Volts:
        valueInDataUnits = valueInOriginalUnits / 1000000.0;  // μV -> V
        break;
    case DataUnit::Microvolts:
        valueInDataUnits = valueInOriginalUnits;  // już w μV
        break;
    default:
        // Dla raw/unknown - zakładamy że to μV
        break;
    }

    // Wysokość = wartość * bazowy współczynnik * gain
    double height = valueInDataUnits * m_scaleFactor * gain;

    return height;
}

QVector<QVector<double>> AutoScaleManager::scaleChunk(const std::vector<std::vector<float>>& chunk,
                                                      const QVector<int>& selectedChannelIndices,
                                                      double channelSpacing,
                                                      double gain) const
{
    const int numSamples = static_cast<int>(chunk.size());
    const int numChannels = selectedChannelIndices.size();
    const int totalChunkChannels = static_cast<int>(chunk[0].size());

    QVector<QVector<double>> result(numChannels);

    // Pre-alokacja
    for (int ch = 0; ch < numChannels; ++ch)
    {
        result[ch].reserve(numSamples);
    }

    // Centrum danych (do wycentrowania przed skalowaniem)
    double dataCenter = (m_calibratedMax + m_calibratedMin) / 2.0;

    // Jeśli nie ma kalibracji, użyj aktualnych globalnych wartości
    if (m_calibrationState == CalibrationState::NotStarted)
    {
        if (std::isfinite(m_globalMin) && std::isfinite(m_globalMax))
        {
            dataCenter = (m_globalMax + m_globalMin) / 2.0;
        }
        else
        {
            dataCenter = 0.0;
        }
    }

    // Całkowity współczynnik skalowania = bazowy * gain (użytkownik)
    const double totalScale = m_scaleFactor * gain;

    // Przetwórz wszystkie kanały i próbki
    for (int ch = 0; ch < numChannels; ++ch)
    {
        int channelIndex = selectedChannelIndices[ch];

        if (channelIndex < 0 || channelIndex >= totalChunkChannels)
        {
            // Kanał poza zakresem - wypełnij zerami
            for (int s = 0; s < numSamples; ++s)
            {
                result[ch].append(0.0);
            }
            continue;
        }

        // Offset dla tego kanału (kanał 0 na górze, ostatni na dole)
        double offset = (numChannels - 1 - ch) * channelSpacing;

        for (int s = 0; s < numSamples; ++s)
        {
            double rawValue = static_cast<double>(chunk[s][channelIndex]);

            // 1. Wycentruj dane (odejmij środek zakresu)
            double centered = rawValue - dataCenter;

            // 2. Przeskaluj: bazowy współczynnik * gain (użytkownik)
            //    Ten sam współczynnik dla WSZYSTKICH kanałów!
            double scaled = centered * totalScale;

            // 3. Dodaj offset dla separacji kanałów
            double final = scaled + offset;

            result[ch].append(final);
        }
    }

    return result;
}
