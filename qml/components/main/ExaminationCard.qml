import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: root
    color: cardColor
    radius: 8
    border.color: cardMouseArea.containsMouse ? accentColor : borderColor
    border.width: cardMouseArea.containsMouse ? 2 : 1

    // Required properties
    required property color cardColor
    required property color borderColor
    required property color accentColor
    required property color textColor

    // Data properties (can be bound from parent)
    property string patientName: "Jan Kowalski"
    property string examinationType: "EEG - Standard examination"
    property string status: "Completed"
    property string statusColor: "#d4edda"
    property string statusBorderColor: "#c3e6cb"
    property string statusTextColor: "#155724"
    property string examDate: "14.01.2025 10:30"
    property string duration: "45 min"
    property string channelsInfo: "14 channels"

    signal previewClicked()
    signal reportClicked()

    MouseArea {
        id: cardMouseArea
        anchors.fill: parent
        hoverEnabled: true
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 15
        spacing: 8

        // Header row
        RowLayout {
            Layout.fillWidth: true
            spacing: 10

            Rectangle {
                Layout.preferredWidth: 50
                Layout.preferredHeight: 50
                color: "#e8f4f8"
                radius: 25

                Label {
                    anchors.centerIn: parent
                    text: "[~]"
                    font.pixelSize: 24
                }
            }

            ColumnLayout {
                Layout.fillWidth: true
                spacing: 2

                Label {
                    text: patientName
                    font.pixelSize: 14
                    font.bold: true
                    color: textColor
                    elide: Text.ElideRight
                    Layout.fillWidth: true
                }

                Label {
                    text: examinationType
                    font.pixelSize: 11
                    color: "#7f8c8d"
                    elide: Text.ElideRight
                    Layout.fillWidth: true
                }
            }

            // Status badge
            Rectangle {
                Layout.preferredWidth: 80
                Layout.preferredHeight: 24
                color: statusColor
                radius: 12
                border.color: statusBorderColor
                border.width: 1

                Label {
                    anchors.centerIn: parent
                    text: status
                    font.pixelSize: 9
                    color: statusTextColor
                }
            }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 1
            color: borderColor
        }

        // Details grid
        GridLayout {
            Layout.fillWidth: true
            columns: 2
            columnSpacing: 10
            rowSpacing: 5

            Label {
                text: "[D] Date:"
                font.pixelSize: 11
                color: "#7f8c8d"
            }

            Label {
                text: examDate
                font.pixelSize: 11
                color: textColor
                Layout.fillWidth: true
            }

            Label {
                text: "[T] Duration:"
                font.pixelSize: 11
                color: "#7f8c8d"
            }

            Label {
                text: duration
                font.pixelSize: 11
                color: textColor
                Layout.fillWidth: true
            }

            Label {
                text: "[C] Channels:"
                font.pixelSize: 11
                color: "#7f8c8d"
            }

            Label {
                text: channelsInfo
                font.pixelSize: 11
                color: textColor
                Layout.fillWidth: true
            }
        }

        Item { Layout.fillHeight: true }

        // Action buttons
        RowLayout {
            Layout.fillWidth: true
            spacing: 8

            Button {
                text: "[>] Preview"
                font.pixelSize: 10
                Layout.fillWidth: true
                Layout.preferredHeight: 30
                onClicked: root.previewClicked()
            }

            Button {
                text: "[R] Report"
                font.pixelSize: 10
                Layout.fillWidth: true
                Layout.preferredHeight: 30
                onClicked: root.reportClicked()
            }
        }
    }
}
