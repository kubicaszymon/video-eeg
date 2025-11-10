import QtQuick
import QtQuick.Controls
import QtQuick.Window
import videoEeg

ApplicationWindow {
    id: eegWindow
    width: 1920
    height: 1080
    title: "EEG Channels"
    visible: true

    property var channelIndices: []
    property var channelNames: []
    property string amplifierId: ""

    EegViewModel {
        id: viewModel
    }

    Component.onCompleted: {
        console.log("EegWindow opened with channels:", channelIndices)
        console.log("Amplifier ID:", amplifierId)

        viewModel.initialize(amplifierId, channelIndices)
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
                model: channelNames

                EegChannel {
                    width: graphColumn.width - 20
                    height: 100
                    channelIndex: index
                    name: modelData
                }
            }
        }
    }
}


