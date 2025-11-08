import QtQuick
import QtQuick.Controls
import QtQuick.Window

ApplicationWindow {
    id: graphsWindow
    width: 1920
    height: 1080
    title: "EEG Channels - " + eegViewModel.channel_count + " channels"
    visible: false

    onClosing: (close) => {
                   close.accepted = true
                   graphsWindow.hide()
               }

    header: ToolBar {
        Row {
            spacing: 10
            anchors.verticalCenter: parent.verticalCenter

            Label {
                text: "EEG Graphs"
                font.bold: true
                anchors.verticalCenter: parent.verticalCenter
            }

            ToolSeparator {}

            Rectangle {
                width: 15
                height: 15
                radius: 7.5
                color: eegViewModel.is_streaming ? "#4CAF50" : "#F44336"
                anchors.verticalCenter: parent.verticalCenter
            }

            Label {
                text: eegViewModel.is_streaming ? "Streaming" : "Stopped"
                anchors.verticalCenter: parent.verticalCenter
            }
        }
    }

    // Main graphs
    ScrollView{
        anchors.fill: parent
        clip: true

        Column {
            id: graphColumn
            width: graphsWindow.width
            spacing: 5
            padding: 10

            Repeater {
                model: eegViewModel.channel_count

                EegChannelGraph {
                    width: graphColumn.width - 20
                    height: 100
                    channelIndex: index
                }
            }
        }
    }
}


