#ifndef EEGDATAMODEL_H
#define EEGDATAMODEL_H

#include <QObject>
#include "amplifiermanager.h"

class EegDataModel : public QObject
{
    Q_OBJECT
public:
    explicit EegDataModel(QObject *parent = nullptr);

signals:

private:
    AmplifierManager* amplifier_manager_;
    bool is_recording_ = false;
};

#endif // EEGDATAMODEL_H
