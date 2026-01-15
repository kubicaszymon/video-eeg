import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: root
    color: cardColor
    border.color: borderColor
    border.width: 1

    // Required properties
    required property color cardColor
    required property color borderColor
    required property color sidebarColor
    required property color textColor
    required property color hoverColor

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 0
        spacing: 0

        // Header
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 60
            color: sidebarColor

            RowLayout {
                anchors.fill: parent
                anchors.margins: 15
                spacing: 10

                Label {
                    text: "[P] Patients"
                    font.pixelSize: 18
                    font.bold: true
                    color: "white"
                    Layout.fillWidth: true
                }

                Button {
                    text: "+ New"
                    font.pixelSize: 12
                    Layout.preferredWidth: 80
                    Layout.preferredHeight: 35
                    palette.button: "#2ecc71"
                    palette.buttonText: "white"
                }
            }
        }

        // Search bar
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 50
            color: "white"
            border.color: borderColor
            border.width: 1

            RowLayout {
                anchors.fill: parent
                anchors.margins: 10
                spacing: 5

                Label {
                    text: "[?]"
                    font.pixelSize: 16
                }

                TextField {
                    Layout.fillWidth: true
                    placeholderText: "Search patient (ID, surname...)"
                    font.pixelSize: 12
                }
            }
        }

        // Patient list
        ScrollView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true

            ListView {
                anchors.fill: parent
                model: 8
                spacing: 1

                delegate: Rectangle {
                    width: ListView.view.width
                    height: 80
                    color: mouseArea.containsMouse ? hoverColor : "white"
                    border.color: borderColor
                    border.width: 1

                    MouseArea {
                        id: mouseArea
                        anchors.fill: parent
                        hoverEnabled: true
                    }

                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: 12
                        spacing: 4

                        Label {
                            text: "Jan Kowalski"
                            font.pixelSize: 14
                            font.bold: true
                            color: textColor
                        }

                        Label {
                            text: "ID: 85010112345"
                            font.pixelSize: 11
                            color: "#7f8c8d"
                        }

                        Label {
                            text: "Last examination: 10.01.2025"
                            font.pixelSize: 11
                            color: "#7f8c8d"
                        }
                    }
                }
            }
        }
    }
}
