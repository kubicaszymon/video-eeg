#ifndef AMPLIFIERMANAGER_H
#define AMPLIFIERMANAGER_H

#include <QByteArray>
#include <QList>
#include <QString>
#include <QProcess>
#include <QThread>

#include "lslstreamreader.h"
#include "amplifiermodel.h"

struct Channel;

/*
 * @author Szymon Kubica
 * @source https://braintech.pl/pliki/svarog/manuals/Perun32_instrukcja_obslugi.pdf
 *
 * This class acts as a mediator between LSLReader and ViewModel
 */

class AmplifierManager : public QObject
{
    Q_OBJECT

public:
    static AmplifierManager* instance();

    AmplifierManager(const AmplifierManager&) = delete;
    AmplifierManager& operator=(const AmplifierManager&) = delete;

    QList<Amplifier> refreshAmplifiersList();
    void refreshAmplifiersListAsync();
    Amplifier* getAmplifierById(const QString& id);

    void startStream(const QString& amplifierId);
    void stopStream();

    QList<Amplifier> parseRawOutputToAmplifiers(const QByteArray& output);

    QString svarogPath() const;
    void setSvarogPath(const QString& newSvarogPath);

signals:
    void startLslReading();
    void stopLslReading();

    void acquisitionStatusChanged();
    void amplifiersListRefreshed(const QList<Amplifier>& amplifiers);
    void dataReceived(const std::vector<std::vector<float>>& chunk);
    void samplingRateDetected(double samplingRate);
    void streamConnected();
    void streamDisconnected();

public slots:
    void onProcessData(const std::vector<std::vector<float>>& chunk);
    void onSamplingRateDetected(double samplingRate);

private slots:
    void onScanProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);

private:
    AmplifierManager(QObject* parent = nullptr);
    ~AmplifierManager();

    QProcess* m_scanProcess = nullptr;
    QProcess* m_streamProcess = nullptr;
    // TODO: Make configurable via settings/options
    QString m_svarogPath{"C:\\Program Files (x86)\\Svarog Streamer\\svarog_streamer\\svarog_streamer.exe"};

    QThread* m_lslThread = nullptr;
    std::unique_ptr<LSLStreamReader> m_lslReader;

    QList<Amplifier> m_amplifiers{};

    static constexpr int PROCESS_TIMEOUT_MS = 3000;
    static constexpr int STREAM_STARTUP_DELAY_MS = 1000;
};

#endif // AMPLIFIERMANAGER_H
