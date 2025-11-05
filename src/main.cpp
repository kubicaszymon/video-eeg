#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include "../include/videoeegapp.h"
#include "../include/amplifiermanager.h"
#include "../include/eegviewmodel.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    AmplifierManager amplifierManager("");
    VideoEegApp veapp;
    EegViewModel eegViewModel;

    QObject::connect(&amplifierManager, &AmplifierManager::DataReceived, &eegViewModel, &EegViewModel::UpdateChannelData);
    QObject::connect(&amplifierManager, &AmplifierManager::StartLSLReading, &eegViewModel, &EegViewModel::StreamStarted);
    QObject::connect(&amplifierManager, &AmplifierManager::StopLSLReading, &eegViewModel, &EegViewModel::StreamStopped);

    QQmlApplicationEngine engine;

    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);
    engine.loadFromModule("videoEeg", "Main");


    return app.exec();
}
