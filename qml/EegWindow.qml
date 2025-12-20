import QtQuick
import QtQuick.Controls
import QtQuick.Window
import videoEeg

ApplicationWindow {
    id: eegWindow
    width: 1920
    height: 1080
    title: "EEG Viewer"
    visible: true

    property int amplifierId: -1
    property var channels: []
    property int channelCount: channels.length

    EegBackend {
        id: backend
        amplifierId: eegWindow.amplifierId
        channels: eegWindow.channels
    }

    Component.onCompleted: {
        console.log("EegWindow opened with channels:", channels)
        console.log("Channels count:", channelCount)
        console.log("Amplifier ID:", amplifierId)
    }

    header: ToolBar {
        Row {
            spacing: 20
            anchors.verticalCenter: parent.verticalCenter
            leftPadding: 10

            Label {
                text: "EEG Viewer"
                font.bold: true
                font.pixelSize: 14
                anchors.verticalCenter: parent.verticalCenter
            }

            Label {
                text: channelCount + " channels"
                anchors.verticalCenter: parent.verticalCenter
                color: "#888888"
            }

            Rectangle {
                width: 1
                height: 30
                color: "#555555"
                anchors.verticalCenter: parent.verticalCenter
            }

            Label {
                text: "Time:"
                anchors.verticalCenter: parent.verticalCenter
            }

            Slider {
                id: timeSlider
                from: 5
                to: 30
                value: 10
                stepSize: 1
                width: 150
                anchors.verticalCenter: parent.verticalCenter
            }

            Label {
                text: timeSlider.value.toFixed(0) + "s"
                anchors.verticalCenter: parent.verticalCenter
                color: "#000000"
                font.pixelSize: 12
            }

            Rectangle {
                width: 1
                height: 30
                color: "#000000"
                anchors.verticalCenter: parent.verticalCenter
            }

            Label {
                text: "Gain:"
                anchors.verticalCenter: parent.verticalCenter
            }

            Slider {
                id: amplitudeSlider
                from: 0.05
                to: 2
                value: 0.5
                stepSize: 0.05
                width: 150
                anchors.verticalCenter: parent.verticalCenter
            }

            Label {
                text: amplitudeSlider.value.toFixed(1)
                anchors.verticalCenter: parent.verticalCenter
                color: "#000000"
                font.pixelSize: 12
            }

            Rectangle {
                width: 1
                height: 30
                color: "#555555"
                anchors.verticalCenter: parent.verticalCenter
            }

            Label {
                text: "Spacing:"
                anchors.verticalCenter: parent.verticalCenter
            }

            Slider {
                id: spacingSlider
                from: 50
                to: 500
                value: 150
                stepSize: 10
                width: 150
                anchors.verticalCenter: parent.verticalCenter
            }

            CheckBox {
                id: gridCheckbox
                text: "Grid"
                checked: true
                anchors.verticalCenter: parent.verticalCenter
            }
        }
    }
    EegPlotItem {
        id: myEeg
        anchors.fill: parent
        anchors.margins: 20
        backend: backend
    }
}
