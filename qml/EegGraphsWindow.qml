import QtQuick
import QtQuick.Controls
import QtQuick.Window

ApplicationWindow {
    id: graphsWindow
    width: 1920
    height: 1080
    title: "EEG Channels - " + eegViewModel.channelCount + " channels"
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

            Label {
                text: "Display Window:"
                anchors.verticalCenter: parent.verticalCenter
            }

            ComboBox {
                id: windowSizeCombo
                model: ["1 sec", "5 sec", "10 sec", "30 sec"]
                currentIndex: 1
                anchors.verticalCenter: parent.verticalCenter

                onCurrentIndexChanged: {
                    var sizes = [1000, 5000, 10000, 30000]
                    eegViewModel.setDisplayWindowSize(sizes[currentIndex])
                }
            }

            ToolButton {
                text: "Clear All"
                onClicked: eegViewModel.clearAllChannels()
            }

            ToolSeparator {}

            Rectangle {
                width: 15
                height: 15
                radius: 7.5
                color: eegViewModel.isStreaming ? "#4CAF50" : "#F44336"
                anchors.verticalCenter: parent.verticalCenter
            }

            Label {
                text: eegViewModel.isStreaming ? "Streaming" : "Stopped"
                anchors.verticalCenter: parent.verticalCenter
            }
        }
    }

    // Main graphs
    ScrollView{
        anchors.fill: parent
        clip: true

        Grid {
            id: graphGrid
            columns: 6
            rows: 5
            columnSpacing: 5
            rowSpacing: 5
            padding: 10

            Repeater {
                model: eegViewModel.channelCount

                EegChannelGraph {
                    width: (graphsWindow.width - graphGrid.columnSpacing * 5 - 20) / 6
                    height: (graphsWindow.height - graphGrid.rowSpacing * 4 - 100) / 5
                    channelIndex: index
                }
            }
        }
    }
}


