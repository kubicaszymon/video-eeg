#ifndef EEGBACKEND_H
#define EEGBACKEND_H

#include <QObject>
#include <QVariantMap>
#include <QVector>
#include <QList>
#include <QtQml/qqmlregistration.h>
#include "amplifiermodel.h"
#include "amplifiermanager.h"
#include "eegdatamodel.h"
#include "markermanager.h"

class EegBackend : public QObject
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(QVariantList channels READ channels WRITE setChannels NOTIFY channelsChanged FINAL)
    Q_PROPERTY(int amplifierIdx READ amplifierIdx WRITE setAmplifierIdx NOTIFY amplifierIdxChanged FINAL)
    Q_PROPERTY(QString amplifierId READ amplifierId WRITE setAmplifierId NOTIFY amplifierIdChanged FINAL)

    // Changed to double to handle dynamic values from QML
    Q_PROPERTY(double spacing READ spacing WRITE setSpacing NOTIFY spacingChanged FINAL)

    // Sampling rate from LSL stream (read-only, set automatically)
    Q_PROPERTY(double samplingRate READ samplingRate NOTIFY samplingRateChanged FINAL)

    // Time window in seconds (default 10)
    Q_PROPERTY(double timeWindowSeconds READ timeWindowSeconds WRITE setTimeWindowSeconds NOTIFY timeWindowSecondsChanged FINAL)

    // Channel names for display (read-only, derived from selected channels)
    Q_PROPERTY(QStringList channelNames READ channelNames NOTIFY channelsChanged FINAL)

    // Marker manager - do zarządzania znacznikami na wykresie
    Q_PROPERTY(MarkerManager* markerManager READ markerManager CONSTANT FINAL)

    // Stream connection state - do wyświetlania loadingu
    Q_PROPERTY(bool isConnecting READ isConnecting NOTIFY isConnectingChanged FINAL)
    Q_PROPERTY(bool isConnected READ isConnected NOTIFY isConnectedChanged FINAL)

    // Display scaling properties
    // Sensitivity in μV/mm - controls vertical scaling
    Q_PROPERTY(double sensitivity READ sensitivity WRITE setSensitivity NOTIFY sensitivityChanged FINAL)
    // Available sensitivity values for UI
    Q_PROPERTY(QList<double> sensitivityOptions READ sensitivityOptions CONSTANT FINAL)
    // Screen DPI (read-only, detected from system)
    Q_PROPERTY(double screenDpi READ screenDpi NOTIFY screenDpiChanged FINAL)
    // Display gain in px/μV (read-only, calculated from sensitivity and DPI)
    Q_PROPERTY(double displayGain READ displayGain NOTIFY displayGainChanged FINAL)

public:
    explicit EegBackend(QObject *parent = nullptr);
    ~EegBackend();

    Q_INVOKABLE void registerDataModel(EegDataModel* dataModel);
    Q_INVOKABLE void startStream();

    Q_INVOKABLE void generateTestData();

    // Dodaj znacznik w aktualnej pozycji zapisu danych
    Q_INVOKABLE void addMarker(const QString& type);

    QVariantList GetChannelNames() const;
    QVariantList channels() const;
    QStringList channelNames() const;

    void setChannels(const QVariantList &newChannels);

    int amplifierIdx() const;
    void setAmplifierIdx(int newAmplifierIdx);

    double spacing() const;
    void setSpacing(double newSpacing);

    QString amplifierId() const;
    void setAmplifierId(const QString &newAmplifierId);

    double samplingRate() const;

    double timeWindowSeconds() const;
    void setTimeWindowSeconds(double newTimeWindowSeconds);

    // Marker manager getter
    MarkerManager* markerManager() const { return m_markerManager; }

    // Stream connection state getters
    bool isConnecting() const { return m_isConnecting; }
    bool isConnected() const { return m_isConnected; }

    // Display scaling getters/setters
    double sensitivity() const;
    void setSensitivity(double newSensitivity);
    QList<double> sensitivityOptions() const;
    double screenDpi() const;
    double displayGain() const;

    // Set screen DPI (called from QML after getting actual screen DPI)
    Q_INVOKABLE void setScreenDpi(double dpi);

public slots:
    void onStreamConnected();
    void onStreamDisconnected();
    void onSamplingRateDetected(double samplingRate);
    void DataReceived(const std::vector<std::vector<float>>& chunk);

signals:
    void updateData(const std::vector<std::vector<float>>& chunk);

    void channelsChanged();

    void amplifierIdxChanged();

    void amplifierIdChanged();

    void spacingChanged();

    void samplingRateChanged();

    void timeWindowSecondsChanged();

    // Stream connection signals
    void isConnectingChanged();
    void isConnectedChanged();

    // Display scaling signals
    void sensitivityChanged();
    void screenDpiChanged();
    void displayGainChanged();

private:
    Amplifier* amplifier_ = nullptr;
    AmplifierManager* amplifier_manager_ = nullptr;
    QVariantList m_channels;
    int m_amplifierIdx = 0;

    EegDataModel* m_dataModel = nullptr;
    double m_spacing = 100.0;  // Default value, will be overwritten by QML
    QString m_amplifierId;
    double m_samplingRate = 0.0;  // Will be set from LSL stream info
    double m_timeWindowSeconds = 10.0;  // Default 10 second window

    // Cached channel indices to avoid repeated QVariant::toInt() calls
    QVector<int> m_channelIndexCache;

    // Marker manager
    MarkerManager* m_markerManager = nullptr;

    // Stream connection state
    bool m_isConnecting = false;
    bool m_isConnected = false;

    // Display scaling
    double m_sensitivity = 10.0;  // Default 10 μV/mm
    double m_screenDpi = 96.0;    // Default 96 DPI, will be updated from QML
    static const QList<double> s_sensitivityOptions;

    // Helper to calculate display gain
    void updateDisplayGain();
};

#endif // EEGBACKEND_H
