#ifndef EEGDISPLAYSCALER_H
#define EEGDISPLAYSCALER_H

#include <QObject>
#include <QList>
#include <QVector>
#include <QtQml/qqmlregistration.h>

/**
 * @brief EegDisplayScaler - handles physical-parameter-based EEG signal scaling
 *
 * This class encapsulates all scaling logic for EEG display:
 * - Sensitivity in μV/mm (standard EEG measurement)
 * - Screen DPI for physical accuracy
 * - Display gain calculation (px/μV)
 * - Signal transformation with proper Y-axis inversion
 *
 * Formulas:
 *   displayGain [px/μV] = DPI / (25.4 × Sensitivity[μV/mm])
 *   scaledValue [px] = baselineOffset - (rawValue[μV] × displayGain)
 *
 * The Y-axis inversion ensures positive μV goes UP on screen
 * (Qt/QML Y-axis increases downward).
 */
class EegDisplayScaler : public QObject
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(double sensitivity READ sensitivity WRITE setSensitivity NOTIFY sensitivityChanged FINAL)
    Q_PROPERTY(QList<double> sensitivityOptions READ sensitivityOptions CONSTANT FINAL)
    Q_PROPERTY(double screenDpi READ screenDpi WRITE setScreenDpi NOTIFY screenDpiChanged FINAL)
    Q_PROPERTY(double displayGain READ displayGain NOTIFY displayGainChanged FINAL)

public:
    explicit EegDisplayScaler(QObject *parent = nullptr);

    // Available sensitivity steps in μV/mm (standard EEG values)
    static const QList<double> SENSITIVITY_OPTIONS;

    // Physical constants
    static constexpr double MM_PER_INCH = 25.4;
    static constexpr double DEFAULT_DPI = 96.0;
    static constexpr double DEFAULT_SENSITIVITY = 10.0;  // μV/mm

    // Getters
    double sensitivity() const { return m_sensitivity; }
    double screenDpi() const { return m_screenDpi; }
    double displayGain() const;
    QList<double> sensitivityOptions() const { return SENSITIVITY_OPTIONS; }

    // Setters
    void setSensitivity(double sensitivity);
    void setScreenDpi(double dpi);

    /**
     * @brief Transform a single EEG sample from μV to display pixels
     * @param rawValueMicrovolts Input value in μV
     * @param baselineOffset Y-coordinate of channel baseline (in pixels)
     * @return Scaled Y-coordinate (positive μV goes UP)
     */
    double transformSample(double rawValueMicrovolts, double baselineOffset) const;

    /**
     * @brief Transform an entire chunk of EEG data
     * @param chunk Raw data [samples][channels] in μV
     * @param channelIndices Which channels to extract from chunk
     * @param channelSpacing Vertical spacing between channels (pixels)
     * @return Transformed data [channels][samples] ready for display
     */
    QVector<QVector<double>> transformChunk(
        const std::vector<std::vector<float>>& chunk,
        const QVector<int>& channelIndices,
        double channelSpacing) const;

    /**
     * @brief Calculate baseline Y-offset for a channel
     * @param channelIndex Index of channel (0 = top)
     * @param totalChannels Total number of channels
     * @param channelSpacing Vertical spacing between channels
     * @return Y-coordinate for channel baseline
     */
    static double calculateChannelOffset(int channelIndex, int totalChannels, double channelSpacing);

signals:
    void sensitivityChanged();
    void screenDpiChanged();
    void displayGainChanged();

private:
    double m_sensitivity = DEFAULT_SENSITIVITY;
    double m_screenDpi = DEFAULT_DPI;
};

#endif // EEGDISPLAYSCALER_H
