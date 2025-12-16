#ifndef GLOBALS_H
#define GLOBALS_H

#include <QObject>
#include <QQmlEngine>

class Globals : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

    Q_PROPERTY(Status status MEMBER m_status NOTIFY statusChanged FINAL)

public:
    explicit Globals(QObject *parent = nullptr);

    enum Status { Ready = 0, Loading};
    Q_ENUM(Status)

signals:
    void statusChanged();

private:
    Status m_status = Ready;
};

#endif // GLOBALS_H
