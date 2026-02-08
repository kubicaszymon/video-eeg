#ifndef VIDEOFRAMEPACKET_H
#define VIDEOFRAMEPACKET_H

#include <QImage>
#include <QObject>
#include <QtQml/qqmlregistration.h>

/**
 * @brief VideoFramePacket - Structure for video frame with LSL timestamp
 *
 * This structure holds a single video frame along with its LSL timestamp,
 * enabling precise synchronization with EEG data for multimodal data fusion.
 *
 * Usage:
 * - Each frame captured from the camera is wrapped in this structure
 * - The lslTimestamp is obtained via lsl::local_clock() at capture time
 * - This allows correlation with EEG samples that also have LSL timestamps
 */
struct VideoFramePacket
{
    Q_GADGET
    Q_PROPERTY(double lslTimestamp MEMBER lslTimestamp)
    Q_PROPERTY(qint64 frameNumber MEMBER frameNumber)

public:
    QImage frame;           ///< The captured video frame as QImage
    double lslTimestamp;    ///< LSL timestamp at frame capture (from lsl::local_clock())
    qint64 frameNumber;     ///< Sequential frame number for ordering

    VideoFramePacket() : lslTimestamp(0.0), frameNumber(0) {}

    VideoFramePacket(const QImage& img, double timestamp, qint64 number = 0)
        : frame(img), lslTimestamp(timestamp), frameNumber(number) {}

    bool isValid() const { return !frame.isNull() && lslTimestamp > 0.0; }
};

Q_DECLARE_METATYPE(VideoFramePacket)

/**
 * @brief CameraFormat - Structure representing a camera video format
 *
 * Stores resolution and frame rate information for camera configuration.
 */
struct CameraFormat
{
    Q_GADGET
    Q_PROPERTY(int width MEMBER width)
    Q_PROPERTY(int height MEMBER height)
    Q_PROPERTY(double minFrameRate MEMBER minFrameRate)
    Q_PROPERTY(double maxFrameRate MEMBER maxFrameRate)

public:
    int width = 0;
    int height = 0;
    double minFrameRate = 0.0;
    double maxFrameRate = 0.0;

    CameraFormat() = default;

    CameraFormat(int w, int h, double minFps, double maxFps)
        : width(w), height(h), minFrameRate(minFps), maxFrameRate(maxFps) {}

    QString toString() const {
        return QString("%1x%2 @ %3-%4 FPS")
            .arg(width).arg(height)
            .arg(minFrameRate, 0, 'f', 1)
            .arg(maxFrameRate, 0, 'f', 1);
    }

    QString resolutionString() const {
        return QString("%1x%2").arg(width).arg(height);
    }

    bool operator==(const CameraFormat& other) const {
        return width == other.width &&
               height == other.height &&
               qFuzzyCompare(minFrameRate, other.minFrameRate) &&
               qFuzzyCompare(maxFrameRate, other.maxFrameRate);
    }
};

Q_DECLARE_METATYPE(CameraFormat)

/**
 * @brief CameraInfo - Structure representing a camera device
 *
 * Contains device identification and available formats.
 */
struct CameraInfo
{
    Q_GADGET
    Q_PROPERTY(QString id MEMBER id)
    Q_PROPERTY(QString description MEMBER description)
    Q_PROPERTY(bool isDefault MEMBER isDefault)

public:
    QString id;                         ///< Unique device identifier
    QString description;                ///< Human-readable device name
    bool isDefault = false;             ///< Whether this is the system default camera
    QList<CameraFormat> formats;        ///< Available video formats

    CameraInfo() = default;

    CameraInfo(const QString& deviceId, const QString& desc, bool defaultCam = false)
        : id(deviceId), description(desc), isDefault(defaultCam) {}
};

Q_DECLARE_METATYPE(CameraInfo)

#endif // VIDEOFRAMEPACKET_H
