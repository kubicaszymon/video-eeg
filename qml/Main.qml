import QtQuick
import QtQuick.Controls
import QtQuick.Window

Window {
    id: mainWindow
    visible: true
    width: 800
    height: 600
    title: qsTr("VideoEEG")

    EegGraphsWindow {
        id: graphsWindow
    }

    Column {
        anchors.centerIn: parent
        spacing: 20

        Label {
            text: "Video EEG Application"
            font.pixelSize: 24
            anchors.horizontalCenter: parent.horizontalCenter
        }

        Button {
            text: "Scan for amplifiers"
            anchors.horizontalCenter: parent.horizontalCenter
            onClicked: veegapp.refreshAmplifiersList();
        }

        ComboBox {
            id: amplifierCombo
            anchors.horizontalCenter: parent.horizontalCenter
            model: veegapp.amplifierNames
            onCurrentIndexChanged: {
                veegapp.selectedAmplifierIndex = currentIndex
            }
        }

        ListView {
            model: veegapp.currentAmplifierChannels
            delegate: Text {
                text: modelData
            }
        }

        Button {
            text: "Start Preview"
            anchors.horizontalCenter: parent.horizontalCenter
            onClicked: graphsWindow.show()
        }

        Row {
            spacing: 10
            anchors.horizontalCenter: parent.horizontalCenter

            Label {
                text: "Streaming:"
                anchors.verticalCenter: parent.verticalCenter
            }

            Rectangle {
                width: 20
                height: 20
                radius: 10
                color: eegViewModel.isStreaming ? "green" : "red"
                anchors.verticalCenter: parent.verticalCenter
            }
        }

        Label {
            text: "Channels: " + eegViewModel.channelCount
            anchors.horizontalCenter: parent.horizontalCenter
        }
    }
}
