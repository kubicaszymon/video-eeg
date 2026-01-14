#ifndef EEGDATAMODEL_H
#define EEGDATAMODEL_H

#include <QAbstractTableModel>
#include <QPointF>
#include <QVector>
#include <QTimer>
#include <QElapsedTimer>
#include <QtQmlIntegration>
#include <limits>

class EegDataModel : public QAbstractTableModel
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(double minValue READ minValue NOTIFY minMaxChanged)
    Q_PROPERTY(double maxValue READ maxValue NOTIFY minMaxChanged)
    Q_PROPERTY(int writePosition READ writePosition NOTIFY writePositionChanged)

    // Sampling rate from LSL stream (samples per second)
    Q_PROPERTY(double samplingRate READ samplingRate WRITE setSamplingRate NOTIFY samplingRateChanged)

    // Time window in seconds (how many seconds of data to display)
    Q_PROPERTY(double timeWindowSeconds READ timeWindowSeconds WRITE setTimeWindowSeconds NOTIFY timeWindowSecondsChanged)

    // Calculated max samples (samplingRate * timeWindowSeconds) - read only
    Q_PROPERTY(int maxSamples READ maxSamples NOTIFY maxSamplesChanged)

public:
    EegDataModel();

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    Q_INVOKABLE void updateAllData(const QVector<QVector<double>>& incomingData);

    int channelCount() const;
    void setChannelCount(int newChannelCount);

    double minValue() const;
    double maxValue() const;
    int writePosition() const;

    double samplingRate() const;
    void setSamplingRate(double newSamplingRate);

    double timeWindowSeconds() const;
    void setTimeWindowSeconds(double newTimeWindowSeconds);

    int maxSamples() const;

signals:
    void channelCountChanged();
    void minMaxChanged();
    void writePositionChanged();
    void samplingRateChanged();
    void timeWindowSecondsChanged();
    void maxSamplesChanged();

private:
    void initializeBuffer(int numChannels);
    void emitDataChanged(int startRow, int endRow);
    void updateMinMaxCache(double value);
    void recalculateMaxSamples();

    // Data storage
    QVector<QVector<double>> m_data;
    double m_channelSpacing = 100.0;
    int m_currentIndex = 0;
    int m_totalSamples = 0;
    int m_writePosition = 0;

    // Sampling configuration
    double m_samplingRate = 256.0;       // Default fallback sampling rate (Hz)
    double m_timeWindowSeconds = 10.0;   // Default 10 second window
    int m_maxSamples = 2560;             // samplingRate * timeWindowSeconds

    // Constants
    static constexpr int GAP_SIZE = 50;
    static constexpr double GAP_VALUE = 1e9;
    static constexpr int DEFAULT_MAX_SAMPLES = 2560;  // Fallback before sampling rate is known

    // Cached min/max values
    double m_cachedMin = std::numeric_limits<double>::infinity();
    double m_cachedMax = -std::numeric_limits<double>::infinity();
    bool m_minMaxDirty = true;

    // Rate limiting for UI updates
    QElapsedTimer m_updateTimer;
    static constexpr int MIN_UPDATE_INTERVAL_MS = 16; // ~60 FPS max
    bool m_pendingUpdate = false;
    int m_pendingStartRow = 0;
    int m_pendingEndRow = 0;

    // Buffer initialization flag
    bool m_bufferInitialized = false;
    int m_numChannels = 0;
};

#endif // EEGDATAMODEL_H
