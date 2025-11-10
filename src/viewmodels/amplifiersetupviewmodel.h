#ifndef AMPLIFIERSETUPVIEWMODEL_H
#define AMPLIFIERSETUPVIEWMODEL_H

#include <QObject>
#include <QProperty>
#include <QVariant>
#include <QtQml/qqmlregistration.h>
#include "amplifiermodel.h"
#include "amplifiermanager.h"

class AmplifierSetupViewModel : public QObject
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(QVariantList availableAmplifiers READ getAvailableAmplifiers NOTIFY availableAmplifiersChanged FINAL)
    Q_PROPERTY(int selectedAmplifierIndex READ getSelectedAmplifierIndex WRITE setSelectedAmplifierIndex NOTIFY selectedAmplifierIndexChanged FINAL)
    Q_PROPERTY(QString selectedAmplifierId READ getSelectedAmplifierId NOTIFY selectedAmplifierIndexChanged FINAL)
    Q_PROPERTY(QVariantList currentChannels READ getCurrentChannels NOTIFY selectedAmplifierIndexChanged FINAL)

public:
    explicit AmplifierSetupViewModel(QObject *parent = nullptr);

    QVariantList getAvailableAmplifiers() const;
    QString getSelectedAmplifierId() const;
    int getSelectedAmplifierIndex() const;
    QVariantList getCurrentChannels() const;

    Q_INVOKABLE void initialize();
    Q_INVOKABLE void setSelectedAmplifierIndex(int index);

signals:
    void availableAmplifiersChanged();
    void selectedAmplifierIndexChanged();
    void currentChannelsChanged();

private:
    const Amplifier* GetCurrentAmplifier() const;

    AmplifierManager* manager_;

    QList<Amplifier> amplifiers_;
    QProperty<int> selected_amplifier_index_{-1};
};

#endif // AMPLIFIERSETUPVIEWMODEL_H
