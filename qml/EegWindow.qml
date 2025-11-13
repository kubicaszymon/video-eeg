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

            // Time window control - NAPRAWIONE
            Label {
                text: "Time:"
                anchors.verticalCenter: parent.verticalCenter
            }

            Slider {
                id: timeSlider
                from: 5           // Min: 5 sekund
                to: 30            // Max: 30 sekund
                value: 10         // Domyślnie: 10 sekund
                stepSize: 1
                width: 150
                anchors.verticalCenter: parent.verticalCenter
            }

            Label {
                text: timeSlider.value.toFixed(0) + "s"
                anchors.verticalCenter: parent.verticalCenter
                color: "#FFD93D"
                font.pixelSize: 12
            }

            Rectangle {
                width: 1
                height: 30
                color: "#555555"
                anchors.verticalCenter: parent.verticalCenter
            }

            // Amplitude control - NAPRAWIONE (O WIELE MNIEJSZE WARTOŚCI)
            Label {
                text: "Gain:"
                anchors.verticalCenter: parent.verticalCenter
            }

            Slider {
                id: amplitudeSlider
                from: 0.1         // Min: 0.1 (bardzo mały gain)
                to: 10            // Max: 10
                value: 1.0        // Domyślnie: 1.0
                stepSize: 0.1
                width: 150
                anchors.verticalCenter: parent.verticalCenter
            }

            Label {
                text: amplitudeSlider.value.toFixed(1)
                anchors.verticalCenter: parent.verticalCenter
                color: "#00BCD4"
                font.pixelSize: 12
            }

            Rectangle {
                width: 1
                height: 30
                color: "#555555"
                anchors.verticalCenter: parent.verticalCenter
            }

            // Channel spacing - NAPRAWIONE (DUŻO WIĘKSZE)
            Label {
                text: "Spacing:"
                anchors.verticalCenter: parent.verticalCenter
            }

            Slider {
                id: spacingSlider
                from: 50          // Min: 50px
                to: 500           // Max: 500px (było 150!)
                value: 150        // Domyślnie: 150px
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

        EegUnifiedCanvas {
            id: unifiedCanvas
            width: eegWindow.width
            height: Math.max(eegWindow.height, eegViewModel.channelCount * spacingSlider.value + 100)

            viewModel: eegViewModel
            amplitudeScale: amplitudeSlider.value
            channelSpacing: spacingSlider.value
            showGrid: gridCheckbox.checked
            timeWindow: timeSlider.value
        }
    }

    // Scale reference
    Rectangle {
        anchors.bottom: parent.bottom
        anchors.right: parent.right
        anchors.margins: 20
        width: 120
        height: 80
        color: "#2E2E2E"
        border.color: "#555555"
        border.width: 1
        radius: 4

        Column {
            anchors.centerIn: parent
            spacing: 8

            Label {
                anchors.horizontalCenter: parent.horizontalCenter
                text: "Scale:"
                color: "#888888"
                font.pixelSize: 9
            }

            Row {
                spacing: 10

                Rectangle {
                    width: 2
                    height: 50
                    color: "#00BCD4"
                }

                Label {
                    text: (50 / amplitudeSlider.value).toFixed(1) + " µV"
                    color: "#CCCCCC"
                    font.pixelSize: 10
                }
            }

            Label {
                anchors.horizontalCenter: parent.horizontalCenter
                text: timeSlider.value.toFixed(0) + " seconds"
                color: "#888888"
                font.pixelSize: 9
            }
        }
    }
}
