#include "recordingmanager.h"
#include "recordingworker.h"
#include "cameramanager.h"

#include <QDir>
#include <QStorageInfo>
#include <QFileInfo>
#include <QMediaFormat>
#include <QDebug>

RecordingManager* RecordingManager::s_instance = nullptr;

RecordingManager* RecordingManager::instance()
{
    if (!s_instance)
        s_instance = new RecordingManager();
    return s_instance;
}

RecordingManager* RecordingManager::create(QQmlEngine* qmlEngine, QJSEngine* jsEngine)
{
    Q_UNUSED(jsEngine)
    auto* inst = instance();
    QJSEngine::setObjectOwnership(inst, QJSEngine::CppOwnership);
    if (qmlEngine)
        qmlEngine->setObjectOwnership(inst, QQmlEngine::CppOwnership);
    return inst;
}

RecordingManager::RecordingManager(QObject* parent)
    : QObject(parent)
{
    qRegisterMetaType<RecordingSummary>("RecordingSummary");
    qRegisterMetaType<QVector<QVector<float>>>("QVector<QVector<float>>");
    qRegisterMetaType<QVector<double>>("QVector<double>");

    // Flush timer - forces EEG batch write every 5 seconds
    m_flushTimer = new QTimer(this);
    m_flushTimer->setInterval(5000);
    connect(m_flushTimer, &QTimer::timeout, this, &RecordingManager::onFlushTimer);

    // Disk check timer - checks free space every 5 minutes
    m_diskCheckTimer = new QTimer(this);
    m_diskCheckTimer->setInterval(5 * 60 * 1000);
    connect(m_diskCheckTimer, &QTimer::timeout, this, &RecordingManager::onDiskCheckTimer);

    // Stats timer - updates UI stats every second
    m_statsTimer = new QTimer(this);
    m_statsTimer->setInterval(1000);
    connect(m_statsTimer, &QTimer::timeout, this, &RecordingManager::onStatsTimer);
}

RecordingManager::~RecordingManager()
{
    if (m_isRecording)
        stopRecording();
    cleanupWorkerThread();
}

bool RecordingManager::startRecording(const QString& saveFolderPath,
                                       const QString& sessionName,
                                       const QStringList& channelNames,
                                       const QString& cameraId,
                                       double samplingRate)
{
    if (m_isRecording) {
        qWarning() << "[RecordingManager] Already recording";
        return false;
    }

    // Validate folder
    QDir saveDir(saveFolderPath);
    if (!saveDir.exists()) {
        if (!saveDir.mkpath(".")) {
            emit recordingError("Cannot create save folder: " + saveFolderPath);
            return false;
        }
    }

    // Check disk space (need at least 500 MB)
    if (!checkDiskSpace(saveFolderPath, 500)) {
        emit recordingError("Insufficient disk space. Need at least 500 MB free.");
        return false;
    }

    // Configure session
    m_config.saveFolderPath = saveFolderPath;
    m_config.sessionName = sessionName;
    m_config.channelNames = channelNames;
    m_config.cameraId = cameraId;
    m_config.samplingRate = samplingRate;

    // Reset state
    m_sessionStartLslTime = lsl::local_clock();
    m_totalPausedDuration = 0.0;
    m_pauseStartLslTime = 0.0;
    m_videoSegmentCount = 1;
    m_recordedSamples = 0;
    m_recordedFrames = 0;
    m_eegFileSize = 0;
    m_videoFileSize = 0;
    m_eegBatch.clear();
    m_timestampBatch.clear();

    // Create worker thread
    cleanupWorkerThread();
    m_workerThread = new QThread(this);
    m_worker = new RecordingWorker();
    m_worker->moveToThread(m_workerThread);

    // Connect worker signals
    connect(m_worker, &RecordingWorker::filesInitialized,
            this, &RecordingManager::onFilesInitialized, Qt::QueuedConnection);
    connect(m_worker, &RecordingWorker::batchWritten,
            this, &RecordingManager::onBatchWritten, Qt::QueuedConnection);
    connect(m_worker, &RecordingWorker::filesClosed,
            this, &RecordingManager::onFilesClosed, Qt::QueuedConnection);
    connect(m_worker, &RecordingWorker::errorOccurred,
            this, &RecordingManager::onWorkerError, Qt::QueuedConnection);

    // Clean up worker when thread finishes
    connect(m_workerThread, &QThread::finished, m_worker, &QObject::deleteLater);

    m_workerThread->start();

    // Initialize files on worker thread
    QMetaObject::invokeMethod(m_worker, "initializeFiles",
                              Qt::QueuedConnection,
                              Q_ARG(QString, m_config.eegFilePath()),
                              Q_ARG(QString, m_config.markersFilePath()),
                              Q_ARG(QString, m_config.framesFilePath()),
                              Q_ARG(QString, m_config.metadataFilePath()),
                              Q_ARG(QStringList, channelNames),
                              Q_ARG(QString, sessionName),
                              Q_ARG(double, samplingRate));

    // Start video recording if camera is selected
    if (!cameraId.isEmpty()) {
        startVideoRecording();

        // Connect to CameraManager for frame timestamps
        connect(CameraManager::instance(), &CameraManager::frameTimestampUpdated,
                this, [this]() {
            onFrameReady(CameraManager::instance()->lastFrameTimestamp());
        });
    }

    m_isRecording = true;
    m_isPaused = false;
    emit isRecordingChanged();
    emit isPausedChanged();

    // Start timers
    m_flushTimer->start();
    m_diskCheckTimer->start();
    m_statsTimer->start();

    qDebug() << "[RecordingManager] Recording started:" << sessionName;
    emit recordingStarted(sessionName);
    return true;
}

void RecordingManager::pauseRecording()
{
    if (!m_isRecording || m_isPaused)
        return;

    m_isPaused = true;
    m_pauseStartLslTime = lsl::local_clock();

    // Flush any pending EEG data before pause
    flushEegBatch();

    // Write pause marker
    double sessTime = sessionTimeSec(m_pauseStartLslTime);
    QMetaObject::invokeMethod(m_worker, "writePauseMarker",
                              Qt::QueuedConnection,
                              Q_ARG(QString, "PAUSE_START"),
                              Q_ARG(double, m_pauseStartLslTime),
                              Q_ARG(double, sessTime));

    // Stop video recording
    stopVideoRecording();

    emit isPausedChanged();
    qDebug() << "[RecordingManager] Recording paused at LSL:" << m_pauseStartLslTime;
}

void RecordingManager::resumeRecording()
{
    if (!m_isRecording || !m_isPaused)
        return;

    double resumeTime = lsl::local_clock();
    m_totalPausedDuration += (resumeTime - m_pauseStartLslTime);
    m_isPaused = false;

    // Write resume marker
    double sessTime = sessionTimeSec(resumeTime);
    QMetaObject::invokeMethod(m_worker, "writePauseMarker",
                              Qt::QueuedConnection,
                              Q_ARG(QString, "PAUSE_STOP"),
                              Q_ARG(double, resumeTime),
                              Q_ARG(double, sessTime));

    // Start new video segment
    if (!m_config.cameraId.isEmpty()) {
        m_videoSegmentCount++;
        startVideoRecording();
    }

    emit isPausedChanged();
    qDebug() << "[RecordingManager] Recording resumed. Total paused:" << m_totalPausedDuration << "s";
}

void RecordingManager::stopRecording()
{
    if (!m_isRecording)
        return;

    // If paused, account for final pause duration
    if (m_isPaused) {
        m_totalPausedDuration += (lsl::local_clock() - m_pauseStartLslTime);
        m_isPaused = false;
    }

    // Flush remaining EEG data
    flushEegBatch();

    // Stop video
    stopVideoRecording();

    // Disconnect camera frame signal
    if (!m_config.cameraId.isEmpty()) {
        disconnect(CameraManager::instance(), &CameraManager::frameTimestampUpdated,
                   this, nullptr);
    }

    // Calculate video file size (sum of all segments)
    qint64 totalVideoSize = 0;
    for (int seg = 1; seg <= m_videoSegmentCount; ++seg) {
        QFileInfo fi(m_config.videoSegmentFilePath(seg));
        if (fi.exists())
            totalVideoSize += fi.size();
    }
    m_videoFileSize = totalVideoSize;

    // Close files on worker thread
    double duration = recordedDurationSec();
    QMetaObject::invokeMethod(m_worker, "closeFiles",
                              Qt::QueuedConnection,
                              Q_ARG(double, duration),
                              Q_ARG(qint64, totalVideoSize));

    m_isRecording = false;
    emit isRecordingChanged();
    emit isPausedChanged();

    // Stop timers
    m_flushTimer->stop();
    m_diskCheckTimer->stop();
    m_statsTimer->stop();

    qDebug() << "[RecordingManager] Recording stopped. Duration:" << duration << "s";
}

void RecordingManager::writeEegData(const std::vector<std::vector<float>>& chunk,
                                     const std::vector<double>& timestamps,
                                     const QVector<int>& channelIndices)
{
    if (!m_isRecording || m_isPaused)
        return;

    // Extract selected channels and add to batch
    for (size_t i = 0; i < chunk.size(); ++i) {
        QVector<float> selectedChannels;
        selectedChannels.reserve(channelIndices.size());

        for (int chIdx : channelIndices) {
            if (chIdx < static_cast<int>(chunk[i].size()))
                selectedChannels.append(chunk[i][chIdx]);
            else
                selectedChannels.append(0.0f);
        }

        m_eegBatch.append(selectedChannels);
        m_timestampBatch.append(timestamps[i]);
    }

    // Flush when batch is full
    if (m_eegBatch.size() >= EEG_BATCH_SIZE) {
        flushEegBatch();
    }
}

void RecordingManager::writeMarker(const QString& type, const QString& label,
                                    double lslTimestamp)
{
    if (!m_isRecording || !m_worker)
        return;

    double sessTime = sessionTimeSec(lslTimestamp);
    QMetaObject::invokeMethod(m_worker, "writeMarker",
                              Qt::QueuedConnection,
                              Q_ARG(QString, type),
                              Q_ARG(QString, label),
                              Q_ARG(double, lslTimestamp),
                              Q_ARG(double, sessTime));
}

double RecordingManager::recordedDurationSec() const
{
    if (!m_isRecording)
        return 0.0;

    double now = lsl::local_clock();
    double totalElapsed = now - m_sessionStartLslTime;
    double pausedNow = m_isPaused ? (now - m_pauseStartLslTime) : 0.0;
    return totalElapsed - m_totalPausedDuration - pausedNow;
}

qint64 RecordingManager::diskSpaceMB() const
{
    if (m_config.saveFolderPath.isEmpty())
        return -1;
    QStorageInfo storage(m_config.saveFolderPath);
    return storage.bytesAvailable() / (1024 * 1024);
}

bool RecordingManager::checkDiskSpace(const QString& path, qint64 requiredMB)
{
    QStorageInfo storage(path);
    if (!storage.isValid())
        return false;
    qint64 availableMB = storage.bytesAvailable() / (1024 * 1024);
    return availableMB >= requiredMB;
}

// --- Private slots ---

void RecordingManager::onFilesInitialized(bool success, const QString& error)
{
    if (!success) {
        qWarning() << "[RecordingManager] File init failed:" << error;
        emit recordingError(error);
        stopRecording();
    }
}

void RecordingManager::onBatchWritten(int sampleCount, qint64 eegFileSize)
{
    m_recordedSamples += sampleCount;
    m_eegFileSize = eegFileSize;
}

void RecordingManager::onFilesClosed(const RecordingSummary& summary)
{
    qDebug() << "[RecordingManager] Summary - EEG:" << summary.eegSizeFormatted()
             << "Video:" << summary.videoSizeFormatted()
             << "Duration:" << summary.durationFormatted();

    emit recordingStopped(summary.sessionName,
                         summary.savePath,
                         summary.durationFormatted(),
                         summary.eegSizeFormatted(),
                         summary.videoSizeFormatted(),
                         summary.eegSamples,
                         summary.videoFrames,
                         summary.markerCount);

    cleanupWorkerThread();
}

void RecordingManager::onWorkerError(const QString& error)
{
    qWarning() << "[RecordingManager] Worker error:" << error;
    emit recordingError(error);
}

void RecordingManager::onFrameReady(double lslTimestamp)
{
    if (!m_isRecording || m_isPaused || !m_worker)
        return;

    m_recordedFrames++;
    QString segmentFile = QFileInfo(m_config.videoSegmentFilePath(m_videoSegmentCount)).fileName();

    QMetaObject::invokeMethod(m_worker, "writeFrameTimestamp",
                              Qt::QueuedConnection,
                              Q_ARG(double, lslTimestamp),
                              Q_ARG(qint64, m_recordedFrames),
                              Q_ARG(QString, segmentFile));
}

void RecordingManager::onFlushTimer()
{
    if (!m_eegBatch.isEmpty()) {
        flushEegBatch();
    }
}

void RecordingManager::onDiskCheckTimer()
{
    if (!m_isRecording)
        return;

    qint64 freeMB = diskSpaceMB();
    if (freeMB >= 0 && freeMB < 500) {
        qWarning() << "[RecordingManager] Low disk space:" << freeMB << "MB";
        emit recordingError(QString("Low disk space: %1 MB remaining. Recording will stop.").arg(freeMB));
        stopRecording();
    }
}

void RecordingManager::onStatsTimer()
{
    // Update video file size
    if (!m_config.cameraId.isEmpty()) {
        qint64 totalVideoSize = 0;
        for (int seg = 1; seg <= m_videoSegmentCount; ++seg) {
            QFileInfo fi(m_config.videoSegmentFilePath(seg));
            if (fi.exists())
                totalVideoSize += fi.size();
        }
        m_videoFileSize = totalVideoSize;
    }

    emit statsUpdated();
}

// --- Private methods ---

void RecordingManager::flushEegBatch()
{
    if (m_eegBatch.isEmpty() || !m_worker)
        return;

    // Move batch data to worker
    QMetaObject::invokeMethod(m_worker, "writeEegBatch",
                              Qt::QueuedConnection,
                              Q_ARG(QVector<QVector<float>>, m_eegBatch),
                              Q_ARG(QVector<double>, m_timestampBatch));

    m_eegBatch.clear();
    m_timestampBatch.clear();
}

void RecordingManager::startVideoRecording()
{
    auto* cameraMgr = CameraManager::instance();
    if (!cameraMgr || !cameraMgr->captureSession())
        return;

    // Create recorder
    if (m_videoRecorder) {
        delete m_videoRecorder;
        m_videoRecorder = nullptr;
    }

    m_videoRecorder = new QMediaRecorder(this);
    cameraMgr->captureSession()->setRecorder(m_videoRecorder);

    // Configure encoder: H.264 in MKV container
    QMediaFormat format;
    format.setFileFormat(QMediaFormat::Matroska);
    format.setVideoCodec(QMediaFormat::VideoCodec::H264);
    m_videoRecorder->setMediaFormat(format);
    m_videoRecorder->setQuality(QMediaRecorder::HighQuality);

    // Set output location
    QString videoPath = m_config.videoSegmentFilePath(m_videoSegmentCount);
    m_videoRecorder->setOutputLocation(QUrl::fromLocalFile(videoPath));

    // Connect error signal
    connect(m_videoRecorder, &QMediaRecorder::errorOccurred,
            this, [this](QMediaRecorder::Error error, const QString& errorString) {
        Q_UNUSED(error)
        qWarning() << "[RecordingManager] Video recorder error:" << errorString;
        emit recordingError("Video recording error: " + errorString);
    });

    m_videoRecorder->record();
    qDebug() << "[RecordingManager] Video recording started:" << videoPath;
}

void RecordingManager::stopVideoRecording()
{
    if (m_videoRecorder) {
        m_videoRecorder->stop();
        qDebug() << "[RecordingManager] Video recording stopped";
    }
}

double RecordingManager::sessionTimeSec(double lslTimestamp) const
{
    double elapsed = lslTimestamp - m_sessionStartLslTime;
    return elapsed - m_totalPausedDuration;
}

void RecordingManager::cleanupWorkerThread()
{
    if (m_workerThread) {
        m_workerThread->quit();
        m_workerThread->wait(5000);
        delete m_workerThread;
        m_workerThread = nullptr;
        m_worker = nullptr; // Deleted by QThread::finished → deleteLater
    }
}
