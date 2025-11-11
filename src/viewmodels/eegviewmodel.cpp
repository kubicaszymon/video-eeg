#include "eegviewmodel.h"

#include <qtimer.h>

EegViewModel::EegViewModel(QObject *parent)
    : QObject{parent}, amplifier_manager_{AmplifierManager::instance()}
{
    connect(amplifier_manager_, &AmplifierManager::DataReceived, this, &EegViewModel::UpdateChannelData);

    update_timer_ = new QTimer(this);
    update_timer_->setInterval(batch_interval_ms_);
    update_timer_->setSingleShot(true);
    connect(update_timer_, &QTimer::timeout, this, [this](){
        for(int ch : dirty_channels_)
        {
            normalizeChannelData(ch);
            emit channelDataChanged(ch);
        }
        dirty_channels_.clear();
    });
}

QVariantMap EegViewModel::getChannelRenderData(int channel_index) const
{
    if(channel_index < 0 || channel_index >= channel_render_cache_.size())
    {
        QVariantMap empty;
        empty["points"] = QVariantList();
        empty["isEmpty"] = true;
        return empty;
    }
    return channel_render_cache_.at(channel_index);
}

QVariantList EegViewModel::GetChannelNames() const
{
    if(amplifier_ == nullptr)
    {
        return {};
    }

    QVariantList channels;
    for(const auto& channel : std::as_const(amplifier_->available_channels))
    {
        channels.append(channel);
    }

    return channels;
}

void EegViewModel::UpdateChannelData(const std::vector<std::vector<float>>& chunk)
{
    if(chunk.empty())
    {
        return;
    }

    int num_samples = chunk.size();
    int num_channels = chunk[0].size();

    double time_increment = 1.0 / sample_rate_;

    for (int sample_idx = 0; sample_idx < num_samples; ++sample_idx)
    {
        const auto& sample = chunk[sample_idx];
        for(int ch = 0; ch < num_channels; ++ch)
        {
            channel_data_[ch].push_back(QPointF(current_time_, sample[ch]));
            if(channel_data_[ch].size() > max_samples_per_channel_)
            {
                channel_data_[ch].pop_front();
            }

            // Mark channel as dirty
            dirty_channels_.insert(ch);
        }

        current_time_ += time_increment;
    }

    // Start/restart the batch timer
    if (!update_timer_->isActive())
    {
        update_timer_->start();
    }
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

void EegViewModel::normalizeChannelData(int channel_index)
{
    if (channel_index < 0 || channel_index >= channel_data_.size())
        return;

    const auto& data = channel_data_.at(channel_index);

    QVariantMap result;

    if (data.empty())
    {
        result["points"] = QVariantList();
        result["isEmpty"] = true;
        channel_render_cache_[channel_index] = result;
        return;
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
    if (range == 0) range = 1.0;

    minY -= range * 0.1;
    maxY += range * 0.1;
    range = maxY - minY;

    qreal timeRange = maxX - minX;
    if (timeRange == 0) timeRange = 1.0;

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

    channel_render_cache_[channel_index] = result;
}

void EegViewModel::initialize(QString amplifier_id, QVariantList selected_channel_indices)
{
    amplifier_ = amplifier_manager_->GetAmplifierById(amplifier_id);

    int num_channels = amplifier_->available_channels.size();
    channel_data_.resize(num_channels);
    channel_render_cache_.resize(num_channels);

    for(int i = 0; i < num_channels; ++i)
    {
        QVariantMap empty;
        empty["points"] = QVariantList();
        empty["isEmpty"] = true;
        channel_render_cache_[i] = empty;
    }

    emit initializeEnded();
    amplifier_manager_->StartStream(amplifier_id);
}
