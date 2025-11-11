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
    Q_PROPERTY(QVariantList channelNames READ GetChannelNames NOTIFY initializeEnded FINAL)

public:
    explicit EegViewModel(QObject *parent = nullptr);

    QVariantList GetChannelNames() const;

    Q_INVOKABLE void initialize(QString amplifier_id, QVariantList selected_channel_indices);
    Q_INVOKABLE QVariantMap getChannelRenderData(int channel_index) const;

public slots:
    void UpdateChannelData(const std::vector<std::vector<float>>& chunk);
    void StreamStarted();
    void StreamStopped();

signals:
    void channelCountChanged();
    void channelDataChanged(int channelIndex);
    void initializeEnded();

private:
    void normalizeChannelData(int channel_index);

    Amplifier* amplifier_ = nullptr;
    AmplifierManager* amplifier_manager_ = nullptr;

    QVector<QVector<QPointF>> channel_data_; // [channel_count][max_samples_per_channel_]
    QVector<QVariantMap> channel_render_cache_; // prenormalized render data

    double sample_rate_ = 128;
    int max_samples_per_channel_ = 512;
    bool is_streaming_;
    double current_time_ = 0.0;

    QTimer* update_timer_ = nullptr;
    QSet<int> dirty_channels_;
    int batch_interval_ms_ = 50;
};

#endif // EEGVIEWMODEL_H
