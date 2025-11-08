#include "../include/videoeegapp.h"
#include <QQmlContext>
#include <QDebug>
#include <qcoreapplication.h>
#include <QGuiApplication>
#include <QCursor>

VideoEegApp::VideoEegApp(QObject *parent)
    : QObject{parent}
{
    // TEST CODE
    //auto amp_path = "C:\\Program Files (x86)\\Svarog Streamer\\svarog_streamer\\svarog_streamer.exe";
    //InitializeAmplifier(amp_path);
    //amplifier_manager_->GetAmplifiersList();

    //amplifier_manager_->StartStream();
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

void VideoEegApp::setupGraphsWindow(QVariantList selected_channels)
{
    QStringList channels{};
    for (const QVariant& var : selected_channels)
    {
        channels.append(var.toString());
    }
    eeg_view_model_->Initialize(channels);

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

void VideoEegApp::SetupConnections()
{
    QObject::connect(amplifier_manager_.get(), &AmplifierManager::DataReceived, eeg_view_model_.get(), &EegViewModel::UpdateChannelData);
    QObject::connect(amplifier_manager_.get(), &AmplifierManager::StartLSLReading, eeg_view_model_.get(), &EegViewModel::StreamStarted);
    QObject::connect(amplifier_manager_.get(), &AmplifierManager::StopLSLReading, eeg_view_model_.get(), &EegViewModel::StreamStopped);
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
