import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import videoEeg

Window {
    id: window
    visible: false
    width: 1400
    height: 700
    title: qsTr("Amplifier Setup")

    signal accepted(amplifierId: string, channels: var)
    signal rejected()

    property int loading: Globals.status

    AmplifierSetupBackend {
        id: backend
    }

    Timer {
        id: timer

        running: true
        repeat: true

        onTriggered: {
            if(window.loading === Globals.Loading){
                Globals.status = Globals.Ready
            }
        }
    }

    BusyIndicator {
        width: 50
        height: 50
        anchors.horizontalCenter: parent.horizontalCenter
        running: window.loading
    }

    StackView {
        id: stackView
        anchors.fill: parent
        initialItem: amplifierSelectionPage
    }

    Component {
        id: amplifierSelectionPage

        // Left panel - Amplifiers
        Rectangle {
            Layout.fillHeight: true
            Layout.preferredWidth: 400
            border.color: "#cccccc"
            border.width: 1
            color: "#f5f5f5"

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 10
                spacing: 10

                Label {
                    text: "Online amplifiers"
                    font.pixelSize: 16
                    font.bold: true
                }

                Label {
                    text: "Choose amplifier"
                    font.pixelSize: 12
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    border.color: "#cccccc"
                    border.width: 1
                    color: "white"

                    ListView {
                        id: amplifierListView
                        anchors.fill: parent
                        anchors.margins: 5
                        currentIndex: backend.selectedAmplifierIndex
                        clip: true
                        model: backend.availableAmplifiers
                        delegate: Rectangle {
                            width: amplifierListView.width - 10
                            height: 60
                            color: backend.selectedAmplifierIndex === index ? "#ffa726" : "white"
                            border.color: "#cccccc"
                            border.width: 1

                            ColumnLayout {
                                anchors.fill: parent
                                anchors.margins: 5

                                Label {
                                    text: modelData
                                    font.pixelSize: 14
                                    font.bold: true
                                }

                                Label {
                                    text: "virtual"
                                    font.pixelSize: 10
                                    color: "#666666"
                                }
                            }

                            MouseArea {
                                anchors.fill: parent
                                onClicked: {
                                    console.log("Selected amplifier " + index)
                                    backend.selectedAmplifierIndex = index
                                }
                            }
                        }

                        // Show message when no amplifiers found
                        Label {
                            anchors.centerIn: parent
                            text: "No amplifiers found\nClick Refresh to scan"
                            color: "#999999"
                            horizontalAlignment: Text.AlignHCenter
                            visible: amplifierListView.count === 0
                        }
                    }
                }

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 10

                    Button {
                        text: "Refresh"
                        enabled: !window.loading
                        Layout.fillWidth: true
                        onClicked: {
                            Globals.status = Globals.Loading
                            timer.restart()
                            backend.refreshAmplifiersList()
                        }
                    }

                    Button {
                        text: "Next"
                        enabled: backend.selectedAmplifierIndex !== -1
                        onClicked: stackView.push(channelSelectionPage)
                    }
                }
            }
        }
    }

    Component {
        id: channelSelectionPage

        // Right panel - Channels
        Rectangle {
            Layout.fillHeight: true
            Layout.fillWidth: true
            border.color: "#cccccc"
            border.width: 1
            color: "#f5f5f5"

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 10
                spacing: 10

                Label {
                    text: "Available channels"
                    font.pixelSize: 16
                    font.bold: true
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    border.color: "#cccccc"
                    border.width: 1
                    color: "white"

                    // Header
                    Rectangle {
                        id: tableHeader
                        anchors.top: parent.top
                        anchors.left: parent.left
                        anchors.right: parent.right
                        height: 30
                        color: "#f0f0f0"
                        border.color: "#cccccc"
                        border.width: 1

                        RowLayout {
                            anchors.fill: parent
                            spacing: 0

                            Label {
                                text: "channel number"
                                Layout.preferredWidth: 120
                                Layout.fillHeight: true
                                leftPadding: 10
                                verticalAlignment: Text.AlignVCenter
                                font.pixelSize: 11
                            }

                            Rectangle {
                                Layout.preferredWidth: 1
                                Layout.fillHeight: true
                                color: "#cccccc"
                            }

                            Label {
                                text: "label"
                                Layout.fillWidth: true
                                Layout.fillHeight: true
                                leftPadding: 10
                                verticalAlignment: Text.AlignVCenter
                                font.pixelSize: 11
                            }

                            Rectangle {
                                Layout.preferredWidth: 1
                                Layout.fillHeight: true
                                color: "#cccccc"
                            }

                            Label {
                                text: "selected"
                                Layout.preferredWidth: 80
                                Layout.fillHeight: true
                                leftPadding: 10
                                verticalAlignment: Text.AlignVCenter
                                font.pixelSize: 11
                            }
                        }

                        RowLayout {
                            anchors.fill: parent

                            CheckBox {
                                id: selectAllCheck
                                Layout.alignment: Qt.AlignRight
                                checked: false

                                onClicked: {
                                    for (var i = 0; i < channelsListView.count; i++)
                                    {
                                        var item = channelsListView.itemAtIndex(i)
                                        if (item)
                                        {
                                            item.isSelected = checked
                                        }
                                    }
                                }
                            }
                        }
                    }

                    // Channels list
                    ListView {
                        id: channelsListView
                        anchors.top: tableHeader.bottom
                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.bottom: parent.bottom
                        clip: true
                        model: backend.currentChannels

                        delegate: Rectangle {
                            width: channelsListView.width
                            height: 25
                            color: index % 2 === 0 ? "white" : "#f9f9f9"
                            border.color: "#e0e0e0"
                            border.width: 1

                            property bool channelChecked: false

                            RowLayout {
                                anchors.fill: parent
                                spacing: 0

                                Label {
                                    text: (index + 1).toString()
                                    Layout.preferredWidth: 120
                                    Layout.fillHeight: true
                                    leftPadding: 10
                                    verticalAlignment: Text.AlignVCenter
                                    font.pixelSize: 11
                                }

                                Rectangle {
                                    Layout.preferredWidth: 1
                                    Layout.fillHeight: true
                                    color: "#e0e0e0"
                                }

                                Label {
                                    text: modelData
                                    Layout.fillWidth: true
                                    Layout.fillHeight: true
                                    leftPadding: 10
                                    verticalAlignment: Text.AlignVCenter
                                    font.pixelSize: 11
                                }

                                Rectangle {
                                    Layout.preferredWidth: 1
                                    Layout.fillHeight: true
                                    color: "#e0e0e0"
                                }

                                CheckBox {
                                    Layout.preferredWidth: 80
                                    Layout.fillHeight: true
                                    Layout.alignment: Qt.AlignHCenter
                                    checked: parent.parent.channelChecked
                                    onCheckedChanged: {
                                        parent.parent.channelChecked = checked
                                    }
                                }
                            }
                        }
                    }
                }

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 10

                    Button {
                        text: "Back"
                        onClicked: {
                            stackView.pop()
                        }
                    }

                    Button {
                        text: "Finish"
                        onClicked: {
                            var selectedChannels = []
                            for (var i = 0; i < channelsListView.count; i++)
                            {
                                var item = channelsListView.itemAtIndex(i)
                                if (item.channelChecked)
                                {
                                    selectedChannels.push(i)
                                    console.log("Selected channel: " + i)
                                }
                            }
                            accepted(backend.getSelectedAmplifierId(), selectedChannels)
                            window.close()
                        }
                    }
                }
            }
        }
    }
}
