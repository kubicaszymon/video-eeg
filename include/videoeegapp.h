#ifndef VIDEOEEGAPP_H
#define VIDEOEEGAPP_H

#include <QObject>
#include "amplifiermanager.h"

class VideoEegApp : public QObject
{
    Q_OBJECT
public:
    explicit VideoEegApp(QObject *parent = nullptr);

signals:

private:
    std::unique_ptr<AmplifierManager> m_amplifierManager;
};

#endif // VIDEOEEGAPP_H
