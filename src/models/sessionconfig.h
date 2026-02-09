#ifndef SESSIONCONFIG_H
#define SESSIONCONFIG_H

#include <QString>
#include <QStringList>
#include <QVector>
#include <QDir>

/**
 * @brief SessionConfig - Configuration for a recording session
 *
 * Transferred from AmplifierSetupWindow to EegWindow,
 * contains all info needed to start recording.
 */
struct SessionConfig
{
    QString saveFolderPath;     // Base folder chosen by user
    QString sessionName;        // Auto-generated: REC_YYYYMMDD_HHMMSS
    QString amplifierId;
    QVector<int> channels;
    QStringList channelNames;
    QString cameraId;
    double samplingRate = 0.0;

    QString eegFilePath() const {
        return QDir(saveFolderPath).filePath(sessionName + "_eeg.csv");
    }

    QString videoFilePath() const {
        return QDir(saveFolderPath).filePath(sessionName + "_video.mkv");
    }

    QString videoSegmentFilePath(int segment) const {
        if (segment <= 1)
            return videoFilePath();
        return QDir(saveFolderPath).filePath(
            sessionName + QString("_video_seg%1.mkv").arg(segment, 3, 10, QChar('0')));
    }

    QString markersFilePath() const {
        return QDir(saveFolderPath).filePath(sessionName + "_markers.csv");
    }

    QString framesFilePath() const {
        return QDir(saveFolderPath).filePath(sessionName + "_frames.csv");
    }

    QString metadataFilePath() const {
        return QDir(saveFolderPath).filePath(sessionName + "_metadata.json");
    }

    bool isValid() const {
        return !saveFolderPath.isEmpty() &&
               !sessionName.isEmpty() &&
               !channels.isEmpty();
    }
};

#endif // SESSIONCONFIG_H
