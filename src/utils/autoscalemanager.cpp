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
    m_channelStats.clear();

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
        // USE ROBUST SCALING: Use median of per-channel ranges instead of global min/max
        // This prevents one bad channel from ruining the scale for all channels
        double robustRange = calculateRobustRange();

        if (robustRange > 0 && std::isfinite(robustRange))
        {
            // Use robust range for scaling
            m_calibratedRange = robustRange;
            // Set min/max symmetrically around 0 for display purposes
            m_calibratedMin = -robustRange / 2.0;
            m_calibratedMax = robustRange / 2.0;
        }
        else
        {
            // Fallback to global min/max if robust calculation fails
            m_calibratedMin = m_globalMin;
            m_calibratedMax = m_globalMax;
            m_calibratedRange = m_calibratedMax - m_calibratedMin;
        }

        // Wykryj jednostkę based on robust range
        m_detectedUnit = detectUnit(-m_calibratedRange/2, m_calibratedRange/2);

        // Oblicz bazowy współczynnik skalowania
        m_scaleFactor = calculateOptimalScaleFactor(m_calibratedRange, m_targetSpacing);

        m_calibrationState = CalibrationState::Calibrated;

        qDebug() << "[AutoScale] Scale set from first data (robust):"
                 << "robustRange:" << m_calibratedRange
                 << "globalRange:" << (m_globalMax - m_globalMin)
                 << "(" << dataRangeInMicrovolts() << "μV)"
                 << "unit:" << unitString()
                 << "scaleFactor:" << m_scaleFactor
                 << "channels:" << m_channelStats.size();

        // Log per-channel ranges for debugging
        if (m_channelStats.size() <= 32)
        {
            for (auto it = m_channelStats.constBegin(); it != m_channelStats.constEnd(); ++it)
            {
                qDebug() << "  [Ch" << it.key() << "] range:" << it.value().range()
                         << "min:" << it.value().min << "max:" << it.value().max;
            }
        }
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

    // Per-channel min/max for this chunk (for robust scaling)
    QMap<int, ChannelStats> chunkChannelStats;

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

            // Update per-channel statistics
            ChannelStats& stats = chunkChannelStats[channelIndex];
            stats.min = std::min(stats.min, value);
            stats.max = std::max(stats.max, value);

            // Aktualizuj min/max chunk (global)
            chunkMin = std::min(chunkMin, value);
            chunkMax = std::max(chunkMax, value);

            // Aktualizuj sumy do obliczenia średniej/wariancji
            m_globalSum += value;
            m_globalSumSquares += value * value;
        }

        m_totalSamplesProcessed++;
    }

    // Update global per-channel statistics
    for (auto it = chunkChannelStats.constBegin(); it != chunkChannelStats.constEnd(); ++it)
    {
        int channelIndex = it.key();
        const ChannelStats& chunkStats = it.value();

        ChannelStats& globalStats = m_channelStats[channelIndex];
        globalStats.min = std::min(globalStats.min, chunkStats.min);
        globalStats.max = std::max(globalStats.max, chunkStats.max);
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

double AutoScaleManager::calculateRobustRange() const
{
    if (m_channelStats.isEmpty())
    {
        // Fallback to global range
        return m_globalMax - m_globalMin;
    }

    // Collect all channel ranges
    QVector<double> ranges;
    ranges.reserve(m_channelStats.size());

    for (auto it = m_channelStats.constBegin(); it != m_channelStats.constEnd(); ++it)
    {
        double range = it.value().range();
        if (std::isfinite(range) && range > 0)
        {
            ranges.append(range);
        }
    }

    if (ranges.isEmpty())
    {
        return m_globalMax - m_globalMin;
    }

    // Sort ranges to find percentiles
    std::sort(ranges.begin(), ranges.end());

    // Use 75th percentile (Q3) instead of max to be robust against outliers
    // This way even if a few channels have crazy values, scaling will be based on "normal" channels
    int q3Index = static_cast<int>(ranges.size() * 0.75);
    q3Index = std::min(q3Index, ranges.size() - 1);

    double robustRange = ranges[q3Index];

    // Additional safety: if median is much smaller than Q3, use median instead
    // This handles the case where most channels are normal but a few are extreme
    int medianIndex = ranges.size() / 2;
    double medianRange = ranges[medianIndex];

    // If Q3 is more than 10x median, something is probably wrong with outlier channels
    // In that case, prefer median * 2 (to give some headroom)
    if (robustRange > medianRange * 10.0 && ranges.size() > 4)
    {
        qDebug() << "[AutoScale] Outlier detected: Q3=" << robustRange
                 << "median=" << medianRange << "- using median*2";
        robustRange = medianRange * 2.0;
    }

    return robustRange;
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

    // Całkowity współczynnik skalowania = bazowy * gain (użytkownik)
    // TEN SAM dla wszystkich kanałów!
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

        // PER-CHANNEL CENTERING: Each channel is centered around its own midpoint
        // This handles channels with different DC offsets correctly
        double channelCenter = 0.0;
        if (m_channelStats.contains(channelIndex))
        {
            const ChannelStats& stats = m_channelStats[channelIndex];
            if (std::isfinite(stats.min) && std::isfinite(stats.max))
            {
                channelCenter = (stats.max + stats.min) / 2.0;
            }
        }

        // Offset dla tego kanału (kanał 0 na górze, ostatni na dole)
        double offset = (numChannels - 1 - ch) * channelSpacing;

        for (int s = 0; s < numSamples; ++s)
        {
            double rawValue = static_cast<double>(chunk[s][channelIndex]);

            // 1. Wycentruj dane względem WŁASNEGO centrum kanału
            //    (usuwa DC offset specyficzny dla tego kanału)
            double centered = rawValue - channelCenter;

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
