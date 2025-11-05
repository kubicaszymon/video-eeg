#include <QGuiApplication>
#include "../include/videoeegapp.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    QCoreApplication::setOrganizationName("PWR W11");
    QCoreApplication::setApplicationName("VideoEEG");
    QCoreApplication::setApplicationVersion("0.0.1");

    VideoEegApp video_eeg_app;
    if(!video_eeg_app.Initialize())
    {
        qCritical() << "Failed to initialize application";
        return -1;
    }

    video_eeg_app.Run();
    return app.exec();
}
