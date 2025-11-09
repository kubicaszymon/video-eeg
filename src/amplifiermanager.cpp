#include "../include/amplifiermanager.h"
#include <QDebug>
#include <QRegularExpression>
#include <lsl_cpp.h>

AmplifierManager::AmplifierManager(const QString svarog_path, QObject* parent)
    : QObject(parent), svarog_path_{svarog_path}
{
    lsl_thread_ = new QThread(this);
    lsl_reader_ = std::make_unique<LSLStreamReader>();
    lsl_reader_->moveToThread(lsl_thread_);

    connect(this, &AmplifierManager::StartLSLReading, lsl_reader_.get(), &LSLStreamReader::StartReading);
    connect(this, &AmplifierManager::StopLSLReading, lsl_reader_.get(), &LSLStreamReader::StopReading);

    connect(lsl_reader_.get(), &LSLStreamReader::DataReceived, this, &AmplifierManager::ProcessData);
}

AmplifierManager::~AmplifierManager()
{
    StopStream();
}

QList<AmplifierInfo> AmplifierManager::GetAmplifiersList()
{
    QList<AmplifierInfo> rv{};
    QProcess process;
    process.setProgram(svarog_path_);
    process.setArguments({"-l"});
    process.start();

    if(process.waitForFinished(3000))
    {
        QByteArray output = process.readAllStandardOutput();
        rv = AmplifierParser::Parse(output);
    }
    else
    {
        qDebug() << "Nie udalo sie pozyskac listy wzmacniaczy";
    }

    return rv;
}

void AmplifierManager::StartStream(const QString amplifier_id, QList<quint8> selected_channels)
{
    if(stream_process_ && stream_process_->state() == QProcess::Running)
    {
        StopStream();
    }

    if(!stream_process_)
    {
        stream_process_ = new QProcess(this);
    }

    selected_channels_ = selected_channels;

    stream_process_->setProgram(svarog_path_);
    stream_process_->setArguments({"-a", amplifier_id});
    stream_process_->start();

    // TO CHANGE IN FUTURE TO SOME BETTER MECHANISM (WAIT FOR DATA)
    QThread::msleep(1000);

    lsl_thread_->start();
    emit StartLSLReading();
}

void AmplifierManager::StopStream()
{
    emit StopLSLReading();

    if(stream_process_ && stream_process_->state() == QProcess::Running)
    {
        stream_process_->terminate();
        stream_process_->waitForFinished();
        stream_process_ = nullptr;
    }

    lsl_thread_->quit();
    lsl_thread_->wait();
}

QString AmplifierManager::SvarogPath() const
{
    return svarog_path_;
}

void AmplifierManager::SetSvarogPath(const QString &new_svarog_path)
{
    svarog_path_ = new_svarog_path;
}

void AmplifierManager::ProcessData(const std::vector<std::vector<float>>& chunk)
{
    qDebug() << "PROCESS DATA";
    emit DataReceived(chunk);
    /*
    for(auto& sample : chunk)
    {
        std::vector<float> filtered;
        filtered.reserve(selected_channels_.size());
        for(int channel : selected_channels_)
        {
            if(channel >= 0 && channel < static_cast<int>(sample.size()))
            {
                filtered.push_back(sample[channel]);
            }
        }
        sample = std::move(filtered);
    }
    */
}
