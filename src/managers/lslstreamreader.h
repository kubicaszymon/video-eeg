#ifndef LSLSTREAMREADER_H
#define LSLSTREAMREADER_H

#include <QObject>
#include <QThread>
#include <lsl_cpp.h>
#include <vector>
#include <atomic>

class LSLStreamReader : public QObject
{
    Q_OBJECT

public:
    explicit LSLStreamReader(QObject* parent = nullptr);
    ~LSLStreamReader();

signals:
    void dataReceived(const std::vector<std::vector<float>>& chunk);
    void dataReceivedWithTimestamps(const std::vector<std::vector<float>>& chunk,
                                     const std::vector<double>& timestamps);
    void errorOccurred(const QString& error);
    void streamConnected();
    void streamDisconnected();
    void samplingRateDetected(double samplingRate);

public slots:
    void onStartReading();
    void onStopReading();

private:
    void readLoop();

    std::atomic<bool> m_isRunning{false};
    lsl::stream_inlet* m_inlet = nullptr;
};

#endif // LSLSTREAMREADER_H
