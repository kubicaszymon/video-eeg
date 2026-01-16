#include "lslstreamreader.h"
#include <QDebug>

LSLStreamReader::LSLStreamReader(QObject* parent)
    : QObject(parent)
{}

LSLStreamReader::~LSLStreamReader()
{
    onStopReading();
}

void LSLStreamReader::onStartReading()
{
    if(is_running_)
    {
        qDebug() << "LSL reader already running";
        return;
    }

    try {
        qDebug() << "Resolving LSL STREAM";
        std::vector<lsl::stream_info> results = lsl::resolve_stream("type", "EEG", 1, 5.0);

        if(results.empty())
        {
            emit ErrorOccurred("No EEG stream found");
            return;
        }

        lsl::stream_info info = results[0];
        double samplingRate = info.nominal_srate();
        qDebug() << "LSL stream sampling rate:" << samplingRate << "Hz";

        inlet_ = new lsl::stream_inlet(info);
        qDebug() << "Connected to LSL stream";
        emit SamplingRateDetected(samplingRate);
        emit StreamConnected();

        is_running_ = true;
        ReadLoop();
    }
    catch (const std::exception& e)
    {
        emit ErrorOccurred(QString("LSL error: %1").arg(e.what()));
    }
}

void LSLStreamReader::onStopReading()
{
    is_running_ = false;

    if(inlet_)
    {
        delete inlet_;
        inlet_ = nullptr;
        emit StreamDisconnected();
    }
}

void LSLStreamReader::ReadLoop()
{
    std::vector<std::vector<float>> chunk;

    while(is_running_)
    {
        try
        {
            inlet_->pull_chunk(chunk);
            if(!chunk.empty())
            {
                emit DataReceived(chunk);
                chunk.clear();
            }
        }
        catch(const std::exception& e)
        {
            emit ErrorOccurred(QString("Read error: %1").arg(e.what()));
        }
        QThread::sleep(std::chrono::milliseconds(20));
    }
}


