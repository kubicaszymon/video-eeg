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
public:
    EegDataModel();

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    Q_INVOKABLE void updateAllData(const QVector<QVector<double>>& incomingData);

private:
    QVector<QVector<double>> m_data;
};

#endif // EEGDATAMODEL_H
