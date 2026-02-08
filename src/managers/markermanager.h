#ifndef MARKERMANAGER_H
#define MARKERMANAGER_H

#include <QObject>
#include <QList>
#include <QString>
#include <QColor>
#include <QVariantList>
#include <QtQml/qqmlregistration.h>

// Structure representing a single marker
struct Marker {
    Q_GADGET
    Q_PROPERTY(QString type MEMBER type)
    Q_PROPERTY(QString label MEMBER label)
    Q_PROPERTY(double xPosition MEMBER xPosition)
    Q_PROPERTY(double absoluteTime MEMBER absoluteTime)
    Q_PROPERTY(QColor color MEMBER color)

public:
    QString type;           // Marker type (eyes_open, seizure_start, etc.)
    QString label;          // Label displayed on the graph
    double xPosition;       // X position on the graph (0 to timeWindowSeconds) - FIXED
    double absoluteTime;    // Absolute time of creation (for export)
    QColor color;           // Marker line color
};

Q_DECLARE_METATYPE(Marker)

// Marker manager - stores all markers for the session
class MarkerManager : public QObject
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(QVariantList markers READ markersAsVariant NOTIFY markersChanged FINAL)
    Q_PROPERTY(int markerCount READ markerCount NOTIFY markersChanged FINAL)

public:
    explicit MarkerManager(QObject *parent = nullptr);

    // Add a marker at a specific X position (0 to timeWindowSeconds)
    Q_INVOKABLE void addMarkerAtPosition(const QString& type, double xPosition, double absoluteTime, const QString& customLabel = "");

    // Remove a marker by index
    Q_INVOKABLE void removeMarker(int index);

    // Remove markers in an X range (when the circular buffer overwrites them)
    void removeMarkersInRange(double startX, double endX, double timeWindowSeconds);

    // Clear all markers
    Q_INVOKABLE void clearMarkers();

    // Get all markers as QVariantList
    QVariantList markersAsVariant() const;

    // Marker count
    int markerCount() const { return m_markers.size(); }

    // Get configuration for a marker type
    static QString getLabelForType(const QString& type);
    static QColor getColorForType(const QString& type);

signals:
    void markersChanged();
    void markerAdded(const QString& type, double xPosition, const QString& label, const QColor& color);

private:
    QList<Marker> m_markers;

    // Map of marker types to their properties
    static const QMap<QString, QPair<QString, QColor>>& markerTypeConfig();
};

#endif // MARKERMANAGER_H
