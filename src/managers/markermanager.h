#ifndef MARKERMANAGER_H
#define MARKERMANAGER_H

#include <QObject>
#include <QList>
#include <QString>
#include <QColor>
#include <QVariantList>
#include <QtQml/qqmlregistration.h>

// Struktura reprezentująca pojedynczy znacznik
struct Marker {
    Q_GADGET
    Q_PROPERTY(QString type MEMBER type)
    Q_PROPERTY(QString label MEMBER label)
    Q_PROPERTY(double xPosition MEMBER xPosition)
    Q_PROPERTY(double absoluteTime MEMBER absoluteTime)
    Q_PROPERTY(QColor color MEMBER color)

public:
    QString type;           // Typ znacznika (eyes_open, seizure_start, etc.)
    QString label;          // Etykieta wyświetlana na wykresie
    double xPosition;       // Aktualna pozycja X na wykresie (0 do timeWindowSeconds)
    double absoluteTime;    // Absolutny czas dodania (do eksportu)
    QColor color;           // Kolor linii znacznika

    bool operator==(const Marker& other) const {
        return type == other.type &&
               qFuzzyCompare(absoluteTime, other.absoluteTime);
    }
};

Q_DECLARE_METATYPE(Marker)

// Manager znaczników - przechowuje wszystkie znaczniki sesji
class MarkerManager : public QObject
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(QVariantList visibleMarkers READ visibleMarkersAsVariant NOTIFY markersChanged FINAL)
    Q_PROPERTY(QVariantList allMarkers READ allMarkersAsVariant NOTIFY markersChanged FINAL)
    Q_PROPERTY(int markerCount READ markerCount NOTIFY markersChanged FINAL)
    Q_PROPERTY(double timeWindowSeconds READ timeWindowSeconds WRITE setTimeWindowSeconds NOTIFY timeWindowSecondsChanged FINAL)

public:
    // Definicje typów znaczników i ich właściwości
    enum class MarkerType {
        EyesOpen,
        EyesClosed,
        SeizureStart,
        SeizureStop,
        Artifact,
        Custom
    };
    Q_ENUM(MarkerType)

    explicit MarkerManager(QObject *parent = nullptr);

    // Dodaj znacznik określonego typu (pojawi się na prawej krawędzi)
    Q_INVOKABLE void addMarker(const QString& type, const QString& customLabel = "");

    // Aktualizuj pozycje znaczników gdy przychodzą nowe dane
    // deltaSeconds = ile sekund danych przyszło
    Q_INVOKABLE void updatePositions(double deltaSeconds);

    // Usuń znacznik po indeksie
    Q_INVOKABLE void removeMarker(int index);

    // Wyczyść wszystkie znaczniki
    Q_INVOKABLE void clearMarkers();

    // Pobierz widoczne znaczniki (xPosition >= 0) jako QVariantList
    QVariantList visibleMarkersAsVariant() const;

    // Pobierz wszystkie znaczniki (do eksportu)
    QVariantList allMarkersAsVariant() const;

    // Liczba wszystkich znaczników
    int markerCount() const { return m_markers.size(); }

    // Time window
    double timeWindowSeconds() const { return m_timeWindowSeconds; }
    void setTimeWindowSeconds(double seconds);

    // Pobierz konfigurację typu znacznika
    static QString getLabelForType(const QString& type);
    static QColor getColorForType(const QString& type);

signals:
    void markersChanged();
    void markerAdded(const QString& type, const QString& label, const QColor& color);
    void timeWindowSecondsChanged();

private:
    QList<Marker> m_markers;
    double m_timeWindowSeconds = 10.0;
    double m_totalElapsedTime = 0.0;  // Całkowity czas od startu (do absoluteTime)

    // Mapa typów znaczników do ich właściwości
    static const QMap<QString, QPair<QString, QColor>>& markerTypeConfig();
};

#endif // MARKERMANAGER_H
