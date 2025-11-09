#ifndef EEGVIEWMODEL_H
#define EEGVIEWMODEL_H

#include <QObject>
#include <QVariantList>
#include <QVector>
#include <QPointF>
#include <QTimer>

class EegViewModel : public QObject
{
    Q_OBJECT

    Q_PROPERTY(int channel_count READ ChannelCount WRITE SetChannelCount NOTIFY channelCountChanged FINAL)

public:
    explicit EegViewModel(QObject *parent = nullptr);

    void Initialize(QStringList channels);

    int ChannelCount() const;
    void SetChannelCount(int new_channel_count);

    Q_INVOKABLE QVariantList getChannelData(int channel_index) const;
    Q_INVOKABLE QString getChannelName(int channel_index) const;

public slots:
    void UpdateChannelData(const std::vector<std::vector<float>>& chunk);
    void StreamStarted();
    void StreamStopped();

signals:
    void channelCountChanged();

    void allChannelsUpdated();

private:
    int channel_count_;
    QVector<QVector<QPointF>> channel_data_; // [channel_count][max_samples_per_channel_]
    QVector<QString> channel_names_;

    double sample_rate_ = 128;
    int max_samples_per_channel_ = 512;

    bool is_streaming_;

    double current_time_ = 0.0;
};

#endif // EEGVIEWMODEL_H
