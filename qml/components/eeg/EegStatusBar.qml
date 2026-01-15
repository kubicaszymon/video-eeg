import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: root
    height: 35
    color: panelColor

    // Required properties
    required property string amplifierId
    required property var backend
    required property var eegGraph
    required property double timeSliderValue
    required property color panelColor
    required property color textSecondary
    required property color accentColor
    required property color successColor
    required property color warningColor
    required property color dangerColor

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 15
        anchors.rightMargin: 15
        spacing: 20

        Label {
            text: "[amp] Amplifier: " + (amplifierId || "Unknown")
            font.pixelSize: 10
            color: textSecondary
        }

        Rectangle {
            width: 1
            height: 20
            color: "#2d3e50"
        }

        Label {
            text: "[Hz] Frequency: " + (backend.samplingRate > 0 ? backend.samplingRate.toFixed(0) + " Hz" : "detecting...")
            font.pixelSize: 10
            color: textSecondary
        }

        Rectangle {
            width: 1
            height: 20
            color: "#2d3e50"
        }

        Label {
            text: "[buf] Buffer: " + eegGraph.dataModel.maxSamples + " samples (" + timeSliderValue.toFixed(0) + "s)"
            font.pixelSize: 10
            color: textSecondary
        }

        Rectangle {
            width: 1
            height: 20
            color: "#2d3e50"
        }

        Label {
            text: "[sp] Spacing: " + eegGraph.dynamicChannelSpacing.toFixed(0)
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
                ? ("[~] " + backend.dataRangeInMicrovolts.toFixed(0) + " uV | Gain: " + backend.gain.toFixed(1) + "x")
                : "[~] Detecting scale..."
            font.pixelSize: 10
            color: backend.scaleCalibrated ? accentColor : textSecondary
        }

        Item { Layout.fillWidth: true }

        // Connection status indicator
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
