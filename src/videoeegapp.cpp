#include "../include/videoeegapp.h"

VideoEegApp::VideoEegApp(QObject *parent)
    : QObject{parent}
{
    // TEST CODE
    //auto amp_path = "C:\\Program Files (x86)\\Svarog Streamer\\svarog_streamer\\svarog_streamer.exe";
    //InitializeAmplifier(amp_path);
    //amplifier_manager_->GetAmplifiersList();

    //amplifier_manager_->StartStream();
}

void VideoEegApp::InitializeAmplifier(const QString amp_path)
{
    amplifier_manager_ = std::make_unique<AmplifierManager>(amp_path);
}
