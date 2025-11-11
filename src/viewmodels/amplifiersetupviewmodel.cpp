#include "amplifiersetupviewmodel.h"
#include <qdebug.h>

AmplifierSetupViewModel::AmplifierSetupViewModel(QObject *parent)
    : QObject{parent}
{

}

QVariantList AmplifierSetupViewModel::getAvailableAmplifiers() const
{
    QVariantList available_amplifiers{};
    const auto amps = manager_->RefreshAmplifiersList();
    for(auto amp : amps)
    {
        available_amplifiers.append(amp.name);
    }
    return available_amplifiers;
}

QString AmplifierSetupViewModel::getSelectedAmplifierId() const
{
    const auto& amp = GetCurrentAmplifier();
    if(amp == nullptr)
    {
        return "";
    }
    return amp->id;
}

int AmplifierSetupViewModel::getSelectedAmplifierIndex() const
{
    int val = selected_amplifier_index_.value();
    return val;
}

QVariantList AmplifierSetupViewModel::getCurrentChannels() const
{
    QVariantList current_channels{};
    if(const auto& amplifier = GetCurrentAmplifier(); amplifier != nullptr)
    {
        for(const auto& channel : amplifier->available_channels)
        {
            current_channels.append(channel);
        }
    }
    return current_channels;
}

void AmplifierSetupViewModel::initialize()
{
    qDebug() << "initialize";

    manager_ = AmplifierManager::instance();
    amplifiers_ = manager_->RefreshAmplifiersList();
    emit availableAmplifiersChanged();
}

void AmplifierSetupViewModel::setSelectedAmplifierIndex(int index)
{
    if (index == selected_amplifier_index_)
    {
        return;
    }
    selected_amplifier_index_ = index;
    emit selectedAmplifierIndexChanged();
}

const Amplifier* AmplifierSetupViewModel::GetCurrentAmplifier() const
{
    if(amplifiers_.empty() || selected_amplifier_index_.value() >= amplifiers_.size())
    {
        return nullptr;
    }
    return &amplifiers_[selected_amplifier_index_];
}
