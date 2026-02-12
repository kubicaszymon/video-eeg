#ifndef RECORDINGWORKER_H
#define RECORDINGWORKER_H

#include <QObject>
#include <QFile>
#include <QTextStream>
#include <QVector>
#include <QStringList>
#include "recordingsummary.h"

/**
 * @brief RecordingWorker - Background thread worker for disk I/O
 *
 * Runs on a dedicated QThread. All public slots are invoked via
 * Qt::QueuedConnection from RecordingManager on the main thread.
 *
 * Handles:
 * - EEG CSV writing with batched flush
 * - Markers CSV writing (immediate flush)
 * - Video frame index CSV writing
 * - File lifecycle (open, write headers, close, summary)
 */
class RecordingWorker : public QObject
{
    Q_OBJECT

public:
    explicit RecordingWorker(QObject* parent = nullptr);
    ~RecordingWorker();

public slots:
    /**
     * @brief Open all output files and write CSV headers
     * @param eegPath Path for EEG CSV file
     * @param markersPath Path for markers CSV file
     * @param framesPath Path for frame index CSV file
     * @param metadataPath Path for metadata JSON file
     * @param channelNames List of EEG channel names
     * @param sessionName Session identifier
     * @param samplingRate EEG sampling rate in Hz
     */
    void initializeFiles(const QString& eegPath,
                         const QString& markersPath,
                         const QString& framesPath,
                         const QString& metadataPath,
                         const QStringList& channelNames,
                         const QString& sessionName,
                         double samplingRate);

    /**
     * @brief Write a batch of EEG samples to CSV
     * @param samples Flattened: samples[sampleIdx] = vector of channel values
     * @param timestamps LSL timestamp per sample
     */
    void writeEegBatch(const QVector<QVector<float>>& samples,
                       const QVector<double>& timestamps);

    /**
     * @brief Write a PAUSE_START or PAUSE_STOP line in EEG and markers CSV
     */
    void writePauseMarker(const QString& type, double lslTimestamp, double sessionTimeSec);

    /**
     * @brief Write an event marker to markers CSV
     */
    void writeMarker(const QString& type, const QString& label,
                     double lslTimestamp, double sessionTimeSec);

    /**
     * @brief Write a video frame timestamp to frames CSV
     */
    void writeFrameTimestamp(double lslTimestamp, qint64 frameNumber,
                            const QString& segmentFile);

    /**
     * @brief Close all files, compute summary
     * @param durationSeconds Total recording duration
     * @param videoFileSizeBytes Total video file size
     */
    void closeFiles(double durationSeconds, qint64 videoFileSizeBytes);

signals:
    void filesInitialized(bool success, const QString& error);
    void batchWritten(int sampleCount, qint64 eegFileSize);
    void filesClosed(const RecordingSummary& summary);
    void errorOccurred(const QString& error);

private:
    void writeEegHeader(const QStringList& channelNames,
                        const QString& sessionName,
                        double samplingRate);
    void writeMarkersHeader();
    void writeFramesHeader();
    void writeMetadata(const QString& sessionName,
                       const QStringList& channelNames,
                       double samplingRate);

    QFile m_eegFile;
    QFile m_markersFile;
    QFile m_framesFile;
    QTextStream m_eegStream;
    QTextStream m_markersStream;
    QTextStream m_framesStream;

    QString m_sessionName;
    QString m_savePath;
    qint64 m_sampleCount = 0;
    qint64 m_markerCount = 0;
    qint64 m_frameCount = 0;
};

#endif // RECORDINGWORKER_H
