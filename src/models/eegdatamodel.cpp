#include "eegdatamodel.h"
#include <QtCore/qnumeric.h>
#include <algorithm>

EegDataModel::EegDataModel()
{
    qInfo() << "EEGDATAMODEL CREATED " << this;
    m_updateTimer.start();
}

int EegDataModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    if (!m_bufferInitialized || m_data.empty())
    {
        return 0;
    }
    return m_maxSamples;
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

void EegDataModel::initializeBuffer(int numChannels)
{
    if (m_bufferInitialized && m_numChannels == numChannels && !m_data.empty() && m_data[0].size() == m_maxSamples)
    {
        return;
    }

    // Use beginResetModel only during initialization
    beginResetModel();

    m_data.clear();
    m_data.resize(numChannels + 1);

    // Pre-allocate all vectors with exact size
    for (int col = 0; col <= numChannels; ++col)
    {
        m_data[col].resize(m_maxSamples);
    }

    // Initialize X-axis column with time in SECONDS and set gap values for channels
    // X-axis: sample_index / sampling_rate = time in seconds
    for (int i = 0; i < m_maxSamples; ++i)
    {
        m_data[0][i] = static_cast<double>(i) / m_samplingRate;  // Time in seconds
        for (int ch = 0; ch < numChannels; ++ch)
        {
            m_data[ch + 1][i] = GAP_VALUE;
        }
    }

    m_currentIndex = 0;
    m_writePosition = 0;
    m_numChannels = numChannels;
    m_bufferInitialized = true;
    m_minMaxDirty = true;

    endResetModel();

    qInfo() << "EegDataModel buffer initialized for" << numChannels << "channels,"
            << m_maxSamples << "samples," << m_samplingRate << "Hz,"
            << m_timeWindowSeconds << "seconds window";
}

void EegDataModel::emitDataChanged(int startRow, int endRow)
{
    // Rate limit UI updates to prevent overload
    qint64 elapsed = m_updateTimer.elapsed();

    if (elapsed < MIN_UPDATE_INTERVAL_MS)
    {
        // Accumulate the range of changed rows
        if (m_pendingUpdate)
        {
            m_pendingStartRow = std::min(m_pendingStartRow, startRow);
            m_pendingEndRow = std::max(m_pendingEndRow, endRow);
        }
        else
        {
            m_pendingUpdate = true;
            m_pendingStartRow = startRow;
            m_pendingEndRow = endRow;
        }
        return;
    }

    // Include any pending updates
    if (m_pendingUpdate)
    {
        startRow = std::min(m_pendingStartRow, startRow);
        endRow = std::max(m_pendingEndRow, endRow);
        m_pendingUpdate = false;
    }

    // Emit incremental dataChanged signal instead of full model reset
    QModelIndex topLeft = index(startRow, 0);
    QModelIndex bottomRight = index(endRow, m_data.size() - 1);
    emit QAbstractItemModel::dataChanged(topLeft, bottomRight);

    m_updateTimer.restart();
}

void EegDataModel::updateMinMaxCache(double value)
{
    if (value >= GAP_VALUE) return;

    bool changed = false;
    if (value < m_cachedMin)
    {
        m_cachedMin = value;
        changed = true;
    }
    if (value > m_cachedMax)
    {
        m_cachedMax = value;
        changed = true;
    }

    if (changed)
    {
        emit minMaxChanged();
    }
}

void EegDataModel::updateAllData(const QVector<QVector<double>>& incomingData)
{
    if (incomingData.isEmpty() || incomingData[0].isEmpty())
    {
        return;
    }

    int newSamples = incomingData[0].size();
    int numChannels = incomingData.size();

    // Initialize buffer if needed (only happens once or on channel count change)
    if (!m_bufferInitialized || m_numChannels != numChannels)
    {
        initializeBuffer(numChannels);
    }

    int startWriteIndex = m_currentIndex % m_maxSamples;

    // Write incoming data to circular buffer
    for (int s = 0; s < newSamples; ++s)
    {
        int writeIndex = m_currentIndex % m_maxSamples;

        // X-axis: time in seconds (writeIndex / samplingRate)
        m_data[0][writeIndex] = static_cast<double>(writeIndex) / m_samplingRate;
        for (int ch = 0; ch < numChannels; ++ch)
        {
            double value = incomingData[ch][s];
            m_data[ch + 1][writeIndex] = value;

            // Update min/max cache incrementally
            updateMinMaxCache(value);
        }

        m_currentIndex++;
    }

    int endWriteIndex = (m_currentIndex - 1 + m_maxSamples) % m_maxSamples;

    // Update write position for cursor
    m_writePosition = endWriteIndex;
    emit writePositionChanged();

    // Create gap after write position (clear ahead)
    for (int g = 1; g <= GAP_SIZE; ++g)
    {
        int gapIndex = (endWriteIndex + g) % m_maxSamples;
        for (int ch = 0; ch < numChannels; ++ch)
        {
            m_data[ch + 1][gapIndex] = GAP_VALUE;
        }
    }

    // Calculate the range of changed rows for incremental update
    int changedStart, changedEnd;

    if (startWriteIndex <= endWriteIndex)
    {
        // No wraparound - simple case
        changedStart = startWriteIndex;
        changedEnd = std::min(endWriteIndex + GAP_SIZE, m_maxSamples - 1);
    }
    else
    {
        // Wraparound occurred - update whole buffer
        // This happens rarely, only when we cross the buffer boundary
        changedStart = 0;
        changedEnd = m_maxSamples - 1;
    }

    // Emit incremental update with rate limiting
    emitDataChanged(changedStart, changedEnd);
}

double EegDataModel::minValue() const
{
    if (m_cachedMin == std::numeric_limits<double>::infinity())
    {
        return 0.0;
    }
    return m_cachedMin;
}

double EegDataModel::maxValue() const
{
    if (m_cachedMax == -std::numeric_limits<double>::infinity())
    {
        return 1000.0;
    }
    return m_cachedMax;
}

int EegDataModel::writePosition() const
{
    return m_writePosition;
}

double EegDataModel::samplingRate() const
{
    return m_samplingRate;
}

void EegDataModel::setSamplingRate(double newSamplingRate)
{
    if (newSamplingRate <= 0 || qFuzzyCompare(m_samplingRate, newSamplingRate))
        return;

    m_samplingRate = newSamplingRate;
    emit samplingRateChanged();

    recalculateMaxSamples();
}

double EegDataModel::timeWindowSeconds() const
{
    return m_timeWindowSeconds;
}

void EegDataModel::setTimeWindowSeconds(double newTimeWindowSeconds)
{
    if (newTimeWindowSeconds <= 0 || qFuzzyCompare(m_timeWindowSeconds, newTimeWindowSeconds))
        return;

    m_timeWindowSeconds = newTimeWindowSeconds;
    emit timeWindowSecondsChanged();

    recalculateMaxSamples();
}

int EegDataModel::maxSamples() const
{
    return m_maxSamples;
}

void EegDataModel::recalculateMaxSamples()
{
    int newMaxSamples = static_cast<int>(m_samplingRate * m_timeWindowSeconds);

    // Ensure minimum buffer size
    if (newMaxSamples < 100)
    {
        newMaxSamples = 100;
    }

    if (m_maxSamples != newMaxSamples)
    {
        qInfo() << "EegDataModel: recalculating maxSamples from" << m_maxSamples
                << "to" << newMaxSamples
                << "(samplingRate:" << m_samplingRate << "Hz,"
                << "timeWindow:" << m_timeWindowSeconds << "s)";

        m_maxSamples = newMaxSamples;
        emit maxSamplesChanged();

        // Force buffer reinitialization with new size
        if (m_bufferInitialized && m_numChannels > 0)
        {
            m_bufferInitialized = false;  // Force reinitialize
            initializeBuffer(m_numChannels);
        }
    }
}
