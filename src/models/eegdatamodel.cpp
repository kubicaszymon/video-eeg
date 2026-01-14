#include "eegdatamodel.h"
#include <QtCore/qnumeric.h>

EegDataModel::EegDataModel()
{
    qInfo() << "EEGDATAMODEL CREATED " << this;
}

int EegDataModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    if(m_data.empty())
    {
        return 0;
    }
    return m_data[0].size();
}

int EegDataModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
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
    const int GAP_SIZE = 50;
    const double GAP_VALUE = 1e9;

    if (incomingData.isEmpty() || incomingData[0].isEmpty())
    {
        return;
    }

    int newSamples = incomingData[0].size();
    int numChannels = incomingData.size();

    beginResetModel();

    // Inicjalizacja bufora
    if (m_data.isEmpty())
    {
        m_data.resize(numChannels + 1);
        for (int i = 0; i < MAX_SAMPLES; ++i)
        {
            m_data[0].append(i);
            for (int ch = 0; ch < numChannels; ++ch)
            {
                m_data[ch + 1].append(GAP_VALUE);
            }
        }
        m_currentIndex = 0;
    }

    // Zapisz nowe dane
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

    // Po zapisie: m_currentIndex wskazuje na NASTĘPNĄ pozycję do zapisu
    // Ostatni zapisany sample jest na (m_currentIndex - 1) % MAX_SAMPLES

    int lastWritten = (m_currentIndex - 1 + MAX_SAMPLES) % MAX_SAMPLES;

    // Wyczyść gap ZA ostatnim zapisanym
    for (int g = 1; g <= GAP_SIZE; ++g)
    {
        int gapIndex = (lastWritten + g) % MAX_SAMPLES;
        for (int ch = 0; ch < numChannels; ++ch)
        {
            m_data[ch + 1][gapIndex] = GAP_VALUE;
        }
    }

    // KLUCZOWE: writePosition = indeks OSTATNIEGO ZAPISANEGO sample'a
    // To jest pozycja X na wykresie gdzie kończy się nowa linia danych
    m_writePosition = lastWritten;

    qInfo() << "Head at X =" << m_writePosition << "(last written sample)";

    emit writePositionChanged();
    endResetModel();
}

double EegDataModel::minValue() const
{
    double min = std::numeric_limits<double>::infinity();
    for (const auto& col : m_data) {
        for (double val : col) {
            if (val < 1e8) min = std::min(min, val);
        }
    }
    return min;
}

double EegDataModel::maxValue() const
{
    double max = -std::numeric_limits<double>::infinity();
    for (const auto& col : m_data) {
        for (double val : col) {
            if (val < 1e8) max = std::max(max, val);
        }
    }
    return max;
}

int EegDataModel::writePosition() const
{
    return m_writePosition;
}
