#ifndef AMPLIFIERMODEL_H
#define AMPLIFIERMODEL_H

#include <QStringList>

struct Amplifier
{
    QString name;
    QString id;
    QStringList available_channels;
    QStringList available_samplings;
};

#endif
