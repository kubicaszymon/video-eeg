#ifndef EEGBACKEND_H
#define EEGBACKEND_H

#include <QObject>
#include <QVariantMap>
#include <QVector>
#include <QtQml/qqmlregistration.h>
#include "amplifiermodel.h"
#include "amplifiermanager.h"
#include "eegdatamodel.h"

class EegBackend : public QObject
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(QVariantList channels READ channels WRITE setChannels NOTIFY channelsChanged FINAL)
    Q_PROPERTY(int amplifierId READ amplifierId WRITE setAmplifierId NOTIFY amplifierIdChanged FINAL)

    Q_PROPERTY(int spacing READ spacing WRITE setSpacing FINAL)

public:
    explicit EegBackend(QObject *parent = nullptr);
    ~EegBackend();

    Q_INVOKABLE void registerDataModel(EegDataModel* dataModel);

    Q_INVOKABLE void generateTestData();

    QVariantList GetChannelNames() const;
    QVariantList channels() const;

    void setChannels(const QVariantList &newChannels);

    int amplifierId() const;
    void setAmplifierId(int newAmplifierId);

    int spacing() const;
    void setSpacing(int newSpacing);

public slots:
    void DataReceived(const std::vector<std::vector<float>>& chunk);

signals:
    void updateData(const std::vector<std::vector<float>>& chunk);

    void channelsChanged();

    void amplifierIdChanged();

private:
    Amplifier* amplifier_ = nullptr;
    AmplifierManager* amplifier_manager_ = nullptr;
    QVariantList m_channels;
    int m_amplifierId;

    EegDataModel* m_dataModel = nullptr;
    int m_spacing = 5;
};

#endif // EEGBACKEND_H
