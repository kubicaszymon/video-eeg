import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import videoEeg

/**
 * SyncMonitorPanel - Developer panel for monitoring EEG-Video synchronization.
 *
 * Shows real-time sync offset, clock drift, buffer health, and status.
 * Toggle visibility from the EEG window toolbar.
 */
Rectangle {
    id: root
    width: 320
    height: contentLayout.implicitHeight + 24
    color: "#d0101820"
    radius: 10
    border.color: statusBorderColor
    border.width: 2

    property color textColor: "#e8eef5"
    property color textSecondary: "#8a9cb5"
    property color accentColor: "#4a90e2"
    property color successColor: "#2ecc71"
    property color warningColor: "#f39c12"
    property color dangerColor: "#c0392b"

    // Status-dependent colors
    readonly property color statusColor: {
        var status = EegSyncManager.healthStatus
        if (status === "DESYNC") return dangerColor
        if (status === "WARNING") return warningColor
        return successColor
    }
    readonly property color statusBorderColor: Qt.rgba(statusColor.r, statusColor.g, statusColor.b, 0.5)

    signal closeRequested()

    ColumnLayout {
        id: contentLayout
        anchors.fill: parent
        anchors.margins: 12
        spacing: 8

        // Header
        RowLayout {
            Layout.fillWidth: true
            spacing: 8

            Rectangle {
                width: 10
                height: 10
                radius: 5
                color: statusColor

                SequentialAnimation on opacity {
                    running: EegSyncManager.bufferSize > 0
                    loops: Animation.Infinite
                    NumberAnimation { from: 1; to: 0.4; duration: 800 }
                    NumberAnimation { from: 0.4; to: 1; duration: 800 }
                }
            }

            Label {
                text: "SYNC MONITOR"
                font.pixelSize: 12
                font.bold: true
                font.letterSpacing: 1.5
                color: textColor
            }

            Item { Layout.fillWidth: true }

            Label {
                text: EegSyncManager.healthStatus
                font.pixelSize: 11
                font.bold: true
                color: statusColor
            }

            Button {
                text: "X"
                font.pixelSize: 10
                implicitWidth: 24
                implicitHeight: 24
                palette.button: "transparent"
                palette.buttonText: textSecondary
                onClicked: root.closeRequested()
            }
        }

        // Separator
        Rectangle { Layout.fillWidth: true; height: 1; color: "#2d3e50" }

        // Sync Offset
        RowLayout {
            Layout.fillWidth: true
            Label {
                text: "Offset:"
                font.pixelSize: 11
                color: textSecondary
            }
            Item { Layout.fillWidth: true }
            Label {
                text: EegSyncManager.lastSyncOffsetMs.toFixed(2) + " ms"
                font.pixelSize: 12
                font.bold: true
                color: EegSyncManager.lastSyncOffsetMs > 15 ? dangerColor :
                       (EegSyncManager.lastSyncOffsetMs > 5 ? warningColor : successColor)
                font.family: "Consolas"
            }
        }

        // Average Offset
        RowLayout {
            Layout.fillWidth: true
            Label {
                text: "Avg Offset:"
                font.pixelSize: 11
                color: textSecondary
            }
            Item { Layout.fillWidth: true }
            Label {
                text: EegSyncManager.avgSyncOffsetMs.toFixed(2) + " ms"
                font.pixelSize: 11
                color: textColor
                font.family: "Consolas"
            }
        }

        // Clock Drift
        RowLayout {
            Layout.fillWidth: true
            Label {
                text: "Clock Drift:"
                font.pixelSize: 11
                color: textSecondary
            }
            Item { Layout.fillWidth: true }
            Label {
                text: EegSyncManager.clockDriftMs.toFixed(3) + " ms"
                font.pixelSize: 11
                color: Math.abs(EegSyncManager.clockDriftMs) > 1.0 ? warningColor : textColor
                font.family: "Consolas"
            }
        }

        // Time Correction
        RowLayout {
            Layout.fillWidth: true
            Label {
                text: "Time Corr:"
                font.pixelSize: 11
                color: textSecondary
            }
            Item { Layout.fillWidth: true }
            Label {
                text: EegSyncManager.timeCorrectionMs.toFixed(3) + " ms"
                font.pixelSize: 11
                color: textColor
                font.family: "Consolas"
            }
        }

        // Separator
        Rectangle { Layout.fillWidth: true; height: 1; color: "#2d3e50" }

        // Buffer Health
        RowLayout {
            Layout.fillWidth: true
            Label {
                text: "Buffer:"
                font.pixelSize: 11
                color: textSecondary
            }
            Item { Layout.fillWidth: true }
            Label {
                text: EegSyncManager.bufferSize + " / " + EegSyncManager.maxBufferSize
                font.pixelSize: 11
                color: textColor
                font.family: "Consolas"
            }
        }

        // Buffer progress bar
        Rectangle {
            Layout.fillWidth: true
            height: 6
            radius: 3
            color: "#1a2332"

            Rectangle {
                width: parent.width * Math.min(1.0, EegSyncManager.bufferSize / Math.max(1, EegSyncManager.maxBufferSize))
                height: parent.height
                radius: 3
                color: {
                    var ratio = EegSyncManager.bufferSize / Math.max(1, EegSyncManager.maxBufferSize)
                    if (ratio > 0.9) return warningColor
                    if (ratio > 0.5) return successColor
                    if (ratio > 0.1) return accentColor
                    return dangerColor
                }

                Behavior on width { NumberAnimation { duration: 200 } }
            }
        }

        // Buffer Duration
        RowLayout {
            Layout.fillWidth: true
            Label {
                text: "Duration:"
                font.pixelSize: 11
                color: textSecondary
            }
            Item { Layout.fillWidth: true }
            Label {
                text: EegSyncManager.bufferDurationSec.toFixed(1) + " s"
                font.pixelSize: 11
                color: textColor
                font.family: "Consolas"
            }
        }

        // Separator
        Rectangle { Layout.fillWidth: true; height: 1; color: "#2d3e50" }

        // Sampling info
        RowLayout {
            Layout.fillWidth: true
            Label {
                text: "SR:"
                font.pixelSize: 11
                color: textSecondary
            }
            Item { Layout.fillWidth: true }
            Label {
                text: EegSyncManager.samplingRate.toFixed(0) + " Hz"
                font.pixelSize: 11
                color: accentColor
                font.family: "Consolas"
            }
        }

        RowLayout {
            Layout.fillWidth: true
            Label {
                text: "Samples/Frame:"
                font.pixelSize: 11
                color: textSecondary
            }
            Item { Layout.fillWidth: true }
            Label {
                text: "~" + EegSyncManager.samplesPerFrame.toFixed(2)
                font.pixelSize: 11
                color: accentColor
                font.family: "Consolas"
            }
        }
    }
}
