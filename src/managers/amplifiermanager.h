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
 */

class AmplifierManager : public QObject
{
    Q_OBJECT

public:
    static AmplifierManager* instance();

    AmplifierManager(const AmplifierManager&) = delete;
    AmplifierManager& operator=(const AmplifierManager&) = delete;

    QList<Amplifier> GetAmplifiersList();
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
    QString svarog_path_{"C:\\Program Files (x86)\\Svarog Streamer\\svarog_streamer\\svarog_streamer.exe"};

    QThread lsl_thread_;
    std::unique_ptr<LSLStreamReader> lsl_reader_;
};

#endif // AMPLIFIERMANAGER_H
