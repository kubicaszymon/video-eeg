import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: root
    width: 80
    height: Math.max(scaleBarHeight, 20) + 40
    color: "#1a2332"
    radius: 6
    border.color: "#2d3e50"
    border.width: 1
    opacity: 0.95
    visible: scaleCalibrated

    // Required properties
    required property bool scaleCalibrated
    required property double scaleBarValue
    required property double scaleBarHeight
    required property color accentColor
    required property color textColor

    Column {
        anchors.centerIn: parent
        spacing: 4

        // Scale bar (vertical line)
        Rectangle {
            id: scaleBarLine
            width: 3
            height: Math.max(root.scaleBarHeight, 20)
            color: accentColor
            anchors.horizontalCenter: parent.horizontalCenter

            // Top crossbar
            Rectangle {
                width: 12
                height: 2
                color: accentColor
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.top: parent.top
            }

            // Bottom crossbar
            Rectangle {
                width: 12
                height: 2
                color: accentColor
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.bottom: parent.bottom
            }
        }

        // Value label
        Label {
            text: root.scaleBarValue.toFixed(0) + " uV"
            font.pixelSize: 11
            font.bold: true
            color: textColor
            anchors.horizontalCenter: parent.horizontalCenter
        }
    }
}
