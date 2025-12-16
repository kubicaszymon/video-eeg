#include "globals.h"

Globals::Globals(QObject *parent)
    : QObject{parent}
{
    qInfo() << "Singleton " << this << " created!";
}
