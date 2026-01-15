import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: root
    color: bgColor

    // Required properties from parent
    required property var backend
    required property var channelSelectionModel
    required property color bgColor
    required property color cardColor
    required property color borderColor
    required property color textColor
    required property color accentColor

    signal nextClicked()
    signal backClicked()
    signal cancelClicked()
    signal toggleChannel(int index)
    signal selectAllChannels(bool checked)

    function getSelectedCount() {
        var count = 0
        for (var i = 0; i < channelSelectionModel.length; i++) {
            if (channelSelectionModel[i]) count++
        }
        return count
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 30
        spacing: 20

        // Success banner showing selected amplifier
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 60
            color: "#d4edda"
            radius: 8
            border.color: "#c3e6cb"
            border.width: 1

            RowLayout {
                anchors.fill: parent
                anchors.margins: 15
                spacing: 12

                Label {
                    text: "ok"
                    font.pixelSize: 20
                    font.bold: true
                    color: "#155724"
                }

                ColumnLayout {
                    spacing: 2
                    Layout.fillWidth: true

                    Label {
                        text: "Selected amplifier: " + backend.availableAmplifiers[backend.selectedAmplifierIndex]
                        font.pixelSize: 12
                        font.bold: true
                        color: "#155724"
                    }

                    Label {
                        text: backend.currentChannels.length + " available channels"
                        font.pixelSize: 11
                        color: "#155724"
                    }
                }
            }
        }

        // Channels table
        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: cardColor
            radius: 8
            border.color: borderColor
            border.width: 1

            ColumnLayout {
                anchors.fill: parent
                spacing: 0

                // Table header
                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 50
                    color: "#f8f9fa"
                    radius: 8

                    RowLayout {
                        anchors.fill: parent
                        anchors.leftMargin: 20
                        anchors.rightMargin: 20
                        spacing: 0

                        Label {
                            text: "No."
                            font.pixelSize: 12
                            font.bold: true
                            color: textColor
                            Layout.preferredWidth: 60
                        }

                        Label {
                            text: "Channel Name"
                            font.pixelSize: 12
                            font.bold: true
                            color: textColor
                            Layout.fillWidth: true
                        }

                        CheckBox {
                            text: "Select all"
                            font.pixelSize: 11
                            checked: false
                            onClicked: root.selectAllChannels(checked)
                        }
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
                        model: backend.currentChannels
                        spacing: 0

                        delegate: Rectangle {
                            width: ListView.view.width
                            height: 45
                            color: {
                                if (channelSelectionModel[index]) return "#e8f4f8"
                                return index % 2 === 0 ? "white" : "#f8f9fa"
                            }

                            MouseArea {
                                anchors.fill: parent
                                onClicked: root.toggleChannel(index)
                            }

                            RowLayout {
                                anchors.fill: parent
                                anchors.leftMargin: 20
                                anchors.rightMargin: 20
                                spacing: 15

                                Label {
                                    text: (index + 1).toString()
                                    font.pixelSize: 12
                                    color: textColor
                                    Layout.preferredWidth: 60
                                }

                                Label {
                                    text: modelData
                                    font.pixelSize: 12
                                    color: textColor
                                    Layout.fillWidth: true
                                }

                                CheckBox {
                                    checked: channelSelectionModel[index] || false
                                    onClicked: root.toggleChannel(index)
                                }
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

            Label {
                text: getSelectedCount() + " / " + backend.currentChannels.length + " selected"
                font.pixelSize: 12
                color: "#7f8c8d"
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
                enabled: getSelectedCount() > 0
                Layout.preferredWidth: 120
                Layout.preferredHeight: 45
                palette.button: accentColor
                palette.buttonText: "white"
                onClicked: root.nextClicked()
            }
        }
    }
}
