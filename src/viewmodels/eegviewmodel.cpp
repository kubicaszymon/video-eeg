#include "eegviewmodel.h"

#include <qtimer.h>

EegViewModel::EegViewModel(QObject *parent)
    : QObject{parent}, amplifier_manager_{AmplifierManager::instance()}
{
    connect(amplifier_manager_, &AmplifierManager::DataReceived, this, &EegViewModel::DataReceived, Qt::QueuedConnection);
}

EegViewModel::~EegViewModel()
{
    qDebug() << "[EegViewModel] DESTRUCTOR CALLED";
}

void EegViewModel::initialize(QString amplifier_id, QVariantList selected_channel_indices)
{
    amplifier_ = amplifier_manager_->GetAmplifierById(amplifier_id);
    if(amplifier_ == nullptr)
    {
        qWarning() << "[EegViewModel] Amplifier is nullptr";
        return;
    }

    emit initializeEnded();
    amplifier_manager_->StartStream(amplifier_id);
}

void EegViewModel::DataReceived(const std::vector<std::vector<float>>& chunk)
{
    if(chunk.empty())
    {
        return;
    }
    emit updateData(chunk);
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
    if(amplifier_ == nullptr)
    {
        return 0;
    }
    return amplifier_->available_channels.size();
}
