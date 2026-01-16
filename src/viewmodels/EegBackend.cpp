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

    // Initialize auto-scale manager
    m_autoScaleManager = new AutoScaleManager(this);

    // Initialize marker manager
    m_markerManager = new MarkerManager(this);

    // Connect auto-scale signals
    connect(m_autoScaleManager, &AutoScaleManager::scaleFactorChanged, this, [this]() {
        emit scaleFactorChanged();
    });
    connect(m_autoScaleManager, &AutoScaleManager::detectedUnitChanged, this, [this]() {
        emit scaleUnitChanged();
    });
    connect(m_autoScaleManager, &AutoScaleManager::calibrationStateChanged, this, [this]() {
        emit scaleCalibrationChanged();
    });
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

    if (!m_autoScaleManager)
        return;

    // Process chunk through AutoScaleManager
    // This updates statistics and calculates optimal scaling
    m_autoScaleManager->processChunk(chunk, m_channelIndexCache);

    // Get scaled data with proper offsets
    // AutoScaleManager applies: baseScale * gain (user controlled)
    // The SAME factor for ALL channels!
    QVector<QVector<double>> scaledData = m_autoScaleManager->scaleChunk(
        chunk, m_channelIndexCache, channelSpacing, m_gain);

    // Zapamiętaj poprzednią pozycję przed zapisem
    int prevWritePos = m_dataModel->writePosition();

    // Send to data model
    m_dataModel->updateAllData(scaledData);

    // Pobierz nową pozycję po zapisie
    int newWritePos = m_dataModel->writePosition();

    // Usuń znaczniki które zostały nadpisane przez nowe dane
    if (m_markerManager && m_samplingRate > 0) {
        // Oblicz zakres X który został nadpisany
        // prevWritePos+1 to pierwszy nadpisany sample, newWritePos to ostatni
        double startX = static_cast<double>((prevWritePos + 1) % m_dataModel->maxSamples()) / m_samplingRate;
        double endX = static_cast<double>(newWritePos) / m_samplingRate;

        m_markerManager->removeMarkersInRange(startX, endX, m_timeWindowSeconds);
    }

    // Emit data range changed for UI updates
    emit dataRangeChanged();
    emit scaleBarChanged();
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

// Auto-scale getters and setters

double EegBackend::scaleFactor() const
{
    return m_autoScaleManager ? m_autoScaleManager->scaleFactor() : 1.0;
}

QString EegBackend::scaleUnit() const
{
    return m_autoScaleManager ? m_autoScaleManager->unitString() : "?";
}

bool EegBackend::scaleCalibrated() const
{
    return m_autoScaleManager ? m_autoScaleManager->isCalibrated() : false;
}

double EegBackend::dataRangeMin() const
{
    return m_autoScaleManager ? m_autoScaleManager->globalMin() : 0.0;
}

double EegBackend::dataRangeMax() const
{
    return m_autoScaleManager ? m_autoScaleManager->globalMax() : 0.0;
}

// Gain control

double EegBackend::gain() const
{
    return m_gain;
}

void EegBackend::setGain(double newGain)
{
    // Clamp to reasonable range
    newGain = std::clamp(newGain, 0.01, 100.0);

    if (qFuzzyCompare(m_gain, newGain))
        return;

    m_gain = newGain;
    emit gainChanged();
    emit scaleBarChanged();

    qDebug() << "[EegBackend] Gain changed to:" << m_gain;
}

// Scale bar

double EegBackend::scaleBarValue() const
{
    if (!m_autoScaleManager)
        return 50.0;  // Default 50 μV

    return m_autoScaleManager->suggestedScaleBarValue();
}

double EegBackend::scaleBarHeight() const
{
    if (!m_autoScaleManager)
        return 0.0;

    double value = m_autoScaleManager->suggestedScaleBarValue();
    return m_autoScaleManager->calculateScaleBarHeight(value, m_gain);
}

double EegBackend::dataRangeInMicrovolts() const
{
    if (!m_autoScaleManager)
        return 0.0;

    return m_autoScaleManager->dataRangeInMicrovolts();
}
