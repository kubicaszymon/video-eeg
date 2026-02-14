/*
 * ==========================================================================
 *  cameramanager.cpp — Camera Device Manager Implementation
 * ==========================================================================
 *  See cameramanager.h for architecture overview, dual-sink model,
 *  LSL timestamping strategy, and format auto-selection heuristic.
 * ==========================================================================
 */

#include "cameramanager.h"

#include <QDebug>
#include <QTimer>
#include <QDateTime>
#include <QVideoFrame>

CameraManager* CameraManager::s_instance = nullptr;

CameraManager* CameraManager::instance()
{
    if (!s_instance)
        s_instance = new CameraManager();
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

    // Internal sink: receives frames when no external (QML) sink is attached.
    // The connection is permanent; onVideoFrameChanged() checks m_isCapturing
    // before acting so it is a no-op during preview-only mode.
    m_videoSink = new QVideoSink(this);
    connect(m_videoSink, &QVideoSink::videoFrameChanged,
            this, &CameraManager::onVideoFrameChanged);

    // The capture session acts as the switchboard: it holds the QCamera
    // source and routes decoded frames to whichever QVideoSink is active.
    m_captureSession = new QMediaCaptureSession(this);
    m_captureSession->setVideoSink(m_videoSink);

    m_fpsTimer = new QTimer(this);
    m_fpsTimer->setInterval(1000);
    connect(m_fpsTimer, &QTimer::timeout, this, &CameraManager::updateFpsCounter);

    refreshCameraList();
}

CameraManager::~CameraManager()
{
    qInfo() << "CameraManager destroyed";
    stopCapture();
    cleanupCamera();

    if (s_instance == this)
        s_instance = nullptr;
}

// ============================================================================
// Camera Enumeration
// ============================================================================

void CameraManager::refreshCameraList()
{
    qInfo() << "CameraManager: Refreshing camera list...";

    m_cameras.clear();

    const QList<QCameraDevice> devices = QMediaDevices::videoInputs();
    const QCameraDevice defaultDevice  = QMediaDevices::defaultVideoInput();

    for (const QCameraDevice& device : devices)
    {
        CameraInfo info;
        info.id          = device.id();
        info.description = device.description();
        info.isDefault   = (device == defaultDevice);

        for (const QCameraFormat& format : device.videoFormats())
        {
            CameraFormat fmt;
            fmt.width        = format.resolution().width();
            fmt.height       = format.resolution().height();
            fmt.minFrameRate = format.minFrameRate();
            fmt.maxFrameRate = format.maxFrameRate();
            info.formats.append(fmt);
        }

        m_cameras.append(info);
        qInfo() << "  Found camera:" << info.description
                << "(" << info.formats.size() << "formats)";
    }

    emit availableCamerasChanged();

    // Auto-select the system default camera on first enumeration
    if (m_currentCameraIndex < 0 && !m_cameras.isEmpty())
    {
        for (int i = 0; i < m_cameras.size(); ++i)
        {
            if (m_cameras[i].isDefault)
            {
                setCurrentCameraIndex(i);
                return;
            }
        }
        setCurrentCameraIndex(0); // No default found — use first
    }
}

QVariantList CameraManager::availableCameras() const
{
    QVariantList result;
    for (const CameraInfo& info : m_cameras)
    {
        QVariantMap map;
        map["id"]          = info.id;
        map["description"] = info.description;
        map["isDefault"]   = info.isDefault;
        map["formatCount"] = info.formats.size();
        result.append(map);
    }
    return result;
}

// ============================================================================
// Camera Selection
// ============================================================================

void CameraManager::setCurrentCameraIndex(int index)
{
    if (index == m_currentCameraIndex)
        return;

    if (index < -1 || index >= m_cameras.size())
    {
        qWarning() << "CameraManager: Invalid camera index:" << index;
        return;
    }

    // Preserve operational state so we can restore it after switching hardware
    bool wasCapturing    = m_isCapturing;
    bool wasPreviewActive = m_isPreviewActive;

    if (m_isCapturing)    stopCapture();
    if (m_isPreviewActive) stopPreview();

    m_currentCameraIndex = index;
    m_currentFormatIndex = -1;

    qInfo() << "CameraManager: Selected camera index:" << index;

    if (index >= 0)
    {
        const QList<QCameraDevice> devices = QMediaDevices::videoInputs();
        for (const QCameraDevice& device : devices)
        {
            if (device.id() == m_cameras[index].id)
            {
                setupCamera(device);
                break;
            }
        }

        populateFormatsForCurrentCamera();

        // Auto-select best format using the scoring heuristic (see header):
        // prefer 1080p at 30-60 fps; fall back to highest resolution.
        if (!m_cameras[index].formats.isEmpty())
        {
            int bestIndex = 0;
            int bestScore = 0;

            for (int i = 0; i < m_cameras[index].formats.size(); ++i)
            {
                const CameraFormat& fmt = m_cameras[index].formats[i];
                int score = fmt.width * fmt.height;

                if (fmt.maxFrameRate >= 30 && fmt.maxFrameRate <= 60)
                    score += 1000000;

                if (fmt.height == 1080)
                    score += 500000;

                if (score > bestScore)
                {
                    bestScore = score;
                    bestIndex = i;
                }
            }

            setCurrentFormatIndex(bestIndex);
        }
    }

    emit currentCameraIndexChanged();
    emit availableFormatsChanged();

    // Restore previous operational state with the new camera
    if (wasPreviewActive && index >= 0) startPreview();
    if (wasCapturing     && index >= 0) startCapture();
}

QString CameraManager::currentCameraName() const
{
    if (m_currentCameraIndex >= 0 && m_currentCameraIndex < m_cameras.size())
        return m_cameras[m_currentCameraIndex].description;
    return QString();
}

// ============================================================================
// Format Selection
// ============================================================================

QVariantList CameraManager::availableFormats() const
{
    QVariantList result;

    if (m_currentCameraIndex >= 0 && m_currentCameraIndex < m_cameras.size())
    {
        const QList<CameraFormat>& formats = m_cameras[m_currentCameraIndex].formats;
        for (int i = 0; i < formats.size(); ++i)
        {
            QVariantMap map;
            map["index"]         = i;
            map["width"]         = formats[i].width;
            map["height"]        = formats[i].height;
            map["minFps"]        = formats[i].minFrameRate;
            map["maxFps"]        = formats[i].maxFrameRate;
            map["displayString"] = formats[i].toString();
            map["resolution"]    = formats[i].resolutionString();
            result.append(map);
        }
    }

    return result;
}

void CameraManager::setCurrentFormatIndex(int index)
{
    if (index == m_currentFormatIndex)
        return;

    if (m_currentCameraIndex < 0 || m_currentCameraIndex >= m_cameras.size())
        return;

    const QList<CameraFormat>& formats = m_cameras[m_currentCameraIndex].formats;
    if (index < -1 || index >= formats.size())
    {
        qWarning() << "CameraManager: Invalid format index:" << index;
        return;
    }

    m_currentFormatIndex = index;
    qInfo() << "CameraManager: Selected format index:" << index;

    // Locate the matching QCameraFormat by resolution and FPS, then apply it.
    // We re-query QMediaDevices to get the live QCameraFormat object because
    // our CameraFormat struct is a value copy that does not hold a Qt handle.
    if (m_camera && index >= 0)
    {
        const CameraFormat& fmt = formats[index];
        const QList<QCameraDevice> devices = QMediaDevices::videoInputs();

        for (const QCameraDevice& device : devices)
        {
            if (device.id() != m_cameras[m_currentCameraIndex].id)
                continue;

            for (const QCameraFormat& qfmt : device.videoFormats())
            {
                if (qfmt.resolution().width()  == fmt.width  &&
                    qfmt.resolution().height() == fmt.height &&
                    qFuzzyCompare(static_cast<float>(qfmt.maxFrameRate()),
                                  static_cast<float>(fmt.maxFrameRate)))
                {
                    m_camera->setCameraFormat(qfmt);
                    qInfo() << "CameraManager: Applied format:" << fmt.toString();
                    break;
                }
            }
            break;
        }
    }

    emit currentFormatIndexChanged();
}

QString CameraManager::currentFormatString() const
{
    if (m_currentCameraIndex >= 0 && m_currentCameraIndex < m_cameras.size() &&
        m_currentFormatIndex  >= 0 && m_currentFormatIndex  < m_cameras[m_currentCameraIndex].formats.size())
    {
        return m_cameras[m_currentCameraIndex].formats[m_currentFormatIndex].toString();
    }
    return QString();
}

// ============================================================================
// Camera Lifecycle (private helpers)
// ============================================================================

void CameraManager::setupCamera(const QCameraDevice& device)
{
    cleanupCamera();

    m_camera = new QCamera(device, this);
    connect(m_camera, &QCamera::errorOccurred,  this, &CameraManager::onCameraErrorOccurred);
    connect(m_camera, &QCamera::activeChanged,  this, &CameraManager::onCameraActiveChanged);

    m_captureSession->setCamera(m_camera);

    qInfo() << "CameraManager: Camera setup complete for:" << device.description();
}

void CameraManager::cleanupCamera()
{
    if (m_camera)
    {
        m_camera->stop();
        m_captureSession->setCamera(nullptr);
        delete m_camera;
        m_camera = nullptr;
    }
}

void CameraManager::populateFormatsForCurrentCamera()
{
    // Formats are already populated in refreshCameraList().
    // Emitting the signal is enough to refresh any bound QML ComboBox.
    emit availableFormatsChanged();
}

// ============================================================================
// Capture Control
// ============================================================================

void CameraManager::startCapture()
{
    if (m_isCapturing)
    {
        qWarning() << "CameraManager: Already capturing";
        return;
    }

    if (!m_camera)
    {
        qWarning() << "CameraManager: No camera selected";
        emit errorOccurred("No camera selected");
        return;
    }

    qInfo() << "CameraManager: Starting capture...";

    // Transition cleanly from preview mode (camera may already be running)
    if (m_isPreviewActive)
    {
        m_isPreviewActive = false;
        emit isPreviewActiveChanged();
    }

    // Route frames through the external sink if one is registered (QML VideoOutput).
    // This is the preferred path: the hardware decoder feeds one sink that serves
    // both display and the timestamp callback — no double decode needed.
    if (m_externalSink)
    {
        m_captureSession->setVideoSink(m_externalSink.data());

        // Re-connect the callback to the external sink (disconnect first to
        // avoid double-firing if setExternalVideoSink() was called earlier).
        disconnect(m_externalSink, &QVideoSink::videoFrameChanged,
                   this, &CameraManager::onVideoFrameChanged);
        connect(m_externalSink, &QVideoSink::videoFrameChanged,
                this, &CameraManager::onVideoFrameChanged);
    }
    else
    {
        m_captureSession->setVideoSink(m_videoSink.data());
    }

    m_frameCount         = 0;
    m_framesAtLastUpdate = 0;
    m_lastFpsUpdateTime  = QDateTime::currentMSecsSinceEpoch();

    if (!m_camera->isActive())
        m_camera->start();

    m_isCapturing = true;
    m_fpsTimer->start();

    emit isCapturingChanged();
}

void CameraManager::stopCapture()
{
    if (!m_isCapturing)
        return;

    qInfo() << "CameraManager: Stopping capture...";

    m_fpsTimer->stop();

    if (m_camera)
        m_camera->stop();

    m_isCapturing = false;
    m_currentFps  = 0.0;

    emit isCapturingChanged();
    emit fpsUpdated();
}

// ============================================================================
// Preview Control
// ============================================================================

void CameraManager::startPreview()
{
    if (m_isPreviewActive)
        return;

    if (!m_camera)
    {
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
    if (!m_isPreviewActive)
        return;

    qInfo() << "CameraManager: Stopping preview...";

    if (m_externalSink)
    {
        m_externalSink = nullptr;
        if (m_captureSession)
            m_captureSession->setVideoSink(m_videoSink.data());
    }

    if (!m_isCapturing && m_camera)
        m_camera->stop();

    m_isPreviewActive = false;

    emit isPreviewActiveChanged();
}

// ============================================================================
// Video Sink Management
// ============================================================================

void CameraManager::setExternalVideoSink(QVideoSink* sink)
{
    if (m_externalSink)
    {
        disconnect(m_externalSink, &QVideoSink::videoFrameChanged,
                   this, &CameraManager::onVideoFrameChanged);
    }

    m_externalSink = sink;

    if (m_captureSession)
        m_captureSession->setVideoSink(sink ? sink : m_videoSink.data());

    // In capture mode, also hook the frame callback to the new external sink
    if (sink && m_isCapturing)
    {
        connect(sink, &QVideoSink::videoFrameChanged,
                this, &CameraManager::onVideoFrameChanged);
    }
}

// ============================================================================
// Frame Hot Path
// ============================================================================

void CameraManager::onVideoFrameChanged(const QVideoFrame& frame)
{
    if (!frame.isValid())
        return;

    // Stamp the LSL timestamp as early as possible — before any processing.
    // This minimizes the jitter between the physical moment of capture and
    // the recorded timestamp. Any delay after this line adds to sync error.
    //
    // This timestamp flows through the entire synchronization pipeline:
    //   1. VideoBackend stores it in the frame ring buffer
    //   2. VideoDisplayWindow.qml receives it via frameReceived(lslTimestamp)
    //   3. QML calls EegSyncManager.getEEGForFrame(lslTimestamp)
    //   4. EegSyncManager applies time_correction(), searches EEG buffer,
    //      validates timestamp range, and returns matched EEG data + offsetMs
    //   5. QML updates the sync health overlay with the result
    //
    // The timestamp is also forwarded to RecordingManager for the frames CSV,
    // enabling post-hoc synchronization in offline analysis tools.
    double lslTimestamp = lsl::local_clock();

    m_frameCount++;

    if (m_isCapturing)
    {
        m_lastFrameTimestamp = lslTimestamp;
        emit frameTimestampUpdated();

        // Emit a lightweight packet (no QImage conversion).
        // The QML VideoOutput already displays the frame via the capture
        // session; we only need the timestamp for EEG sync and recording.
        emit frameReady(VideoFramePacket(QImage(), lslTimestamp, m_frameCount));
    }
}

QImage CameraManager::videoFrameToImage(const QVideoFrame& frame)
{
    // Used only for still capture or when pixel data is explicitly needed.
    // Not called on the normal frame hot path.
    QVideoFrame frameCopy = frame;

    if (!frameCopy.map(QVideoFrame::ReadOnly))
    {
        qWarning() << "CameraManager: Failed to map video frame";
        return QImage();
    }

    QImage image = frameCopy.toImage();
    frameCopy.unmap();

    if (image.isNull())
    {
        qWarning() << "CameraManager: Failed to convert frame to image";
        return QImage();
    }

    if (image.format() != QImage::Format_RGB32 &&
        image.format() != QImage::Format_ARGB32)
    {
        image = image.convertToFormat(QImage::Format_RGB32);
    }

    return image;
}

// ============================================================================
// Event Handlers
// ============================================================================

void CameraManager::onCameraErrorOccurred(QCamera::Error error, const QString& errorString)
{
    qWarning() << "CameraManager: Camera error:" << error << errorString;
    emit errorOccurred(errorString);
}

void CameraManager::onCameraActiveChanged(bool active)
{
    qInfo() << "CameraManager: Camera active changed:" << active;

    if (!active && m_isCapturing)
    {
        // Camera became inactive unexpectedly — propagate the state change
        m_isCapturing = false;
        emit isCapturingChanged();
    }
}

void CameraManager::updateFpsCounter()
{
    qint64 now     = QDateTime::currentMSecsSinceEpoch();
    qint64 elapsed = now - m_lastFpsUpdateTime;

    if (elapsed > 0)
    {
        qint64 framesDelta = m_frameCount - m_framesAtLastUpdate;
        m_currentFps = (framesDelta * 1000.0) / elapsed;
    }

    m_lastFpsUpdateTime  = now;
    m_framesAtLastUpdate = m_frameCount;

    emit fpsUpdated();
}
