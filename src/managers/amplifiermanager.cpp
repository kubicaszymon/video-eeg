#include <QDebug>
#include <QRegularExpression>

#include "amplifiermodel.h"
#include "amplifiermanager.h"
#include <lsl_cpp.h>
#include <qtimer.h>

AmplifierManager::AmplifierManager(QObject* parent)
    : QObject(parent)
{

}

AmplifierManager::~AmplifierManager()
{
    StopStream();
}

AmplifierManager *AmplifierManager::instance()
{
    static AmplifierManager instance;
    return &instance;
}

QList<Amplifier> AmplifierManager::RefreshAmplifiersList()
{
    qInfo() << "ODSWIEZAM LISTE AMPLIFIEROW";
    QList<Amplifier> rv{};
    QProcess process;
    process.setProgram(svarog_path_);
    process.setArguments({"-l"});
    process.start();

    if(process.waitForFinished(PROCESS_TIMEOUT_MS))
    {
        QByteArray output = process.readAllStandardOutput();
        rv = ParseRawOutputToAmplifiers(output);
    }
    else
    {
        qDebug() << "Nie udalo sie pozyskac listy wzmacniaczy";
    }

    amplifiers_ = rv;
    return rv;
}

Amplifier* AmplifierManager::GetAmplifierById(QString id)
{
    auto it = std::find_if(amplifiers_.begin(), amplifiers_.end(),[&](Amplifier& amp) {
        return amp.id == id;
    });
    return it != amplifiers_.end() ? &*it : nullptr;
}

void AmplifierManager::StartStream(const QString amplifier_id)
{
    if(stream_process_ && stream_process_->state() == QProcess::Running)
    {
        qDebug() << "Stopping existing stream";
        StopStream();
    }

    stream_process_ = new QProcess(this);

    stream_process_->setProgram(svarog_path_);
    stream_process_->setArguments({"-a", amplifier_id});
    stream_process_->start();

    if(!stream_process_->waitForStarted(PROCESS_TIMEOUT_MS))
    {
        qDebug() << "Failed to start stream process";
        delete stream_process_;
        stream_process_ = nullptr;
        return;
    }

    if(!lsl_thread_)
    {
        lsl_thread_ = new QThread(this);
        lsl_reader_ = std::make_unique<LSLStreamReader>();
        lsl_reader_->moveToThread(lsl_thread_);

        connect(lsl_reader_.get(), &LSLStreamReader::DataReceived, this, &AmplifierManager::onProcessData);
        connect(this, &AmplifierManager::StartLSLReading, lsl_reader_.get(), &LSLStreamReader::onStartReading);
        connect(this, &AmplifierManager::StopLSLReading, lsl_reader_.get(), &LSLStreamReader::onStopReading);

        lsl_thread_->start();
    }

    //@TODO think about something better
    QTimer::singleShot(STREAM_STARTUP_DELAY_MS, this, [this](){
        emit StartLSLReading();
    });
}

void AmplifierManager::StopStream()
{
    //@TODO maybe put disconenct from startstream here?

    qDebug() <<"Stopping Stream";

    if(lsl_reader_)
    {
        emit StopLSLReading();
        QThread::msleep(100);
    }

    if(lsl_thread_)
    {
        if(lsl_thread_->isRunning())
        {
            lsl_thread_->quit();
            if(!lsl_thread_->wait(3000))
            {
                lsl_thread_->terminate();
                lsl_thread_->wait();
            }
        }

        lsl_thread_->deleteLater();
        lsl_thread_ = nullptr;
    }

    lsl_reader_.reset();

    if(stream_process_)
    {
        if(stream_process_->state() == QProcess::Running)
        {
            stream_process_->terminate();
            if(!stream_process_->waitForFinished(3000))
            {
                stream_process_->kill();
                stream_process_->waitForFinished();
            }
        }

        stream_process_->deleteLater();
        stream_process_ = nullptr;
    }
}

QString AmplifierManager::SvarogPath() const
{
    return svarog_path_;
}

void AmplifierManager::SetSvarogPath(const QString &new_svarog_path)
{
    svarog_path_ = new_svarog_path;
}

void AmplifierManager::onProcessData(const std::vector<std::vector<float>>& chunk)
{
    // we get new chunk, we want to process it and send nice data to viewmodel
    emit DataReceived(chunk);
}

QList<Amplifier> AmplifierManager::ParseRawOutputToAmplifiers(const QByteArray& output)
{
    QList<Amplifier> amplifiers;
    QString text = QString::fromUtf8(output);
    QStringList lines = text.split('\n', Qt::SkipEmptyParts);

    Amplifier current_amp;
    bool in_amplifier = false;
    bool in_channels = false;
    bool in_sampling_rates = false;

    for(const QString& line : std::as_const(lines))
    {
        QString trimmed = line.trimmed();

        // Check if we are starting new ampl
        if(trimmed.startsWith("* "))
        {
            if(in_amplifier && !current_amp.name.isEmpty())
            {
                amplifiers.append(current_amp);
            }

            // Start new one
            current_amp = Amplifier();
            current_amp.name = trimmed.mid(2).trimmed(); // remove * prefix
            in_amplifier = true;
            in_channels = false;
            in_sampling_rates = false;
        }
        else if(trimmed.startsWith("id:"))
        {
            static QRegularExpression id_regex(R"(id:\s*"([^"]+)\")");
            QRegularExpressionMatch match = id_regex.match(trimmed);
            if(match.hasMatch())
            {
                current_amp.id = match.captured(1);
            }
        }
        else if(trimmed.contains("available channels:"))
        {
            in_channels = true;
            in_sampling_rates = false;
        }
        else if(trimmed.contains("available sampling rates:"))
        {
            in_channels = false;
            in_sampling_rates = true;
        }
        else if(in_channels && !trimmed.isEmpty() && !trimmed.contains("available"))
        {
            static QRegularExpression re("\\s+");
            QStringList channels = trimmed.split(re, Qt::SkipEmptyParts);
            current_amp.available_channels.append(channels);
        }
        else if(in_sampling_rates && !trimmed.isEmpty() && !trimmed.contains("available"))
        {
            static QRegularExpression re("\\s+");
            QStringList rates = trimmed.split(re, Qt::SkipEmptyParts);
            current_amp.available_samplings.append(rates);
        }
    }

    // Last amp
    if(in_amplifier && !current_amp.name.isEmpty())
    {
        amplifiers.append(current_amp);
    }

    return amplifiers;
}
