#ifndef EEGVIEWMODEL_H
#define EEGVIEWMODEL_H

#include <QObject>
#include <QVariantMap>
#include <QVector>
#include <QPointF>
#include <QMutex>
#include <QtQml/qqmlregistration.h>
#include "amplifiermodel.h"
#include "amplifiermanager.h"

class EegViewModel : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(QVariantList channelNames READ GetChannelNames NOTIFY initializeEnded FINAL)
    Q_PROPERTY(int channelCount READ GetChannelCount NOTIFY channelCountChanged FINAL)

public:
    explicit EegViewModel(QObject *parent = nullptr);
    ~EegViewModel();

    QVariantList GetChannelNames() const;
    int GetChannelCount() const;

    Q_INVOKABLE void initialize(QString amplifier_id, QVariantList selected_channel_indices);

public slots:
    void DataReceived(const std::vector<std::vector<float>>& chunk);

signals:
    void channelCountChanged();
    void showData(const std::vector<std::vector<float>>& chunk);
    void initializeEnded();

private:
    /* to remove */Amplifier* amplifier_ = nullptr;
    AmplifierManager* amplifier_manager_ = nullptr;
};

#endif // EEGVIEWMODEL_H
