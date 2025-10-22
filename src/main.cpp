#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include "../include/videoeegapp.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;
    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);
    engine.loadFromModule("videoEeg", "Main");

    VideoEegApp veapp{};

    return app.exec();
}
