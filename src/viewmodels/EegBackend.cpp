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

void EegBackend::initializeBuffers(int numChannels, int numSamples)
{
    // Only reallocate if size changed significantly
    if (numChannels != m_lastNumChannels || numSamples > m_lastBufferSize)
    {
        m_scaledDataBuffer.resize(numChannels);

        // Pre-allocate with some extra capacity to avoid frequent reallocations
        int reserveSize = numSamples * 2;
        for (int ch = 0; ch < numChannels; ++ch)
        {
            m_scaledDataBuffer[ch].clear();
            m_scaledDataBuffer[ch].reserve(reserveSize);
        }

        m_lastNumChannels = numChannels;
        m_lastBufferSize = reserveSize;
    }
    else
    {
        // Just clear existing buffers for reuse
        for (int ch = 0; ch < numChannels; ++ch)
        {
            m_scaledDataBuffer[ch].clear();
        }
    }

    // Cache channel indices to avoid repeated QVariant::toInt() calls
    if (m_channelIndexCache.size() != numChannels)
    {
        m_channelIndexCache.resize(numChannels);
        for (int i = 0; i < numChannels; ++i)
        {
            m_channelIndexCache[i] = m_channels[i].toInt();
        }
    }
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
    if(chunk.empty() || chunk[0].empty() || m_channels.isEmpty() || !m_dataModel)
    {
        return;
    }

    const int numSamples = static_cast<int>(chunk.size());
    const int numSelectedChannels = m_channels.size();
    const double channelSpacing = m_spacing;
    const int totalChunkChannels = static_cast<int>(chunk[0].size());

    // Initialize/reuse pre-allocated buffers
    initializeBuffers(numSelectedChannels, numSamples);

    // Pre-calculate offsets for each channel (from top to bottom)
    // Channel 0 = highest, channel N-1 = lowest
    // Combined transposition, conversion and scaling in single pass
    for (int ch = 0; ch < numSelectedChannels; ++ch)
    {
        const int channelIndex = m_channelIndexCache[ch];

        // Skip invalid channel indices
        if (channelIndex < 0 || channelIndex >= totalChunkChannels)
        {
            continue;
        }

        // Pre-calculate offset for this channel
        const double offset = (numSelectedChannels - 1 - ch) * channelSpacing;

        // Direct access to output buffer
        QVector<double>& outputChannel = m_scaledDataBuffer[ch];

        // Process all samples for this channel
        for (int sample = 0; sample < numSamples; ++sample)
        {
            // Combined: read float -> convert to double -> add offset
            outputChannel.append(static_cast<double>(chunk[sample][channelIndex]) + offset);
        }
    }

    // Send to data model
    m_dataModel->updateAllData(m_scaledDataBuffer);
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

    // Update channel index cache when channels change
    const int numChannels = m_channels.size();
    m_channelIndexCache.resize(numChannels);
    for (int i = 0; i < numChannels; ++i)
    {
        m_channelIndexCache[i] = m_channels[i].toInt();
    }

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
