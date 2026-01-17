#include "EegBackend.h"
#include <QDebug>
#include <QtMath>

EegBackend::EegBackend(QObject *parent)
    : QObject{parent}
    , amplifier_manager_{AmplifierManager::instance()}
    , m_markerManager{new MarkerManager(this)}
    , m_scaler{new EegDisplayScaler(this)}
{
    qInfo() << "[EegBackend] Created:" << this;

    // Connect amplifier manager signals
    connect(amplifier_manager_, &AmplifierManager::DataReceived,
            this, &EegBackend::DataReceived, Qt::QueuedConnection);
    connect(amplifier_manager_, &AmplifierManager::SamplingRateDetected,
            this, &EegBackend::onSamplingRateDetected, Qt::QueuedConnection);
    connect(amplifier_manager_, &AmplifierManager::StreamConnected,
            this, &EegBackend::onStreamConnected, Qt::QueuedConnection);
    connect(amplifier_manager_, &AmplifierManager::StreamDisconnected,
            this, &EegBackend::onStreamDisconnected, Qt::QueuedConnection);
}

EegBackend::~EegBackend()
{
    qDebug() << "[EegBackend] Destructor called";
}

// ============================================================================
// Initialization
// ============================================================================

void EegBackend::registerDataModel(EegDataModel *dataModel)
{
    if (dataModel)
    {
        m_dataModel = dataModel;
        qInfo() << "[EegBackend] Data model registered:" << m_dataModel;
    }
}

void EegBackend::startStream()
{
    m_isConnecting = true;
    m_isConnected = false;
    emit isConnectingChanged();
    emit isConnectedChanged();

    amplifier_manager_->StartStream(m_amplifierId);
}

// ============================================================================
// Connection state handlers
// ============================================================================

void EegBackend::onStreamConnected()
{
    qInfo() << "[EegBackend] Stream connected";
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

void EegBackend::onSamplingRateDetected(double samplingRate)
{
    qDebug() << "[EegBackend] Sampling rate detected:" << samplingRate << "Hz";

    if (!qFuzzyCompare(m_samplingRate, samplingRate))
    {
        m_samplingRate = samplingRate;
        emit samplingRateChanged();

        if (m_dataModel)
        {
            m_dataModel->setSamplingRate(samplingRate);
            m_dataModel->setTimeWindowSeconds(m_timeWindowSeconds);
        }
    }
}

// ============================================================================
// Data processing
// ============================================================================

void EegBackend::DataReceived(const std::vector<std::vector<float>>& chunk)
{
    if (chunk.empty() || chunk[0].empty() || m_channels.isEmpty() || !m_dataModel)
    {
        return;
    }

    // Update channel index cache if needed
    updateChannelIndexCache();

    // Delegate transformation to scaler
    QVector<QVector<double>> scaledData = m_scaler->transformChunk(
        chunk, m_channelIndexCache, m_spacing);

    // Store current write position for marker cleanup
    int prevWritePos = m_dataModel->writePosition();

    // Send transformed data to model
    m_dataModel->updateAllData(scaledData);

    // Clean up overwritten markers
    updateMarkersAfterWrite(prevWritePos, m_dataModel->writePosition());
}

void EegBackend::updateChannelIndexCache()
{
    const int numChannels = m_channels.size();

    if (m_channelIndexCache.size() != numChannels)
    {
        m_channelIndexCache.resize(numChannels);
        for (int i = 0; i < numChannels; ++i)
        {
            m_channelIndexCache[i] = m_channels[i].toInt();
        }
    }
}

void EegBackend::updateMarkersAfterWrite(int prevWritePos, int newWritePos)
{
    if (!m_markerManager || m_samplingRate <= 0 || !m_dataModel)
        return;

    // Calculate X range that was overwritten
    double startX = static_cast<double>((prevWritePos + 1) % m_dataModel->maxSamples()) / m_samplingRate;
    double endX = static_cast<double>(newWritePos) / m_samplingRate;

    m_markerManager->removeMarkersInRange(startX, endX, m_timeWindowSeconds);
}

// ============================================================================
// Markers
// ============================================================================

void EegBackend::addMarker(const QString& type)
{
    if (!m_markerManager || !m_dataModel || m_samplingRate <= 0)
    {
        qWarning() << "[EegBackend] Cannot add marker - not ready";
        return;
    }

    int writePos = m_dataModel->writePosition();
    double xPosition = static_cast<double>(writePos) / m_samplingRate;

    qInfo() << "[EegBackend] Adding marker" << type << "at X:" << xPosition;
    m_markerManager->addMarkerAtPosition(type, xPosition, xPosition);
}

// ============================================================================
// Test data generation
// ============================================================================

void EegBackend::generateTestData()
{
    if (!m_dataModel)
        return;

    const int numChannels = m_channels.size();
    const int numSamples = 500;

    QVector<QVector<double>> testData(numChannels);

    for (int ch = 0; ch < numChannels; ++ch)
    {
        testData[ch].reserve(numSamples);
        double offset = EegDisplayScaler::calculateChannelOffset(ch, numChannels, m_spacing);

        for (int i = 0; i < numSamples; ++i)
        {
            double time = i * 0.1;
            double value = qSin(time * (1.0 + ch * 0.3)) * (m_spacing * 0.3);
            testData[ch].append(offset - value);  // Invert for proper Y-axis
        }
    }

    m_dataModel->updateAllData(testData);
}

// ============================================================================
// Channel configuration
// ============================================================================

QVariantList EegBackend::channels() const
{
    return m_channels;
}

void EegBackend::setChannels(const QVariantList &newChannels)
{
    if (m_channels == newChannels)
        return;

    m_channels = newChannels;
    m_channelIndexCache.clear();  // Force cache rebuild
    emit channelsChanged();
}

QStringList EegBackend::channelNames() const
{
    QStringList names;

    Amplifier* amp = amplifier_manager_->GetAmplifierById(m_amplifierId);

    for (const auto& channelVar : m_channels)
    {
        int channelIndex = channelVar.toInt();

        if (amp && channelIndex >= 0 && channelIndex < amp->available_channels.size())
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

// ============================================================================
// Amplifier configuration
// ============================================================================

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

// ============================================================================
// Display configuration
// ============================================================================

double EegBackend::spacing() const
{
    return m_spacing;
}

void EegBackend::setSpacing(double newSpacing)
{
    if (qFuzzyCompare(m_spacing, newSpacing))
        return;

    m_spacing = newSpacing;
    emit spacingChanged();
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

    if (m_dataModel)
    {
        m_dataModel->setTimeWindowSeconds(m_timeWindowSeconds);
    }

    if (m_markerManager)
    {
        m_markerManager->clearMarkers();
    }
}

double EegBackend::samplingRate() const
{
    return m_samplingRate;
}
