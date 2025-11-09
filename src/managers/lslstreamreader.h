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
    void DataReceived(const std::vector<std::vector<float>>& chunk);
    void ErrorOccurred(const QString& error);
    void StreamConnected();
    void StreamDisconnected();

public slots:
    void onStartReading();
    void onStopReading();

private:
    void ReadLoop();

    std::atomic<bool> is_running_{false};
    lsl::stream_inlet* inlet_ = nullptr;
};

#endif // LSLSTREAMREADER_H
