#include "AmplifierSetupBackend.h"
#include <qdebug.h>

AmplifierSetupBackend::AmplifierSetupBackend(QObject *parent)
    : QObject{parent},
      m_manager{AmplifierManager::instance()},
      m_cameraManager{CameraManager::instance()}
{
    qInfo() << "AmplifierSetupBackend " << this << " created!";

    if(m_manager)
    {
        connect(m_manager, &AmplifierManager::amplifiersListRefreshed,
                this, &AmplifierSetupBackend::onAmplifiersListRefreshed);
    }

    if(m_cameraManager)
    {
        connect(m_cameraManager, &CameraManager::availableCamerasChanged,
                this, &AmplifierSetupBackend::availableCamerasChanged);
        connect(m_cameraManager, &CameraManager::currentCameraIndexChanged,
                this, &AmplifierSetupBackend::selectedCameraIndexChanged);
        connect(m_cameraManager, &CameraManager::availableFormatsChanged,
                this, &AmplifierSetupBackend::availableFormatsChanged);
        connect(m_cameraManager, &CameraManager::currentFormatIndexChanged,
                this, &AmplifierSetupBackend::selectedFormatIndexChanged);
        connect(m_cameraManager, &CameraManager::isPreviewActiveChanged,
                this, &AmplifierSetupBackend::cameraPreviewActiveChanged);
        connect(m_cameraManager, &CameraManager::errorOccurred,
                this, &AmplifierSetupBackend::onCameraErrorOccurred);
    }
}

AmplifierSetupBackend::~AmplifierSetupBackend()
{
    qInfo() << "AmplifierSetupBackend " << this << " destroyed!";

    // Stop camera preview when settings window closes
    if(m_cameraManager && m_cameraManager->isPreviewActive())
    {
        m_cameraManager->stopPreview();
    }
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
    const auto& amp = getCurrentAmplifier();
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
    if(const auto& amplifier = getCurrentAmplifier(); amplifier != nullptr)
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

        m_manager->refreshAmplifiersListAsync();
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

const Amplifier* AmplifierSetupBackend::getCurrentAmplifier() const
{
    int index = m_selectedAmplifierIndex.value();
    if(m_amplifiers.empty() || index < 0 || index >= m_amplifiers.size())
    {
        return nullptr;
    }
    return &m_amplifiers[index];
}

// Camera methods implementation

QVariantList AmplifierSetupBackend::availableCameras() const
{
    if(m_cameraManager)
    {
        return m_cameraManager->availableCameras();
    }
    return QVariantList();
}

int AmplifierSetupBackend::selectedCameraIndex() const
{
    if(m_cameraManager)
    {
        return m_cameraManager->currentCameraIndex();
    }
    return -1;
}

QVariantList AmplifierSetupBackend::availableFormats() const
{
    if(m_cameraManager)
    {
        return m_cameraManager->availableFormats();
    }
    return QVariantList();
}

int AmplifierSetupBackend::selectedFormatIndex() const
{
    if(m_cameraManager)
    {
        return m_cameraManager->currentFormatIndex();
    }
    return -1;
}

QString AmplifierSetupBackend::selectedCameraName() const
{
    if(m_cameraManager)
    {
        return m_cameraManager->currentCameraName();
    }
    return QString();
}

QString AmplifierSetupBackend::selectedFormatString() const
{
    if(m_cameraManager)
    {
        return m_cameraManager->currentFormatString();
    }
    return QString();
}

bool AmplifierSetupBackend::isCameraPreviewActive() const
{
    if(m_cameraManager)
    {
        return m_cameraManager->isPreviewActive();
    }
    return false;
}

void AmplifierSetupBackend::refreshCameraList()
{
    if(m_cameraManager)
    {
        qInfo() << "AmplifierSetupBackend: Refreshing camera list";
        m_cameraManager->refreshCameraList();
    }
}

void AmplifierSetupBackend::setSelectedCameraIndex(int index)
{
    if(m_cameraManager)
    {
        m_cameraManager->setCurrentCameraIndex(index);
    }
}

void AmplifierSetupBackend::setSelectedFormatIndex(int index)
{
    if(m_cameraManager)
    {
        m_cameraManager->setCurrentFormatIndex(index);
    }
}

void AmplifierSetupBackend::startCameraPreview()
{
    if(m_cameraManager)
    {
        qInfo() << "AmplifierSetupBackend: Starting camera preview";
        m_cameraManager->startPreview();
    }
}

void AmplifierSetupBackend::stopCameraPreview()
{
    if(m_cameraManager)
    {
        qInfo() << "AmplifierSetupBackend: Stopping camera preview";
        m_cameraManager->stopPreview();
    }
}

QString AmplifierSetupBackend::getSelectedCameraId() const
{
    if(m_cameraManager && m_cameraManager->currentCameraIndex() >= 0)
    {
        QVariantList cameras = m_cameraManager->availableCameras();
        int idx = m_cameraManager->currentCameraIndex();
        if(idx < cameras.size())
        {
            return cameras[idx].toMap()["id"].toString();
        }
    }
    return QString();
}

void AmplifierSetupBackend::onCameraErrorOccurred(const QString& error)
{
    qWarning() << "AmplifierSetupBackend: Camera error:" << error;
    emit cameraErrorOccurred(error);
}
