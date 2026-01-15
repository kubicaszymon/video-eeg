import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: root
    color: bgColor

    // Required properties from parent
    required property var backend
    required property var availableCameras
    required property int selectedCameraIndex
    required property string savePath
    required property color bgColor
    required property color cardColor
    required property color borderColor
    required property color textColor
    required property color successColor

    // Functions to be provided by parent
    property var getSelectedChannelsCount: function() { return 0 }
    property var getSelectedChannelsList: function() { return [] }

    signal startClicked()
    signal backClicked()
    signal cancelClicked()
    signal browseClicked()

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 30
        spacing: 20

        Label {
            text: "Configuration Summary"
            font.pixelSize: 18
            font.bold: true
            color: textColor
        }

        ScrollView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true

            ColumnLayout {
                width: parent.width
                spacing: 15

                // Amplifier section
                Rectangle {
                    Layout.fillWidth: true
                    height: contentCol1.implicitHeight + 30
                    color: cardColor
                    radius: 8
                    border.color: borderColor
                    border.width: 1

                    ColumnLayout {
                        id: contentCol1
                        anchors.fill: parent
                        anchors.margins: 15
                        spacing: 8

                        Label {
                            text: "Amplifier"
                            font.pixelSize: 14
                            font.bold: true
                            color: textColor
                        }

                        Rectangle {
                            Layout.fillWidth: true
                            height: 1
                            color: borderColor
                        }

                        Label {
                            text: backend.availableAmplifiers[backend.selectedAmplifierIndex]
                            font.pixelSize: 13
                            color: "#7f8c8d"
                        }
                    }
                }

                // Channels section
                Rectangle {
                    Layout.fillWidth: true
                    height: contentCol2.implicitHeight + 30
                    color: cardColor
                    radius: 8
                    border.color: borderColor
                    border.width: 1

                    ColumnLayout {
                        id: contentCol2
                        anchors.fill: parent
                        anchors.margins: 15
                        spacing: 8

                        Label {
                            text: "Selected Channels (" + getSelectedChannelsCount() + ")"
                            font.pixelSize: 14
                            font.bold: true
                            color: textColor
                        }

                        Rectangle {
                            Layout.fillWidth: true
                            height: 1
                            color: borderColor
                        }

                        Label {
                            text: getSelectedChannelsList().join(", ")
                            font.pixelSize: 12
                            color: "#7f8c8d"
                            wrapMode: Text.WordWrap
                            Layout.fillWidth: true
                        }
                    }
                }

                // Camera section
                Rectangle {
                    Layout.fillWidth: true
                    height: contentCol3.implicitHeight + 30
                    color: cardColor
                    radius: 8
                    border.color: borderColor
                    border.width: 1

                    ColumnLayout {
                        id: contentCol3
                        anchors.fill: parent
                        anchors.margins: 15
                        spacing: 8

                        Label {
                            text: "Camera"
                            font.pixelSize: 14
                            font.bold: true
                            color: textColor
                        }

                        Rectangle {
                            Layout.fillWidth: true
                            height: 1
                            color: borderColor
                        }

                        Label {
                            text: selectedCameraIndex >= 0 ? availableCameras[selectedCameraIndex] : "None (EEG only)"
                            font.pixelSize: 13
                            color: "#7f8c8d"
                        }
                    }
                }

                // Save location section
                Rectangle {
                    Layout.fillWidth: true
                    height: contentCol4.implicitHeight + 30
                    color: cardColor
                    radius: 8
                    border.color: borderColor
                    border.width: 1

                    ColumnLayout {
                        id: contentCol4
                        anchors.fill: parent
                        anchors.margins: 15
                        spacing: 8

                        Label {
                            text: "Recording Save Location"
                            font.pixelSize: 14
                            font.bold: true
                            color: textColor
                        }

                        Rectangle {
                            Layout.fillWidth: true
                            height: 1
                            color: borderColor
                        }

                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 10

                            TextField {
                                Layout.fillWidth: true
                                text: savePath || "Select location..."
                                readOnly: true
                                font.pixelSize: 11
                            }

                            Button {
                                text: "Browse"
                                font.pixelSize: 11
                                Layout.preferredHeight: 35
                                onClicked: root.browseClicked()
                            }
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
                text: "Back"
                font.pixelSize: 13
                Layout.preferredWidth: 120
                Layout.preferredHeight: 45
                onClicked: root.backClicked()
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
                text: "Start Examination"
                font.pixelSize: 13
                font.bold: true
                Layout.preferredWidth: 180
                Layout.preferredHeight: 45
                palette.button: successColor
                palette.buttonText: "white"
                onClicked: root.startClicked()
            }
        }
    }
}
