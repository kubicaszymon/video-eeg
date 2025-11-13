#include "eegviewmodel.h"

#include <qtimer.h>

EegViewModel::EegViewModel(QObject *parent)
    : QObject{parent}, amplifier_manager_{AmplifierManager::instance()}
{
    connect(amplifier_manager_, &AmplifierManager::DataReceived, this, &EegViewModel::UpdateChannelData, Qt::QueuedConnection);
}

EegViewModel::~EegViewModel()
{
    qDebug() << "[EegViewModel] DESTRUCTOR CALLED";
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

int EegViewModel::GetChannelCount() const
{
    return channel_buffers_.size();
}

int EegViewModel::GetMaxSamples() const
{
    return MAX_SAMPLES_PER_CHANNEL;
}

QVector<float> EegViewModel::getChannelData(int channel_index) const
{
    QMutexLocker locker(&data_mutex_);

    if(channel_index < 0 || channel_index >= channel_buffers_.size())
    {
        return {};
    }

    const auto& buffer = channel_buffers_[channel_index];

    QVector<float> result;
    result.reserve(buffer.count);

    if(buffer.count < MAX_SAMPLES_PER_CHANNEL)
    {
        std::copy(buffer.values.begin(), buffer.values.begin() + buffer.count, std::back_inserter(result));
    }
    else
    {
        int start = buffer.write_index;

        // Copy from write_index to end
        std::copy(buffer.values.begin() + start, buffer.values.end(), std::back_inserter(result));

        // Copy from beginning to write_index
        std::copy(buffer.values.begin(), buffer.values.begin() + start, std::back_inserter(result));
    }

    return result;
}

float EegViewModel::getChannelMin(int channel_index) const
{
    QMutexLocker locker(&data_mutex_);
    if(channel_index < 0 || channel_index >= channel_buffers_.size())
    {
        return 0.0f;
    }
    return channel_buffers_[channel_index].min_value;
}

float EegViewModel::getChannelMax(int channel_index) const
{
    QMutexLocker locker(&data_mutex_);
    if(channel_index < 0 || channel_index >= channel_buffers_.size())
    {
        return 0.0f;
    }
    return channel_buffers_[channel_index].max_value;
}

void EegViewModel::UpdateChannelData(const std::vector<std::vector<float>>& chunk)
{
    if(chunk.empty())
    {
        return;
    }

    QMutexLocker locker(&data_mutex_);

    int num_samples = chunk.size();
    int num_channels = chunk[0].size();

    for(int sample_idx = 0; sample_idx < num_samples; sample_idx++)
    {
        const auto& sample = chunk[sample_idx];

        for(int ch = 0; ch < num_channels && ch < channel_buffers_.size(); ch++)
        {
            auto& buffer = channel_buffers_[ch];

            // write to buffer
            buffer.values[buffer.write_index] = sample[ch];
            buffer.write_index = (buffer.write_index + 1) % MAX_SAMPLES_PER_CHANNEL;

            if(buffer.count < MAX_SAMPLES_PER_CHANNEL)
            {
                buffer.count++;
            }
        }
    }

    locker.unlock();

    // THROTTLING: sygnał maksymalnie co 33ms (30 FPS)
    static auto last_emit = std::chrono::steady_clock::now();
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - last_emit);

    if(elapsed.count() >= 33) // 30 FPS
    {
        // Update min/max tylko raz na render
        for(int ch = 0; ch < channel_buffers_.size(); ++ch)
        {
            updateMinMax(ch);
        }

        emit dataUpdated();
        last_emit = now;
    }
}

void EegViewModel::updateMinMax(int channel_index)
{
    QMutexLocker locker(&data_mutex_);

    if(channel_index < 0 || channel_index >= channel_buffers_.size())
    {
        return;
    }

    auto& buffer = channel_buffers_[channel_index];
    if(buffer.count == 0)
    {
        return;
    }

    auto min_it = std::min_element(buffer.values.begin(), buffer.values.begin() + buffer.count);
    auto max_it = std::max_element(buffer.values.begin(), buffer.values.begin() + buffer.count);

    buffer.min_value = *min_it;
    buffer.max_value = *max_it;

    // 10% padding
    float range = buffer.max_value - buffer.min_value;
    if(range < 1e-6f)
    {
        range = 1.0f;
    }

    buffer.min_value -= range * 0.1f;
    buffer.max_value += range * 0.1f;
}

void EegViewModel::initialize(QString amplifier_id, QVariantList selected_channel_indices)
{
    amplifier_ = amplifier_manager_->GetAmplifierById(amplifier_id);
    if(amplifier_ == nullptr)
    {
        qWarning() << "[EegViewModel] Amplifier is nullptr";
        return;
    }

    int num_channels = amplifier_->available_channels.size();

    QMutexLocker locker(&data_mutex_);
    channel_buffers_.resize(num_channels);

    for(auto& buffer : channel_buffers_)
    {
        buffer.values.resize(MAX_SAMPLES_PER_CHANNEL, 0.0f);
        buffer.write_index = 0;
        buffer.count = 0;
        buffer.needs_update = false;
    }

    locker.unlock();

    emit channelCountChanged();
    emit initializeEnded();
    amplifier_manager_->StartStream(amplifier_id);
}
