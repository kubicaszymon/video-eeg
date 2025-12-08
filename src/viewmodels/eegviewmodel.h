#ifndef EEGVIEWMODEL_H
#define EEGVIEWMODEL_H

#include <QObject>
#include <QVariantMap>
#include <QVector>
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
    void initializeEnded();
    void channelCountChanged();
    void updateData(const std::vector<std::vector<float>>& chunk);

private:
    Amplifier* amplifier_ = nullptr;
    AmplifierManager* amplifier_manager_ = nullptr;
};

#endif // EEGVIEWMODEL_H
