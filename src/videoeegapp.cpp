#include "../include/videoeegapp.h"
#include <QQmlContext>
#include <QDebug>
#include <qcoreapplication.h>
#include <QGuiApplication>
#include <QCursor>

VideoEegApp::VideoEegApp(QObject *parent)
    : QObject{parent}
{

}

void VideoEegApp::SetupConnections()
{
    QObject::connect(amplifier_manager_.get(), &AmplifierManager::DataReceived, eeg_view_model_.get(), &EegViewModel::UpdateChannelData);
    QObject::connect(amplifier_manager_.get(), &AmplifierManager::StartLSLReading, eeg_view_model_.get(), &EegViewModel::StreamStarted);
    QObject::connect(amplifier_manager_.get(), &AmplifierManager::StopLSLReading, eeg_view_model_.get(), &EegViewModel::StreamStopped);
}

VideoEegApp::~VideoEegApp()
{
    Shutdown();
}

bool VideoEegApp::Initialize()
{
    if(is_initialized_)
    {
        qWarning() << "VideoEegApp already initialized";
    }

    try{
        CreateComponents();
        SetupConnections();
        RegisterQmlTypes();

        is_initialized_ = true;
        return true;
    }
    catch(const std::exception& e)
    {
        qCritical() << "Failed to initialize VideoEegApp: " << e.what();
        return false;
    }
}

void VideoEegApp::Run()
{
    if(!is_initialized_)
    {
        if(!Initialize())
        {
            qCritical() << "Cannot run: initialization failed";
            return;
        }
    }

    LoadQml();
}

void VideoEegApp::Shutdown()
{
    if(amplifier_manager_)
    {
        amplifier_manager_->StopStream();
    }

    eeg_view_model_.reset();
    amplifier_manager_.reset();
    engine_.reset();

    is_initialized_ = false;
}

void VideoEegApp::refreshAmplifiersList()
{
    available_amplifiers_ = amplifier_manager_->GetAmplifiersList();
    emit amplifiersChanged();
}

void VideoEegApp::setupGraphsWindow(const QVariantList& selected_channels)
{
    if(selected_amplifier_index_ >= available_amplifiers_.size())
    {
        return;
    }

    const auto& amp = available_amplifiers_.at(selected_amplifier_index_);

    QStringList channels{};
    QList<quint8> indexes{};

    for (const QVariant& channel : selected_channels)
    {
        const auto index = static_cast<quint8>(channel.toInt());
        indexes.append(index);
        if(amp.available_channels.size() > index)
        {
            channels.append(amp.available_channels.at(index));
        }
        else
        {
            qWarning() << "[setupGraphsWindow] Invalid channel index: " << index;
        }
    }
    eeg_view_model_->Initialize(channels);
    amplifier_manager_->StartStream(amp.id, indexes);
}

void VideoEegApp::setBusyCursor(bool busy)
{
    if(busy)
    {
        QGuiApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    }
    else
    {
        QGuiApplication::restoreOverrideCursor();
    }
}

QStringList VideoEegApp::amplifierNames() const
{
    QStringList names;
    for(const auto& amp : std::as_const(available_amplifiers_))
    {
        names << amp.name;
    }
    return names;
}

QStringList VideoEegApp::currentAmplifierChannels() const
{
    if(selected_amplifier_index_ >= 0 && selected_amplifier_index_ < available_amplifiers_.size())
    {
        return available_amplifiers_.at(selected_amplifier_index_).available_channels;
    }
    return QStringList();
}

int VideoEegApp::selectedAmplifierIndex() const
{
    return selected_amplifier_index_;
}

void VideoEegApp::setSelectedAmplifierIndex(int index)
{
    if(index >= 0 && index < available_amplifiers_.size())
    {
        selected_amplifier_index_ = index;
        emit selectedAmplifierChanged();
    }
}

void VideoEegApp::CreateComponents()
{
    engine_ = std::make_unique<QQmlApplicationEngine>();

    QString svarog_path = "C:\\Program Files (x86)\\Svarog Streamer\\svarog_streamer\\svarog_streamer.exe";
    amplifier_manager_ = std::make_unique<AmplifierManager>(svarog_path);

    eeg_view_model_ = std::make_unique<EegViewModel>();
}

void VideoEegApp::RegisterQmlTypes()
{
    engine_->rootContext()->setContextProperty("veegapp", this);
    engine_->rootContext()->setContextProperty("eegViewModel", eeg_view_model_.get());
}

void VideoEegApp::LoadQml()
{
    const QUrl url(QStringLiteral("qrc:/qt/qml/videoEeg/qml/Main.qml"));

    connect(engine_.get(), &QQmlApplicationEngine::objectCreated, this, [url](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl) {
            qCritical() << "Failed to load QML";
            QCoreApplication::exit(-1);
        }
    });

    engine_->load(url);
}
