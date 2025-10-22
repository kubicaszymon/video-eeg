#include "../include/videoeegapp.h"

VideoEegApp::VideoEegApp(QObject *parent)
    : QObject{parent}, m_amplifierManager(std::make_unique<AmplifierManager>())
{
    m_amplifierManager->GetAmplifiersList();
}
