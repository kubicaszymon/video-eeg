#include "EegBackend.h"

#include <qtimer.h>

EegBackend::EegBackend(QObject *parent)
    : QObject{parent}, amplifier_manager_{AmplifierManager::instance()}
{
    qInfo() << "EEGBACKEND CREATED: " << this;
    connect(amplifier_manager_, &AmplifierManager::DataReceived, this, &EegBackend::DataReceived, Qt::QueuedConnection);
}

EegBackend::~EegBackend()
{
    qDebug() << "[EegViewModel] DESTRUCTOR CALLED";
}

void EegBackend::registerDataModel(EegDataModel *dataModel)
{
    if(dataModel)
    {
        m_dataModel = dataModel;
        qInfo() << "eeg data model linked successfuly: " << m_dataModel;
    }
}

void EegBackend::startStream()
{
    amplifier_manager_->StartStream(m_amplifierId);
}

void EegBackend::generateTestData()
{
    if (!m_dataModel) return;

    // Use dynamic spacing from QML
    double channel_spacing = m_spacing;

    const int numChannels = m_channels.size();
    const int numSamples = 500; // 500 points per 5-second window

    QVector<QVector<double>> testData(numChannels);

    for(int ch = 0; ch < numChannels; ++ch)
    {
        testData[ch].reserve(numSamples);
    }

    for (int i = 0; i < numSamples; ++i) {
        double time = i * 0.1;

        for (int ch = 0; ch < numChannels; ++ch) {
            // Different frequencies for each channel
            double value = qSin(time * (1.0 + ch * 0.3)) * (channel_spacing * 0.3);

            // Offset - channel 0 at the top, last at the bottom
            double offset = (numChannels - 1 - ch) * channel_spacing;

            testData[ch].append(value + offset);
        }
    }

    m_dataModel->updateAllData(testData);
}

void EegBackend::DataReceived(const std::vector<std::vector<float>>& chunk)
{
    if(chunk.empty() || chunk[0].empty() || m_channels.isEmpty())
    {
        return;
    }

    int numSamples = chunk.size();
    int numSelectedChannels = m_channels.size();

    // Use dynamic spacing from QML (set by EegGraph)
    const double channelSpacing = m_spacing;

    // STEP 1: Transposition and conversion
    QVector<QVector<double>> transposedData(numSelectedChannels);
    for(int i = 0; i < numSelectedChannels; ++i)
    {
        transposedData[i].reserve(numSamples);
    }

    for(int sample = 0; sample < numSamples; ++sample)
    {
        for(int i = 0; i < numSelectedChannels; ++i)
        {
            int channelIndex = m_channels[i].toInt();

            if(channelIndex >= 0 && channelIndex < static_cast<int>(chunk[sample].size()))
            {
                transposedData[i].append(static_cast<double>(chunk[sample][channelIndex]));
            }
        }
    }

    // STEP 2: SCALING - use dynamic channelSpacing
    QVector<QVector<double>> scaledData(numSelectedChannels);

    for(int ch = 0; ch < numSelectedChannels; ++ch)
    {
        if(transposedData[ch].isEmpty()) continue;

        scaledData[ch].reserve(transposedData[ch].size());

        // Offset for this channel (from top to bottom)
        // Channel 0 = highest, channel N-1 = lowest
        double offset = (numSelectedChannels - 1 - ch) * channelSpacing;

        // Option A: Without normalization, offset only
        // EEG signal is usually in microvolts, so it may require scaling
        for(double val : transposedData[ch])
        {
            scaledData[ch].append(val + offset);
        }

        /* Option B: With normalization (uncomment if signals are too large/small)
        double minVal = *std::min_element(transposedData[ch].begin(), transposedData[ch].end());
        double maxVal = *std::max_element(transposedData[ch].begin(), transposedData[ch].end());
        double range = maxVal - minVal;

        // Amplitude is ~40% spacing so signals don't overlap
        const double TARGET_AMPLITUDE = channelSpacing * 0.4;

        if(range > 0.001)
        {
            for(double val : transposedData[ch])
            {
                double normalized = ((val - minVal) / range - 0.5) * TARGET_AMPLITUDE;
                scaledData[ch].append(normalized + offset);
            }
        }
        else
        {
            for(double val : transposedData[ch])
            {
                scaledData[ch].append(offset);
            }
        }
        */
    }

    m_dataModel->updateAllData(scaledData);
}

QVariantList EegBackend::GetChannelNames() const
{
    if(amplifier_ == nullptr)
    {
        return {};
    }

    QVariantList channels;
    for(const auto& channel : std::as_const(amplifier_->available_channels))
    {
        channels.append(channel);
    }

    return channels;
}

QVariantList EegBackend::channels() const
{
    return m_channels;
}

void EegBackend::setChannels(const QVariantList &newChannels)
{
    if (m_channels == newChannels)
        return;
    m_channels = newChannels;
    emit channelsChanged();
}

int EegBackend::amplifierIdx() const
{
    return m_amplifierIdx;
}

void EegBackend::setAmplifierIdx(int newAmplifierIdx)
{
    if (m_amplifierIdx == newAmplifierIdx)
        return;
    m_amplifierIdx = newAmplifierIdx;
    emit amplifierIdxChanged();
}

double EegBackend::spacing() const
{
    return m_spacing;
}

void EegBackend::setSpacing(double newSpacing)
{
    if(qFuzzyCompare(m_spacing, newSpacing))
        return;
    m_spacing = newSpacing;
    emit spacingChanged();
}

QString EegBackend::amplifierId() const
{
    return m_amplifierId;
}

void EegBackend::setAmplifierId(const QString &newAmplifierId)
{
    if (m_amplifierId == newAmplifierId)
        return;
    m_amplifierId = newAmplifierId;
    emit amplifierIdChanged();
}
