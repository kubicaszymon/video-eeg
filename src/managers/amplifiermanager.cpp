#include <QDebug>
#include <QRegularExpression>

#include "amplifiermodel.h"
#include "amplifiermanager.h"
#include <lsl_cpp.h>

AmplifierManager::AmplifierManager(QObject* parent)
    : QObject(parent)
{
    lsl_reader_ = std::make_unique<LSLStreamReader>();
    lsl_reader_->moveToThread(&lsl_thread_);

    connect(lsl_reader_.get(), &LSLStreamReader::DataReceived, this, &AmplifierManager::onProcessData);

    connect(this, &AmplifierManager::StartLSLReading, lsl_reader_.get(), &LSLStreamReader::onStartReading);
    connect(this, &AmplifierManager::StopLSLReading, lsl_reader_.get(), &LSLStreamReader::onStopReading);
}

AmplifierManager::~AmplifierManager()
{
    StopStream();
}

QList<Amplifier> AmplifierManager::GetAmplifiersList()
{
    QList<Amplifier> rv{};
    QProcess process;
    process.setProgram(svarog_path_);
    process.setArguments({"-l"});
    process.start();

    if(process.waitForFinished(3000))
    {
        QByteArray output = process.readAllStandardOutput();
        rv = ParseRawOutputToAmplifiers(output);
    }
    else
    {
        qDebug() << "Nie udalo sie pozyskac listy wzmacniaczy";
    }

    return rv;
}

AmplifierManager *AmplifierManager::instance()
{
    static AmplifierManager instance;
    return &instance;
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

    lsl_thread_.start();
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

    lsl_thread_.quit();
    lsl_thread_.wait();
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
