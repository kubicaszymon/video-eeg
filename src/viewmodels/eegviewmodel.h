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

struct ChannelBuffer
{
    std::vector<float> values;
    int write_index = 0;
    int count = 0;
    float min_value = 0.0f;
    float max_value = 0.0f;
    bool needs_update = false;
};

class EegViewModel : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(QVariantList channelNames READ GetChannelNames NOTIFY initializeEnded FINAL)
    Q_PROPERTY(int channelCount READ GetChannelCount NOTIFY channelCountChanged FINAL)
    Q_PROPERTY(int maxSamples READ GetMaxSamples CONSTANT FINAL)
    Q_PROPERTY(int sampleRate READ GetSampleRate CONSTANT FINAL)

public:
    explicit EegViewModel(QObject *parent = nullptr);
    ~EegViewModel();

    QVariantList GetChannelNames() const;
    int GetChannelCount() const;
    int GetMaxSamples() const;
    int GetSampleRate() const { return static_cast<int>(sample_rate_); }

    Q_INVOKABLE QVector<float> getChannelData(int channel_index) const;
    Q_INVOKABLE float getChannelMin(int channel_index) const;
    Q_INVOKABLE float getChannelMax(int channel_index) const;

    Q_INVOKABLE void initialize(QString amplifier_id, QVariantList selected_channel_indices);

public slots:
    void UpdateChannelData(const std::vector<std::vector<float>>& chunk);

signals:
    void channelCountChanged();
    void dataUpdated();
    void initializeEnded();

private:
    void updateMinMax(int channel_index);

    Amplifier* amplifier_ = nullptr;
    AmplifierManager* amplifier_manager_ = nullptr;

    std::vector<ChannelBuffer> channel_buffers_;
    mutable QMutex data_mutex_;

    static constexpr int MAX_SAMPLES_PER_CHANNEL = 2560;
    double current_time_ = 0.0;
    double sample_rate_ = 128.0;
    int max_samples_per_channel_ = 640;
};

#endif // EEGVIEWMODEL_H
