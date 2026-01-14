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

    // Zmiana stanu kalibracji
    if (m_calibrationState == CalibrationState::NotStarted)
    {
        m_calibrationState = CalibrationState::Collecting;
        qDebug() << "[AutoScale] Started collecting calibration data";
    }

    // Sprawdź postęp kalibracji
    if (m_calibrationState == CalibrationState::Collecting)
    {
        emit calibrationProgress(m_totalSamplesProcessed, m_calibrationSamples);

        if (m_totalSamplesProcessed >= m_calibrationSamples)
        {
            // Zakończ kalibrację
            m_calibratedMin = m_globalMin;
            m_calibratedMax = m_globalMax;
            m_calibratedRange = m_calibratedMax - m_calibratedMin;

            // Wykryj jednostkę
            m_detectedUnit = detectUnit(m_calibratedMin, m_calibratedMax);

            // Oblicz współczynnik skalowania
            m_scaleFactor = calculateOptimalScaleFactor(m_calibratedRange, m_targetSpacing);

            m_calibrationState = CalibrationState::Calibrated;

            qDebug() << "[AutoScale] Calibration complete:"
                     << "range:" << m_calibratedMin << "to" << m_calibratedMax
                     << "unit:" << unitString()
                     << "scaleFactor:" << m_scaleFactor;
        }
    }
    else if (m_calibrationState == CalibrationState::Calibrated)
    {
        // Sprawdź czy potrzebna rekalibracja
        if (needsRecalibration(m_globalMin, m_globalMax))
        {
            m_calibrationState = CalibrationState::Adapting;
            qDebug() << "[AutoScale] Data out of range, adapting...";
        }
    }
    else if (m_calibrationState == CalibrationState::Adapting)
    {
        // Dostosuj skalę płynnie
        double currentRange = m_globalMax - m_globalMin;
        double targetScale = calculateOptimalScaleFactor(currentRange, m_targetSpacing);
        m_scaleFactor = smoothScaleTransition(m_scaleFactor, targetScale);

        // Zaktualizuj skalibrowane wartości
        m_calibratedMin = m_globalMin;
        m_calibratedMax = m_globalMax;
        m_calibratedRange = currentRange;

        // Sprawdź czy możemy wrócić do stabilnego stanu
        // (gdy skala się ustabilizowała)
        if (std::abs(m_scaleFactor - targetScale) / targetScale < 0.05)
        {
            m_calibrationState = CalibrationState::Calibrated;
            qDebug() << "[AutoScale] Adaptation complete, new scale:" << m_scaleFactor;
        }
    }

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
    double range = maxVal - minVal;
    double absMax = std::max(std::abs(minVal), std::abs(maxVal));

    // Wykrywanie na podstawie typowych zakresów EEG:
    //
    // Mikrowolty (μV): sygnał EEG typowo ±50-200 μV
    // - Jeśli urządzenie podaje μV wprost: wartości ±50 do ±200
    //
    // Miliwolty (mV): niektóre urządzenia skalują do mV
    // - Wartości ±0.05 do ±0.2 (czyli μV / 1000)
    //
    // Wolty (V): rzadko używane
    // - Wartości bardzo małe: ±0.00005 do ±0.0002
    //
    // Raw ADC: surowe wartości przetwornika
    // - Mogą być 12-bit (0-4095), 16-bit (0-65535), lub signed

    if (absMax < 0.0001)
    {
        // Bardzo małe wartości - prawdopodobnie wolty
        return DataUnit::Volts;
    }
    else if (absMax < 0.01)
    {
        // Małe wartości - prawdopodobnie miliwolty
        return DataUnit::Millivolts;
    }
    else if (absMax < 1000)
    {
        // Umiarkowane wartości - prawdopodobnie mikrowolty
        return DataUnit::Microvolts;
    }
    else if (absMax < 100000)
    {
        // Duże wartości - prawdopodobnie raw ADC
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
    const double MAX_SCALE = 10000.0;

    scale = std::clamp(scale, MIN_SCALE, MAX_SCALE);

    return scale;
}

bool AutoScaleManager::needsRecalibration(double newMin, double newMax) const
{
    if (m_calibratedRange <= 0)
    {
        return true;
    }

    // Sprawdź czy dane wyszły poza zakres o więcej niż próg histerezy
    double exceedance = 0.0;

    if (newMin < m_calibratedMin)
    {
        exceedance = std::max(exceedance, (m_calibratedMin - newMin) / m_calibratedRange);
    }
    if (newMax > m_calibratedMax)
    {
        exceedance = std::max(exceedance, (newMax - m_calibratedMax) / m_calibratedRange);
    }

    // Sprawdź też czy zakres znacząco się zmniejszył
    double currentRange = newMax - newMin;
    double rangeChange = std::abs(currentRange - m_calibratedRange) / m_calibratedRange;

    return (exceedance > m_hysteresisThreshold) || (rangeChange > m_hysteresisThreshold * 2);
}

double AutoScaleManager::smoothScaleTransition(double currentScale, double targetScale) const
{
    // Wygładzanie eksponencjalne
    return currentScale + m_smoothingFactor * (targetScale - currentScale);
}

QVector<QVector<double>> AutoScaleManager::scaleChunk(const std::vector<std::vector<float>>& chunk,
                                                      const QVector<int>& selectedChannelIndices,
                                                      double channelSpacing) const
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
    if (m_calibrationState == CalibrationState::NotStarted ||
        m_calibrationState == CalibrationState::Collecting)
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

            // 2. Przeskaluj tym samym współczynnikiem dla wszystkich kanałów
            double scaled = centered * m_scaleFactor;

            // 3. Dodaj offset dla separacji kanałów
            double final = scaled + offset;

            result[ch].append(final);
        }
    }

    return result;
}
