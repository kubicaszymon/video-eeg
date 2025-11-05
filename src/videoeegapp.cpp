#include "../include/videoeegapp.h"
#include "../include/amplifiermanager.h"
#include "../include/eegviewmodel.h"
#include <QQmlContext>
#include <QDebug>
#include <qcoreapplication.h>

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
    engine_->rootContext()->setContextProperty("amplifierManager", amplifier_manager_.get());
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
