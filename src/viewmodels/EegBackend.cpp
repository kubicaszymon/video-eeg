#include "EegBackend.h"

#include <qtimer.h>

EegBackend::EegBackend(QObject *parent)
    : QObject{parent}, amplifier_manager_{AmplifierManager::instance()}
{
    qInfo() << "EEGBACKEND CREATED: " << this;
    connect(amplifier_manager_, &AmplifierManager::DataReceived, this, &EegBackend::DataReceived, Qt::QueuedConnection);
}

EegBackend::~EegBackend()
{
    qDebug() << "[EegViewModel] DESTRUCTOR CALLED";
}

void EegBackend::registerDataModel(EegDataModel *dataModel)
{
    if(dataModel)
    {
        m_dataModel = dataModel;
        qInfo() << "eeg data model linked successfuly: " << m_dataModel;
    }
}

void EegBackend::generateTestData()
{
    if (!m_dataModel) return;

    auto channel_spacing = m_spacing;

    const int numChannels = m_channels.size();
    const int numSamples = 500; // 500 points per 5-second window

    QVector<QVector<double>> testData(numChannels + 1);

    for (int i = 0; i < numSamples; ++i) {
        double time = i * 0.1;
        testData[0].append(time); // Column 0: Time (X-axis)

        for (int ch = 1; ch <= numChannels; ++ch) {
            // Create different frequencies for each channel
            double value = qSin(time + ch) + (ch * 0.5);
            ///////////////////////////////////////


            auto offset = channel_spacing * (numChannels - ch + 1);

            testData[ch].append(value + offset);
        }
    }

    m_dataModel->updateAllData(testData);
}

void EegBackend::DataReceived(const std::vector<std::vector<float>>& chunk)
{
    if(chunk.empty())
    {
        return;
    }
    emit updateData(chunk);
}

QVariantList EegBackend::GetChannelNames() const
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

QVariantList EegBackend::channels() const
{
    return m_channels;
}

void EegBackend::setChannels(const QVariantList &newChannels)
{
    if (m_channels == newChannels)
        return;
    m_channels = newChannels;
    emit channelsChanged();
}

int EegBackend::amplifierId() const
{
    return m_amplifierId;
}

void EegBackend::setAmplifierId(int newAmplifierId)
{
    if (m_amplifierId == newAmplifierId)
        return;
    m_amplifierId = newAmplifierId;
    emit amplifierIdChanged();
}

int EegBackend::spacing() const
{
    return m_spacing;
}

void EegBackend::setSpacing(int newSpacing)
{
    m_spacing = newSpacing;
}
