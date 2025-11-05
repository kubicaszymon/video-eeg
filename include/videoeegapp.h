#ifndef VIDEOEEGAPP_H
#define VIDEOEEGAPP_H

#include <QObject>
#include <memory>
#include "amplifiermanager.h"

class VideoEegApp : public QObject
{
    Q_OBJECT
public:
    explicit VideoEegApp(QObject *parent = nullptr);

    void InitializeAmplifier(const QString amp_path);

signals:

private:
    std::unique_ptr<AmplifierManager> amplifier_manager_;
};

#endif // VIDEOEEGAPP_H
