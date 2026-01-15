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
    Q_PROPERTY(double timestamp MEMBER timestamp)
    Q_PROPERTY(QColor color MEMBER color)

public:
    QString type;       // Typ znacznika (eyes_open, seizure_start, etc.)
    QString label;      // Etykieta wyświetlana na wykresie
    double timestamp;   // Czas w sekundach od początku nagrania
    QColor color;       // Kolor linii znacznika

    bool operator==(const Marker& other) const {
        return type == other.type &&
               qFuzzyCompare(timestamp, other.timestamp);
    }
};

Q_DECLARE_METATYPE(Marker)

// Manager znaczników - przechowuje wszystkie znaczniki sesji
class MarkerManager : public QObject
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(QVariantList markers READ markersAsVariant NOTIFY markersChanged FINAL)
    Q_PROPERTY(int markerCount READ markerCount NOTIFY markersChanged FINAL)

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

    // Dodaj znacznik określonego typu w danym czasie
    Q_INVOKABLE void addMarker(const QString& type, double timestamp, const QString& customLabel = "");

    // Usuń znacznik po indeksie
    Q_INVOKABLE void removeMarker(int index);

    // Usuń znacznik po timestamp (z tolerancją)
    Q_INVOKABLE void removeMarkerAtTime(double timestamp, double tolerance = 0.1);

    // Wyczyść wszystkie znaczniki
    Q_INVOKABLE void clearMarkers();

    // Pobierz znaczniki w zakresie czasowym (do wyświetlania na wykresie)
    Q_INVOKABLE QVariantList getMarkersInRange(double startTime, double endTime) const;

    // Pobierz wszystkie znaczniki jako QVariantList (dla QML)
    QVariantList markersAsVariant() const;

    // Liczba znaczników
    int markerCount() const { return m_markers.size(); }

    // Pobierz surową listę znaczników (dla C++)
    const QList<Marker>& markers() const { return m_markers; }

    // Pobierz konfigurację typu znacznika
    static QString getLabelForType(const QString& type);
    static QColor getColorForType(const QString& type);

signals:
    void markersChanged();
    void markerAdded(const QString& type, double timestamp, const QString& label, const QColor& color);

private:
    QList<Marker> m_markers;

    // Mapa typów znaczników do ich właściwości
    static const QMap<QString, QPair<QString, QColor>>& markerTypeConfig();
};

#endif // MARKERMANAGER_H
