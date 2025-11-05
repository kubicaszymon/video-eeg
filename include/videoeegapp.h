#ifndef VIDEOEEGAPP_H
#define VIDEOEEGAPP_H

#include <QObject>
#include <QQmlApplicationEngine>
#include <memory>

class AmplifierManager;
class LSLStreamerReader;
class EegViewModel;

class VideoEegApp : public QObject
{
    Q_OBJECT
public:
    explicit VideoEegApp(QObject *parent = nullptr);
    ~VideoEegApp();

    bool Initialize();
    void Run();
    void Shutdown();

    void InitializeAmplifier(const QString amp_path);
private:
    void CreateComponents();
    void SetupConnections();
    void RegisterQmlTypes();
    void LoadQml();

    std::unique_ptr<QQmlApplicationEngine> engine_;
    std::unique_ptr<AmplifierManager> amplifier_manager_;
    std::unique_ptr<EegViewModel> eeg_view_model_;

    bool is_initialized_ = false;
};

#endif // VIDEOEEGAPP_H
