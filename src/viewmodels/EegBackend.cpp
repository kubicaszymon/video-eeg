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

void EegBackend::startStream()
{
    amplifier_manager_->StartStream(m_amplifierId);
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
    if(chunk.empty() || chunk[0].empty() || m_channels.isEmpty())
    {
        return;
    }

    int numSamples = chunk.size();
    int numSelectedChannels = m_channels.size();

    // KROK 1: Transpozycja i konwersja
    QVector<QVector<double>> transposedData(numSelectedChannels);
    for(int i = 0; i < numSelectedChannels; ++i)
    {
        transposedData[i].reserve(numSamples);
    }

    for(int sample = 0; sample < numSamples; ++sample)
    {
        for(int i = 0; i < numSelectedChannels; ++i)
        {
            int channelIndex = m_channels[i].toInt();

            if(channelIndex >= 0 && channelIndex < chunk[sample].size())
            {
                transposedData[i].append(static_cast<double>(chunk[sample][channelIndex]));
            }
        }
    }

    // KROK 2: SKALOWANIE - każdy kanał niezależnie
    const double CHANNEL_SPACING = 100.0;  // Odstęp między kanałami na wykresie

    QVector<QVector<double>> scaledData(numSelectedChannels);

    for(int ch = 0; ch < numSelectedChannels; ++ch)
    {
        if(transposedData[ch].isEmpty()) continue;

        scaledData[ch].reserve(transposedData[ch].size());

        // Offset dla tego kanału (od góry do dołu)
        double offset = (numSelectedChannels - 1 - ch) * CHANNEL_SPACING;

        // Opcja A: Bez normalizacji, tylko offset
        for(double val : transposedData[ch])
        {
            scaledData[ch].append(val + offset);
        }

        /* Opcja B: Z normalizacją (odkomentuj jeśli potrzebujesz)
        double minVal = *std::min_element(transposedData[ch].begin(), transposedData[ch].end());
        double maxVal = *std::max_element(transposedData[ch].begin(), transposedData[ch].end());
        double range = maxVal - minVal;

        const double TARGET_AMPLITUDE = 80.0; // Amplituda po skalowaniu

        if(range > 0.001)
        {
            for(double val : transposedData[ch])
            {
                // Normalizuj do [-TARGET_AMPLITUDE/2, +TARGET_AMPLITUDE/2] i dodaj offset
                double normalized = ((val - minVal) / range - 0.5) * TARGET_AMPLITUDE;
                scaledData[ch].append(normalized + offset);
            }
        }
        else
        {
            // Jeśli dane płaskie, tylko offset
            for(double val : transposedData[ch])
            {
                scaledData[ch].append(offset);
            }
        }
        */
    }

    m_dataModel->updateAllData(scaledData);
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

int EegBackend::amplifierIdx() const
{
    return m_amplifierIdx;
}

void EegBackend::setAmplifierIdx(int newAmplifierIdx)
{
    if (m_amplifierIdx == newAmplifierIdx)
        return;
    m_amplifierIdx = newAmplifierIdx;
    emit amplifierIdxChanged();
}

int EegBackend::spacing() const
{
    return m_spacing;
}

void EegBackend::setSpacing(int newSpacing)
{
    m_spacing = newSpacing;
}

QString EegBackend::amplifierId() const
{
    return m_amplifierId;
}

void EegBackend::setAmplifierId(const QString &newAmplifierId)
{
    if (m_amplifierId == newAmplifierId)
        return;
    m_amplifierId = newAmplifierId;
    emit amplifierIdChanged();
}
