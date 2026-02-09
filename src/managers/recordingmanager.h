#ifndef RECORDINGMANAGER_H
#define RECORDINGMANAGER_H

#include <QObject>
#include <QThread>
#include <QTimer>
#include <QVector>
#include <QStringList>
#include <QQmlEngine>
#include <QtQml/qqmlregistration.h>
#include <QMediaRecorder>
#include <QMediaCaptureSession>
#include <vector>
#include <lsl_cpp.h>

#include "sessionconfig.h"
#include "recordingsummary.h"

class RecordingWorker;

/**
 * @brief RecordingManager - Singleton coordinator for all recording operations
 *
 * Manages:
 * - EEG data batching and routing to RecordingWorker (background thread)
 * - Video recording via QMediaRecorder (H.264/MKV)
 * - Pause/resume with video segmentation
 * - Disk space monitoring
 * - Recording statistics
 *
 * Thread safety:
 * - Main thread: all public methods called from UI/EegBackend
 * - Worker thread: RecordingWorker handles disk I/O
 * - Communication via Qt::QueuedConnection signals
 */
class RecordingManager : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

    Q_PROPERTY(bool isRecording READ isRecording NOTIFY isRecordingChanged FINAL)
    Q_PROPERTY(bool isPaused READ isPaused NOTIFY isPausedChanged FINAL)
    Q_PROPERTY(qint64 recordedSamples READ recordedSamples NOTIFY statsUpdated FINAL)
    Q_PROPERTY(qint64 recordedFrames READ recordedFrames NOTIFY statsUpdated FINAL)
    Q_PROPERTY(double recordedDurationSec READ recordedDurationSec NOTIFY statsUpdated FINAL)
    Q_PROPERTY(qint64 eegFileSizeBytes READ eegFileSizeBytes NOTIFY statsUpdated FINAL)
    Q_PROPERTY(qint64 videoFileSizeBytes READ videoFileSizeBytes NOTIFY statsUpdated FINAL)
    Q_PROPERTY(qint64 diskSpaceMB READ diskSpaceMB NOTIFY statsUpdated FINAL)

public:
    static RecordingManager* instance();
    static RecordingManager* create(QQmlEngine* qmlEngine, QJSEngine* jsEngine);

    explicit RecordingManager(QObject* parent = nullptr);
    ~RecordingManager();

    // --- Session control (called from QML) ---

    Q_INVOKABLE bool startRecording(const QString& saveFolderPath,
                                    const QString& sessionName,
                                    const QStringList& channelNames,
                                    const QString& cameraId,
                                    double samplingRate);

    Q_INVOKABLE void pauseRecording();
    Q_INVOKABLE void resumeRecording();
    Q_INVOKABLE void stopRecording();

    // --- Data input (called from EegBackend) ---

    void writeEegData(const std::vector<std::vector<float>>& chunk,
                      const std::vector<double>& timestamps,
                      const QVector<int>& channelIndices);

    void writeMarker(const QString& type, const QString& label, double lslTimestamp);

    // --- State ---

    bool isRecording() const { return m_isRecording; }
    bool isPaused() const { return m_isPaused; }
    qint64 recordedSamples() const { return m_recordedSamples; }
    qint64 recordedFrames() const { return m_recordedFrames; }
    double recordedDurationSec() const;
    qint64 eegFileSizeBytes() const { return m_eegFileSize; }
    qint64 videoFileSizeBytes() const { return m_videoFileSize; }
    qint64 diskSpaceMB() const;

    Q_INVOKABLE bool checkDiskSpace(const QString& path, qint64 requiredMB = 500);

signals:
    void isRecordingChanged();
    void isPausedChanged();
    void statsUpdated();
    void recordingStarted(const QString& sessionName);
    void recordingStopped(const QString& sessionName,
                         const QString& savePath,
                         const QString& duration,
                         const QString& eegSize,
                         const QString& videoSize,
                         qint64 eegSamples,
                         qint64 videoFrames,
                         int markerCount);
    void recordingError(const QString& error);

private slots:
    void onFilesInitialized(bool success, const QString& error);
    void onBatchWritten(int sampleCount, qint64 eegFileSize);
    void onFilesClosed(const RecordingSummary& summary);
    void onWorkerError(const QString& error);
    void onFrameReady(double lslTimestamp);
    void onFlushTimer();
    void onDiskCheckTimer();
    void onStatsTimer();

private:
    void flushEegBatch();
    void startVideoRecording();
    void stopVideoRecording();
    double sessionTimeSec(double lslTimestamp) const;
    void cleanupWorkerThread();

    static RecordingManager* s_instance;

    // Worker thread
    QThread* m_workerThread = nullptr;
    RecordingWorker* m_worker = nullptr;

    // Video recording
    QMediaRecorder* m_videoRecorder = nullptr;

    // Session config
    SessionConfig m_config;
    bool m_isRecording = false;
    bool m_isPaused = false;

    // Timing
    double m_sessionStartLslTime = 0.0;
    double m_pauseStartLslTime = 0.0;
    double m_totalPausedDuration = 0.0;
    int m_videoSegmentCount = 1;

    // EEG batching
    QVector<QVector<float>> m_eegBatch;
    QVector<double> m_timestampBatch;
    static constexpr int EEG_BATCH_SIZE = 100;

    // Statistics
    qint64 m_recordedSamples = 0;
    qint64 m_recordedFrames = 0;
    qint64 m_eegFileSize = 0;
    qint64 m_videoFileSize = 0;

    // Timers
    QTimer* m_flushTimer = nullptr;      // Force flush every 5s
    QTimer* m_diskCheckTimer = nullptr;  // Check disk every 5 min
    QTimer* m_statsTimer = nullptr;      // Update stats every 1s
};

#endif // RECORDINGMANAGER_H
