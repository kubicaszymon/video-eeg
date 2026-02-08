#include "cameramanager.h"

#include <QDebug>
#include <QTimer>
#include <QDateTime>
#include <QVideoFrame>

CameraManager* CameraManager::s_instance = nullptr;

CameraManager* CameraManager::instance()
{
    if (!s_instance) {
        s_instance = new CameraManager();
    }
    return s_instance;
}

CameraManager* CameraManager::create(QQmlEngine* qmlEngine, QJSEngine* jsEngine)
{
    Q_UNUSED(qmlEngine)
    Q_UNUSED(jsEngine)
    return instance();
}

CameraManager::CameraManager(QObject* parent)
    : QObject(parent)
{
    qInfo() << "CameraManager created";

    // Create video sink for frame capture
    m_videoSink = new QVideoSink(this);
    connect(m_videoSink, &QVideoSink::videoFrameChanged,
            this, &CameraManager::onVideoFrameChanged);

    // Create capture session
    m_captureSession = new QMediaCaptureSession(this);
    m_captureSession->setVideoSink(m_videoSink);

    // FPS update timer
    m_fpsTimer = new QTimer(this);
    m_fpsTimer->setInterval(1000);
    connect(m_fpsTimer, &QTimer::timeout, this, &CameraManager::updateFpsCounter);

    // Initial camera list refresh
    refreshCameraList();
}

CameraManager::~CameraManager()
{
    qInfo() << "CameraManager destroyed";
    stopCapture();
    cleanupCamera();

    if (s_instance == this) {
        s_instance = nullptr;
    }
}

void CameraManager::refreshCameraList()
{
    qInfo() << "CameraManager: Refreshing camera list...";

    m_cameras.clear();

    const QList<QCameraDevice> devices = QMediaDevices::videoInputs();
    const QCameraDevice defaultDevice = QMediaDevices::defaultVideoInput();

    for (const QCameraDevice& device : devices) {
        CameraInfo info;
        info.id = device.id();
        info.description = device.description();
        info.isDefault = (device == defaultDevice);

        // Populate available formats
        for (const QCameraFormat& format : device.videoFormats()) {
            CameraFormat fmt;
            fmt.width = format.resolution().width();
            fmt.height = format.resolution().height();
            fmt.minFrameRate = format.minFrameRate();
            fmt.maxFrameRate = format.maxFrameRate();
            info.formats.append(fmt);
        }

        m_cameras.append(info);
        qInfo() << "  Found camera:" << info.description
                << "(" << info.formats.size() << "formats )";
    }

    emit availableCamerasChanged();

    // Auto-select default camera if none selected
    if (m_currentCameraIndex < 0 && !m_cameras.isEmpty()) {
        for (int i = 0; i < m_cameras.size(); ++i) {
            if (m_cameras[i].isDefault) {
                setCurrentCameraIndex(i);
                return;
            }
        }
        // No default found, select first
        setCurrentCameraIndex(0);
    }
}

QVariantList CameraManager::availableCameras() const
{
    QVariantList result;
    for (const CameraInfo& info : m_cameras) {
        QVariantMap map;
        map["id"] = info.id;
        map["description"] = info.description;
        map["isDefault"] = info.isDefault;
        map["formatCount"] = info.formats.size();
        result.append(map);
    }
    return result;
}

void CameraManager::setCurrentCameraIndex(int index)
{
    if (index == m_currentCameraIndex) {
        return;
    }

    if (index < -1 || index >= m_cameras.size()) {
        qWarning() << "CameraManager: Invalid camera index:" << index;
        return;
    }

    bool wasCapturing = m_isCapturing;
    bool wasPreviewActive = m_isPreviewActive;

    // Stop current capture/preview
    if (m_isCapturing) {
        stopCapture();
    }
    if (m_isPreviewActive) {
        stopPreview();
    }

    m_currentCameraIndex = index;
    m_currentFormatIndex = -1;

    qInfo() << "CameraManager: Selected camera index:" << index;

    if (index >= 0) {
        // Setup new camera
        const QList<QCameraDevice> devices = QMediaDevices::videoInputs();
        for (const QCameraDevice& device : devices) {
            if (device.id() == m_cameras[index].id) {
                setupCamera(device);
                break;
            }
        }

        populateFormatsForCurrentCamera();

        // Auto-select best format (prefer 1080p @ 30fps, fallback to highest resolution)
        if (!m_cameras[index].formats.isEmpty()) {
            int bestIndex = 0;
            int bestScore = 0;

            for (int i = 0; i < m_cameras[index].formats.size(); ++i) {
                const CameraFormat& fmt = m_cameras[index].formats[i];
                int score = fmt.width * fmt.height;

                // Prefer 30fps
                if (fmt.maxFrameRate >= 30 && fmt.maxFrameRate <= 60) {
                    score += 1000000;
                }

                // Prefer 1080p
                if (fmt.height == 1080) {
                    score += 500000;
                }

                if (score > bestScore) {
                    bestScore = score;
                    bestIndex = i;
                }
            }

            setCurrentFormatIndex(bestIndex);
        }
    }

    emit currentCameraIndexChanged();
    emit availableFormatsChanged();

    // Restore state
    if (wasPreviewActive && index >= 0) {
        startPreview();
    }
    if (wasCapturing && index >= 0) {
        startCapture();
    }
}

QString CameraManager::currentCameraName() const
{
    if (m_currentCameraIndex >= 0 && m_currentCameraIndex < m_cameras.size()) {
        return m_cameras[m_currentCameraIndex].description;
    }
    return QString();
}

QVariantList CameraManager::availableFormats() const
{
    QVariantList result;

    if (m_currentCameraIndex >= 0 && m_currentCameraIndex < m_cameras.size()) {
        const QList<CameraFormat>& formats = m_cameras[m_currentCameraIndex].formats;
        for (int i = 0; i < formats.size(); ++i) {
            QVariantMap map;
            map["index"] = i;
            map["width"] = formats[i].width;
            map["height"] = formats[i].height;
            map["minFps"] = formats[i].minFrameRate;
            map["maxFps"] = formats[i].maxFrameRate;
            map["displayString"] = formats[i].toString();
            map["resolution"] = formats[i].resolutionString();
            result.append(map);
        }
    }

    return result;
}

void CameraManager::setCurrentFormatIndex(int index)
{
    if (index == m_currentFormatIndex) {
        return;
    }

    if (m_currentCameraIndex < 0 || m_currentCameraIndex >= m_cameras.size()) {
        return;
    }

    const QList<CameraFormat>& formats = m_cameras[m_currentCameraIndex].formats;
    if (index < -1 || index >= formats.size()) {
        qWarning() << "CameraManager: Invalid format index:" << index;
        return;
    }

    m_currentFormatIndex = index;
    qInfo() << "CameraManager: Selected format index:" << index;

    // Apply format to camera
    if (m_camera && index >= 0) {
        const CameraFormat& fmt = formats[index];

        const QList<QCameraDevice> devices = QMediaDevices::videoInputs();
        for (const QCameraDevice& device : devices) {
            if (device.id() == m_cameras[m_currentCameraIndex].id) {
                for (const QCameraFormat& qfmt : device.videoFormats()) {
                    if (qfmt.resolution().width() == fmt.width &&
                        qfmt.resolution().height() == fmt.height &&
                        qFuzzyCompare(static_cast<float>(qfmt.maxFrameRate()), static_cast<float>(fmt.maxFrameRate))) {
                        m_camera->setCameraFormat(qfmt);
                        qInfo() << "CameraManager: Applied format:" << fmt.toString();
                        break;
                    }
                }
                break;
            }
        }
    }

    emit currentFormatIndexChanged();
}

QString CameraManager::currentFormatString() const
{
    if (m_currentCameraIndex >= 0 && m_currentCameraIndex < m_cameras.size() &&
        m_currentFormatIndex >= 0 && m_currentFormatIndex < m_cameras[m_currentCameraIndex].formats.size()) {
        return m_cameras[m_currentCameraIndex].formats[m_currentFormatIndex].toString();
    }
    return QString();
}

void CameraManager::setupCamera(const QCameraDevice& device)
{
    cleanupCamera();

    m_camera = new QCamera(device, this);

    connect(m_camera, &QCamera::errorOccurred,
            this, &CameraManager::onCameraErrorOccurred);
    connect(m_camera, &QCamera::activeChanged,
            this, &CameraManager::onCameraActiveChanged);

    m_captureSession->setCamera(m_camera);

    qInfo() << "CameraManager: Camera setup complete for:" << device.description();
}

void CameraManager::cleanupCamera()
{
    if (m_camera) {
        m_camera->stop();
        m_captureSession->setCamera(nullptr);
        delete m_camera;
        m_camera = nullptr;
    }
}

void CameraManager::populateFormatsForCurrentCamera()
{
    // Formats are already populated in refreshCameraList()
    emit availableFormatsChanged();
}

void CameraManager::startCapture()
{
    if (m_isCapturing) {
        qWarning() << "CameraManager: Already capturing";
        return;
    }

    if (!m_camera) {
        qWarning() << "CameraManager: No camera selected";
        emit errorOccurred("No camera selected");
        return;
    }

    qInfo() << "CameraManager: Starting capture...";

    // If we were in preview mode, keep the external sink for display
    if (m_isPreviewActive) {
        qInfo() << "CameraManager: Transitioning from preview to capture (keeping display sink)";
        m_isPreviewActive = false;
        emit isPreviewActiveChanged();
    }

    // Keep using external sink if available (for QML VideoOutput display)
    // The internal sink's videoFrameChanged callback still tracks timestamps
    // because we connect to whichever sink the capture session uses
    if (m_externalSink) {
        qInfo() << "CameraManager: Using external sink for display during capture";
        // Ensure capture session uses external sink for display
        m_captureSession->setVideoSink(m_externalSink.data());

        // Connect to external sink for frame callbacks (if not already connected)
        disconnect(m_externalSink, &QVideoSink::videoFrameChanged,
                   this, &CameraManager::onVideoFrameChanged);
        connect(m_externalSink, &QVideoSink::videoFrameChanged,
                this, &CameraManager::onVideoFrameChanged);
    } else {
        // No external sink - use internal sink
        m_captureSession->setVideoSink(m_videoSink.data());
    }

    m_frameCount = 0;
    m_framesAtLastUpdate = 0;
    m_lastFpsUpdateTime = QDateTime::currentMSecsSinceEpoch();

    // Only start camera if not already running
    if (!m_camera->isActive()) {
        m_camera->start();
    }

    m_isCapturing = true;
    m_fpsTimer->start();

    emit isCapturingChanged();
}

void CameraManager::stopCapture()
{
    if (!m_isCapturing) {
        return;
    }

    qInfo() << "CameraManager: Stopping capture...";

    m_fpsTimer->stop();

    if (m_camera) {
        m_camera->stop();
    }

    m_isCapturing = false;
    m_currentFps = 0.0;

    emit isCapturingChanged();
    emit fpsUpdated();
}

void CameraManager::startPreview()
{
    if (m_isPreviewActive) {
        return;
    }

    if (!m_camera) {
        qWarning() << "CameraManager: No camera selected for preview";
        return;
    }

    qInfo() << "CameraManager: Starting preview...";

    m_camera->start();
    m_isPreviewActive = true;

    emit isPreviewActiveChanged();
}

void CameraManager::stopPreview()
{
    if (!m_isPreviewActive) {
        return;
    }

    qInfo() << "CameraManager: Stopping preview...";

    // Clear external sink
    if (m_externalSink) {
        m_externalSink = nullptr;
        // Restore internal sink
        if (m_captureSession) {
            m_captureSession->setVideoSink(m_videoSink.data());
        }
    }

    // Only stop camera if not capturing
    if (!m_isCapturing && m_camera) {
        m_camera->stop();
    }

    m_isPreviewActive = false;

    emit isPreviewActiveChanged();
}

void CameraManager::setExternalVideoSink(QVideoSink* sink)
{
    // Disconnect from previous external sink if any
    if (m_externalSink) {
        disconnect(m_externalSink, &QVideoSink::videoFrameChanged,
                   this, &CameraManager::onVideoFrameChanged);
    }

    m_externalSink = sink;

    if (m_captureSession) {
        // Set external sink for display in QML
        m_captureSession->setVideoSink(sink ? sink : m_videoSink.data());
    }

    // If capturing with external sink, connect frame callback
    if (sink && m_isCapturing) {
        connect(sink, &QVideoSink::videoFrameChanged,
                this, &CameraManager::onVideoFrameChanged);
    }
}

void CameraManager::onVideoFrameChanged(const QVideoFrame& frame)
{
    if (!frame.isValid()) {
        return;
    }

    // Get LSL timestamp immediately for best synchronization
    double lslTimestamp = lsl::local_clock();

    m_frameCount++;

    // Only process for capture if actively capturing (not just preview)
    if (m_isCapturing) {
        // Track timestamp without heavy QImage conversion
        // VideoOutput displays directly from camera - we just record LSL timestamps
        m_lastFrameTimestamp = lslTimestamp;

        // Notify QML of new timestamp for display
        emit frameTimestampUpdated();

        // Emit frame packet with just timestamp info (skip QImage conversion)
        // Full frame conversion would be done only when saving/recording
        emit frameReady(VideoFramePacket(QImage(), lslTimestamp, m_frameCount));
    }
}

QImage CameraManager::videoFrameToImage(const QVideoFrame& frame)
{
    QVideoFrame frameCopy = frame;

    if (!frameCopy.map(QVideoFrame::ReadOnly)) {
        qWarning() << "CameraManager: Failed to map video frame";
        return QImage();
    }

    QImage image = frameCopy.toImage();
    frameCopy.unmap();

    if (image.isNull()) {
        qWarning() << "CameraManager: Failed to convert frame to image";
        return QImage();
    }

    // Convert to RGB32 for consistent format
    if (image.format() != QImage::Format_RGB32 &&
        image.format() != QImage::Format_ARGB32) {
        image = image.convertToFormat(QImage::Format_RGB32);
    }

    return image;
}

void CameraManager::onCameraErrorOccurred(QCamera::Error error, const QString& errorString)
{
    qWarning() << "CameraManager: Camera error:" << error << errorString;
    emit errorOccurred(errorString);
}

void CameraManager::onCameraActiveChanged(bool active)
{
    qInfo() << "CameraManager: Camera active changed:" << active;

    if (!active && m_isCapturing) {
        // Camera became inactive unexpectedly
        m_isCapturing = false;
        emit isCapturingChanged();
    }
}

void CameraManager::updateFpsCounter()
{
    qint64 now = QDateTime::currentMSecsSinceEpoch();
    qint64 elapsed = now - m_lastFpsUpdateTime;

    if (elapsed > 0) {
        qint64 framesDelta = m_frameCount - m_framesAtLastUpdate;
        m_currentFps = (framesDelta * 1000.0) / elapsed;
    }

    m_lastFpsUpdateTime = now;
    m_framesAtLastUpdate = m_frameCount;

    emit fpsUpdated();
}
