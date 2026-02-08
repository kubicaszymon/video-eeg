#include "eegdisplayscaler.h"
#include <QtMath>
#include <QDebug>

// Standard EEG sensitivity steps in μV/mm
const QList<double> EegDisplayScaler::SENSITIVITY_OPTIONS = {
    1.0, 2.0, 3.0, 5.0, 7.0, 10.0, 15.0, 20.0, 30.0, 50.0, 100.0
};

EegDisplayScaler::EegDisplayScaler(QObject *parent)
    : QObject(parent)
{
}

double EegDisplayScaler::displayGain() const
{
    // G[px/μV] = DPI / (25.4 × Sensitivity[μV/mm])
    // This converts μV to pixels based on physical display parameters
    //
    // Example at 96 DPI, 7 μV/mm:
    //   G = 96 / (25.4 × 7) = 0.54 px/μV
    //   70 μV signal = 37.8 px = 1 cm (correct!)
    return m_screenDpi / (MM_PER_INCH * m_sensitivity);
}

void EegDisplayScaler::setSensitivity(double sensitivity)
{
    // Validate against available options
    if (!SENSITIVITY_OPTIONS.contains(sensitivity))
    {
        qWarning() << "[EegDisplayScaler] Invalid sensitivity:" << sensitivity
                   << "μV/mm. Valid options:" << SENSITIVITY_OPTIONS;
        return;
    }

    if (qFuzzyCompare(m_sensitivity, sensitivity))
        return;

    m_sensitivity = sensitivity;
    emit sensitivityChanged();
    emit displayGainChanged();

    qDebug() << "[EegDisplayScaler] Sensitivity:" << m_sensitivity
             << "μV/mm, displayGain:" << displayGain() << "px/μV";
}

void EegDisplayScaler::setScreenDpi(double dpi)
{
    if (dpi <= 0)
    {
        qWarning() << "[EegDisplayScaler] Invalid DPI:" << dpi;
        return;
    }

    if (qFuzzyCompare(m_screenDpi, dpi))
        return;

    m_screenDpi = dpi;
    emit screenDpiChanged();
    emit displayGainChanged();

    qDebug() << "[EegDisplayScaler] Screen DPI:" << m_screenDpi
             << ", displayGain:" << displayGain() << "px/μV";
}

double EegDisplayScaler::transformSample(double rawValueMicrovolts, double baselineOffset) const
{
    // IMPORTANT: Subtract from baseline so positive μV goes UP on screen
    // Qt/QML Y-axis increases downward, but in EEG positive voltage should go up
    return baselineOffset - (rawValueMicrovolts * displayGain());
}

double EegDisplayScaler::calculateChannelOffset(int channelIndex, int totalChannels, double channelSpacing)
{
    // Channel 0 at the top, last channel at the bottom
    // Offset increases as we go down the screen
    return (totalChannels - 1 - channelIndex) * channelSpacing;
}

QVector<QVector<double>> EegDisplayScaler::transformChunk(
    const std::vector<std::vector<float>>& chunk,
    const QVector<int>& channelIndices,
    double channelSpacing) const
{
    if (chunk.empty() || chunk[0].empty())
    {
        return {};
    }

    const int numSamples = static_cast<int>(chunk.size());
    const int numChannels = channelIndices.size();
    const int totalChunkChannels = static_cast<int>(chunk[0].size());
    const double gain = displayGain();

    QVector<QVector<double>> result(numChannels);

    for (int ch = 0; ch < numChannels; ++ch)
    {
        result[ch].reserve(numSamples);

        const int sourceChannel = channelIndices[ch];

        // Validate channel index
        if (sourceChannel < 0 || sourceChannel >= totalChunkChannels)
        {
            // Invalid channel - fill with baseline
            double offset = calculateChannelOffset(ch, numChannels, channelSpacing);
            for (int s = 0; s < numSamples; ++s)
            {
                result[ch].append(offset);
            }
            continue;
        }

        // Calculate baseline offset for this channel
        const double offset = calculateChannelOffset(ch, numChannels, channelSpacing);

        // Transform each sample
        for (int s = 0; s < numSamples; ++s)
        {
            const double rawValue = static_cast<double>(chunk[s][sourceChannel]);

            // Scale and invert: positive μV goes UP
            const double scaledValue = offset - (rawValue * gain);

            result[ch].append(scaledValue);
        }
    }

    return result;
}
