import QtQuick
import QtQuick.Controls
import QtQuick.Window
import QtQuick.Layouts
import QtMultimedia
import videoEeg

// EegWindow - Main EEG recording window (separate ApplicationWindow)
ApplicationWindow {
    id: eegWindow
    width: 1920
    height: 1080
    title: "EEG Recording System"
    visible: true
    visibility: Window.Maximized

    // Signal emitted when examination ends (to return to main menu)
    signal examinationEnded()

    property string amplifierId: ""
    property var channels: []
    property int channelCount: channels.length
    property string cameraId: ""

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

        // Set screen DPI from actual screen pixel density
        // Screen.pixelDensity returns pixels per millimeter, convert to DPI (pixels per inch)
        var screenDpi = Screen.pixelDensity * 25.4
        console.log("Screen DPI detected:", screenDpi)
        backend.scaler.screenDpi = screenDpi

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
                                ControlSection {
                                    title: "⏺ Recording Control"
                                    textColor: eegWindow.textColor

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
                                        onClicked: isPaused = !isPaused
                                    }

                                    Button {
                                        Layout.fillWidth: true
                                        Layout.preferredHeight: 40
                                        text: "🧪 Generate Test Data"
                                        font.pixelSize: 11
                                        palette.button: accentColor
                                        palette.buttonText: "white"
                                        onClicked: backend.generateTestData()
                                    }
                                }

                                // EVENT MARKERS
                                ControlSection {
                                    title: "🏷️ Event Markers"
                                    textColor: eegWindow.textColor

                                    GridLayout {
                                        Layout.fillWidth: true
                                        columns: 2
                                        columnSpacing: 8
                                        rowSpacing: 8

                                        MarkerButton {
                                            text: "👁️ Eyes Open"
                                            markerType: "eyes_open"
                                            buttonColor: "#3498db"
                                            onMarkerClicked: function(type) { addMarker(type) }
                                        }

                                        MarkerButton {
                                            text: "😴 Eyes Closed"
                                            markerType: "eyes_closed"
                                            buttonColor: "#9b59b6"
                                            onMarkerClicked: function(type) { addMarker(type) }
                                        }

                                        MarkerButton {
                                            text: "⚡ Seizure Start"
                                            markerType: "seizure_start"
                                            buttonColor: "#e74c3c"
                                            onMarkerClicked: function(type) { addMarker(type) }
                                        }

                                        MarkerButton {
                                            text: "✓ Seizure Stop"
                                            markerType: "seizure_stop"
                                            buttonColor: "#27ae60"
                                            onMarkerClicked: function(type) { addMarker(type) }
                                        }

                                        MarkerButton {
                                            text: "⚠️ Artifact"
                                            markerType: "artifact"
                                            buttonColor: "#f39c12"
                                            onMarkerClicked: function(type) { addMarker(type) }
                                        }

                                        MarkerButton {
                                            text: "✏️ Custom"
                                            markerType: "custom"
                                            buttonColor: "#95a5a6"
                                            onMarkerClicked: function(type) { addMarker(type) }
                                        }
                                    }
                                }

                                // DISPLAY PARAMETERS
                                ControlSection {
                                    title: "⚙️ Display Parameters"
                                    textColor: eegWindow.textColor

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
                                                text: "📊 Sensitivity:"
                                                font.pixelSize: 11
                                                color: textSecondary
                                                Layout.fillWidth: true
                                            }

                                            Label {
                                                text: backend.scaler.sensitivity.toFixed(0) + " μV/mm"
                                                font.pixelSize: 11
                                                font.bold: true
                                                color: accentColor
                                            }
                                        }

                                        ComboBox {
                                            id: sensitivityCombo
                                            Layout.fillWidth: true
                                            model: backend.scaler.sensitivityOptions
                                            currentIndex: backend.scaler.sensitivityOptions.indexOf(backend.scaler.sensitivity)

                                            displayText: currentValue + " μV/mm"

                                            delegate: ItemDelegate {
                                                width: sensitivityCombo.width
                                                text: modelData + " μV/mm"
                                                highlighted: sensitivityCombo.highlightedIndex === index
                                            }

                                            onActivated: function(index) {
                                                backend.scaler.sensitivity = backend.scaler.sensitivityOptions[index]
                                            }
                                        }

                                        Label {
                                            text: "Lower values = more sensitive"
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
                        scaler: backend.scaler
                    }

                    // Loading overlay - shown while waiting for stream connection
                    Rectangle {
                        id: loadingOverlay
                        anchors.fill: parent
                        color: "#e00d0f12"
                        visible: backend.isConnecting
                        z: 100

                        Behavior on opacity {
                            NumberAnimation { duration: 300 }
                        }

                        Column {
                            anchors.centerIn: parent
                            spacing: 20

                            // Spinning loader
                            Item {
                                width: 80
                                height: 80
                                anchors.horizontalCenter: parent.horizontalCenter

                                Rectangle {
                                    id: spinnerOuter
                                    anchors.fill: parent
                                    radius: 40
                                    color: "transparent"
                                    border.width: 4
                                    border.color: "#2d3e50"
                                }

                                Rectangle {
                                    id: spinnerArc
                                    width: 80
                                    height: 80
                                    radius: 40
                                    color: "transparent"
                                    border.width: 4
                                    border.color: accentColor

                                    // Create arc effect with clip
                                    layer.enabled: true
                                    layer.effect: Item {
                                        Rectangle {
                                            width: 40
                                            height: 80
                                            color: "transparent"
                                        }
                                    }

                                    RotationAnimation on rotation {
                                        from: 0
                                        to: 360
                                        duration: 1200
                                        loops: Animation.Infinite
                                        running: backend.isConnecting
                                    }
                                }

                                // Inner pulse
                                Rectangle {
                                    anchors.centerIn: parent
                                    width: 40
                                    height: 40
                                    radius: 20
                                    color: accentColor
                                    opacity: 0.3

                                    SequentialAnimation on scale {
                                        running: backend.isConnecting
                                        loops: Animation.Infinite
                                        NumberAnimation { from: 0.8; to: 1.2; duration: 800; easing.type: Easing.InOutQuad }
                                        NumberAnimation { from: 1.2; to: 0.8; duration: 800; easing.type: Easing.InOutQuad }
                                    }
                                }

                                // EEG wave icon in center
                                Label {
                                    anchors.centerIn: parent
                                    text: "📊"
                                    font.pixelSize: 24
                                }
                            }

                            Label {
                                text: "Connecting to EEG Stream..."
                                font.pixelSize: 18
                                font.bold: true
                                color: textColor
                                anchors.horizontalCenter: parent.horizontalCenter
                            }

                            Label {
                                text: "Searching for LSL stream from amplifier"
                                font.pixelSize: 12
                                color: textSecondary
                                anchors.horizontalCenter: parent.horizontalCenter
                            }

                            // Animated dots
                            Row {
                                anchors.horizontalCenter: parent.horizontalCenter
                                spacing: 8

                                Repeater {
                                    model: 3

                                    Rectangle {
                                        width: 10
                                        height: 10
                                        radius: 5
                                        color: accentColor

                                        SequentialAnimation on opacity {
                                            running: backend.isConnecting
                                            loops: Animation.Infinite
                                            PauseAnimation { duration: index * 200 }
                                            NumberAnimation { from: 0.3; to: 1; duration: 400 }
                                            NumberAnimation { from: 1; to: 0.3; duration: 400 }
                                            PauseAnimation { duration: (2 - index) * 200 }
                                        }
                                    }
                                }
                            }

                            // Tips section
                            Rectangle {
                                width: 350
                                height: 70
                                color: "#1a2332"
                                radius: 8
                                border.color: "#2d3e50"
                                border.width: 1
                                anchors.horizontalCenter: parent.horizontalCenter

                                Column {
                                    anchors.centerIn: parent
                                    spacing: 5

                                    Label {
                                        text: "💡 Tip"
                                        font.pixelSize: 11
                                        font.bold: true
                                        color: warningColor
                                        anchors.horizontalCenter: parent.horizontalCenter
                                    }

                                    Label {
                                        text: "Make sure the amplifier is turned on\nand properly connected"
                                        font.pixelSize: 10
                                        color: textSecondary
                                        horizontalAlignment: Text.AlignHCenter
                                        anchors.horizontalCenter: parent.horizontalCenter
                                    }
                                }
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
                        text: "📊 Sensitivity: " + backend.scaler.sensitivity.toFixed(0) + " μV/mm"
                        font.pixelSize: 10
                        color: accentColor
                    }

                    Item { Layout.fillWidth: true }

                    Rectangle {
                        width: 12
                        height: 12
                        radius: 6
                        color: backend.isConnecting ? warningColor : (backend.isConnected ? successColor : dangerColor)

                        SequentialAnimation on opacity {
                            running: backend.isConnecting || backend.isConnected
                            loops: Animation.Infinite
                            NumberAnimation { from: 1; to: 0.3; duration: backend.isConnecting ? 500 : 1000 }
                            NumberAnimation { from: 0.3; to: 1; duration: backend.isConnecting ? 500 : 1000 }
                        }
                    }

                    Label {
                        text: backend.isConnecting ? "Connecting..." : (backend.isConnected ? "Connected" : "Disconnected")
                        font.pixelSize: 10
                        color: backend.isConnecting ? warningColor : (backend.isConnected ? successColor : dangerColor)
                        font.bold: true
                    }
                }
            }
        }
    }

    // Separate Video Window (child of EegWindow)
    Window {
        id: videoWindow
        width: 800
        height: 600
        minimumWidth: 400
        minimumHeight: 300
        title: "Video Recording - " + (CameraManager.currentCameraName || "No Camera")
        visible: eegWindow.cameraId !== ""
        flags: Qt.Window

        color: "#0e1419"

        property color panelColor: "#1a2332"
        property color accentColorVideo: "#4a90e2"
        property color successColorVideo: "#2ecc71"
        property color warningColorVideo: "#f39c12"
        property color dangerColorVideo: "#c0392b"
        property color textColorVideo: "#e8eef5"
        property color textSecondaryVideo: "#8a9cb5"

        Component.onCompleted: {
            if (eegWindow.cameraId !== "") {
                console.log("Video window ready, will start capture for camera:", eegWindow.cameraId)
                // Connect VideoOutput sink to CameraManager for direct display
                CameraManager.setExternalVideoSink(videoOutput.videoSink)
                // Delay capture start to ensure all QML bindings are complete
                captureStartTimer.start()
            }
        }

        Timer {
            id: captureStartTimer
            interval: 100
            repeat: false
            onTriggered: {
                console.log("Starting capture after delay...")
                CameraManager.startCapture()
            }
        }

        onClosing: function(close) {
            console.log("Video window closing")
            CameraManager.stopCapture()
        }

        Column {
            anchors.fill: parent
            spacing: 0

            // TOP TOOLBAR
            Rectangle {
                width: parent.width
                height: 50
                color: videoWindow.panelColor

                Row {
                    anchors.fill: parent
                    anchors.leftMargin: 15
                    anchors.rightMargin: 15
                    spacing: 15

                    Row {
                        anchors.verticalCenter: parent.verticalCenter
                        spacing: 10

                        Text {
                            text: "📹"
                            font.pixelSize: 24
                            anchors.verticalCenter: parent.verticalCenter
                        }

                        Column {
                            anchors.verticalCenter: parent.verticalCenter

                            Text {
                                text: videoBackend.cameraName || "Video Recording"
                                font.pixelSize: 14
                                font.bold: true
                                color: videoWindow.textColorVideo
                            }

                            Text {
                                text: "LSL Synchronized"
                                font.pixelSize: 10
                                color: videoWindow.textSecondaryVideo
                            }
                        }
                    }

                    Item { width: 1; height: 1 }

                    Rectangle {
                        width: 120
                        height: 35
                        anchors.verticalCenter: parent.verticalCenter
                        color: CameraManager.isCapturing ? videoWindow.dangerColorVideo : "#2d3e50"
                        radius: 4

                        Row {
                            anchors.centerIn: parent
                            spacing: 8

                            Rectangle {
                                width: 10
                                height: 10
                                radius: 5
                                color: "white"
                                visible: CameraManager.isCapturing
                                anchors.verticalCenter: parent.verticalCenter

                                SequentialAnimation on opacity {
                                    running: CameraManager.isCapturing
                                    loops: Animation.Infinite
                                    NumberAnimation { from: 1; to: 0.3; duration: 500 }
                                    NumberAnimation { from: 0.3; to: 1; duration: 500 }
                                }
                            }

                            Text {
                                text: CameraManager.isCapturing ? "CAPTURING" : "STOPPED"
                                font.pixelSize: 11
                                font.bold: true
                                color: "white"
                                anchors.verticalCenter: parent.verticalCenter
                            }
                        }
                    }
                }
            }

            // VIDEO DISPLAY
            Item {
                width: parent.width
                height: parent.height - 85

                VideoOutput {
                    id: videoOutput
                    anchors.fill: parent
                    anchors.margins: 10
                    fillMode: VideoOutput.PreserveAspectFit

                    Rectangle {
                        anchors.top: parent.top
                        anchors.right: parent.right
                        anchors.margins: 10
                        width: timestampColumn.width + 20
                        height: timestampColumn.height + 16
                        color: "#cc000000"
                        radius: 6
                        visible: CameraManager.isCapturing

                        Column {
                            id: timestampColumn
                            anchors.centerIn: parent
                            spacing: 4

                            Text {
                                text: "LSL Time"
                                font.pixelSize: 9
                                color: videoWindow.textSecondaryVideo
                            }

                            Text {
                                text: CameraManager.lastFrameTimestamp.toFixed(6)
                                font.pixelSize: 12
                                font.family: "monospace"
                                font.bold: true
                                color: videoWindow.accentColorVideo
                            }
                        }
                    }
                }

                Column {
                    anchors.centerIn: parent
                    spacing: 15
                    visible: !CameraManager.isCapturing

                    Text {
                        text: "📹"
                        font.pixelSize: 64
                        color: "#7f8c8d"
                        anchors.horizontalCenter: parent.horizontalCenter
                    }

                    Text {
                        text: "Starting video capture..."
                        font.pixelSize: 14
                        color: videoWindow.textSecondaryVideo
                        anchors.horizontalCenter: parent.horizontalCenter
                    }
                }
            }

            // STATUS BAR
            Rectangle {
                width: parent.width
                height: 35
                color: videoWindow.panelColor

                Row {
                    anchors.fill: parent
                    anchors.leftMargin: 15
                    anchors.rightMargin: 15
                    spacing: 20

                    Text {
                        text: "FPS: " + CameraManager.currentFps.toFixed(1)
                        font.pixelSize: 10
                        color: CameraManager.currentFps > 20 ? videoWindow.successColorVideo :
                               (CameraManager.currentFps > 10 ? videoWindow.warningColorVideo : videoWindow.dangerColorVideo)
                        anchors.verticalCenter: parent.verticalCenter
                    }

                    Rectangle { width: 1; height: 20; color: "#2d3e50"; anchors.verticalCenter: parent.verticalCenter }

                    Text {
                        text: "Camera: " + (CameraManager.currentCameraName || "None")
                        font.pixelSize: 10
                        color: videoWindow.textSecondaryVideo
                        anchors.verticalCenter: parent.verticalCenter
                    }

                    Rectangle { width: 1; height: 20; color: "#2d3e50"; anchors.verticalCenter: parent.verticalCenter }

                    Text {
                        text: "Format: " + (CameraManager.currentFormatString || "Auto")
                        font.pixelSize: 10
                        color: videoWindow.textSecondaryVideo
                        anchors.verticalCenter: parent.verticalCenter
                    }

                    Rectangle { width: 1; height: 20; color: "#2d3e50"; anchors.verticalCenter: parent.verticalCenter }

                    Text {
                        text: "LSL: " + (CameraManager.lastFrameTimestamp > 0 ?
                              CameraManager.lastFrameTimestamp.toFixed(3) + "s" : "N/A")
                        font.pixelSize: 10
                        color: videoWindow.accentColorVideo
                        anchors.verticalCenter: parent.verticalCenter
                    }

                    Item { width: 1; height: 1 }

                    Row {
                        anchors.verticalCenter: parent.verticalCenter
                        spacing: 5

                        Rectangle {
                            width: 10
                            height: 10
                            radius: 5
                            color: CameraManager.isCapturing ? videoWindow.successColorVideo : videoWindow.dangerColorVideo
                            anchors.verticalCenter: parent.verticalCenter
                        }

                        Text {
                            text: CameraManager.isCapturing ? "Capturing" : "Stopped"
                            font.pixelSize: 10
                            color: CameraManager.isCapturing ? videoWindow.successColorVideo : videoWindow.dangerColorVideo
                            anchors.verticalCenter: parent.verticalCenter
                        }
                    }
                }
            }
        }
    }

    // Handle EegWindow closing
    onClosing: function(close) {
        console.log("EegWindow closing, stopping streams")
        backend.stopStream()
        if (videoWindow.visible) {
            CameraManager.stopCapture()
            videoWindow.close()
        }
        examinationEnded()
    }
}
