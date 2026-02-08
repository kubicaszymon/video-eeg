#ifndef VIDEOBACKEND_H
#define VIDEOBACKEND_H

#include <QObject>
#include <QImage>
#include <QVideoSink>
#include <QVideoFrame>
#include <QElapsedTimer>
#include <QMutex>
#include <QVariant>
#include <deque>
#include <QtQml/qqmlregistration.h>
#include "cameramanager.h"
#include "videoframepacket.h"

/**
 * @brief VideoBackend - Main backend for VideoDisplayWindow
 *
 * This class provides:
 * - Video capture management
 * - Frame display via QVideoSink for QML VideoOutput
 * - Frame buffer with LSL timestamps for synchronization
 * - Statistics (FPS, latency, frame count)
 *
 * Synchronization Architecture:
 * - Each captured frame is timestamped with lsl::local_clock()
 * - Frames are stored in a buffer for potential replay/sync
 * - Use getFrameAtTime() to retrieve frame closest to a given LSL timestamp
 * - This enables synchronization with EEG data that also uses LSL timestamps
 *
 * Usage:
 * 1. Create in QML and set cameraId if known
 * 2. Call startCapture() to begin
 * 3. VideoOutput displays via videoSink property
 * 4. Use frameBuffer for synchronization with EEG
 */
class VideoBackend : public QObject
{
    Q_OBJECT
    QML_ELEMENT

    // Camera identification
    Q_PROPERTY(QString cameraId READ cameraId WRITE setCameraId NOTIFY cameraIdChanged FINAL)
    Q_PROPERTY(QString cameraName READ cameraName NOTIFY cameraIdChanged FINAL)

    // Video sink for QML VideoOutput
    Q_PROPERTY(QVideoSink* videoSink READ videoSink WRITE setVideoSink NOTIFY videoSinkChanged FINAL)

    // Capture state
    Q_PROPERTY(bool isCapturing READ isCapturing NOTIFY isCapturingChanged FINAL)
    Q_PROPERTY(bool isConnected READ isConnected NOTIFY isConnectedChanged FINAL)

    // Statistics
    Q_PROPERTY(double currentFps READ currentFps NOTIFY statsUpdated FINAL)
    Q_PROPERTY(qint64 frameCount READ frameCount NOTIFY statsUpdated FINAL)
    Q_PROPERTY(double lastFrameTimestamp READ lastFrameTimestamp NOTIFY frameReceived FINAL)
    Q_PROPERTY(int bufferSize READ bufferSize NOTIFY statsUpdated FINAL)

    // Configuration
    Q_PROPERTY(int maxBufferSize READ maxBufferSize WRITE setMaxBufferSize NOTIFY maxBufferSizeChanged FINAL)

public:
    explicit VideoBackend(QObject* parent = nullptr);
    ~VideoBackend();

    // Camera identification
    QString cameraId() const { return m_cameraId; }
    void setCameraId(const QString& id);
    QString cameraName() const;

    // Video sink
    QVideoSink* videoSink() const { return m_videoSink; }
    void setVideoSink(QVideoSink* sink);

    // Capture control
    Q_INVOKABLE void startCapture();
    Q_INVOKABLE void stopCapture();
    bool isCapturing() const { return m_isCapturing; }
    bool isConnected() const { return m_isConnected; }

    // Statistics
    double currentFps() const { return m_currentFps; }
    qint64 frameCount() const { return m_frameCount; }
    double lastFrameTimestamp() const { return m_lastFrameTimestamp; }
    int bufferSize() const;

    // Buffer configuration
    int maxBufferSize() const { return m_maxBufferSize; }
    void setMaxBufferSize(int size);

    // Synchronization API
    Q_INVOKABLE VideoFramePacket getFrameAtTime(double lslTimestamp) const;
    Q_INVOKABLE QVariantList getFrameTimestamps() const;
    Q_INVOKABLE double getLatestTimestamp() const;
    Q_INVOKABLE double getOldestTimestamp() const;

    // Frame buffer access (for EEG synchronization)
    std::deque<VideoFramePacket> frameBuffer() const;
    void clearBuffer();

signals:
    void cameraIdChanged();
    void videoSinkChanged();
    void isCapturingChanged();
    void isConnectedChanged();
    void statsUpdated();
    void frameReceived(double lslTimestamp);
    void maxBufferSizeChanged();
    void errorOccurred(const QString& error);

private slots:
    void onFrameReady(const VideoFramePacket& packet);
    void onCameraError(const QString& error);

private:
    void addFrameToBuffer(const VideoFramePacket& packet);
    void updateVideoSink(const QImage& image);

    CameraManager* m_cameraManager = nullptr;

    // Camera identification
    QString m_cameraId;

    // Video sink for QML
    QVideoSink* m_videoSink = nullptr;

    // State
    bool m_isCapturing = false;
    bool m_isConnected = false;

    // Statistics
    double m_currentFps = 0.0;
    qint64 m_frameCount = 0;
    double m_lastFrameTimestamp = 0.0;

    // Frame buffer for synchronization
    mutable QMutex m_bufferMutex;
    std::deque<VideoFramePacket> m_frameBuffer;
    int m_maxBufferSize = 300; // ~10 seconds at 30fps
};

#endif // VIDEOBACKEND_H
