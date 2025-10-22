#ifndef AMPLIFIERMANAGER_H
#define AMPLIFIERMANAGER_H

#include <QByteArray>
#include <QList>
#include <QString>

class AmplifierManager
{
public:
    explicit AmplifierManager();
    ~AmplifierManager() noexcept;

    QList<QString> GetAmplifiersList();

private:
    QList<QString> GetAmplifierNames(const QByteArray& output);
};

#endif // AMPLIFIERMANAGER_H
