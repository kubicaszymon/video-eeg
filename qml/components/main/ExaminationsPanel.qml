import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: root
    color: bgColor

    // Required properties
    required property color bgColor
    required property color cardColor
    required property color borderColor
    required property color sidebarColor
    required property color accentColor
    required property color textColor

    signal newExaminationClicked()

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
                    text: "[E] EEG Examinations"
                    font.pixelSize: 18
                    font.bold: true
                    color: "white"
                    Layout.fillWidth: true
                }

                Button {
                    text: "[+] New EEG Examination"
                    font.pixelSize: 12
                    font.bold: true
                    Layout.preferredWidth: 180
                    Layout.preferredHeight: 40
                    palette.button: accentColor
                    palette.buttonText: "white"
                    onClicked: root.newExaminationClicked()
                }
            }
        }

        // Filters bar
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 50
            color: "white"
            border.color: borderColor
            border.width: 1

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 15
                anchors.rightMargin: 15
                spacing: 15

                Label {
                    text: "Filter:"
                    font.pixelSize: 12
                    color: textColor
                }

                ComboBox {
                    Layout.preferredWidth: 150
                    Layout.preferredHeight: 30
                    model: ["All", "Today", "This week", "This month"]
                    font.pixelSize: 11
                }

                ComboBox {
                    Layout.preferredWidth: 150
                    Layout.preferredHeight: 30
                    model: ["All statuses", "Completed", "In progress", "Cancelled"]
                    font.pixelSize: 11
                }

                Item { Layout.fillWidth: true }

                Label {
                    text: "Sort:"
                    font.pixelSize: 12
                    color: textColor
                }

                ComboBox {
                    Layout.preferredWidth: 150
                    Layout.preferredHeight: 30
                    model: ["Newest", "Oldest", "By patient"]
                    font.pixelSize: 11
                }
            }
        }

        // Examination cards grid
        ScrollView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            contentWidth: availableWidth

            Flow {
                width: parent.width
                padding: 15
                spacing: 15

                Repeater {
                    model: 6

                    ExaminationCard {
                        width: (parent.width - parent.padding * 2 - parent.spacing) / 2
                        height: 200
                        cardColor: root.cardColor
                        borderColor: root.borderColor
                        accentColor: root.accentColor
                        textColor: root.textColor
                    }
                }
            }
        }
    }
}
