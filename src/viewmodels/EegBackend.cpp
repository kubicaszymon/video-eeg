#include "EegBackend.h"

#include <qtimer.h>
#include <algorithm>

EegBackend::EegBackend(QObject *parent)
    : QObject{parent}, amplifier_manager_{AmplifierManager::instance()}
{
    qInfo() << "EEGBACKEND CREATED: " << this;
    connect(amplifier_manager_, &AmplifierManager::DataReceived, this, &EegBackend::DataReceived, Qt::QueuedConnection);
    connect(amplifier_manager_, &AmplifierManager::SamplingRateDetected, this, &EegBackend::onSamplingRateDetected, Qt::QueuedConnection);
    connect(amplifier_manager_, &AmplifierManager::StreamConnected, this, &EegBackend::onStreamConnected, Qt::QueuedConnection);
    connect(amplifier_manager_, &AmplifierManager::StreamDisconnected, this, &EegBackend::onStreamDisconnected, Qt::QueuedConnection);

    // Initialize marker manager
    m_markerManager = new MarkerManager(this);
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
    // Set connecting state
    m_isConnecting = true;
    m_isConnected = false;
    emit isConnectingChanged();
    emit isConnectedChanged();

    amplifier_manager_->StartStream(m_amplifierId);
}

void EegBackend::onStreamConnected()
{
    qInfo() << "[EegBackend] Stream connected!";
    m_isConnecting = false;
    m_isConnected = true;
    emit isConnectingChanged();
    emit isConnectedChanged();
}

void EegBackend::onStreamDisconnected()
{
    qInfo() << "[EegBackend] Stream disconnected";
    m_isConnecting = false;
    m_isConnected = false;
    emit isConnectingChanged();
    emit isConnectedChanged();
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

void EegBackend::addMarker(const QString& type)
{
    if (!m_markerManager || !m_dataModel || m_samplingRate <= 0) {
        qWarning() << "[EegBackend] Cannot add marker - not ready";
        return;
    }

    // Pozycja X = aktualna pozycja zapisu w buforze
    int writePos = m_dataModel->writePosition();
    double xPosition = static_cast<double>(writePos) / m_samplingRate;

    qInfo() << "[EegBackend] Adding marker" << type << "at X:" << xPosition;
    m_markerManager->addMarkerAtPosition(type, xPosition, xPosition);
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

    // Update channel index cache if needed
    if (m_channelIndexCache.size() != numSelectedChannels)
    {
        m_channelIndexCache.resize(numSelectedChannels);
        for (int i = 0; i < numSelectedChannels; ++i)
        {
            m_channelIndexCache[i] = m_channels[i].toInt();
        }
    }

    // Prepare data with channel offsets (raw data, no scaling)
    QVector<QVector<double>> data(numSelectedChannels);

    for (int ch = 0; ch < numSelectedChannels; ++ch)
    {
        data[ch].reserve(numSamples);

        int channelIndex = m_channelIndexCache[ch];

        if (channelIndex < 0 || channelIndex >= totalChunkChannels)
        {
            // Channel out of range - fill with zeros
            for (int s = 0; s < numSamples; ++s)
            {
                data[ch].append(0.0);
            }
            continue;
        }

        // Offset for this channel (channel 0 at the top, last at the bottom)
        double offset = (numSelectedChannels - 1 - ch) * channelSpacing;

        for (int s = 0; s < numSamples; ++s)
        {
            double rawValue = static_cast<double>(chunk[s][channelIndex]);

            // Just add offset for channel separation, no scaling
            data[ch].append(rawValue + offset);
        }
    }

    // Remember position before write
    int prevWritePos = m_dataModel->writePosition();

    // Send to data model
    m_dataModel->updateAllData(data);

    // Get new position after write
    int newWritePos = m_dataModel->writePosition();

    // Remove markers that were overwritten by new data
    if (m_markerManager && m_samplingRate > 0) {
        // Calculate X range that was overwritten
        double startX = static_cast<double>((prevWritePos + 1) % m_dataModel->maxSamples()) / m_samplingRate;
        double endX = static_cast<double>(newWritePos) / m_samplingRate;

        m_markerManager->removeMarkersInRange(startX, endX, m_timeWindowSeconds);
    }
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

QStringList EegBackend::channelNames() const
{
    QStringList names;

    // Get amplifier by ID to access channel names
    Amplifier* amp = amplifier_manager_->GetAmplifierById(m_amplifierId);
    if (amp == nullptr)
    {
        // Return generic names if amplifier not found
        for (int i = 0; i < m_channels.size(); ++i)
        {
            names.append(QString("Ch %1").arg(m_channels[i].toInt()));
        }
        return names;
    }

    // Map selected channel indices to their actual names
    for (const auto& channelVar : m_channels)
    {
        int channelIndex = channelVar.toInt();
        if (channelIndex >= 0 && channelIndex < amp->available_channels.size())
        {
            names.append(amp->available_channels[channelIndex]);
        }
        else
        {
            names.append(QString("Ch %1").arg(channelIndex));
        }
    }

    return names;
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

void EegBackend::onSamplingRateDetected(double samplingRate)
{
    qDebug() << "EegBackend: Sampling rate detected:" << samplingRate << "Hz";
    if (!qFuzzyCompare(m_samplingRate, samplingRate))
    {
        m_samplingRate = samplingRate;
        emit samplingRateChanged();

        // Update data model with new sampling rate
        if (m_dataModel)
        {
            m_dataModel->setSamplingRate(samplingRate);
            m_dataModel->setTimeWindowSeconds(m_timeWindowSeconds);
        }
    }
}

double EegBackend::samplingRate() const
{
    return m_samplingRate;
}

double EegBackend::timeWindowSeconds() const
{
    return m_timeWindowSeconds;
}

void EegBackend::setTimeWindowSeconds(double newTimeWindowSeconds)
{
    if (qFuzzyCompare(m_timeWindowSeconds, newTimeWindowSeconds))
        return;
    m_timeWindowSeconds = newTimeWindowSeconds;
    emit timeWindowSecondsChanged();

    // Update data model
    if (m_dataModel)
    {
        m_dataModel->setTimeWindowSeconds(m_timeWindowSeconds);
    }

    // Clear markers when time window changes (buffer is reset)
    if (m_markerManager)
    {
        m_markerManager->clearMarkers();
    }
}
