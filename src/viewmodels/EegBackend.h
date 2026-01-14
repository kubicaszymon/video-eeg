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
    Q_PROPERTY(int amplifierIdx READ amplifierIdx WRITE setAmplifierIdx NOTIFY amplifierIdxChanged FINAL)
    Q_PROPERTY(QString amplifierId READ amplifierId WRITE setAmplifierId NOTIFY amplifierIdChanged FINAL)

    // Zmieniony na double żeby obsłużyć dynamiczne wartości z QML
    Q_PROPERTY(double spacing READ spacing WRITE setSpacing NOTIFY spacingChanged FINAL)

public:
    explicit EegBackend(QObject *parent = nullptr);
    ~EegBackend();

    Q_INVOKABLE void registerDataModel(EegDataModel* dataModel);
    Q_INVOKABLE void startStream();

    Q_INVOKABLE void generateTestData();

    QVariantList GetChannelNames() const;
    QVariantList channels() const;

    void setChannels(const QVariantList &newChannels);

    int amplifierIdx() const;
    void setAmplifierIdx(int newAmplifierIdx);

    double spacing() const;
    void setSpacing(double newSpacing);

    QString amplifierId() const;
    void setAmplifierId(const QString &newAmplifierId);

public slots:
    void DataReceived(const std::vector<std::vector<float>>& chunk);

signals:
    void updateData(const std::vector<std::vector<float>>& chunk);

    void channelsChanged();

    void amplifierIdxChanged();

    void amplifierIdChanged();

    void spacingChanged();

private:
    Amplifier* amplifier_ = nullptr;
    AmplifierManager* amplifier_manager_ = nullptr;
    QVariantList m_channels;
    int m_amplifierIdx = 0;

    EegDataModel* m_dataModel = nullptr;
    double m_spacing = 100.0;  // Domyślna wartość, będzie nadpisana przez QML
    QString m_amplifierId;
};

#endif // EEGBACKEND_H
