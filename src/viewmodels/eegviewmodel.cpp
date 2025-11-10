#include "eegviewmodel.h"

EegViewModel::EegViewModel(QObject *parent)
    : QObject{parent}, amplifier_manager_{AmplifierManager::instance()}
{
    connect(amplifier_manager_, &AmplifierManager::DataReceived, this, &EegViewModel::UpdateChannelData);
}

int EegViewModel::GetChannelCount() const
{
    return channel_count_;
}

void EegViewModel::SetChannelCount(int new_channel_count)
{
    if (channel_count_ == new_channel_count)
        return;

    channel_count_ = new_channel_count;
    channel_data_.resize(channel_count_);
    channel_names_.resize(channel_count_);

    emit channelCountChanged();
}

QVariantMap EegViewModel::getChannelRenderData(int channel_index) const
{
    QVariantMap result;

    if (channel_index < 0 || channel_index >= channel_count_)
    {
        result["points"] = QVariantList();
        result["isEmpty"] = true;
        return result;
    }

    const auto& data = channel_data_.at(channel_index);

    if (data.empty())
    {
        result["points"] = QVariantList();
        result["isEmpty"] = true;
        return result;
    }

    qreal minY = std::numeric_limits<qreal>::max();
    qreal maxY = std::numeric_limits<qreal>::lowest();
    qreal minX = std::numeric_limits<qreal>::max();
    qreal maxX = std::numeric_limits<qreal>::lowest();

    for (const QPointF& point : data)
    {
        minY = qMin(minY, point.y());
        maxY = qMax(maxY, point.y());
        minX = qMin(minX, point.x());
        maxX = qMax(maxX, point.x());
    }

    // Add 10% padding to Y range
    qreal range = maxY - minY;
    if (range == 0)
    {
        range = 1.0;
    }
    minY -= range * 0.1;
    maxY += range * 0.1;
    range = maxY - minY;

    qreal timeRange = maxX - minX;
    if (timeRange == 0)
    {
        timeRange = 1.0;
    }

    QVariantList normalized;
    for (const QPointF& point : data)
    {
        QVariantMap normalizedPoint;
        normalizedPoint["x"] = (point.x() - minX) / timeRange;
        normalizedPoint["y"] = (point.y() - minY) / range;
        normalized.append(normalizedPoint);
    }

    result["points"] = normalized;
    result["isEmpty"] = false;
    result["dataMin"] = minY;
    result["dataMax"] = maxY;
    result["timeMin"] = minX;
    result["timeMax"] = maxX;

    return result;
}

QString EegViewModel::getChannelName(int channel_index) const
{
    if (channel_index >= 0 && channel_index < channel_count_)
    {
        return channel_names_.at(channel_index);
    }
    return QString("Unknown");
}

void EegViewModel::UpdateChannelData(const std::vector<std::vector<float>>& chunk)
{
    //qDebug() << "UPDATE CHANNEL DATA";
    if(chunk.empty())
    {
        return;
    }

    int num_samples = chunk.size();
    int num_channels = chunk[0].size(); // ?

    double time_increment = 1.0 / sample_rate_;

    for (int sample_idx = 0; sample_idx < num_samples; ++sample_idx)
    {
        const auto& sample = chunk[sample_idx];
        for(int ch = 0; ch < std::min(num_channels, channel_count_); ++ch)
        {
            channel_data_[ch].push_back(QPointF(current_time_, sample[ch]));
            if(channel_data_[ch].size() > max_samples_per_channel_)
            {
                channel_data_[ch].pop_front();
            }
        }

        current_time_ += time_increment;
    }

    emit allChannelsUpdated();
}

void EegViewModel::StreamStarted()
{
    // RIGHT KNOW UNUSED, JUST CALL
    if(is_streaming_)
    {
        qDebug() << "EEG VIEW MODEL WAS ALREADY STARTED";
        return;
    }
    is_streaming_ = true;
}

void EegViewModel::StreamStopped()
{
    // RIGHT KNOW UNUSED, JUST CALL
    if(!is_streaming_)
    {
        qDebug() << "EEG VIEW MODEL WAS ALREADY STOPPED";
        return;
    }
    is_streaming_ = false;
}

void EegViewModel::initialize(QString amplifier_id, QVariantList selected_channel_indices)
{
    amplifier_manager_->StartStream(amplifier_id);
}
