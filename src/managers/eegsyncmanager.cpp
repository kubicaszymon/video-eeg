#include "eegsyncmanager.h"
#include <QDebug>
#include <QMutexLocker>
#include <QQmlEngine>
#include <algorithm>
#include <cmath>

EegSyncManager* EegSyncManager::s_instance = nullptr;

EegSyncManager::EegSyncManager(QObject* parent)
    : QObject(parent)
{
    s_instance = this;

    // Timer for periodic LSL time_correction() updates
    m_timeCorrectionTimer = new QTimer(this);
    m_timeCorrectionTimer->setInterval(10000); // every 10 seconds
    connect(m_timeCorrectionTimer, &QTimer::timeout, this, &EegSyncManager::updateTimeCorrection);

    // Timer for throttled stats updates to QML (avoid flooding)
    m_statsTimer = new QTimer(this);
    m_statsTimer->setInterval(250); // 4 Hz update rate for monitoring panel
    connect(m_statsTimer, &QTimer::timeout, this, &EegSyncManager::statsChanged);
    m_statsTimer->start();

    qInfo() << "[EegSyncManager] Created";
}

EegSyncManager::~EegSyncManager()
{
    if (s_instance == this)
        s_instance = nullptr;
    qInfo() << "[EegSyncManager] Destroyed";
}

EegSyncManager* EegSyncManager::instance()
{
    if (!s_instance)
        s_instance = new EegSyncManager();
    return s_instance;
}

EegSyncManager* EegSyncManager::create(QQmlEngine* qmlEngine, QJSEngine* jsEngine)
{
    Q_UNUSED(jsEngine)

    auto* inst = instance();
    QJSEngine::setObjectOwnership(inst, QJSEngine::CppOwnership);
    return inst;
}

// ============================================================================
// Data Input
// ============================================================================

void EegSyncManager::addEegSamples(const std::vector<std::vector<float>>& chunk,
                                    const std::vector<double>& timestamps,
                                    const QVector<int>& channelIndices)
{
    if (chunk.empty() || timestamps.empty())
        return;

    const int numSamples = static_cast<int>(chunk.size());
    const int numTimestamps = static_cast<int>(timestamps.size());

    // Ensure we have matching count (LSL should guarantee this)
    const int count = std::min(numSamples, numTimestamps);

    QMutexLocker locker(&m_mutex);

    for (int i = 0; i < count; ++i)
    {
        const auto& sample = chunk[i];

        // Extract only selected channels
        std::vector<float> selected;
        if (channelIndices.isEmpty())
        {
            // No channel filter - store all
            selected = sample;
        }
        else
        {
            selected.reserve(channelIndices.size());
            for (int idx : channelIndices)
            {
                if (idx >= 0 && idx < static_cast<int>(sample.size()))
                    selected.push_back(sample[idx]);
                else
                    selected.push_back(0.0f);
            }
        }

        m_buffer.emplace_back(timestamps[i], std::move(selected));
    }

    // Enforce max buffer size
    while (static_cast<int>(m_buffer.size()) > m_maxBufferSize)
    {
        m_buffer.pop_front();
    }
}

// ============================================================================
// Synchronization Queries
// ============================================================================

QVariantMap EegSyncManager::getEEGForFrame(double videoTimestamp) const
{
    QVariantMap result;
    result["valid"] = false;

    QMutexLocker locker(&m_mutex);

    if (m_buffer.empty() || videoTimestamp <= 0.0)
        return result;

    // Apply clock drift correction
    double adjustedTs = videoTimestamp - m_timeCorrection;

    EegTimestampedSample sample;
    if (m_interpolationMode == 1)
        sample = linearInterpolate(adjustedTs);
    else
        sample = nearestNeighbor(adjustedTs);

    if (!sample.isValid())
        return result;

    // Calculate sync offset
    double offsetMs = std::abs(adjustedTs - sample.lslTimestamp) * 1000.0;
    m_lastSyncOffsetMs = offsetMs;
    updateRunningAverage(offsetMs);

    // Build result
    result["valid"] = true;
    result["timestamp"] = sample.lslTimestamp;
    result["offsetMs"] = offsetMs;

    QVariantList channels;
    for (float val : sample.channels)
        channels.append(static_cast<double>(val));
    result["channels"] = channels;

    return result;
}

QVariantList EegSyncManager::getEEGRangeForFrame(double startTs, double endTs) const
{
    QVariantList results;

    QMutexLocker locker(&m_mutex);

    if (m_buffer.empty() || startTs >= endTs)
        return results;

    double adjustedStart = startTs - m_timeCorrection;
    double adjustedEnd = endTs - m_timeCorrection;

    // Find start position with binary search
    auto itStart = std::lower_bound(
        m_buffer.begin(), m_buffer.end(), adjustedStart,
        [](const EegTimestampedSample& s, double ts) { return s.lslTimestamp < ts; });

    for (auto it = itStart; it != m_buffer.end() && it->lslTimestamp <= adjustedEnd; ++it)
    {
        QVariantMap entry;
        entry["timestamp"] = it->lslTimestamp;
        QVariantList channels;
        for (float val : it->channels)
            channels.append(static_cast<double>(val));
        entry["channels"] = channels;
        results.append(entry);
    }

    return results;
}

// ============================================================================
// Interpolation Algorithms
// ============================================================================

EegTimestampedSample EegSyncManager::nearestNeighbor(double adjustedTs) const
{
    // Binary search for closest timestamp
    auto it = std::lower_bound(
        m_buffer.begin(), m_buffer.end(), adjustedTs,
        [](const EegTimestampedSample& s, double ts) { return s.lslTimestamp < ts; });

    if (it == m_buffer.end())
        return m_buffer.back();

    if (it == m_buffer.begin())
        return m_buffer.front();

    // Compare neighbors
    auto prevIt = std::prev(it);
    double diffCurrent = std::abs(it->lslTimestamp - adjustedTs);
    double diffPrev = std::abs(prevIt->lslTimestamp - adjustedTs);

    return (diffPrev < diffCurrent) ? *prevIt : *it;
}

EegTimestampedSample EegSyncManager::linearInterpolate(double adjustedTs) const
{
    auto it = std::lower_bound(
        m_buffer.begin(), m_buffer.end(), adjustedTs,
        [](const EegTimestampedSample& s, double ts) { return s.lslTimestamp < ts; });

    // Edge cases - fall back to nearest
    if (it == m_buffer.end())
        return m_buffer.back();

    if (it == m_buffer.begin())
        return m_buffer.front();

    auto prevIt = std::prev(it);

    // Linear interpolation factor
    double dt = it->lslTimestamp - prevIt->lslTimestamp;
    if (dt <= 0.0)
        return *prevIt;

    double alpha = (adjustedTs - prevIt->lslTimestamp) / dt;
    alpha = std::clamp(alpha, 0.0, 1.0);

    // Interpolate channel values
    const auto& chA = prevIt->channels;
    const auto& chB = it->channels;
    int numCh = static_cast<int>(std::min(chA.size(), chB.size()));

    std::vector<float> interpolated(numCh);
    for (int i = 0; i < numCh; ++i)
    {
        interpolated[i] = static_cast<float>(
            chA[i] * (1.0 - alpha) + chB[i] * alpha);
    }

    // Use the closer timestamp as reference
    double refTs = (alpha < 0.5) ? prevIt->lslTimestamp : it->lslTimestamp;

    return EegTimestampedSample(refTs, std::move(interpolated));
}

// ============================================================================
// Clock Drift Correction
// ============================================================================

void EegSyncManager::setLslInlet(lsl::stream_inlet* inlet)
{
    m_lslInlet = inlet;

    if (inlet)
    {
        // Get initial time correction
        updateTimeCorrection();
        m_timeCorrectionTimer->start();
        qInfo() << "[EegSyncManager] LSL inlet set, time correction timer started";
    }
    else
    {
        m_timeCorrectionTimer->stop();
    }
}

void EegSyncManager::updateTimeCorrection()
{
    if (!m_lslInlet)
        return;

    try
    {
        m_prevTimeCorrection = m_timeCorrection;
        m_timeCorrection = m_lslInlet->time_correction(1.0); // 1s timeout
        m_timeCorrectionMs = m_timeCorrection;

        // Calculate drift rate (change per update interval)
        double delta = (m_timeCorrection - m_prevTimeCorrection) * 1000.0; // ms
        m_clockDriftMs = delta;

        qDebug() << "[EegSyncManager] Time correction:" << m_timeCorrection * 1000.0
                 << "ms, drift:" << delta << "ms";
    }
    catch (const std::exception& e)
    {
        qWarning() << "[EegSyncManager] time_correction() failed:" << e.what();
    }
}

// ============================================================================
// Configuration
// ============================================================================

void EegSyncManager::setInterpolationMode(int mode)
{
    m_interpolationMode = (mode == 1) ? 1 : 0;
}

void EegSyncManager::clearBuffer()
{
    QMutexLocker locker(&m_mutex);
    m_buffer.clear();
    m_lastSyncOffsetMs = 0.0;
    m_avgSyncOffsetMs = 0.0;
    m_offsetSampleCount = 0;
    m_offsetSum = 0.0;
    emit statsChanged();
}

void EegSyncManager::setSamplingRate(double rate)
{
    if (rate > 0.0 && !qFuzzyCompare(m_samplingRate, rate))
    {
        m_samplingRate = rate;

        // Adjust buffer to ~30s of data
        m_maxBufferSize = static_cast<int>(rate * 30.0);
        emit maxBufferSizeChanged();
        emit samplingRateChanged();

        qInfo() << "[EegSyncManager] Sampling rate:" << rate
                << "Hz, buffer:" << m_maxBufferSize << "samples"
                << ", samples/frame:" << samplesPerFrame();
    }
}

double EegSyncManager::samplesPerFrame() const
{
    if (m_videoFps > 0.0)
        return m_samplingRate / m_videoFps;
    return 0.0;
}

void EegSyncManager::setMaxBufferSize(int size)
{
    if (size > 0 && size != m_maxBufferSize)
    {
        m_maxBufferSize = size;
        emit maxBufferSizeChanged();

        QMutexLocker locker(&m_mutex);
        while (static_cast<int>(m_buffer.size()) > m_maxBufferSize)
            m_buffer.pop_front();
    }
}

// ============================================================================
// Stats / Health
// ============================================================================

int EegSyncManager::bufferSize() const
{
    QMutexLocker locker(&m_mutex);
    return static_cast<int>(m_buffer.size());
}

double EegSyncManager::oldestTimestamp() const
{
    QMutexLocker locker(&m_mutex);
    return m_buffer.empty() ? 0.0 : m_buffer.front().lslTimestamp;
}

double EegSyncManager::newestTimestamp() const
{
    QMutexLocker locker(&m_mutex);
    return m_buffer.empty() ? 0.0 : m_buffer.back().lslTimestamp;
}

double EegSyncManager::bufferDurationSec() const
{
    QMutexLocker locker(&m_mutex);
    if (m_buffer.size() < 2)
        return 0.0;
    return m_buffer.back().lslTimestamp - m_buffer.front().lslTimestamp;
}

QString EegSyncManager::healthStatus() const
{
    if (m_lastSyncOffsetMs > 15.0)
        return QStringLiteral("DESYNC");
    if (m_lastSyncOffsetMs > 5.0)
        return QStringLiteral("WARNING");
    return QStringLiteral("SYNCED");
}

void EegSyncManager::updateRunningAverage(double offsetMs) const
{
    m_offsetSum += offsetMs;
    m_offsetSampleCount++;

    // Reset running average periodically
    if (m_offsetSampleCount >= RUNNING_AVG_WINDOW)
    {
        m_avgSyncOffsetMs = m_offsetSum / m_offsetSampleCount;
        m_offsetSum = 0.0;
        m_offsetSampleCount = 0;
    }
    else
    {
        m_avgSyncOffsetMs = m_offsetSum / m_offsetSampleCount;
    }
}
