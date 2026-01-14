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

signals:
    void channelCountChanged();
    void minMaxChanged();
    void writePositionChanged();

private:
    void initializeBuffer(int numChannels);
    void emitDataChanged(int startRow, int endRow);
    void updateMinMaxCache(double value);

    // Data storage
    QVector<QVector<double>> m_data;
    double m_channelSpacing = 100.0;
    int m_currentIndex = 0;
    int m_totalSamples = 0;
    int m_writePosition = 0;

    // Constants
    static constexpr int MAX_SAMPLES = 1000;
    static constexpr int GAP_SIZE = 50;
    static constexpr double GAP_VALUE = 1e9;

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
