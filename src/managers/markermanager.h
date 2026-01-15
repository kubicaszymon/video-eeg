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
    double xPosition;       // Pozycja X na wykresie (0 do timeWindowSeconds) - STAŁA
    double absoluteTime;    // Absolutny czas dodania (do eksportu)
    QColor color;           // Kolor linii znacznika
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
    explicit MarkerManager(QObject *parent = nullptr);

    // Dodaj znacznik w konkretnej pozycji X (0 do timeWindowSeconds)
    Q_INVOKABLE void addMarkerAtPosition(const QString& type, double xPosition, double absoluteTime, const QString& customLabel = "");

    // Usuń znacznik po indeksie
    Q_INVOKABLE void removeMarker(int index);

    // Usuń znaczniki w zakresie X (gdy bufor je nadpisuje)
    void removeMarkersInRange(double startX, double endX, double timeWindowSeconds);

    // Wyczyść wszystkie znaczniki
    Q_INVOKABLE void clearMarkers();

    // Pobierz wszystkie znaczniki jako QVariantList
    QVariantList markersAsVariant() const;

    // Liczba znaczników
    int markerCount() const { return m_markers.size(); }

    // Pobierz konfigurację typu znacznika
    static QString getLabelForType(const QString& type);
    static QColor getColorForType(const QString& type);

signals:
    void markersChanged();
    void markerAdded(const QString& type, double xPosition, const QString& label, const QColor& color);

private:
    QList<Marker> m_markers;

    // Mapa typów znaczników do ich właściwości
    static const QMap<QString, QPair<QString, QColor>>& markerTypeConfig();
};

#endif // MARKERMANAGER_H
