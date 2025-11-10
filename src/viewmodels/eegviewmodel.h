#ifndef EEGVIEWMODEL_H
#define EEGVIEWMODEL_H

#include <QObject>
#include <QVariantMap>
#include <QVector>
#include <QPointF>
#include <QtQml/qqmlregistration.h>
#include "amplifiermodel.h"
#include "amplifiermanager.h"

class EegViewModel : public QObject
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(int channelCount READ GetChannelCount WRITE SetChannelCount NOTIFY channelCountChanged FINAL)

public:
    explicit EegViewModel(QObject *parent = nullptr);

    int GetChannelCount() const;
    void SetChannelCount(int new_channel_count);

    Q_INVOKABLE void initialize(QString amplifier_id, QVariantList selected_channel_indices);
    Q_INVOKABLE QVariantMap getChannelRenderData(int channel_index) const;
    Q_INVOKABLE QString getChannelName(int channel_index) const;

public slots:
    void UpdateChannelData(const std::vector<std::vector<float>>& chunk);
    void StreamStarted();
    void StreamStopped();

signals:
    void channelCountChanged();
    void allChannelsUpdated();

private:
    Amplifier amplifier_;
    AmplifierManager* amplifier_manager_ = nullptr;

    int channel_count_;
    QVector<QVector<QPointF>> channel_data_; // [channel_count][max_samples_per_channel_]
    QVector<QString> channel_names_;

    double sample_rate_ = 128;
    int max_samples_per_channel_ = 512;
    bool is_streaming_;
    double current_time_ = 0.0;
};

#endif // EEGVIEWMODEL_H
