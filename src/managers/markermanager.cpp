#include "markermanager.h"
#include <QDebug>

MarkerManager::MarkerManager(QObject *parent)
    : QObject{parent}
{
    qInfo() << "[MarkerManager] Created";
}

const QMap<QString, QPair<QString, QColor>>& MarkerManager::markerTypeConfig()
{
    static const QMap<QString, QPair<QString, QColor>> config = {
        {"eyes_open",     {"Eyes Open",     QColor("#3498db")}},
        {"eyes_closed",   {"Eyes Closed",   QColor("#9b59b6")}},
        {"seizure_start", {"Seizure Start", QColor("#e74c3c")}},
        {"seizure_stop",  {"Seizure Stop",  QColor("#27ae60")}},
        {"artifact",      {"Artifact",      QColor("#f39c12")}},
        {"custom",        {"Custom",        QColor("#95a5a6")}}
    };
    return config;
}

QString MarkerManager::getLabelForType(const QString& type)
{
    const auto& config = markerTypeConfig();
    if (config.contains(type)) {
        return config[type].first;
    }
    return type;
}

QColor MarkerManager::getColorForType(const QString& type)
{
    const auto& config = markerTypeConfig();
    if (config.contains(type)) {
        return config[type].second;
    }
    return QColor("#95a5a6");
}

void MarkerManager::addMarkerAtPosition(const QString& type, double xPosition, double absoluteTime, const QString& customLabel)
{
    Marker marker;
    marker.type = type;
    marker.xPosition = xPosition;
    marker.absoluteTime = absoluteTime;
    marker.color = getColorForType(type);

    if (!customLabel.isEmpty()) {
        marker.label = customLabel;
    } else {
        marker.label = getLabelForType(type);
    }

    m_markers.append(marker);

    qInfo() << "[MarkerManager] Added marker:" << marker.label
            << "at X:" << marker.xPosition
            << "absoluteTime:" << marker.absoluteTime;

    emit markerAdded(marker.type, marker.xPosition, marker.label, marker.color);
    emit markersChanged();
}

void MarkerManager::removeMarker(int index)
{
    if (index >= 0 && index < m_markers.size()) {
        qInfo() << "[MarkerManager] Removing marker:" << m_markers[index].label;
        m_markers.removeAt(index);
        emit markersChanged();
    }
}

void MarkerManager::clearMarkers()
{
    qInfo() << "[MarkerManager] Clearing all" << m_markers.size() << "markers";
    m_markers.clear();
    emit markersChanged();
}

QVariantList MarkerManager::markersAsVariant() const
{
    QVariantList result;

    for (const auto& marker : m_markers) {
        QVariantMap markerMap;
        markerMap["type"] = marker.type;
        markerMap["label"] = marker.label;
        markerMap["xPosition"] = marker.xPosition;
        markerMap["absoluteTime"] = marker.absoluteTime;
        markerMap["color"] = marker.color;
        result.append(markerMap);
    }

    return result;
}
