#include "markermanager.h"
#include <QDebug>

MarkerManager::MarkerManager(QObject *parent)
    : QObject{parent}
{
    qInfo() << "[MarkerManager] Created";
}

const QMap<QString, QPair<QString, QColor>>& MarkerManager::markerTypeConfig()
{
    // Konfiguracja typów znaczników: typ -> (etykieta, kolor)
    static const QMap<QString, QPair<QString, QColor>> config = {
        {"eyes_open",     {"Eyes Open",     QColor("#3498db")}},  // Niebieski
        {"eyes_closed",   {"Eyes Closed",   QColor("#9b59b6")}},  // Fioletowy
        {"seizure_start", {"Seizure Start", QColor("#e74c3c")}},  // Czerwony
        {"seizure_stop",  {"Seizure Stop",  QColor("#27ae60")}},  // Zielony
        {"artifact",      {"Artifact",      QColor("#f39c12")}},  // Pomarańczowy
        {"custom",        {"Custom",        QColor("#95a5a6")}}   // Szary
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

void MarkerManager::addMarker(const QString& type, const QString& customLabel)
{
    Marker marker;
    marker.type = type;
    marker.xPosition = m_timeWindowSeconds;  // Nowy znacznik na prawej krawędzi
    marker.absoluteTime = m_totalElapsedTime;
    marker.color = getColorForType(type);

    if (!customLabel.isEmpty()) {
        marker.label = customLabel;
    } else {
        marker.label = getLabelForType(type);
    }

    m_markers.append(marker);

    qInfo() << "[MarkerManager] Added marker:" << marker.label
            << "at xPos:" << marker.xPosition
            << "absoluteTime:" << marker.absoluteTime
            << "color:" << marker.color.name();

    emit markerAdded(marker.type, marker.label, marker.color);
    emit markersChanged();
}

void MarkerManager::updatePositions(double deltaSeconds)
{
    if (deltaSeconds <= 0) return;

    m_totalElapsedTime += deltaSeconds;

    bool changed = false;
    for (int i = 0; i < m_markers.size(); ++i) {
        m_markers[i].xPosition -= deltaSeconds;
        changed = true;
    }

    if (changed) {
        emit markersChanged();
    }
}

void MarkerManager::removeMarker(int index)
{
    if (index >= 0 && index < m_markers.size()) {
        qInfo() << "[MarkerManager] Removing marker at index:" << index
                << "(" << m_markers[index].label << ")";
        m_markers.removeAt(index);
        emit markersChanged();
    }
}

void MarkerManager::clearMarkers()
{
    qInfo() << "[MarkerManager] Clearing all" << m_markers.size() << "markers";
    m_markers.clear();
    m_totalElapsedTime = 0.0;
    emit markersChanged();
}

QVariantList MarkerManager::visibleMarkersAsVariant() const
{
    QVariantList result;

    for (const auto& marker : m_markers) {
        // Tylko znaczniki widoczne na wykresie (xPosition >= 0)
        if (marker.xPosition >= 0 && marker.xPosition <= m_timeWindowSeconds) {
            QVariantMap markerMap;
            markerMap["type"] = marker.type;
            markerMap["label"] = marker.label;
            markerMap["xPosition"] = marker.xPosition;
            markerMap["absoluteTime"] = marker.absoluteTime;
            markerMap["color"] = marker.color;
            result.append(markerMap);
        }
    }

    return result;
}

QVariantList MarkerManager::allMarkersAsVariant() const
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

void MarkerManager::setTimeWindowSeconds(double seconds)
{
    if (seconds > 0 && !qFuzzyCompare(m_timeWindowSeconds, seconds)) {
        m_timeWindowSeconds = seconds;
        emit timeWindowSecondsChanged();
        emit markersChanged();  // Może zmienić widoczność znaczników
    }
}
