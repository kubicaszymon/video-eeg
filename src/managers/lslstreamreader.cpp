#include "lslstreamreader.h"
#include <QDebug>

LSLStreamReader::LSLStreamReader(QObject* parent)
    : QObject(parent)
{}

LSLStreamReader::~LSLStreamReader()
{
    onStopReading();
    if(inlet_)
    {
        delete inlet_;
    }
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

        auto hej = results[0].as_xml();
        inlet_ = new lsl::stream_inlet(results[0]);
        qDebug() << "Connected to LSL stream";
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
    int sum = 0;

    while(is_running_)
    {
        try
        {
            inlet_->pull_chunk(chunk);
            if(!chunk.empty())
            {
                sum += chunk.size();
                qDebug() << sum;

                emit DataReceived(chunk);
                chunk.clear();
            }
        }
        catch(const std::exception& e)
        {
            emit ErrorOccurred(QString("Read error: %1").arg(e.what()));
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
}


