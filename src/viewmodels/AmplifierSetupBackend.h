#ifndef AMPLIFIERSETUPBACKEND_H
#define AMPLIFIERSETUPBACKEND_H

#include <QObject>
#include <QProperty>
#include <QVariant>
#include <QtQml/qqmlregistration.h>
#include "amplifiermodel.h"
#include "amplifiermanager.h"

class AmplifierSetupBackend : public QObject
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(QVariantList availableAmplifiers READ getAvailableAmplifiers NOTIFY availableAmplifiersChanged FINAL)
    Q_PROPERTY(int selectedAmplifierIndex READ getSelectedAmplifierIndex WRITE setSelectedAmplifierIndex NOTIFY selectedAmplifierIndexChanged FINAL)
    Q_PROPERTY(QString selectedAmplifierId READ getSelectedAmplifierId NOTIFY selectedAmplifierIndexChanged FINAL)
    Q_PROPERTY(QVariantList currentChannels READ getCurrentChannels NOTIFY selectedAmplifierIndexChanged FINAL)

public:
    explicit AmplifierSetupBackend(QObject *parent = nullptr);
    ~AmplifierSetupBackend();

    QVariantList getAvailableAmplifiers() const;
    QString getSelectedAmplifierId() const;
    int getSelectedAmplifierIndex() const;
    QVariantList getCurrentChannels() const;

    Q_INVOKABLE void refreshAmplifiersList();
    Q_INVOKABLE void setSelectedAmplifierIndex(int index);

signals:
    void availableAmplifiersChanged();
    void selectedAmplifierIndexChanged();
    void currentChannelsChanged();

private:
    const Amplifier* GetCurrentAmplifier() const;

    AmplifierManager* m_manager;

    QList<Amplifier> m_amplifiers;
    QProperty<int> m_selectedAmplifierIndex{-1};
};

#endif // AMPLIFIERSETUPBACKEND_H
