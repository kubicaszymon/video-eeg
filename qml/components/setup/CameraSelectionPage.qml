import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: root
    color: bgColor

    // Required properties from parent
    required property var availableCameras
    required property int selectedCameraIndex
    required property color bgColor
    required property color cardColor
    required property color borderColor
    required property color textColor
    required property color accentColor
    required property color successColor
    required property color hoverColor

    signal nextClicked()
    signal backClicked()
    signal cancelClicked()
    signal cameraSelected(int index)
    signal refreshCamerasClicked()

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
                    text: "[cam]"
                    font.pixelSize: 24
                }

                Label {
                    text: "Select a camera to synchronize video with EEG data (optional)"
                    font.pixelSize: 12
                    color: "#0c5460"
                    Layout.fillWidth: true
                    wrapMode: Text.WordWrap
                }
            }
        }

        // Cameras list card
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
                        text: "Available Cameras"
                        font.pixelSize: 16
                        font.bold: true
                        color: textColor
                        Layout.fillWidth: true
                    }

                    Label {
                        text: availableCameras.length + " found"
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
                        model: availableCameras
                        spacing: 10

                        delegate: Rectangle {
                            width: ListView.view.width
                            height: 80
                            color: selectedCameraIndex === index ? "#e8f4f8" : cardColor
                            radius: 6
                            border.color: selectedCameraIndex === index ? accentColor : borderColor
                            border.width: selectedCameraIndex === index ? 2 : 1

                            MouseArea {
                                anchors.fill: parent
                                hoverEnabled: true
                                onEntered: parent.color = selectedCameraIndex === index ? "#e8f4f8" : hoverColor
                                onExited: parent.color = selectedCameraIndex === index ? "#e8f4f8" : cardColor
                                onClicked: root.cameraSelected(index)
                            }

                            RowLayout {
                                anchors.fill: parent
                                anchors.margins: 15
                                spacing: 15

                                Rectangle {
                                    Layout.preferredWidth: 50
                                    Layout.preferredHeight: 50
                                    radius: 25
                                    color: selectedCameraIndex === index ? accentColor : "#ecf0f1"

                                    Label {
                                        anchors.centerIn: parent
                                        text: "[v]"
                                        font.pixelSize: 18
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
                                        text: "1920x1080 - 30 FPS"
                                        font.pixelSize: 11
                                        color: "#7f8c8d"
                                    }
                                }

                                Rectangle {
                                    visible: selectedCameraIndex === index
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
                    }
                }

                // Option: no camera
                Rectangle {
                    Layout.fillWidth: true
                    height: 60
                    color: selectedCameraIndex === -1 ? "#fff3cd" : cardColor
                    radius: 6
                    border.color: selectedCameraIndex === -1 ? "#ffc107" : borderColor
                    border.width: selectedCameraIndex === -1 ? 2 : 1

                    MouseArea {
                        anchors.fill: parent
                        onClicked: root.cameraSelected(-1)
                    }

                    RowLayout {
                        anchors.fill: parent
                        anchors.margins: 15
                        spacing: 12

                        Label {
                            text: "[x]"
                            font.pixelSize: 20
                            color: "#856404"
                        }

                        Label {
                            text: "Continue without camera (EEG data only)"
                            font.pixelSize: 12
                            font.bold: selectedCameraIndex === -1
                            color: "#856404"
                            Layout.fillWidth: true
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
                text: "Refresh cameras"
                font.pixelSize: 13
                Layout.preferredWidth: 140
                Layout.preferredHeight: 45
                palette.button: "#95a5a6"
                palette.buttonText: "white"
                onClicked: root.refreshCamerasClicked()
            }

            Item { Layout.fillWidth: true }

            Button {
                text: "Back"
                font.pixelSize: 13
                Layout.preferredWidth: 120
                Layout.preferredHeight: 45
                onClicked: root.backClicked()
            }

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
                Layout.preferredWidth: 120
                Layout.preferredHeight: 45
                palette.button: accentColor
                palette.buttonText: "white"
                onClicked: root.nextClicked()
            }
        }
    }
}
