#ifndef AMPLIFIERMANAGER_H
#define AMPLIFIERMANAGER_H

#include <QByteArray>
#include <QList>
#include <QString>
#include <QProcess>
#include <QThread>

#include "lslstreamreader.h"

struct Amplifier;
struct Channel;

/*
 * @author Szymon Kubica
 * @source https://braintech.pl/pliki/svarog/manuals/Perun32_instrukcja_obslugi.pdf
 *
 * This class right now act as a model between LSLReader and ViewModel
 */

class AmplifierManager : public QObject
{
    Q_OBJECT

public:
    static AmplifierManager* instance();

    AmplifierManager(const AmplifierManager&) = delete;
    AmplifierManager& operator=(const AmplifierManager&) = delete;

    QList<Amplifier> RefreshAmplifiersList();
    Amplifier* GetAmplifierById(QString id);

    void StartStream(const QString amplifier_id);
    void StopStream();

    QList<Amplifier> ParseRawOutputToAmplifiers(const QByteArray& output);

    QString SvarogPath() const;
    void SetSvarogPath(const QString &new_svarog_path);

signals:
    void StartLSLReading();
    void StopLSLReading();

    void AcquisitionStatusChanged();
    void DataReceived(const std::vector<std::vector<float>>& chunk);

public slots:
    void onProcessData(const std::vector<std::vector<float>>& chunk);

private:
    AmplifierManager(QObject* parent = nullptr);
    ~AmplifierManager();

    QProcess* stream_process_ = nullptr;
    // TODO DO ZMIANY NA JAKIS SET OPTIONS CZY COS TAKIEGO
    QString svarog_path_{"D:\\Svarog Streamer\\svarog_streamer\\svarog_streamer.exe"};

    QThread* lsl_thread_ = nullptr;
    std::unique_ptr<LSLStreamReader> lsl_reader_;

    QList<Amplifier> amplifiers_{};

    static constexpr int PROCESS_TIMEOUT_MS = 3000;
    static constexpr int STREAM_STARTUP_DELAY_MS = 1000;
};

#endif // AMPLIFIERMANAGER_H
