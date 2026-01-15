import QtQuick
import QtQuick.Controls
import QtQuick.Window
import QtQuick.Layouts
import videoEeg

ApplicationWindow {
    id: eegWindow
    width: 1920
    height: 1080
    title: "📊 EEG Recording System"
    visible: true
    visibility: Window.Maximized

    property string amplifierId: ""
    property var channels: []
    property int channelCount: channels.length

    property bool isRecording: false
    property bool isPaused: false
    property int recordingTime: 0
    property string currentPatientName: "Jan Kowalski"

    readonly property color bgColor: "#0e1419"
    readonly property color panelColor: "#1a2332"
    readonly property color accentColor: "#4a90e2"
    readonly property color successColor: "#2ecc71"
    readonly property color warningColor: "#f39c12"
    readonly property color dangerColor: "#c0392b"
    readonly property color textColor: "#e8eef5"
    readonly property color textSecondary: "#8a9cb5"

    EegBackend {
        id: backend
        amplifierId: eegWindow.amplifierId
        channels: eegWindow.channels
        spacing: eegGraph.dynamicChannelSpacing
        timeWindowSeconds: timeSlider.value

        onChannelsChanged: {
            eegGraph.selectedChannels = channels
        }

        onSamplingRateChanged: {
            console.log("Sampling rate updated:", samplingRate, "Hz")
        }
    }

    Timer {
        id: recordingTimer
        interval: 1000
        running: isRecording && !isPaused
        repeat: true
        onTriggered: recordingTime++
    }

    Component.onCompleted: {
        console.log("EegWindow opened with channels:", channels)
        console.log("Channels count:", channelCount)
        console.log("Amplifier ID:", amplifierId)

        backend.registerDataModel(eegGraph.dataModel)
        eegGraph.selectedChannels = channels
        backend.startStream()
    }

    function formatTime(seconds) {
        var h = Math.floor(seconds / 3600)
        var m = Math.floor((seconds % 3600) / 60)
        var s = seconds % 60
        return (h < 10 ? "0" : "") + h + ":" +
               (m < 10 ? "0" : "") + m + ":" +
               (s < 10 ? "0" : "") + s
    }

    function addMarker(type) {
        console.log("Marker added:", type)
        backend.addMarker(type)
    }

    Rectangle {
        anchors.fill: parent
        color: bgColor

        ColumnLayout {
            anchors.fill: parent
            spacing: 0

            // TOP TOOLBAR
            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 70
                color: panelColor
                z: 10

                RowLayout {
                    anchors.fill: parent
                    anchors.margins: 15
                    spacing: 20

                    RowLayout {
                        spacing: 12

                        Label {
                            text: "📊"
                            font.pixelSize: 28
                        }

                        ColumnLayout {
                            spacing: 2

                            Label {
                                text: "EEG Recording System"
                                font.pixelSize: 16
                                font.bold: true
                                color: textColor
                            }

                            Label {
                                text: currentPatientName + " • " + channelCount + " channels"
                                font.pixelSize: 11
                                color: textSecondary
                            }
                        }
                    }

                    Item { Layout.fillWidth: true }

                    Rectangle {
                        Layout.preferredWidth: 300
                        Layout.preferredHeight: 50
                        color: isRecording ? (isPaused ? warningColor : dangerColor) : "#2d3e50"
                        radius: 6
                        border.color: isRecording ? "white" : "#7f8c8d"
                        border.width: 2

                        RowLayout {
                            anchors.fill: parent
                            anchors.margins: 10
                            spacing: 12

                            Rectangle {
                                width: 16
                                height: 16
                                radius: 8
                                color: "white"
                                visible: isRecording && !isPaused

                                SequentialAnimation on opacity {
                                    running: isRecording && !isPaused
                                    loops: Animation.Infinite
                                    NumberAnimation { from: 1; to: 0.3; duration: 500 }
                                    NumberAnimation { from: 0.3; to: 1; duration: 500 }
                                }
                            }

                            ColumnLayout {
                                Layout.fillWidth: true
                                spacing: 0

                                Label {
                                    text: isRecording ? (isPaused ? "⏸ PAUSED" : "⏺ RECORDING") : "⏹ LIVE PREVIEW"
                                    font.pixelSize: 14
                                    font.bold: true
                                    color: "white"
                                }

                                Label {
                                    text: isRecording ? formatTime(recordingTime) : "Not saving data"
                                    font.pixelSize: 10
                                    color: "white"
                                }
                            }
                        }
                    }

                    Label {
                        text: Qt.formatDateTime(new Date(), "dd.MM.yyyy  hh:mm:ss")
                        font.pixelSize: 11
                        color: textSecondary

                        Timer {
                            interval: 1000
                            running: true
                            repeat: true
                            onTriggered: parent.text = Qt.formatDateTime(new Date(), "dd.MM.yyyy  hh:mm:ss")
                        }
                    }
                }
            }

            // MAIN CONTENT
            RowLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true
                spacing: 0

                // LEFT PANEL
                Rectangle {
                    Layout.preferredWidth: 280
                    Layout.fillHeight: true
                    color: panelColor

                    ScrollView {
                        anchors.fill: parent
                        clip: true
                        contentWidth: availableWidth

                        Item {
                            width: 280
                            height: contentColumn.implicitHeight + 30

                            ColumnLayout {
                                id: contentColumn
                                anchors.fill: parent
                                anchors.topMargin: 15
                                anchors.leftMargin: 15
                                anchors.rightMargin: 15
                                anchors.bottomMargin: 15
                                spacing: 15

                                // RECORDING CONTROL
                                ColumnLayout {
                                    Layout.fillWidth: true
                                    spacing: 10

                                    Label {
                                        text: "⏺ Recording Control"
                                        font.pixelSize: 13
                                        font.bold: true
                                        color: textColor
                                    }

                                    Rectangle {
                                        Layout.fillWidth: true
                                        height: 1
                                        color: "#2d3e50"
                                    }

                                    Button {
                                        Layout.fillWidth: true
                                        Layout.preferredHeight: 45
                                        text: isRecording ? "⏹ Stop" : "⏺ Start Recording"
                                        font.pixelSize: 12
                                        font.bold: true
                                        palette.button: isRecording ? dangerColor : successColor
                                        palette.buttonText: "white"

                                        onClicked: {
                                            if (isRecording) {
                                                isRecording = false
                                                isPaused = false
                                                recordingTime = 0
                                            } else {
                                                isRecording = true
                                                isPaused = false
                                                recordingTime = 0
                                            }
                                        }
                                    }

                                    Button {
                                        Layout.fillWidth: true
                                        Layout.preferredHeight: 40
                                        text: isPaused ? "▶ Resume" : "⏸ Pause"
                                        font.pixelSize: 11
                                        enabled: isRecording
                                        palette.button: warningColor
                                        palette.buttonText: "white"

                                        onClicked: {
                                            isPaused = !isPaused
                                        }
                                    }

                                    Button {
                                        Layout.fillWidth: true
                                        Layout.preferredHeight: 40
                                        text: "🧪 Generate Test Data"
                                        font.pixelSize: 11
                                        palette.button: accentColor
                                        palette.buttonText: "white"

                                        onClicked: {
                                            backend.generateTestData()
                                        }
                                    }
                                }

                                // EVENT MARKERS
                                ColumnLayout {
                                    Layout.fillWidth: true
                                    spacing: 10

                                    Label {
                                        text: "🏷️ Event Markers"
                                        font.pixelSize: 13
                                        font.bold: true
                                        color: textColor
                                    }

                                    Rectangle {
                                        Layout.fillWidth: true
                                        height: 1
                                        color: "#2d3e50"
                                    }

                                    GridLayout {
                                        Layout.fillWidth: true
                                        columns: 2
                                        columnSpacing: 8
                                        rowSpacing: 8

                                        Button {
                                            text: "👁️ Eyes Open"
                                            font.pixelSize: 10
                                            Layout.fillWidth: true
                                            Layout.preferredHeight: 35
                                            palette.button: "#3498db"
                                            palette.buttonText: "white"
                                            onClicked: addMarker("eyes_open")
                                        }

                                        Button {
                                            text: "😴 Eyes Closed"
                                            font.pixelSize: 10
                                            Layout.fillWidth: true
                                            Layout.preferredHeight: 35
                                            palette.button: "#9b59b6"
                                            palette.buttonText: "white"
                                            onClicked: addMarker("eyes_closed")
                                        }

                                        Button {
                                            text: "⚡ Seizure Start"
                                            font.pixelSize: 10
                                            Layout.fillWidth: true
                                            Layout.preferredHeight: 35
                                            palette.button: "#e74c3c"
                                            palette.buttonText: "white"
                                            onClicked: addMarker("seizure_start")
                                        }

                                        Button {
                                            text: "✓ Seizure Stop"
                                            font.pixelSize: 10
                                            Layout.fillWidth: true
                                            Layout.preferredHeight: 35
                                            palette.button: "#27ae60"
                                            palette.buttonText: "white"
                                            onClicked: addMarker("seizure_stop")
                                        }

                                        Button {
                                            text: "⚠️ Artifact"
                                            font.pixelSize: 10
                                            Layout.fillWidth: true
                                            Layout.preferredHeight: 35
                                            palette.button: "#f39c12"
                                            palette.buttonText: "white"
                                            onClicked: addMarker("artifact")
                                        }

                                        Button {
                                            text: "✏️ Custom"
                                            font.pixelSize: 10
                                            Layout.fillWidth: true
                                            Layout.preferredHeight: 35
                                            palette.button: "#95a5a6"
                                            palette.buttonText: "white"
                                            onClicked: addMarker("custom")
                                        }
                                    }
                                }

                                // DISPLAY PARAMETERS
                                ColumnLayout {
                                    Layout.fillWidth: true
                                    spacing: 10

                                    Label {
                                        text: "⚙️ Display Parameters"
                                        font.pixelSize: 13
                                        font.bold: true
                                        color: textColor
                                    }

                                    Rectangle {
                                        Layout.fillWidth: true
                                        height: 1
                                        color: "#2d3e50"
                                    }

                                    ColumnLayout {
                                        Layout.fillWidth: true
                                        spacing: 5

                                        RowLayout {
                                            Layout.fillWidth: true

                                            Label {
                                                text: "⏱️ Time Window:"
                                                font.pixelSize: 11
                                                color: textSecondary
                                                Layout.fillWidth: true
                                            }

                                            Label {
                                                text: timeSlider.value.toFixed(0) + "s"
                                                font.pixelSize: 11
                                                font.bold: true
                                                color: accentColor
                                            }
                                        }

                                        Slider {
                                            id: timeSlider
                                            Layout.fillWidth: true
                                            from: 5
                                            to: 30
                                            value: 10
                                            stepSize: 1
                                        }
                                    }

                                    ColumnLayout {
                                        Layout.fillWidth: true
                                        spacing: 5

                                        RowLayout {
                                            Layout.fillWidth: true

                                            Label {
                                                text: "📈 Gain:"
                                                font.pixelSize: 11
                                                color: textSecondary
                                                Layout.fillWidth: true
                                            }

                                            Label {
                                                text: amplitudeSlider.value.toFixed(2) + "x"
                                                font.pixelSize: 11
                                                font.bold: true
                                                color: accentColor
                                            }
                                        }

                                        Slider {
                                            id: amplitudeSlider
                                            Layout.fillWidth: true
                                            from: 0.1
                                            to: 5.0
                                            value: backend.gain
                                            stepSize: 0.1

                                            onValueChanged: {
                                                backend.gain = value
                                            }
                                        }

                                        Label {
                                            text: "Increases/decreases signal amplitude"
                                            font.pixelSize: 9
                                            color: textSecondary
                                            Layout.fillWidth: true
                                        }
                                    }

                                    ColumnLayout {
                                        Layout.fillWidth: true
                                        spacing: 5

                                        RowLayout {
                                            Layout.fillWidth: true

                                            Label {
                                                text: "📏 Channel Spacing:"
                                                font.pixelSize: 11
                                                color: textSecondary
                                                Layout.fillWidth: true
                                            }

                                            Label {
                                                text: eegGraph.dynamicChannelSpacing.toFixed(0) + " (auto)"
                                                font.pixelSize: 11
                                                font.bold: true
                                                color: accentColor
                                            }
                                        }

                                        Label {
                                            text: "Automatically adjusted for " + channelCount + " channels"
                                            font.pixelSize: 9
                                            color: textSecondary
                                            Layout.fillWidth: true
                                        }
                                    }
                                }

                                // SCALE INFO SECTION
                                ColumnLayout {
                                    Layout.fillWidth: true
                                    spacing: 10

                                    Label {
                                        text: "🔬 Scale Info"
                                        font.pixelSize: 13
                                        font.bold: true
                                        color: textColor
                                    }

                                    Rectangle {
                                        Layout.fillWidth: true
                                        height: 1
                                        color: "#2d3e50"
                                    }

                                    // Scale info - pokazuje się gdy są dane
                                    Rectangle {
                                        Layout.fillWidth: true
                                        Layout.preferredHeight: 50
                                        color: "#1a2332"
                                        radius: 6
                                        border.color: backend.scaleCalibrated ? "#2d3e50" : warningColor
                                        border.width: 1

                                        RowLayout {
                                            anchors.fill: parent
                                            anchors.margins: 10
                                            spacing: 10

                                            ColumnLayout {
                                                spacing: 2
                                                Layout.fillWidth: true

                                                Label {
                                                    text: backend.scaleCalibrated
                                                        ? "Range: " + backend.dataRangeInMicrovolts.toFixed(0) + " μV"
                                                        : "Waiting for data..."
                                                    font.pixelSize: 11
                                                    color: backend.scaleCalibrated ? textColor : textSecondary
                                                }

                                                Label {
                                                    text: "Scale bar: " + backend.scaleBarValue.toFixed(0) + " μV"
                                                    font.pixelSize: 9
                                                    color: textSecondary
                                                    visible: backend.scaleCalibrated
                                                }
                                            }

                                            Label {
                                                text: backend.scaleUnit
                                                font.pixelSize: 12
                                                font.bold: true
                                                color: accentColor
                                                visible: backend.scaleCalibrated
                                            }
                                        }
                                    }

                                }

                                // ACTIONS
                                ColumnLayout {
                                    Layout.fillWidth: true
                                    spacing: 8

                                    Rectangle {
                                        Layout.fillWidth: true
                                        height: 1
                                        color: "#2d3e50"
                                    }

                                    Button {
                                        Layout.fillWidth: true
                                        text: "❌ End Examination"
                                        font.pixelSize: 11
                                        Layout.preferredHeight: 40
                                        palette.button: dangerColor
                                        palette.buttonText: "white"

                                        onClicked: {
                                            eegWindow.close()
                                        }
                                    }
                                }

                                Item { Layout.fillHeight: true }
                            }
                        }
                    }
                }

                // CENTRAL - EEG GRAPH
                Rectangle {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    color: "#0d0f12"

                    EegGraph {
                        id: eegGraph
                        anchors.fill: parent
                        anchors.margins: 10
                        timeWindowSeconds: timeSlider.value
                        channelNames: backend.channelNames
                        markerManager: backend.markerManager
                    }

                    // Scale Bar - pokazuje rzeczywistą skalę sygnału
                    Rectangle {
                        id: scaleBarContainer
                        anchors.right: parent.right
                        anchors.bottom: parent.bottom
                        anchors.margins: 30
                        width: 80
                        height: Math.max(backend.scaleBarHeight, 20) + 40
                        color: "#1a2332"
                        radius: 6
                        border.color: "#2d3e50"
                        border.width: 1
                        opacity: 0.95
                        visible: backend.scaleCalibrated

                        Column {
                            anchors.centerIn: parent
                            spacing: 4

                            // Scale bar (pionowa kreska)
                            Rectangle {
                                id: scaleBarLine
                                width: 3
                                height: Math.max(backend.scaleBarHeight, 20)
                                color: accentColor
                                anchors.horizontalCenter: parent.horizontalCenter

                                // Górna poprzeczka
                                Rectangle {
                                    width: 12
                                    height: 2
                                    color: accentColor
                                    anchors.horizontalCenter: parent.horizontalCenter
                                    anchors.top: parent.top
                                }

                                // Dolna poprzeczka
                                Rectangle {
                                    width: 12
                                    height: 2
                                    color: accentColor
                                    anchors.horizontalCenter: parent.horizontalCenter
                                    anchors.bottom: parent.bottom
                                }
                            }

                            // Etykieta wartości
                            Label {
                                text: backend.scaleBarValue.toFixed(0) + " μV"
                                font.pixelSize: 11
                                font.bold: true
                                color: textColor
                                anchors.horizontalCenter: parent.horizontalCenter
                            }
                        }
                    }

                    Rectangle {
                        anchors.top: parent.top
                        anchors.right: parent.right
                        anchors.margins: 20
                        width: 250
                        height: 80
                        color: "#2d3e50"
                        radius: 8
                        opacity: isRecording ? 0 : 0.95
                        visible: !isRecording

                        Behavior on opacity {
                            NumberAnimation { duration: 300 }
                        }

                        ColumnLayout {
                            anchors.fill: parent
                            anchors.margins: 12
                            spacing: 5

                            Label {
                                text: "ℹ️ Live Preview Mode"
                                font.pixelSize: 12
                                font.bold: true
                                color: textColor
                            }

                            Label {
                                text: "Data is not being saved.\nClick 'Start Recording'"
                                font.pixelSize: 10
                                color: textSecondary
                                wrapMode: Text.WordWrap
                                Layout.fillWidth: true
                            }
                        }
                    }
                }
            }

            // BOTTOM STATUS BAR
            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 35
                color: panelColor

                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: 15
                    anchors.rightMargin: 15
                    spacing: 20

                    Label {
                        text: "🔌 Amplifier: " + (amplifierId || "Unknown")
                        font.pixelSize: 10
                        color: textSecondary
                    }

                    Rectangle {
                        width: 1
                        height: 20
                        color: "#2d3e50"
                    }

                    Label {
                        text: "📊 Frequency: " + (backend.samplingRate > 0 ? backend.samplingRate.toFixed(0) + " Hz" : "detecting...")
                        font.pixelSize: 10
                        color: textSecondary
                    }

                    Rectangle {
                        width: 1
                        height: 20
                        color: "#2d3e50"
                    }

                    Label {
                        text: "💾 Buffer: " + eegGraph.dataModel.maxSamples + " samples (" + timeSlider.value.toFixed(0) + "s)"
                        font.pixelSize: 10
                        color: textSecondary
                    }

                    Rectangle {
                        width: 1
                        height: 20
                        color: "#2d3e50"
                    }

                    Label {
                        text: "📏 Spacing: " + eegGraph.dynamicChannelSpacing.toFixed(0)
                        font.pixelSize: 10
                        color: textSecondary
                    }

                    Rectangle {
                        width: 1
                        height: 20
                        color: "#2d3e50"
                    }

                    Label {
                        text: backend.scaleCalibrated
                            ? ("🔬 " + backend.dataRangeInMicrovolts.toFixed(0) + " μV | Gain: " + backend.gain.toFixed(1) + "x")
                            : "🔬 Detecting scale..."
                        font.pixelSize: 10
                        color: backend.scaleCalibrated ? accentColor : textSecondary
                    }

                    Item { Layout.fillWidth: true }

                    Rectangle {
                        width: 12
                        height: 12
                        radius: 6
                        color: successColor

                        SequentialAnimation on opacity {
                            running: true
                            loops: Animation.Infinite
                            NumberAnimation { from: 1; to: 0.3; duration: 1000 }
                            NumberAnimation { from: 0.3; to: 1; duration: 1000 }
                        }
                    }

                    Label {
                        text: "Connected"
                        font.pixelSize: 10
                        color: successColor
                        font.bold: true
                    }
                }
            }
        }
    }
}
