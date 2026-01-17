#ifndef EEGBACKEND_H
#define EEGBACKEND_H

#include <QObject>
#include <QVariantMap>
#include <QVector>
#include <QtQml/qqmlregistration.h>
#include "amplifiermodel.h"
#include "amplifiermanager.h"
#include "eegdatamodel.h"
#include "markermanager.h"
#include "eegdisplayscaler.h"

/**
 * @brief EegBackend - Main backend for EEG window
 *
 * Responsibilities:
 * - Stream connection management (start/stop, connection state)
 * - Channel selection and configuration
 * - Data routing from LSL stream to display model
 * - Marker management
 *
 * Delegates scaling logic to EegDisplayScaler.
 */
class EegBackend : public QObject
{
    Q_OBJECT
    QML_ELEMENT

    // Channel configuration
    Q_PROPERTY(QVariantList channels READ channels WRITE setChannels NOTIFY channelsChanged FINAL)
    Q_PROPERTY(QStringList channelNames READ channelNames NOTIFY channelsChanged FINAL)

    // Amplifier identification
    Q_PROPERTY(int amplifierIdx READ amplifierIdx WRITE setAmplifierIdx NOTIFY amplifierIdxChanged FINAL)
    Q_PROPERTY(QString amplifierId READ amplifierId WRITE setAmplifierId NOTIFY amplifierIdChanged FINAL)

    // Display configuration
    Q_PROPERTY(double spacing READ spacing WRITE setSpacing NOTIFY spacingChanged FINAL)
    Q_PROPERTY(double timeWindowSeconds READ timeWindowSeconds WRITE setTimeWindowSeconds NOTIFY timeWindowSecondsChanged FINAL)

    // Stream info (read-only)
    Q_PROPERTY(double samplingRate READ samplingRate NOTIFY samplingRateChanged FINAL)

    // Connection state
    Q_PROPERTY(bool isConnecting READ isConnecting NOTIFY isConnectingChanged FINAL)
    Q_PROPERTY(bool isConnected READ isConnected NOTIFY isConnectedChanged FINAL)

    // Sub-components exposed to QML
    Q_PROPERTY(MarkerManager* markerManager READ markerManager CONSTANT FINAL)
    Q_PROPERTY(EegDisplayScaler* scaler READ scaler CONSTANT FINAL)

public:
    explicit EegBackend(QObject *parent = nullptr);
    ~EegBackend();

    // Initialization
    Q_INVOKABLE void registerDataModel(EegDataModel* dataModel);
    Q_INVOKABLE void startStream();
    Q_INVOKABLE void generateTestData();

    // Markers
    Q_INVOKABLE void addMarker(const QString& type);

    // Channel getters/setters
    QVariantList channels() const;
    void setChannels(const QVariantList &newChannels);
    QStringList channelNames() const;

    // Amplifier getters/setters
    int amplifierIdx() const;
    void setAmplifierIdx(int newAmplifierIdx);
    QString amplifierId() const;
    void setAmplifierId(const QString &newAmplifierId);

    // Display configuration getters/setters
    double spacing() const;
    void setSpacing(double newSpacing);
    double timeWindowSeconds() const;
    void setTimeWindowSeconds(double newTimeWindowSeconds);

    // Stream info
    double samplingRate() const;

    // Connection state
    bool isConnecting() const { return m_isConnecting; }
    bool isConnected() const { return m_isConnected; }

    // Sub-components
    MarkerManager* markerManager() const { return m_markerManager; }
    EegDisplayScaler* scaler() const { return m_scaler; }

public slots:
    void onStreamConnected();
    void onStreamDisconnected();
    void onSamplingRateDetected(double samplingRate);
    void DataReceived(const std::vector<std::vector<float>>& chunk);

signals:
    void channelsChanged();
    void amplifierIdxChanged();
    void amplifierIdChanged();
    void spacingChanged();
    void samplingRateChanged();
    void timeWindowSecondsChanged();
    void isConnectingChanged();
    void isConnectedChanged();

private:
    // Helper methods
    void updateChannelIndexCache();
    void updateMarkersAfterWrite(int prevWritePos, int newWritePos);

    // Amplifier connection
    Amplifier* amplifier_ = nullptr;
    AmplifierManager* amplifier_manager_ = nullptr;

    // Channel configuration
    QVariantList m_channels;
    QVector<int> m_channelIndexCache;
    int m_amplifierIdx = 0;
    QString m_amplifierId;

    // Display configuration
    double m_spacing = 100.0;
    double m_timeWindowSeconds = 10.0;

    // Stream info
    double m_samplingRate = 0.0;

    // Connection state
    bool m_isConnecting = false;
    bool m_isConnected = false;

    // Data model
    EegDataModel* m_dataModel = nullptr;

    // Sub-components (owned)
    MarkerManager* m_markerManager = nullptr;
    EegDisplayScaler* m_scaler = nullptr;
};

#endif // EEGBACKEND_H
