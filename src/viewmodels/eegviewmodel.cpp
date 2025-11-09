#include "eegviewmodel.h"

EegViewModel::EegViewModel(QObject *parent)
    : QObject{parent}
{}

int EegViewModel::ChannelCount() const
{
    return channel_count_;
}

void EegViewModel::SetChannelCount(int new_channel_count)
{
    if (channel_count_ == new_channel_count)
        return;
    channel_count_ = new_channel_count;
    emit channelCountChanged();
}

QVariantList EegViewModel::getChannelData(int channel_index) const
{
    //qDebug() << "GET CHANNEL DATA";
    QVariantList result;

    if (channel_index < 0 || channel_index >= channel_count_)
    {
        return result;
    }

    const auto& data = channel_data_.at(channel_index);

    if(data.empty())
    {
        return result;
    }

    for(const QPointF& point : data)
    {
        QVariantMap point_map;
        point_map["x"] = point.x();
        point_map["y"] = point.y();
        result.append(point_map);
    }

    return result;
}

QString EegViewModel::getChannelName(int channel_index) const
{
    if (channel_index >= 0 && channel_index < channel_count_)
    {
        return channel_names_.at(channel_index);
    }
    return QString();
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
    }
    is_streaming_ = true;
}

void EegViewModel::StreamStopped()
{
    // RIGHT KNOW UNUSED, JUST CALL
    if(!is_streaming_)
    {
        qDebug() << "EEG VIEW MODEL WAS ALREADY STOPPED";
    }
    is_streaming_ = false;
}

void EegViewModel::Initialize(QStringList channels)
{
    channel_names_ = channels;
    channel_count_ = channels.count();
    channel_data_.resize(channel_count_);
}

void EegViewModel::initialize(int amplifier_id, QVariantList selected_channel_indices)
{

}
