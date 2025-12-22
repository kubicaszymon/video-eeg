#include "eegdatamodel.h"

EegDataModel::EegDataModel()
{
    qInfo() << "EEGDATAMODEL CREATED " << this;
}

int EegDataModel::rowCount(const QModelIndex &parent) const
{
    if(m_data.empty())
    {
        return 0;
    }
    return m_data[0].size();
}

int EegDataModel::columnCount(const QModelIndex &parent) const
{
    return m_data.size();
}

QVariant EegDataModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) return QVariant();

    if (role == Qt::DisplayRole)
    {
        // mapper asks for (row, col)
        // If m_data is QVector<QVector<double>> where each inner vector is a CHANNEL:
        // You must access it as [column][row]
        if (index.column() < m_data.size() && index.row() < m_data[index.column()].size())
        {
            return m_data[index.column()][index.row()];
        }
    }
    return QVariant();
}

void EegDataModel::updateAllData(const QVector<QVector<double>>& incomingData)
{
    qInfo() << "UPDATUJE DLA: " << this;
    beginResetModel();
    m_data = incomingData;
    endResetModel();
}
