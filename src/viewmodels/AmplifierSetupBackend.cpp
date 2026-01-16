#include "AmplifierSetupBackend.h"
#include <qdebug.h>

AmplifierSetupBackend::AmplifierSetupBackend(QObject *parent)
    : QObject{parent}, m_manager{AmplifierManager::instance()}
{
    qInfo() << "AmplifierSetupBackend " << this << " created!";

    if(m_manager)
    {
        connect(m_manager, &AmplifierManager::AmplifiersListRefreshed,
                this, &AmplifierSetupBackend::onAmplifiersListRefreshed);
    }
}

AmplifierSetupBackend::~AmplifierSetupBackend()
{
    qInfo() << "AmplifierSetupBackend " << this << " destroyed!";
}

QVariantList AmplifierSetupBackend::getAvailableAmplifiers() const
{
    QVariantList ampNames{};
    for(const auto& amp : std::as_const(m_amplifiers))
    {
        ampNames.append(amp.name);
    }
    return ampNames;
}

QString AmplifierSetupBackend::getSelectedAmplifierId() const
{
    const auto& amp = GetCurrentAmplifier();
    if(amp == nullptr)
    {
        return "";
    }
    return amp->id;
}

int AmplifierSetupBackend::getSelectedAmplifierIndex() const
{
    int val = m_selectedAmplifierIndex.value();
    return val;
}

QVariantList AmplifierSetupBackend::getCurrentChannels() const
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

void AmplifierSetupBackend::refreshAmplifiersList()
{
    if(m_manager)
    {
        m_isLoading = true;
        emit isLoadingChanged();

        m_manager->RefreshAmplifiersListAsync();
    }
    else
    {
        qWarning() << "AmplifierSetupBackend: amplifierManager is nullptr";
    }
}

void AmplifierSetupBackend::onAmplifiersListRefreshed(const QList<Amplifier>& amplifiers)
{
    m_amplifiers = amplifiers;
    emit availableAmplifiersChanged();

    m_isLoading = false;
    emit isLoadingChanged();
}

void AmplifierSetupBackend::setSelectedAmplifierIndex(int index)
{
    if (index == m_selectedAmplifierIndex)
    {
        return;
    }
    m_selectedAmplifierIndex = index;
    emit selectedAmplifierIndexChanged();
}

const Amplifier* AmplifierSetupBackend::GetCurrentAmplifier() const
{
    int index = m_selectedAmplifierIndex.value();
    if(m_amplifiers.empty() || index < 0 || index >= m_amplifiers.size())
    {
        return nullptr;
    }
    return &m_amplifiers[index];
}
