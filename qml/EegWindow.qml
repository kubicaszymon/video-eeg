import QtQuick
import QtQuick.Controls
import QtQuick.Window
import QtQuick.Layouts
import videoEeg

import "components/eeg"

ApplicationWindow {
    id: eegWindow
    width: 1920
    height: 1080
    title: "EEG Recording System"
    visible: true
    visibility: Window.Maximized

    // Properties passed from parent
    property string amplifierId: ""
    property var channels: []
    property int channelCount: channels.length

    // Recording state
    property bool isRecording: false
    property bool isPaused: false
    property int recordingTime: 0
    property string currentPatientName: "Jan Kowalski"

    // Time window control
    property double timeWindowValue: 10

    // Theme colors
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
        timeWindowSeconds: timeWindowValue

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

    // Event handlers
    function handleStartStop() {
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

    function handlePauseResume() {
        isPaused = !isPaused
    }

    function handleAddMarker(type) {
        console.log("Marker added:", type)
        backend.addMarker(type)
    }

    Rectangle {
        anchors.fill: parent
        color: bgColor

        ColumnLayout {
            anchors.fill: parent
            spacing: 0

            // Top Toolbar
            EegToolbar {
                Layout.fillWidth: true
                currentPatientName: eegWindow.currentPatientName
                channelCount: eegWindow.channelCount
                isRecording: eegWindow.isRecording
                isPaused: eegWindow.isPaused
                recordingTime: eegWindow.recordingTime
                panelColor: eegWindow.panelColor
                textColor: eegWindow.textColor
                textSecondary: eegWindow.textSecondary
                warningColor: eegWindow.warningColor
                dangerColor: eegWindow.dangerColor
                z: 10
            }

            // Main Content Area
            RowLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true
                spacing: 0

                // Left Control Panel
                EegControlPanel {
                    Layout.fillHeight: true
                    backend: eegWindow.backend
                    isRecording: eegWindow.isRecording
                    isPaused: eegWindow.isPaused
                    channelCount: eegWindow.channelCount
                    dynamicChannelSpacing: eegGraph.dynamicChannelSpacing
                    timeSliderValue: eegWindow.timeWindowValue
                    panelColor: eegWindow.panelColor
                    textColor: eegWindow.textColor
                    textSecondary: eegWindow.textSecondary
                    accentColor: eegWindow.accentColor
                    successColor: eegWindow.successColor
                    warningColor: eegWindow.warningColor
                    dangerColor: eegWindow.dangerColor

                    onStartStopClicked: eegWindow.handleStartStop()
                    onPauseResumeClicked: eegWindow.handlePauseResume()
                    onGenerateTestDataClicked: backend.generateTestData()
                    onAddMarkerClicked: function(type) { eegWindow.handleAddMarker(type) }
                    onTimeWindowChanged: function(value) { eegWindow.timeWindowValue = value }
                    onGainChanged: function(value) { backend.gain = value }
                    onEndExaminationClicked: eegWindow.close()
                }

                // Central EEG Graph Area
                Rectangle {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    color: "#0d0f12"

                    EegGraph {
                        id: eegGraph
                        anchors.fill: parent
                        anchors.margins: 10
                        timeWindowSeconds: timeWindowValue
                        channelNames: backend.channelNames
                        markerManager: backend.markerManager
                    }

                    // Loading Overlay
                    EegLoadingOverlay {
                        anchors.fill: parent
                        isConnecting: backend.isConnecting
                        accentColor: eegWindow.accentColor
                        textColor: eegWindow.textColor
                        textSecondary: eegWindow.textSecondary
                        warningColor: eegWindow.warningColor
                    }

                    // Scale Bar (bottom right)
                    EegScaleBar {
                        anchors.right: parent.right
                        anchors.bottom: parent.bottom
                        anchors.margins: 30
                        scaleCalibrated: backend.scaleCalibrated
                        scaleBarValue: backend.scaleBarValue
                        scaleBarHeight: backend.scaleBarHeight
                        accentColor: eegWindow.accentColor
                        textColor: eegWindow.textColor
                    }

                    // Preview Mode Overlay (top right)
                    EegPreviewOverlay {
                        anchors.top: parent.top
                        anchors.right: parent.right
                        anchors.margins: 20
                        isRecording: eegWindow.isRecording
                        textColor: eegWindow.textColor
                        textSecondary: eegWindow.textSecondary
                    }
                }
            }

            // Bottom Status Bar
            EegStatusBar {
                Layout.fillWidth: true
                amplifierId: eegWindow.amplifierId
                backend: eegWindow.backend
                eegGraph: eegGraph
                timeSliderValue: eegWindow.timeWindowValue
                panelColor: eegWindow.panelColor
                textSecondary: eegWindow.textSecondary
                accentColor: eegWindow.accentColor
                successColor: eegWindow.successColor
                warningColor: eegWindow.warningColor
                dangerColor: eegWindow.dangerColor
            }
        }
    }
}
