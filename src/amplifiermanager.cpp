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

    connect(lsl_reader_.get(), &LSLStreamReader::DataReceived, this, [this](const std::vector<std::vector<float>>& chunk){
        emit DataReceived(chunk);
    });
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

void AmplifierManager::StartStream(const QString amplifier_id)
{
    if(stream_process_ && stream_process_->state() == QProcess::Running)
    {
        StopStream();
    }

    if(!stream_process_)
    {
        stream_process_ = new QProcess(this);
    }

    stream_process_->setProgram(svarog_path_);
    stream_process_->setArguments({"-a", amplifier_id});
    stream_process_->start();

    // TO CHANGE IN FUTURE TO SOME BETTER MECHANISM (WAIT FOR DATA)
    QThread::msleep(1000);

    emit StartLSLReading();
}

void AmplifierManager::StopStream()
{
    emit StopLSLReading();

    if(stream_process_ && stream_process_->state() == QProcess::Running)
    {
        stream_process_->terminate();
        stream_process_->waitForFinished();
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
