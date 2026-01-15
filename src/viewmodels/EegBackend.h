#ifndef EEGBACKEND_H
#define EEGBACKEND_H

#include <QObject>
#include <QVariantMap>
#include <QVector>
#include <QtQml/qqmlregistration.h>
#include "amplifiermodel.h"
#include "amplifiermanager.h"
#include "eegdatamodel.h"
#include "autoscalemanager.h"
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

    // Auto-scale properties (read-only, from AutoScaleManager)
    Q_PROPERTY(double scaleFactor READ scaleFactor NOTIFY scaleFactorChanged FINAL)
    Q_PROPERTY(QString scaleUnit READ scaleUnit NOTIFY scaleUnitChanged FINAL)
    Q_PROPERTY(bool scaleCalibrated READ scaleCalibrated NOTIFY scaleCalibrationChanged FINAL)
    Q_PROPERTY(double dataRangeMin READ dataRangeMin NOTIFY dataRangeChanged FINAL)
    Q_PROPERTY(double dataRangeMax READ dataRangeMax NOTIFY dataRangeChanged FINAL)

    // Gain - mnożnik wzmocnienia kontrolowany przez użytkownika (suwak)
    Q_PROPERTY(double gain READ gain WRITE setGain NOTIFY gainChanged FINAL)

    // Scale bar properties - do wyświetlenia wskaźnika skali na wykresie
    Q_PROPERTY(double scaleBarValue READ scaleBarValue NOTIFY scaleBarChanged FINAL)
    Q_PROPERTY(double scaleBarHeight READ scaleBarHeight NOTIFY scaleBarChanged FINAL)
    Q_PROPERTY(double dataRangeInMicrovolts READ dataRangeInMicrovolts NOTIFY dataRangeChanged FINAL)

    // Marker manager - do zarządzania znacznikami na wykresie
    Q_PROPERTY(MarkerManager* markerManager READ markerManager CONSTANT FINAL)

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

    // Auto-scale getters
    double scaleFactor() const;
    QString scaleUnit() const;
    bool scaleCalibrated() const;
    double dataRangeMin() const;
    double dataRangeMax() const;

    // Gain control
    double gain() const;
    void setGain(double newGain);

    // Scale bar
    double scaleBarValue() const;
    double scaleBarHeight() const;
    double dataRangeInMicrovolts() const;

    // Marker manager getter
    MarkerManager* markerManager() const { return m_markerManager; }

public slots:
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

    // Auto-scale signals
    void scaleFactorChanged();
    void scaleUnitChanged();
    void scaleCalibrationChanged();
    void dataRangeChanged();
    void gainChanged();
    void scaleBarChanged();

private:
    void initializeBuffers(int numChannels, int numSamples);

    Amplifier* amplifier_ = nullptr;
    AmplifierManager* amplifier_manager_ = nullptr;
    QVariantList m_channels;
    int m_amplifierIdx = 0;

    EegDataModel* m_dataModel = nullptr;
    double m_spacing = 100.0;  // Default value, will be overwritten by QML
    QString m_amplifierId;
    double m_samplingRate = 0.0;  // Will be set from LSL stream info
    double m_timeWindowSeconds = 10.0;  // Default 10 second window

    // Pre-allocated buffers to avoid runtime memory allocations
    QVector<QVector<double>> m_scaledDataBuffer;
    QVector<int> m_channelIndexCache;
    int m_lastNumChannels = 0;
    int m_lastBufferSize = 0;

    // Auto-scale manager
    AutoScaleManager* m_autoScaleManager = nullptr;

    // Marker manager
    MarkerManager* m_markerManager = nullptr;

    // Gain - mnożnik kontrolowany przez użytkownika (1.0 = neutralny)
    double m_gain = 1.0;
};

#endif // EEGBACKEND_H
