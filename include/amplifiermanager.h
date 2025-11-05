#ifndef AMPLIFIERMANAGER_H
#define AMPLIFIERMANAGER_H

#include <QByteArray>
#include <QList>
#include <QString>
#include <QProcess>
#include <QThread>

#include "amplifierparser.h"
#include "lslstreamreader.h"

/*
 * @author Szymon Kubica
 * @source https://braintech.pl/pliki/svarog/manuals/Perun32_instrukcja_obslugi.pdf
 */

class AmplifierManager : public QObject
{
    Q_OBJECT

public:
    AmplifierManager(const QString svarog_path, QObject* parent = nullptr);
    ~AmplifierManager();

    QList<AmplifierInfo> GetAmplifiersList();
    void StartStream(const QString amplifier_id);
    void StopStream();

    QString SvarogPath() const;
    void SetSvarogPath(const QString &new_svarog_path);

signals:
    void StartLSLReading();
    void StopLSLReading();

    void AcquisitionStatusChanged();
    void DataReceived(const std::vector<std::vector<float>>& chunk);

private:
    QProcess* stream_process_;
    QString svarog_path_{};

    QThread* lsl_thread_ = nullptr;
    std::unique_ptr<LSLStreamReader> lsl_reader_;
};

#endif // AMPLIFIERMANAGER_H
