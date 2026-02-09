#include <QDebug>
#include <QRegularExpression>

#include "amplifiermodel.h"
#include "amplifiermanager.h"
#include "eegsyncmanager.h"
#include <lsl_cpp.h>
#include <qtimer.h>

AmplifierManager::AmplifierManager(QObject* parent)
    : QObject(parent)
{

}

AmplifierManager::~AmplifierManager()
{
    stopStream();
}

AmplifierManager *AmplifierManager::instance()
{
    static AmplifierManager instance;
    return &instance;
}

QList<Amplifier> AmplifierManager::refreshAmplifiersList()
{
    qInfo() << "Refreshing amplifiers list";
    QList<Amplifier> rv{};
    QProcess process;
    process.setProgram(m_svarogPath);
    process.setArguments({"-l"});
    process.start();

    if(process.waitForFinished(PROCESS_TIMEOUT_MS))
    {
        QByteArray output = process.readAllStandardOutput();
        rv = parseRawOutputToAmplifiers(output);
    }
    else
    {
        qDebug() << "Failed to retrieve amplifiers list";
    }

    m_amplifiers = rv;
    return rv;
}

void AmplifierManager::refreshAmplifiersListAsync()
{
    qInfo() << "Starting async amplifier scan...";

    if(m_scanProcess && m_scanProcess->state() == QProcess::Running)
    {
        qDebug() << "Scan already in progress";
        return;
    }

    if(m_scanProcess)
    {
        m_scanProcess->deleteLater();
        m_scanProcess = nullptr;
    }

    m_scanProcess = new QProcess(this);
    m_scanProcess->setProgram(m_svarogPath);
    m_scanProcess->setArguments({"-l"});

    connect(m_scanProcess, &QProcess::finished, this, &AmplifierManager::onScanProcessFinished);

    m_scanProcess->start();
}

void AmplifierManager::onScanProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    Q_UNUSED(exitCode)
    Q_UNUSED(exitStatus)

    QList<Amplifier> rv{};

    if(m_scanProcess)
    {
        QByteArray output = m_scanProcess->readAllStandardOutput();
        rv = parseRawOutputToAmplifiers(output);
        m_amplifiers = rv;

        m_scanProcess->deleteLater();
        m_scanProcess = nullptr;
    }

    emit amplifiersListRefreshed(rv);
}

Amplifier* AmplifierManager::getAmplifierById(const QString& id)
{
    auto it = std::find_if(m_amplifiers.begin(), m_amplifiers.end(),[&](Amplifier& amp) {
        return amp.id == id;
    });
    return it != m_amplifiers.end() ? &*it : nullptr;
}

void AmplifierManager::startStream(const QString& amplifierId)
{
    if(m_streamProcess && m_streamProcess->state() == QProcess::Running)
    {
        qDebug() << "Stopping existing stream";
        stopStream();
    }

    m_streamProcess = new QProcess(this);

    m_streamProcess->setProgram(m_svarogPath);
    m_streamProcess->setArguments({"-a", amplifierId});
    m_streamProcess->start();

    if(!m_streamProcess->waitForStarted(PROCESS_TIMEOUT_MS))
    {
        qDebug() << "Failed to start stream process";
        delete m_streamProcess;
        m_streamProcess = nullptr;
        return;
    }

    if(!m_lslThread)
    {
        m_lslThread = new QThread(this);
        m_lslReader = std::make_unique<LSLStreamReader>();
        m_lslReader->moveToThread(m_lslThread);

        connect(m_lslReader.get(), &LSLStreamReader::dataReceived, this, &AmplifierManager::onProcessData);
        connect(m_lslReader.get(), &LSLStreamReader::inletReady, this, [](lsl::stream_inlet* inlet) {
            EegSyncManager::instance()->setLslInlet(inlet);
        });
        connect(m_lslReader.get(), &LSLStreamReader::samplingRateDetected, this, &AmplifierManager::onSamplingRateDetected);
        connect(m_lslReader.get(), &LSLStreamReader::streamConnected, this, &AmplifierManager::streamConnected);
        connect(m_lslReader.get(), &LSLStreamReader::streamDisconnected, this, &AmplifierManager::streamDisconnected);
        connect(this, &AmplifierManager::startLslReading, m_lslReader.get(), &LSLStreamReader::onStartReading);
        connect(this, &AmplifierManager::stopLslReading, m_lslReader.get(), &LSLStreamReader::onStopReading);

        m_lslThread->start();
    }

    // @TODO: Consider a more robust synchronization mechanism
    QTimer::singleShot(STREAM_STARTUP_DELAY_MS, this, [this](){
        emit startLslReading();
    });
}

void AmplifierManager::stopStream()
{
    // @TODO: Consider moving disconnect logic from startStream here

    qDebug() << "Stopping stream";

    if(m_lslReader)
    {
        emit stopLslReading();
        QThread::msleep(100);
    }

    if(m_lslThread)
    {
        if(m_lslThread->isRunning())
        {
            m_lslThread->quit();
            if(!m_lslThread->wait(3000))
            {
                m_lslThread->terminate();
                m_lslThread->wait();
            }
        }

        m_lslThread->deleteLater();
        m_lslThread = nullptr;
    }

    m_lslReader.reset();

    if(m_streamProcess)
    {
        if(m_streamProcess->state() == QProcess::Running)
        {
            m_streamProcess->terminate();
            if(!m_streamProcess->waitForFinished(3000))
            {
                m_streamProcess->kill();
                m_streamProcess->waitForFinished();
            }
        }

        m_streamProcess->deleteLater();
        m_streamProcess = nullptr;
    }
}

QString AmplifierManager::svarogPath() const
{
    return m_svarogPath;
}

void AmplifierManager::setSvarogPath(const QString& newSvarogPath)
{
    m_svarogPath = newSvarogPath;
}

void AmplifierManager::onProcessData(const std::vector<std::vector<float>>& chunk,
                                      const std::vector<double>& timestamps)
{
    emit dataReceived(chunk, timestamps);
}

void AmplifierManager::onSamplingRateDetected(double samplingRate)
{
    qDebug() << "AmplifierManager: Sampling rate detected:" << samplingRate << "Hz";
    emit AmplifierManager::samplingRateDetected(samplingRate);
}

QList<Amplifier> AmplifierManager::parseRawOutputToAmplifiers(const QByteArray& output)
{
    QList<Amplifier> amplifiers;
    QString text = QString::fromUtf8(output);
    QStringList lines = text.split('\n', Qt::SkipEmptyParts);

    Amplifier currentAmp;
    bool inAmplifier = false;
    bool inChannels = false;
    bool inSamplingRates = false;

    for(const QString& line : std::as_const(lines))
    {
        QString trimmed = line.trimmed();

        // Check if we are starting a new amplifier
        if(trimmed.startsWith("* "))
        {
            if(inAmplifier && !currentAmp.name.isEmpty())
            {
                amplifiers.append(currentAmp);
            }

            currentAmp = Amplifier();
            currentAmp.name = trimmed.mid(2).trimmed();
            inAmplifier = true;
            inChannels = false;
            inSamplingRates = false;
        }
        else if(trimmed.startsWith("id:"))
        {
            static QRegularExpression idRegex(R"(id:\s*"([^"]+)\")");
            QRegularExpressionMatch match = idRegex.match(trimmed);
            if(match.hasMatch())
            {
                currentAmp.id = match.captured(1);
            }
        }
        else if(trimmed.contains("available channels:"))
        {
            inChannels = true;
            inSamplingRates = false;
        }
        else if(trimmed.contains("available sampling rates:"))
        {
            inChannels = false;
            inSamplingRates = true;
        }
        else if(inChannels && !trimmed.isEmpty() && !trimmed.contains("available"))
        {
            static QRegularExpression re("\\s+");
            QStringList channels = trimmed.split(re, Qt::SkipEmptyParts);
            currentAmp.available_channels.append(channels);
        }
        else if(inSamplingRates && !trimmed.isEmpty() && !trimmed.contains("available"))
        {
            static QRegularExpression re("\\s+");
            QStringList rates = trimmed.split(re, Qt::SkipEmptyParts);
            currentAmp.available_samplings.append(rates);
        }
    }

    // Last amplifier
    if(inAmplifier && !currentAmp.name.isEmpty())
    {
        amplifiers.append(currentAmp);
    }

    return amplifiers;
}
