#ifndef EEGDATAMODEL_H
#define EEGDATAMODEL_H

#include <QAbstractTableModel>
#include <QPointF>
#include <QVector>
#include <QtQmlIntegration>

class EegDataModel : public QAbstractTableModel
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(double minValue READ minValue NOTIFY dataChanged)
    Q_PROPERTY(double maxValue READ maxValue NOTIFY dataChanged)

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

signals:
    void channelCountChanged();
    void dataChanged();

private:
    QVector<QVector<double>> m_data;
    double m_channelSpacing = 100.0;
    int m_currentIndex = 0;
    int m_totalSamples = 0;
};

#endif // EEGDATAMODEL_H
