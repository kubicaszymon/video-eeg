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
    if(m_isRunning)
    {
        qDebug() << "LSL reader already running";
        return;
    }

    try {
        qDebug() << "Resolving LSL stream";
        std::vector<lsl::stream_info> results = lsl::resolve_stream("type", "EEG", 1, 5.0);

        if(results.empty())
        {
            emit errorOccurred("No EEG stream found");
            return;
        }

        lsl::stream_info info = results[0];
        double samplingRate = info.nominal_srate();
        qDebug() << "LSL stream sampling rate:" << samplingRate << "Hz";

        m_inlet = new lsl::stream_inlet(info);
        qDebug() << "Connected to LSL stream";

        emit inletReady(m_inlet);
        emit samplingRateDetected(samplingRate);
        emit streamConnected();

        m_isRunning = true;
        readLoop();
    }
    catch (const std::exception& e)
    {
        emit errorOccurred(QString("LSL error: %1").arg(e.what()));
    }
}

void LSLStreamReader::onStopReading()
{
    m_isRunning = false;

    if(m_inlet)
    {
        emit inletReady(nullptr);
        delete m_inlet;
        m_inlet = nullptr;
        emit streamDisconnected();
    }
}

void LSLStreamReader::readLoop()
{
    std::vector<std::vector<float>> chunk;
    std::vector<double> timestamps;

    while(m_isRunning)
    {
        try
        {
            m_inlet->pull_chunk(chunk, timestamps);
            if(!chunk.empty())
            {
                emit dataReceived(chunk, timestamps);
                chunk.clear();
                timestamps.clear();
            }
        }
        catch(const std::exception& e)
        {
            emit errorOccurred(QString("Read error: %1").arg(e.what()));
        }
        QThread::sleep(std::chrono::milliseconds(20));
    }
}
