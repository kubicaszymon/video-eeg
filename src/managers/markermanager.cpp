#include "markermanager.h"
#include <QDebug>
#include <algorithm>

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
    return type;  // Fallback do samego typu
}

QColor MarkerManager::getColorForType(const QString& type)
{
    const auto& config = markerTypeConfig();
    if (config.contains(type)) {
        return config[type].second;
    }
    return QColor("#95a5a6");  // Domyślny szary
}

void MarkerManager::addMarker(const QString& type, double timestamp, const QString& customLabel)
{
    Marker marker;
    marker.type = type;
    marker.timestamp = timestamp;
    marker.color = getColorForType(type);

    // Użyj customLabel jeśli podany, w przeciwnym razie domyślna etykieta
    if (!customLabel.isEmpty()) {
        marker.label = customLabel;
    } else {
        marker.label = getLabelForType(type);
    }

    // Dodaj znacznik zachowując sortowanie po czasie
    auto it = std::lower_bound(m_markers.begin(), m_markers.end(), marker,
        [](const Marker& a, const Marker& b) {
            return a.timestamp < b.timestamp;
        });

    m_markers.insert(it, marker);

    qInfo() << "[MarkerManager] Added marker:" << marker.label
            << "at" << marker.timestamp << "s"
            << "color:" << marker.color.name();

    emit markerAdded(marker.type, marker.timestamp, marker.label, marker.color);
    emit markersChanged();
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

void MarkerManager::removeMarkerAtTime(double timestamp, double tolerance)
{
    for (int i = 0; i < m_markers.size(); ++i) {
        if (qAbs(m_markers[i].timestamp - timestamp) <= tolerance) {
            qInfo() << "[MarkerManager] Removing marker at time:" << timestamp
                    << "(" << m_markers[i].label << ")";
            m_markers.removeAt(i);
            emit markersChanged();
            return;
        }
    }
}

void MarkerManager::clearMarkers()
{
    qInfo() << "[MarkerManager] Clearing all" << m_markers.size() << "markers";
    m_markers.clear();
    emit markersChanged();
}

QVariantList MarkerManager::getMarkersInRange(double startTime, double endTime) const
{
    QVariantList result;

    for (const auto& marker : m_markers) {
        if (marker.timestamp >= startTime && marker.timestamp <= endTime) {
            QVariantMap markerMap;
            markerMap["type"] = marker.type;
            markerMap["label"] = marker.label;
            markerMap["timestamp"] = marker.timestamp;
            markerMap["color"] = marker.color;
            result.append(markerMap);
        }
    }

    return result;
}

QVariantList MarkerManager::markersAsVariant() const
{
    QVariantList result;

    for (const auto& marker : m_markers) {
        QVariantMap markerMap;
        markerMap["type"] = marker.type;
        markerMap["label"] = marker.label;
        markerMap["timestamp"] = marker.timestamp;
        markerMap["color"] = marker.color;
        result.append(markerMap);
    }

    return result;
}
