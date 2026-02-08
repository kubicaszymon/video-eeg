#ifndef AMPLIFIERSETUPBACKEND_H
#define AMPLIFIERSETUPBACKEND_H

#include <QObject>
#include <QProperty>
#include <QVariant>
#include <QtQml/qqmlregistration.h>
#include "amplifiermodel.h"
#include "amplifiermanager.h"
#include "cameramanager.h"

class AmplifierSetupBackend : public QObject
{
    Q_OBJECT
    QML_ELEMENT

    // Amplifier properties
    Q_PROPERTY(QVariantList availableAmplifiers READ getAvailableAmplifiers NOTIFY availableAmplifiersChanged FINAL)
    Q_PROPERTY(int selectedAmplifierIndex READ getSelectedAmplifierIndex WRITE setSelectedAmplifierIndex NOTIFY selectedAmplifierIndexChanged FINAL)
    Q_PROPERTY(QVariantList currentChannels READ getCurrentChannels NOTIFY selectedAmplifierIndexChanged FINAL)
    Q_PROPERTY(bool isLoading READ isLoading NOTIFY isLoadingChanged FINAL)

    // Camera properties - expose CameraManager functionality
    Q_PROPERTY(CameraManager* cameraManager READ cameraManager CONSTANT FINAL)
    Q_PROPERTY(QVariantList availableCameras READ availableCameras NOTIFY availableCamerasChanged FINAL)
    Q_PROPERTY(int selectedCameraIndex READ selectedCameraIndex WRITE setSelectedCameraIndex NOTIFY selectedCameraIndexChanged FINAL)
    Q_PROPERTY(QVariantList availableFormats READ availableFormats NOTIFY availableFormatsChanged FINAL)
    Q_PROPERTY(int selectedFormatIndex READ selectedFormatIndex WRITE setSelectedFormatIndex NOTIFY selectedFormatIndexChanged FINAL)
    Q_PROPERTY(QString selectedCameraName READ selectedCameraName NOTIFY selectedCameraIndexChanged FINAL)
    Q_PROPERTY(QString selectedFormatString READ selectedFormatString NOTIFY selectedFormatIndexChanged FINAL)
    Q_PROPERTY(bool isCameraPreviewActive READ isCameraPreviewActive NOTIFY cameraPreviewActiveChanged FINAL)

public:
    explicit AmplifierSetupBackend(QObject *parent = nullptr);
    ~AmplifierSetupBackend();

    // Amplifier methods
    QVariantList getAvailableAmplifiers() const;
    int getSelectedAmplifierIndex() const;
    QVariantList getCurrentChannels() const;

    Q_INVOKABLE void refreshAmplifiersList();
    Q_INVOKABLE void setSelectedAmplifierIndex(int index);
    Q_INVOKABLE QString getSelectedAmplifierId() const;

    bool isLoading() const { return m_isLoading; }

    // Camera methods
    CameraManager* cameraManager() const { return m_cameraManager; }
    QVariantList availableCameras() const;
    int selectedCameraIndex() const;
    QVariantList availableFormats() const;
    int selectedFormatIndex() const;
    QString selectedCameraName() const;
    QString selectedFormatString() const;
    bool isCameraPreviewActive() const;

    Q_INVOKABLE void refreshCameraList();
    Q_INVOKABLE void setSelectedCameraIndex(int index);
    Q_INVOKABLE void setSelectedFormatIndex(int index);
    Q_INVOKABLE void startCameraPreview();
    Q_INVOKABLE void stopCameraPreview();
    Q_INVOKABLE QString getSelectedCameraId() const;

signals:
    // Amplifier signals
    void availableAmplifiersChanged();
    void selectedAmplifierIndexChanged();
    void currentChannelsChanged();
    void isLoadingChanged();

    // Camera signals
    void availableCamerasChanged();
    void selectedCameraIndexChanged();
    void availableFormatsChanged();
    void selectedFormatIndexChanged();
    void cameraPreviewActiveChanged();
    void cameraErrorOccurred(const QString& error);

private slots:
    void onAmplifiersListRefreshed(const QList<Amplifier>& amplifiers);
    void onCameraErrorOccurred(const QString& error);

private:
    const Amplifier* getCurrentAmplifier() const;

    AmplifierManager* m_manager;
    CameraManager* m_cameraManager;

    QList<Amplifier> m_amplifiers;
    QProperty<int> m_selectedAmplifierIndex{-1};
    bool m_isLoading = false;
};

#endif // AMPLIFIERSETUPBACKEND_H
