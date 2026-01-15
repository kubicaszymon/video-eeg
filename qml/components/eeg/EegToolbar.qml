import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: root
    height: 70
    color: panelColor

    // Required properties
    required property string currentPatientName
    required property int channelCount
    required property bool isRecording
    required property bool isPaused
    required property int recordingTime
    required property color panelColor
    required property color textColor
    required property color textSecondary
    required property color warningColor
    required property color dangerColor

    function formatTime(seconds) {
        var h = Math.floor(seconds / 3600)
        var m = Math.floor((seconds % 3600) / 60)
        var s = seconds % 60
        return (h < 10 ? "0" : "") + h + ":" +
               (m < 10 ? "0" : "") + m + ":" +
               (s < 10 ? "0" : "") + s
    }

    RowLayout {
        anchors.fill: parent
        anchors.margins: 15
        spacing: 20

        RowLayout {
            spacing: 12

            Label {
                text: "[EEG]"
                font.pixelSize: 28
            }

            ColumnLayout {
                spacing: 2

                Label {
                    text: "EEG Recording System"
                    font.pixelSize: 16
                    font.bold: true
                    color: textColor
                }

                Label {
                    text: currentPatientName + " - " + channelCount + " channels"
                    font.pixelSize: 11
                    color: textSecondary
                }
            }
        }

        Item { Layout.fillWidth: true }

        // Recording status indicator
        Rectangle {
            Layout.preferredWidth: 300
            Layout.preferredHeight: 50
            color: isRecording ? (isPaused ? warningColor : dangerColor) : "#2d3e50"
            radius: 6
            border.color: isRecording ? "white" : "#7f8c8d"
            border.width: 2

            RowLayout {
                anchors.fill: parent
                anchors.margins: 10
                spacing: 12

                Rectangle {
                    width: 16
                    height: 16
                    radius: 8
                    color: "white"
                    visible: isRecording && !isPaused

                    SequentialAnimation on opacity {
                        running: isRecording && !isPaused
                        loops: Animation.Infinite
                        NumberAnimation { from: 1; to: 0.3; duration: 500 }
                        NumberAnimation { from: 0.3; to: 1; duration: 500 }
                    }
                }

                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 0

                    Label {
                        text: isRecording ? (isPaused ? "[||] PAUSED" : "[REC] RECORDING") : "[>] LIVE PREVIEW"
                        font.pixelSize: 14
                        font.bold: true
                        color: "white"
                    }

                    Label {
                        text: isRecording ? root.formatTime(recordingTime) : "Not saving data"
                        font.pixelSize: 10
                        color: "white"
                    }
                }
            }
        }

        // Date/time display
        Label {
            id: dateTimeLabel
            text: Qt.formatDateTime(new Date(), "dd.MM.yyyy  hh:mm:ss")
            font.pixelSize: 11
            color: textSecondary

            Timer {
                interval: 1000
                running: true
                repeat: true
                onTriggered: dateTimeLabel.text = Qt.formatDateTime(new Date(), "dd.MM.yyyy  hh:mm:ss")
            }
        }
    }
}
