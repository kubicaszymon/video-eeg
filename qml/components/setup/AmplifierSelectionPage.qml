import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: root
    color: bgColor

    // Required properties from parent
    required property var backend
    required property color bgColor
    required property color cardColor
    required property color borderColor
    required property color textColor
    required property color accentColor
    required property color successColor
    required property color hoverColor

    signal nextClicked()
    signal cancelClicked()
    signal refreshClicked()

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 30
        spacing: 20

        // Info banner
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 60
            color: "#e8f4f8"
            radius: 8
            border.color: "#bee5eb"
            border.width: 1

            RowLayout {
                anchors.fill: parent
                anchors.margins: 15
                spacing: 12

                Label {
                    text: "i"
                    font.pixelSize: 24
                }

                Label {
                    text: "Make sure the amplifier is turned on and connected to the computer"
                    font.pixelSize: 12
                    color: "#0c5460"
                    Layout.fillWidth: true
                    wrapMode: Text.WordWrap
                }
            }
        }

        // Amplifiers list card
        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: cardColor
            radius: 8
            border.color: borderColor
            border.width: 1

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 20
                spacing: 15

                RowLayout {
                    Layout.fillWidth: true

                    Label {
                        text: "Detected Amplifiers"
                        font.pixelSize: 16
                        font.bold: true
                        color: textColor
                        Layout.fillWidth: true
                    }

                    Label {
                        text: backend.availableAmplifiers.length + " found"
                        font.pixelSize: 12
                        color: "#7f8c8d"
                    }
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 1
                    color: borderColor
                }

                ScrollView {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    clip: true

                    ListView {
                        id: amplifierListView
                        model: backend.availableAmplifiers
                        spacing: 10

                        delegate: Rectangle {
                            width: ListView.view.width
                            height: 80
                            color: backend.selectedAmplifierIndex === index ? "#e8f4f8" : cardColor
                            radius: 6
                            border.color: backend.selectedAmplifierIndex === index ? accentColor : borderColor
                            border.width: backend.selectedAmplifierIndex === index ? 2 : 1

                            MouseArea {
                                anchors.fill: parent
                                hoverEnabled: true
                                onEntered: parent.color = backend.selectedAmplifierIndex === index ? "#e8f4f8" : hoverColor
                                onExited: parent.color = backend.selectedAmplifierIndex === index ? "#e8f4f8" : cardColor
                                onClicked: backend.selectedAmplifierIndex = index
                            }

                            RowLayout {
                                anchors.fill: parent
                                anchors.margins: 15
                                spacing: 15

                                Rectangle {
                                    Layout.preferredWidth: 50
                                    Layout.preferredHeight: 50
                                    radius: 25
                                    color: backend.selectedAmplifierIndex === index ? accentColor : "#ecf0f1"

                                    Label {
                                        anchors.centerIn: parent
                                        text: "~"
                                        font.pixelSize: 24
                                        color: backend.selectedAmplifierIndex === index ? "white" : textColor
                                    }
                                }

                                ColumnLayout {
                                    Layout.fillWidth: true
                                    spacing: 4

                                    Label {
                                        text: modelData
                                        font.pixelSize: 14
                                        font.bold: true
                                        color: textColor
                                    }

                                    Label {
                                        text: "Virtual Amplifier - Ready"
                                        font.pixelSize: 11
                                        color: "#7f8c8d"
                                    }
                                }

                                Rectangle {
                                    visible: backend.selectedAmplifierIndex === index
                                    Layout.preferredWidth: 24
                                    Layout.preferredHeight: 24
                                    radius: 12
                                    color: successColor

                                    Label {
                                        anchors.centerIn: parent
                                        text: "ok"
                                        font.pixelSize: 10
                                        font.bold: true
                                        color: "white"
                                    }
                                }
                            }
                        }

                        Label {
                            anchors.centerIn: parent
                            visible: amplifierListView.count === 0
                            text: "No amplifiers found\nClick 'Refresh' to scan again"
                            font.pixelSize: 13
                            color: "#999999"
                            horizontalAlignment: Text.AlignHCenter
                        }
                    }
                }
            }
        }

        // Bottom buttons
        RowLayout {
            Layout.fillWidth: true
            spacing: 15

            Button {
                text: "Refresh"
                font.pixelSize: 13
                Layout.preferredWidth: 120
                Layout.preferredHeight: 45
                palette.button: "#95a5a6"
                palette.buttonText: "white"
                onClicked: root.refreshClicked()
            }

            Item { Layout.fillWidth: true }

            Button {
                text: "Cancel"
                font.pixelSize: 13
                Layout.preferredWidth: 100
                Layout.preferredHeight: 45
                onClicked: root.cancelClicked()
            }

            Button {
                text: "Next"
                font.pixelSize: 13
                font.bold: true
                enabled: backend.selectedAmplifierIndex !== -1
                Layout.preferredWidth: 120
                Layout.preferredHeight: 45
                palette.button: accentColor
                palette.buttonText: "white"
                onClicked: root.nextClicked()
            }
        }
    }
}
