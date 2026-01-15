#ifndef AUTOSCALEMANAGER_H
#define AUTOSCALEMANAGER_H

#include <QObject>
#include <QVector>
#include <QString>
#include <vector>
#include <limits>
#include <cmath>

/**
 * @brief AutoScaleManager - Inteligentny system wykrywania i skalowania danych EEG
 *
 * Kluczowe założenia:
 * 1. WSZYSTKIE kanały są skalowane tym samym współczynnikiem (nigdy osobno!)
 * 2. System wykrywa skalę danych (μV, mV) na podstawie ich zakresu
 * 3. Kalibracja na początku ustala bazową skalę - potem NIE zmienia się automatycznie
 * 4. Użytkownik kontroluje wzmocnienie przez suwak Gain (mnożnik bazowej skali)
 *
 * Parametry wzmacniacza (dokumentacja):
 * - Zakres pomiarowy: ±341 mV
 * - Rozdzielczość: 24 bity (40 nV/bit)
 * - Typowy sygnał EEG: 10-100 μV = 0.01-0.1 mV
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
     * @brief Stan systemu
     */
    enum class CalibrationState {
        NotStarted,    // Brak danych - czeka na pierwszą paczkę
        Calibrated     // Skala ustalona (od pierwszej paczki danych)
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
     * @brief Skaluje chunk danych używając obliczonego współczynnika i gain
     * @param chunk Surowe dane do przeskalowania
     * @param selectedChannelIndices Indeksy wybranych kanałów
     * @param channelSpacing Odstęp między kanałami (dla obliczenia offsetów)
     * @param gain Mnożnik wzmocnienia ustawiony przez użytkownika (1.0 = neutralny)
     * @return Przeskalowane dane z offsetami [channel][sample]
     */
    QVector<QVector<double>> scaleChunk(const std::vector<std::vector<float>>& chunk,
                                        const QVector<int>& selectedChannelIndices,
                                        double channelSpacing,
                                        double gain = 1.0) const;

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

    /**
     * @brief Oblicza wysokość scale bar w pikselach (jednostkach wykresu)
     * @param valueInOriginalUnits Wartość w oryginalnych jednostkach (np. 50 μV)
     * @param gain Aktualny mnożnik gain
     * @return Wysokość w jednostkach wykresu
     */
    double calculateScaleBarHeight(double valueInOriginalUnits, double gain = 1.0) const;

    /**
     * @brief Zwraca sugerowaną wartość dla scale bar (np. 50 μV, 100 μV)
     * @return Wartość w oryginalnych jednostkach
     */
    double suggestedScaleBarValue() const;

    /**
     * @brief Zwraca zakres danych w μV (konwertuje jeśli dane są w mV)
     */
    double dataRangeInMicrovolts() const;

    // Konfiguracja
    void setCalibrationSamples(int samples) { m_calibrationSamples = samples; }
    void setHysteresisThreshold(double threshold) { m_hysteresisThreshold = threshold; }
    void setTargetAmplitudeRatio(double ratio) { m_targetAmplitudeRatio = ratio; }

signals:
    void scaleFactorChanged(double newFactor);
    void detectedUnitChanged(DataUnit newUnit);
    void calibrationStateChanged(CalibrationState newState);

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
