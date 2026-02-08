#ifndef EEGSYNCMANAGER_H
#define EEGSYNCMANAGER_H

#include <QObject>
#include <QMutex>
#include <QTimer>
#include <QVector>
#include <QVariantMap>
#include <QVariantList>
#include <QtQml/qqmlregistration.h>
#include <deque>
#include <vector>
#include <lsl_cpp.h>

/**
 * @brief Timestamped EEG sample for synchronization buffer
 */
struct EegTimestampedSample
{
    double lslTimestamp = 0.0;
    std::vector<float> channels;

    EegTimestampedSample() = default;
    EegTimestampedSample(double ts, std::vector<float> vals)
        : lslTimestamp(ts), channels(std::move(vals)) {}

    bool isValid() const { return lslTimestamp > 0.0 && !channels.empty(); }
};

/**
 * @brief EegSyncManager - Manages EEG-Video synchronization
 *
 * Maintains a circular buffer of timestamped EEG samples and provides
 * synchronization queries to retrieve EEG data matching a video frame timestamp.
 *
 * Uses LSL time_correction() for clock drift compensation between
 * the EEG device clock and the local machine clock.
 *
 * Thread-safe for concurrent access from EEG worker and UI threads.
 */
class EegSyncManager : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

    // Buffer state
    Q_PROPERTY(int bufferSize READ bufferSize NOTIFY statsChanged FINAL)
    Q_PROPERTY(int maxBufferSize READ maxBufferSize WRITE setMaxBufferSize NOTIFY maxBufferSizeChanged FINAL)
    Q_PROPERTY(double oldestTimestamp READ oldestTimestamp NOTIFY statsChanged FINAL)
    Q_PROPERTY(double newestTimestamp READ newestTimestamp NOTIFY statsChanged FINAL)
    Q_PROPERTY(double bufferDurationSec READ bufferDurationSec NOTIFY statsChanged FINAL)

    // Sync quality metrics
    Q_PROPERTY(double lastSyncOffsetMs READ lastSyncOffsetMs NOTIFY statsChanged FINAL)
    Q_PROPERTY(double avgSyncOffsetMs READ avgSyncOffsetMs NOTIFY statsChanged FINAL)
    Q_PROPERTY(double clockDriftMs READ clockDriftMs NOTIFY statsChanged FINAL)
    Q_PROPERTY(double timeCorrectionMs READ timeCorrectionMs NOTIFY statsChanged FINAL)

    // Health
    Q_PROPERTY(QString healthStatus READ healthStatus NOTIFY statsChanged FINAL)
    Q_PROPERTY(double samplingRate READ samplingRate NOTIFY samplingRateChanged FINAL)
    Q_PROPERTY(double samplesPerFrame READ samplesPerFrame NOTIFY samplingRateChanged FINAL)

public:
    static EegSyncManager* instance();
    static EegSyncManager* create(QQmlEngine* qmlEngine, QJSEngine* jsEngine);

    explicit EegSyncManager(QObject* parent = nullptr);
    ~EegSyncManager();

    // --- Data input ---

    /**
     * @brief Add EEG samples with LSL timestamps to the sync buffer
     * @param chunk Raw EEG chunk [sample][channel]
     * @param timestamps LSL timestamp per sample
     * @param channelIndices Which channels to store (selected channels only)
     */
    void addEegSamples(const std::vector<std::vector<float>>& chunk,
                       const std::vector<double>& timestamps,
                       const QVector<int>& channelIndices);

    // --- Synchronization queries ---

    /**
     * @brief Get EEG data corresponding to a video frame timestamp
     * @param videoTimestamp LSL timestamp of the video frame
     * @return QVariantMap with keys: "timestamp", "channels", "offsetMs", "valid"
     *
     * Uses nearest neighbor or linear interpolation depending on mode.
     * Applies LSL time_correction() for clock drift compensation.
     */
    Q_INVOKABLE QVariantMap getEEGForFrame(double videoTimestamp) const;

    /**
     * @brief Get all EEG samples in a time range
     * @param startTs Start LSL timestamp
     * @param endTs End LSL timestamp
     * @return List of QVariantMaps, each with "timestamp" and "channels"
     */
    Q_INVOKABLE QVariantList getEEGRangeForFrame(double startTs, double endTs) const;

    // --- Configuration ---

    Q_INVOKABLE void setInterpolationMode(int mode); // 0=nearest, 1=linear
    Q_INVOKABLE void clearBuffer();

    void setSamplingRate(double rate);
    double samplingRate() const { return m_samplingRate; }
    double samplesPerFrame() const;

    int bufferSize() const;
    int maxBufferSize() const { return m_maxBufferSize; }
    void setMaxBufferSize(int size);

    double oldestTimestamp() const;
    double newestTimestamp() const;
    double bufferDurationSec() const;

    // Sync quality
    double lastSyncOffsetMs() const { return m_lastSyncOffsetMs; }
    double avgSyncOffsetMs() const { return m_avgSyncOffsetMs; }
    double clockDriftMs() const { return m_clockDriftMs; }
    double timeCorrectionMs() const { return m_timeCorrectionMs * 1000.0; }
    QString healthStatus() const;

    // LSL inlet for time_correction()
    void setLslInlet(lsl::stream_inlet* inlet);

signals:
    void statsChanged();
    void samplingRateChanged();
    void maxBufferSizeChanged();

private:
    void updateTimeCorrection();
    EegTimestampedSample nearestNeighbor(double adjustedTs) const;
    EegTimestampedSample linearInterpolate(double adjustedTs) const;
    void updateRunningAverage(double offsetMs);

    static EegSyncManager* s_instance;

    // Circular buffer (sorted by timestamp)
    mutable QMutex m_mutex;
    std::deque<EegTimestampedSample> m_buffer;
    int m_maxBufferSize = 7680; // ~30s at 256 Hz

    // Configuration
    double m_samplingRate = 256.0;
    int m_interpolationMode = 0; // 0=nearest, 1=linear
    double m_videoFps = 30.0;

    // LSL time correction
    lsl::stream_inlet* m_lslInlet = nullptr;
    double m_timeCorrection = 0.0;     // seconds
    double m_prevTimeCorrection = 0.0;
    QTimer* m_timeCorrectionTimer = nullptr;

    // Monitoring stats
    mutable double m_lastSyncOffsetMs = 0.0;
    mutable double m_avgSyncOffsetMs = 0.0;
    double m_clockDriftMs = 0.0;
    double m_timeCorrectionMs = 0.0;

    // Running average
    mutable int m_offsetSampleCount = 0;
    mutable double m_offsetSum = 0.0;
    static constexpr int RUNNING_AVG_WINDOW = 100;

    // Stats update throttling
    QTimer* m_statsTimer = nullptr;
};

#endif // EEGSYNCMANAGER_H
