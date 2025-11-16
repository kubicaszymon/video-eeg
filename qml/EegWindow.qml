import QtQuick
import QtQuick.Controls
import QtQuick.Window
import videoEeg

ApplicationWindow {
    id: eegWindow
    width: 1920
    height: 1080
    title: "EEG Viewer - Scrolling Window"
    visible: true

    property var channelIndices: []
    property string amplifierId: ""

    EegViewModel {
        id: eegViewModel
    }

    Component.onCompleted: {
        console.log("EegWindow opened with channels:", channelIndices)
        console.log("Amplifier ID:", amplifierId)
        eegViewModel.initialize(amplifierId, channelIndices)
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
                text: eegViewModel.channelCount + " channels"
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

            // Amplitude control - NAPRAWIONE (O WIELE MNIEJSZE WARTOŚCI)
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

    // Main unified canvas
    ScrollView {
        anchors.fill: parent
        clip: true
        contentHeight: eegViewModel.channelCount * spacingSlider.value + 100

        EegCanva {
            id: eegCanvas
            width: eegWindow.width
            height: eegViewModel.channelCount * spacingSlider.value + 100

            viewModel: eegViewModel
            amplitudeScale: amplitudeSlider.value
            channelSpacing: spacingSlider.value
            showGrid: gridCheckbox.checked
            timeWindow: timeSlider.value
        }
    }
}
