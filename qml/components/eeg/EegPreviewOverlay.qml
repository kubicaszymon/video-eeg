import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: root
    width: 250
    height: 80
    color: "#2d3e50"
    radius: 8
    opacity: isRecording ? 0 : 0.95
    visible: !isRecording

    // Required properties
    required property bool isRecording
    required property color textColor
    required property color textSecondary

    Behavior on opacity {
        NumberAnimation { duration: 300 }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 5

        Label {
            text: "[i] Live Preview Mode"
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
