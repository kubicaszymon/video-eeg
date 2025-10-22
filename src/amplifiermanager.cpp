#include "../include/amplifiermanager.h"
#include <QProcess>
#include <QDebug>
#include <QRegularExpression>

AmplifierManager::AmplifierManager() {

}

AmplifierManager::~AmplifierManager() noexcept {

}

QList<QString> AmplifierManager::GetAmplifiersList()
{
    QList<QString> rv{};
    auto program_directory = "C:\\Program Files (x86)\\Svarog Streamer\\svarog_streamer\\svarog_streamer.exe";
    QProcess run;
    run.start(program_directory, {"-l"});

    if(run.waitForFinished(3000)) {
        QByteArray output = run.readAllStandardOutput();
        rv = this->GetAmplifierNames(output);
    }
    else {
        qDebug() << "Nie udalo sie pozyskac listy wzmacniaczy";
    }

    return rv;
}

QList<QString> AmplifierManager::GetAmplifierNames(const QByteArray &output)
{
    QList<QString> rv{};

    QRegularExpression re(R"(\*\s*([^\n]+)\s+id: "[^"]+")");
    QRegularExpressionMatchIterator i = re.globalMatch(output);
    while(i.hasNext()) {
        QRegularExpressionMatch match = i.next();
        QString name = match.captured(1);
        if(!name.isEmpty()) {
            rv.append(name);
        }
    }

    return rv;
}
