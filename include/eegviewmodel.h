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

    Q_PROPERTY(int channel_count READ ChannelCount WRITE SetChannelCount NOTIFY ChannelCountChanged FINAL)
    Q_PROPERTY(bool is_streaming READ IsStreaming WRITE SetIsStreaming NOTIFY IsStreamingChanged FINAL)
    Q_PROPERTY(double sample_rate READ SampleRate WRITE SetSampleRate NOTIFY SampleRateChanged FINAL)
    Q_PROPERTY(int display_window_size READ DisplayWindowSize WRITE SetDisplayWindowSize NOTIFY DisplayWindowSizeChanged FINAL)
public:
    explicit EegViewModel(QObject *parent = nullptr);

    int ChannelCount() const;
    void SetChannelCount(int new_channel_count);

    bool IsStreaming() const;
    void SetIsStreaming(bool new_is_streaming);

    double SampleRate() const;
    void SetSampleRate(double newSample_rate);

    int DisplayWindowSize() const;
    void SetDisplayWindowSize(int newDisplay_window_size);

    Q_INVOKABLE QVariantList GetChannelData(int channel_index) const;
    Q_INVOKABLE QString GetChannelName(int channel_index) const;
    Q_INVOKABLE void ClearAllChannels();

public slots:
    void UpdateChannelData(const std::vector<std::vector<float>>& chunk);
    void StreamStarted();
    void StreamStopped();

signals:
    void ChannelCountChanged();
    void IsStreamingChanged();
    void SampleRateChanged();
    void DisplayWindowSizeChanged();

    void ChannelDataUpdated(int channel_index);
    void AllChannelsUpdated();

private:
    int channel_count_;
    bool is_streaming_;
    double sample_rate_;
    int display_window_size_;
    int max_samples_per_channel_;

    QVector<QVector<QPointF>> channel_data_;
    QVector<QString> channel_names_;

    void InitializeChannels();
    int CalculateMaxSamples() const;

};

#endif // EEGVIEWMODEL_H
