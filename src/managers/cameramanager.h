#ifndef CAMERAMANAGER_H
#define CAMERAMANAGER_H

#include <QObject>
#include <QCamera>
#include <QMediaCaptureSession>
#include <QVideoSink>
#include <QVideoFrame>
#include <QMediaDevices>
#include <QCameraDevice>
#include <QCameraFormat>
#include <QPointer>
#include <QTimer>
#include <QVariant>
#include <QQmlEngine>
#include <QtQml/qqmlregistration.h>
#include <lsl_cpp.h>

#include "videoframepacket.h"

/**
 * @brief CameraManager - Manages camera devices and video capture with LSL synchronization
 *
 * This class provides:
 * - Camera device enumeration via QMediaDevices
 * - Camera selection and configuration (resolution, FPS)
 * - Video frame capture with LSL timestamps via QVideoSink
 * - Thread-safe frame delivery for real-time processing
 *
 * Architecture:
 * - Singleton pattern for global access
 * - Uses Qt Multimedia (QCamera, QMediaCaptureSession, QVideoSink)
 * - Each frame is timestamped with lsl::local_clock() for EEG synchronization
 *
 * Usage:
 * 1. Call refreshCameraList() to enumerate available cameras
 * 2. Select camera with setCurrentCamera()
 * 3. Optionally set format with setCurrentFormat()
 * 4. Call startCapture() to begin frame capture
 * 5. Connect to frameReady() signal to receive VideoFramePacket data
 */
class CameraManager : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

    Q_PROPERTY(QVariantList availableCameras READ availableCameras NOTIFY availableCamerasChanged FINAL)
    Q_PROPERTY(int currentCameraIndex READ currentCameraIndex WRITE setCurrentCameraIndex NOTIFY currentCameraIndexChanged FINAL)
    Q_PROPERTY(QVariantList availableFormats READ availableFormats NOTIFY availableFormatsChanged FINAL)
    Q_PROPERTY(int currentFormatIndex READ currentFormatIndex WRITE setCurrentFormatIndex NOTIFY currentFormatIndexChanged FINAL)
    Q_PROPERTY(bool isCapturing READ isCapturing NOTIFY isCapturingChanged FINAL)
    Q_PROPERTY(bool isPreviewActive READ isPreviewActive NOTIFY isPreviewActiveChanged FINAL)
    Q_PROPERTY(QString currentCameraName READ currentCameraName NOTIFY currentCameraIndexChanged FINAL)
    Q_PROPERTY(QString currentFormatString READ currentFormatString NOTIFY currentFormatIndexChanged FINAL)
    Q_PROPERTY(double currentFps READ currentFps NOTIFY fpsUpdated FINAL)
    Q_PROPERTY(QMediaCaptureSession* captureSession READ captureSession CONSTANT FINAL)
    Q_PROPERTY(double lastFrameTimestamp READ lastFrameTimestamp NOTIFY frameTimestampUpdated FINAL)

public:
    static CameraManager* instance();
    static CameraManager* create(QQmlEngine* qmlEngine, QJSEngine* jsEngine);

    explicit CameraManager(QObject* parent = nullptr);
    ~CameraManager();

    // Camera enumeration
    Q_INVOKABLE void refreshCameraList();
    QVariantList availableCameras() const;
    QList<CameraInfo> cameraInfoList() const { return m_cameras; }

    // Camera selection
    int currentCameraIndex() const { return m_currentCameraIndex; }
    Q_INVOKABLE void setCurrentCameraIndex(int index);
    QString currentCameraName() const;

    // Format selection
    QVariantList availableFormats() const;
    int currentFormatIndex() const { return m_currentFormatIndex; }
    Q_INVOKABLE void setCurrentFormatIndex(int index);
    QString currentFormatString() const;

    // Capture control
    Q_INVOKABLE void startCapture();
    Q_INVOKABLE void stopCapture();
    bool isCapturing() const { return m_isCapturing; }

    // Preview control (for settings window)
    Q_INVOKABLE void startPreview();
    Q_INVOKABLE void stopPreview();
    bool isPreviewActive() const { return m_isPreviewActive; }

    // Video sink access for QML (preview)
    Q_INVOKABLE QVideoSink* videoSink() const { return m_videoSink; }
    Q_INVOKABLE void setExternalVideoSink(QVideoSink* sink);

    // Capture session access for QML VideoOutput binding
    QMediaCaptureSession* captureSession() const { return m_captureSession.data(); }

    // Statistics
    double currentFps() const { return m_currentFps; }
    qint64 frameCount() const { return m_frameCount; }
    double lastFrameTimestamp() const { return m_lastFrameTimestamp; }

signals:
    void availableCamerasChanged();
    void currentCameraIndexChanged();
    void availableFormatsChanged();
    void currentFormatIndexChanged();
    void isCapturingChanged();
    void isPreviewActiveChanged();
    void fpsUpdated();
    void frameTimestampUpdated();

    // Main signal for frame delivery with LSL timestamp
    void frameReady(const VideoFramePacket& packet);

    // Error handling
    void errorOccurred(const QString& error);

private slots:
    void onVideoFrameChanged(const QVideoFrame& frame);
    void onCameraErrorOccurred(QCamera::Error error, const QString& errorString);
    void onCameraActiveChanged(bool active);
    void updateFpsCounter();

private:
    void setupCamera(const QCameraDevice& device);
    void cleanupCamera();
    void populateFormatsForCurrentCamera();
    QImage videoFrameToImage(const QVideoFrame& frame);

    static CameraManager* s_instance;

    // Camera devices
    QList<CameraInfo> m_cameras;
    int m_currentCameraIndex = -1;
    int m_currentFormatIndex = -1;

    // Qt Multimedia objects
    QPointer<QCamera> m_camera;
    QPointer<QMediaCaptureSession> m_captureSession;
    QPointer<QVideoSink> m_videoSink;
    QPointer<QVideoSink> m_externalSink;

    // State
    bool m_isCapturing = false;
    bool m_isPreviewActive = false;

    // Statistics
    qint64 m_frameCount = 0;
    double m_currentFps = 0.0;
    double m_lastFrameTimestamp = 0.0;
    qint64 m_lastFpsUpdateTime = 0;
    qint64 m_framesAtLastUpdate = 0;
    QTimer* m_fpsTimer = nullptr;
};

#endif // CAMERAMANAGER_H
