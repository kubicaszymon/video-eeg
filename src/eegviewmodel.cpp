#include "include/eegviewmodel.h"

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
    emit ChannelCountChanged();
}

bool EegViewModel::IsStreaming() const
{
    return is_streaming_;
}

void EegViewModel::SetIsStreaming(bool new_is_streaming)
{
    if (is_streaming_ == new_is_streaming)
        return;
    is_streaming_ = new_is_streaming;
    emit IsStreamingChanged();
}

double EegViewModel::SampleRate() const
{
    return sample_rate_;
}

void EegViewModel::SetSampleRate(double new_sample_rate)
{
    if (qFuzzyCompare(sample_rate_, new_sample_rate))
        return;
    sample_rate_ = new_sample_rate;
    emit SampleRateChanged();
}

int EegViewModel::DisplayWindowSize() const
{
    return display_window_size_;
}

void EegViewModel::SetDisplayWindowSize(int miliseconds)
{
    if(display_window_size_ != miliseconds) {
        display_window_size_ = miliseconds;
        //max_samples_per_channel_ = CalculateMaxSamples();
        emit DisplayWindowSizeChanged();
    }
}

QVariantList EegViewModel::GetChannelData(int channel_index) const
{
    QVariantList result;

    if (channel_index < 0 || channel_index >= channel_count_)
    {
        return result;
    }

    const auto& data = channel_data_[channel_index];

    for(const QPointF& point : data)
    {
        QVariantMap point_map;
        point_map["x"] = point.x();
        point_map["y"] = point.y();
        result.append(point_map);
    }

    return result;
}

QString EegViewModel::GetChannelName(int channel_index) const
{
    if (channel_index >= 0 && channel_index < channel_count_)
    {
        return channel_names_[channel_index];
    }
    return QString();
}

void EegViewModel::ClearAllChannels()
{
    for (auto& channel : channel_data_)
    {
        channel.clear();
    }
    emit AllChannelsUpdated();
}

void EegViewModel::UpdateChannelData(const std::vector<std::vector<float>>& chunk)
{
    /*
    auto& channel_buffer = channel_data_[channel_index];
    double time_step = 1000.0 / sample_rate_;

    for (int i = 0; i < new_samples.size(); ++i)
    {
        double time_ms = channel_buffer.isEmpty() ? 0 : channel_buffer.last().x() + time_step;
        channel_buffer.append(QPointF(time_ms, new_samples[i]));

        while (channel_buffer.size() > max_samples_per_channel_)
        {
            channel_buffer.removeFirst();
        }
    }

    emit ChannelDataUpdated(channel_index);
*/
}

void EegViewModel::StreamStarted()
{
    if(is_streaming_)
    {
        qDebug() << "EEG VIEW MODEL WAS ALREADY STARTED";
    }
    is_streaming_ = true;
}

void EegViewModel::StreamStopped()
{
    if(!is_streaming_)
    {
        qDebug() << "EEG VIEW MODEL WAS ALREADY STOPPED";
    }
    is_streaming_ = false;
}

void EegViewModel::InitializeChannels()
{
    max_samples_per_channel_ = CalculateMaxSamples();
    channel_data_.resize(channel_count_);
    channel_names_.resize(channel_count_);

    for (int i = 0; i < channel_count_; ++i)
    {
        channel_data_[i].reserve(max_samples_per_channel_);
        channel_names_[i] = QString("Channel %1").arg(i+1);
    }
}

int EegViewModel::CalculateMaxSamples() const
{
    return static_cast<int>((display_window_size_ / 1000.0) * sample_rate_);
}
