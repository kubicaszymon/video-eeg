#include "eegdatamodel.h"
#include <QtCore/qnumeric.h>

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
        int col = index.column();
        int row = index.row();

        if (col < m_data.size() && row < m_data[col].size())
        {
            return m_data[col][row];
        }
    }
    return QVariant();
}

void EegDataModel::updateAllData(const QVector<QVector<double>>& incomingData)
{
    const int MAX_SAMPLES = 1000;

    if (incomingData.isEmpty() || incomingData[0].isEmpty())
    {
        return;
    }

    int newSamples = incomingData[0].size();
    int numChannels = incomingData.size();

    beginResetModel();

    if (m_data.isEmpty())
    {
        m_data.resize(numChannels + 1);
        m_currentIndex = 0;
        m_totalSamples = 0;
    }

    if (m_totalSamples < MAX_SAMPLES)
    {
        // Faza początkowa - wypełniamy bufor
        for (int s = 0; s < newSamples && m_totalSamples < MAX_SAMPLES; ++s)
        {
            m_data[0].append(m_totalSamples);

            for (int ch = 0; ch < numChannels; ++ch)
            {
                m_data[ch + 1].append(incomingData[ch][s]);
            }

            m_totalSamples++;
        }

        m_currentIndex = m_totalSamples;
    }
    else
    {
        // Faza circular buffer
        for (int s = 0; s < newSamples; ++s)
        {
            int writeIndex = m_currentIndex % MAX_SAMPLES;

            m_data[0][writeIndex] = writeIndex;

            for (int ch = 0; ch < numChannels; ++ch)
            {
                m_data[ch + 1][writeIndex] = incomingData[ch][s];
            }

            m_currentIndex++;
        }

        m_currentIndex = m_currentIndex % MAX_SAMPLES;
    }

    // writePosition = miejsce gdzie BĘDĄ zapisane następne dane
    // To jest dokładnie m_currentIndex!
    // Dodajemy GAP (20 sampli) żeby była przerwa między linią a starymi danymi
    const int GAP = 20;
    m_writePosition = (m_currentIndex + GAP) % MAX_SAMPLES;

    qInfo() << "Total samples:" << m_totalSamples
            << "Current index:" << m_currentIndex
            << "Write position (with gap):" << m_writePosition
            << "New samples:" << newSamples;

    emit writePositionChanged();

    endResetModel();
}

double EegDataModel::minValue() const
{
    double min = std::numeric_limits<double>::infinity();
    for (const auto& col : m_data) {
        for (double val : col) {
            min = std::min(min, val);
        }
    }
    return min;
}

double EegDataModel::maxValue() const
{
    double max = -std::numeric_limits<double>::infinity();
    for (const auto& col : m_data) {
        for (double val : col) {
            max = std::max(max, val);
        }
    }
    return max;
}

int EegDataModel::writePosition() const
{
    return m_writePosition;
}
