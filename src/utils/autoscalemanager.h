#ifndef AUTOSCALEMANAGER_H
#define AUTOSCALEMANAGER_H

#include <QObject>
#include <QVector>
#include <QString>
#include <vector>
#include <limits>
#include <cmath>

/**
 * @brief AutoScaleManager - Inteligentny system automatycznego wykrywania i skalowania danych EEG
 *
 * Kluczowe założenia:
 * 1. WSZYSTKIE kanały są skalowane tym samym współczynnikiem (nigdy osobno!)
 * 2. System wykrywa skalę danych (μV, mV, V) na podstawie ich zakresu
 * 3. Skalowanie jest stabilne - nie zmienia się przy małych wahaniach
 * 4. Adaptuje się gdy dane znacząco wyjdą poza przewidywany zakres
 */
class AutoScaleManager : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief Jednostka danych EEG
     */
    enum class DataUnit {
        Unknown,
        Microvolts,    // μV: typowo ±100 μV (wartości ~0.0001)
        Millivolts,    // mV: typowo ±0.1 mV (wartości ~0.0001 do ~1)
        Volts,         // V:  typowo ±0.001V (wartości ~0.001 do ~1)
        RawADC,        // Surowe wartości ADC (wartości mogą być dowolne)
        Normalized     // Dane już znormalizowane (wartości -1 do 1)
    };
    Q_ENUM(DataUnit)

    /**
     * @brief Stan kalibracji systemu
     */
    enum class CalibrationState {
        NotStarted,    // Brak danych
        Collecting,    // Zbieranie próbek do kalibracji
        Calibrated,    // Skalibrowany, stabilna praca
        Adapting       // Dostosowywanie do nowych danych
    };
    Q_ENUM(CalibrationState)

    explicit AutoScaleManager(QObject *parent = nullptr);

    /**
     * @brief Przetwarza chunk danych i aktualizuje statystyki
     * @param chunk Surowe dane [sample][channel] - PRZED offsetami!
     * @param selectedChannelIndices Indeksy wybranych kanałów
     */
    void processChunk(const std::vector<std::vector<float>>& chunk,
                      const QVector<int>& selectedChannelIndices);

    /**
     * @brief Skaluje chunk danych używając obliczonego współczynnika
     * @param chunk Surowe dane do przeskalowania
     * @param selectedChannelIndices Indeksy wybranych kanałów
     * @param channelSpacing Odstęp między kanałami (dla obliczenia offsetów)
     * @return Przeskalowane dane z offsetami [channel][sample]
     */
    QVector<QVector<double>> scaleChunk(const std::vector<std::vector<float>>& chunk,
                                        const QVector<int>& selectedChannelIndices,
                                        double channelSpacing) const;

    /**
     * @brief Resetuje wszystkie statystyki i kalibrację
     */
    void reset();

    // Gettery
    double scaleFactor() const { return m_scaleFactor; }
    DataUnit detectedUnit() const { return m_detectedUnit; }
    CalibrationState calibrationState() const { return m_calibrationState; }
    QString unitString() const;
    double globalMin() const { return m_globalMin; }
    double globalMax() const { return m_globalMax; }
    double globalRange() const { return m_globalMax - m_globalMin; }
    int totalSamplesProcessed() const { return m_totalSamplesProcessed; }
    bool isCalibrated() const { return m_calibrationState == CalibrationState::Calibrated; }

    /**
     * @brief Formatuje wartość z odpowiednią jednostką
     */
    QString formatValue(double value) const;

    // Konfiguracja
    void setCalibrationSamples(int samples) { m_calibrationSamples = samples; }
    void setHysteresisThreshold(double threshold) { m_hysteresisThreshold = threshold; }
    void setTargetAmplitudeRatio(double ratio) { m_targetAmplitudeRatio = ratio; }

signals:
    void scaleFactorChanged(double newFactor);
    void detectedUnitChanged(DataUnit newUnit);
    void calibrationStateChanged(CalibrationState newState);
    void calibrationProgress(int current, int total);

private:
    /**
     * @brief Aktualizuje statystyki globalne na podstawie nowych danych
     */
    void updateStatistics(const std::vector<std::vector<float>>& chunk,
                          const QVector<int>& selectedChannelIndices);

    /**
     * @brief Wykrywa jednostkę na podstawie zakresu wartości
     */
    DataUnit detectUnit(double minVal, double maxVal) const;

    /**
     * @brief Oblicza optymalny współczynnik skalowania
     */
    double calculateOptimalScaleFactor(double dataRange, double targetSpacing) const;

    /**
     * @brief Sprawdza czy potrzebna jest rekalibracja
     */
    bool needsRecalibration(double newMin, double newMax) const;

    /**
     * @brief Wygładza zmianę współczynnika skalowania
     */
    double smoothScaleTransition(double currentScale, double targetScale) const;

    // Statystyki globalne - dla WSZYSTKICH kanałów razem
    double m_globalMin = std::numeric_limits<double>::infinity();
    double m_globalMax = -std::numeric_limits<double>::infinity();
    double m_globalSum = 0.0;
    double m_globalSumSquares = 0.0;

    // Statystyki stabilne (po kalibracji)
    double m_calibratedMin = 0.0;
    double m_calibratedMax = 0.0;
    double m_calibratedRange = 0.0;

    // Stan skalowania
    double m_scaleFactor = 1.0;
    double m_targetSpacing = 100.0;  // Domyślny spacing między kanałami
    DataUnit m_detectedUnit = DataUnit::Unknown;
    CalibrationState m_calibrationState = CalibrationState::NotStarted;

    // Liczniki
    int m_totalSamplesProcessed = 0;
    int m_calibrationSamples = 1000;  // Ile próbek do pełnej kalibracji

    // Parametry stabilizacji
    double m_hysteresisThreshold = 0.25;    // 25% zmiana zanim zmieni skalę
    double m_targetAmplitudeRatio = 0.7;    // Docelowa amplituda to 70% spacing
    double m_smoothingFactor = 0.1;          // Wygładzanie zmian skali

    // Bufor do detekcji trendów
    static constexpr int TREND_BUFFER_SIZE = 10;
    QVector<double> m_recentRanges;
};

#endif // AUTOSCALEMANAGER_H
