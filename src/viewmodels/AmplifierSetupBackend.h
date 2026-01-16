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
    Q_PROPERTY(QVariantList currentChannels READ getCurrentChannels NOTIFY selectedAmplifierIndexChanged FINAL)
    Q_PROPERTY(bool isLoading READ isLoading NOTIFY isLoadingChanged FINAL)

public:
    explicit AmplifierSetupBackend(QObject *parent = nullptr);
    ~AmplifierSetupBackend();

    QVariantList getAvailableAmplifiers() const;
    int getSelectedAmplifierIndex() const;
    QVariantList getCurrentChannels() const;

    Q_INVOKABLE void refreshAmplifiersList();
    Q_INVOKABLE void setSelectedAmplifierIndex(int index);
    Q_INVOKABLE QString getSelectedAmplifierId() const;

    bool isLoading() const { return m_isLoading; }

signals:
    void availableAmplifiersChanged();
    void selectedAmplifierIndexChanged();
    void currentChannelsChanged();
    void isLoadingChanged();

private:
    const Amplifier* GetCurrentAmplifier() const;

    AmplifierManager* m_manager;

    QList<Amplifier> m_amplifiers;
    QProperty<int> m_selectedAmplifierIndex{-1};
    bool m_isLoading = false;
};

#endif // AMPLIFIERSETUPBACKEND_H
