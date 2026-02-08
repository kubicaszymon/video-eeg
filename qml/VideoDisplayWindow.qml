import QtQuick
import QtQuick.Controls
import QtQuick.Window
import QtQuick.Layouts
import QtMultimedia
import videoEeg

/**
 * VideoDisplayWindow - Separate window for video display with LSL synchronization
 *
 * This window displays the video feed from the selected camera, synchronized
 * with EEG data via LSL timestamps. Each frame is timestamped using lsl::local_clock()
 * allowing precise correlation with EEG samples.
 *
 * Features:
 * - Real-time video display via Qt Multimedia
 * - LSL timestamp display for each frame
 * - Frame buffer status for synchronization
 * - FPS and latency statistics
 */
ApplicationWindow {
    id: videoWindow
    width: 800
    height: 600
    minimumWidth: 400
    minimumHeight: 300
    title: "Video Recording - " + (backend.cameraName || "No Camera")
    visible: true

    property string cameraId: ""

    readonly property color bgColor: "#0e1419"
    readonly property color panelColor: "#1a2332"
    readonly property color accentColor: "#4a90e2"
    readonly property color successColor: "#2ecc71"
    readonly property color warningColor: "#f39c12"
    readonly property color dangerColor: "#c0392b"
    readonly property color textColor: "#e8eef5"
    readonly property color textSecondary: "#8a9cb5"

    VideoBackend {
        id: backend
        cameraId: videoWindow.cameraId
        videoSink: videoOutput.videoSink

        onErrorOccurred: function(error) {
            console.error("Video error:", error)
            errorLabel.text = error
            errorLabel.visible = true
        }
    }

    Component.onCompleted: {
        console.log("VideoDisplayWindow opened with camera ID:", cameraId)
        if (cameraId !== "") {
            backend.startCapture()
        }
    }

    Component.onDestruction: {
        backend.stopCapture()
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
                Layout.preferredHeight: 50
                color: panelColor

                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: 15
                    anchors.rightMargin: 15
                    spacing: 15

                    Label {
                        text: "📹"
                        font.pixelSize: 24
                    }

                    ColumnLayout {
                        spacing: 0

                        Label {
                            text: backend.cameraName || "Video Recording"
                            font.pixelSize: 14
                            font.bold: true
                            color: textColor
                        }

                        Label {
                            text: "LSL Synchronized"
                            font.pixelSize: 10
                            color: textSecondary
                        }
                    }

                    Item { Layout.fillWidth: true }

                    // Recording indicator
                    Rectangle {
                        Layout.preferredWidth: 120
                        Layout.preferredHeight: 35
                        color: backend.isCapturing ? dangerColor : "#2d3e50"
                        radius: 4

                        RowLayout {
                            anchors.centerIn: parent
                            spacing: 8

                            Rectangle {
                                width: 10
                                height: 10
                                radius: 5
                                color: "white"
                                visible: backend.isCapturing

                                SequentialAnimation on opacity {
                                    running: backend.isCapturing
                                    loops: Animation.Infinite
                                    NumberAnimation { from: 1; to: 0.3; duration: 500 }
                                    NumberAnimation { from: 0.3; to: 1; duration: 500 }
                                }
                            }

                            Label {
                                text: backend.isCapturing ? "CAPTURING" : "STOPPED"
                                font.pixelSize: 11
                                font.bold: true
                                color: "white"
                            }
                        }
                    }

                    // Control buttons
                    Button {
                        text: backend.isCapturing ? "Stop" : "Start"
                        font.pixelSize: 11
                        Layout.preferredHeight: 35
                        palette.button: backend.isCapturing ? dangerColor : successColor
                        palette.buttonText: "white"

                        onClicked: {
                            if (backend.isCapturing) {
                                backend.stopCapture()
                            } else {
                                backend.startCapture()
                            }
                        }
                    }
                }
            }

            // VIDEO DISPLAY AREA
            Item {
                Layout.fillWidth: true
                Layout.fillHeight: true

                VideoOutput {
                    id: videoOutput
                    anchors.fill: parent
                    anchors.margins: 10
                    fillMode: VideoOutput.PreserveAspectFit

                    // Overlay for timestamp display
                    Rectangle {
                        anchors.top: parent.top
                        anchors.right: parent.right
                        anchors.margins: 10
                        width: timestampColumn.width + 20
                        height: timestampColumn.height + 16
                        color: "#cc000000"
                        radius: 6
                        visible: backend.isCapturing

                        Column {
                            id: timestampColumn
                            anchors.centerIn: parent
                            spacing: 4

                            Label {
                                text: "LSL Time"
                                font.pixelSize: 9
                                color: textSecondary
                            }

                            Label {
                                text: backend.lastFrameTimestamp.toFixed(6)
                                font.pixelSize: 12
                                font.family: "monospace"
                                font.bold: true
                                color: accentColor
                            }
                        }
                    }
                }

                // Placeholder when not capturing
                Column {
                    anchors.centerIn: parent
                    spacing: 15
                    visible: !backend.isCapturing

                    Label {
                        text: "📹"
                        font.pixelSize: 64
                        color: "#7f8c8d"
                        anchors.horizontalCenter: parent.horizontalCenter
                    }

                    Label {
                        text: "Press 'Start' to begin video capture"
                        font.pixelSize: 14
                        color: textSecondary
                        anchors.horizontalCenter: parent.horizontalCenter
                    }

                    Label {
                        text: "Frames will be timestamped with LSL clock"
                        font.pixelSize: 11
                        color: "#7f8c8d"
                        anchors.horizontalCenter: parent.horizontalCenter
                    }
                }

                // Error display
                Rectangle {
                    id: errorLabel
                    anchors.bottom: parent.bottom
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.margins: 10
                    height: 40
                    color: "#cc990000"
                    radius: 4
                    visible: false

                    property alias text: errorText.text

                    Label {
                        id: errorText
                        anchors.centerIn: parent
                        font.pixelSize: 12
                        color: "white"
                    }

                    MouseArea {
                        anchors.fill: parent
                        onClicked: errorLabel.visible = false
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

                    // FPS
                    Label {
                        text: "FPS: " + backend.currentFps.toFixed(1)
                        font.pixelSize: 10
                        color: backend.currentFps > 20 ? successColor : (backend.currentFps > 10 ? warningColor : dangerColor)
                    }

                    Rectangle {
                        width: 1
                        height: 20
                        color: "#2d3e50"
                    }

                    // Frame count
                    Label {
                        text: "Frames: " + backend.frameCount
                        font.pixelSize: 10
                        color: textSecondary
                    }

                    Rectangle {
                        width: 1
                        height: 20
                        color: "#2d3e50"
                    }

                    // Buffer size
                    Label {
                        text: "Buffer: " + backend.bufferSize + "/" + backend.maxBufferSize
                        font.pixelSize: 10
                        color: textSecondary
                    }

                    Rectangle {
                        width: 1
                        height: 20
                        color: "#2d3e50"
                    }

                    // Latest timestamp
                    Label {
                        text: "Last LSL: " + (backend.lastFrameTimestamp > 0 ?
                              backend.lastFrameTimestamp.toFixed(3) + "s" : "N/A")
                        font.pixelSize: 10
                        color: accentColor
                    }

                    Item { Layout.fillWidth: true }

                    // Connection status
                    Rectangle {
                        width: 10
                        height: 10
                        radius: 5
                        color: backend.isConnected ? successColor : dangerColor
                    }

                    Label {
                        text: backend.isConnected ? "Connected" : "Disconnected"
                        font.pixelSize: 10
                        color: backend.isConnected ? successColor : dangerColor
                    }
                }
            }
        }
    }
}
