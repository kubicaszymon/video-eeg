#ifndef RECORDINGSUMMARY_H
#define RECORDINGSUMMARY_H

#include <QString>
#include <QObject>

/**
 * @brief RecordingSummary - Data shown in the summary dialog after recording stops
 */
struct RecordingSummary
{
    Q_GADGET

    Q_PROPERTY(QString sessionName MEMBER sessionName)
    Q_PROPERTY(QString savePath MEMBER savePath)
    Q_PROPERTY(double durationSeconds MEMBER durationSeconds)
    Q_PROPERTY(qint64 eegSamples MEMBER eegSamples)
    Q_PROPERTY(qint64 videoFrames MEMBER videoFrames)
    Q_PROPERTY(int markerCount MEMBER markerCount)
    Q_PROPERTY(qint64 eegFileSizeBytes MEMBER eegFileSizeBytes)
    Q_PROPERTY(qint64 videoFileSizeBytes MEMBER videoFileSizeBytes)
    Q_PROPERTY(QString startTime MEMBER startTime)
    Q_PROPERTY(QString endTime MEMBER endTime)

public:
    QString sessionName;
    QString savePath;
    double durationSeconds = 0.0;
    qint64 eegSamples = 0;
    qint64 videoFrames = 0;
    int markerCount = 0;
    qint64 eegFileSizeBytes = 0;
    qint64 videoFileSizeBytes = 0;
    QString startTime;
    QString endTime;

    QString durationFormatted() const {
        int totalSec = static_cast<int>(durationSeconds);
        int h = totalSec / 3600;
        int m = (totalSec % 3600) / 60;
        int s = totalSec % 60;
        return QString("%1:%2:%3")
            .arg(h, 2, 10, QChar('0'))
            .arg(m, 2, 10, QChar('0'))
            .arg(s, 2, 10, QChar('0'));
    }

    QString eegSizeFormatted() const {
        return formatBytes(eegFileSizeBytes);
    }

    QString videoSizeFormatted() const {
        return formatBytes(videoFileSizeBytes);
    }

private:
    static QString formatBytes(qint64 bytes) {
        if (bytes < 1024)
            return QString::number(bytes) + " B";
        if (bytes < 1024 * 1024)
            return QString::number(bytes / 1024.0, 'f', 1) + " KB";
        if (bytes < 1024LL * 1024 * 1024)
            return QString::number(bytes / (1024.0 * 1024.0), 'f', 1) + " MB";
        return QString::number(bytes / (1024.0 * 1024.0 * 1024.0), 'f', 2) + " GB";
    }
};

Q_DECLARE_METATYPE(RecordingSummary)

#endif // RECORDINGSUMMARY_H
