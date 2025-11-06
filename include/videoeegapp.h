#ifndef VIDEOEEGAPP_H
#define VIDEOEEGAPP_H

#include <QObject>
#include <QQmlApplicationEngine>
#include <memory>

#include "../include/amplifiermanager.h"
#include "../include/eegviewmodel.h"

class VideoEegApp : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QStringList amplifierNames READ amplifierNames NOTIFY amplifiersChanged)
    Q_PROPERTY(int selectedAmplifierIndex READ selectedAmplifierIndex WRITE setSelectedAmplifierIndex NOTIFY selectedAmplifierChanged)
    Q_PROPERTY(QStringList currentAmplifierChannels READ currentAmplifierChannels NOTIFY selectedAmplifierChanged)

public:
    explicit VideoEegApp(QObject *parent = nullptr);
    ~VideoEegApp();

    bool Initialize();
    void Run();
    void Shutdown();

    void InitializeAmplifier(const QString amp_path);

    Q_INVOKABLE void refreshAmplifiersList();

    QStringList amplifierNames() const;
    QStringList currentAmplifierChannels() const;
    int selectedAmplifierIndex() const;

    void setSelectedAmplifierIndex(int index);

signals:
    void amplifiersChanged();
    void selectedAmplifierChanged();

private:
    void CreateComponents();
    void SetupConnections();
    void RegisterQmlTypes();
    void LoadQml();

    std::unique_ptr<QQmlApplicationEngine> engine_;
    std::unique_ptr<AmplifierManager> amplifier_manager_;
    std::unique_ptr<EegViewModel> eeg_view_model_;

    bool is_initialized_ = false;

    QList<AmplifierInfo> available_amplifiers_;
    int selected_amplifier_index_ = -1;
};

#endif // VIDEOEEGAPP_H
