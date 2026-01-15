import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: root
    color: "#e00d0f12"
    visible: isConnecting
    z: 100

    // Required properties
    required property bool isConnecting
    required property color accentColor
    required property color textColor
    required property color textSecondary
    required property color warningColor

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
                    running: isConnecting
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
                    running: isConnecting
                    loops: Animation.Infinite
                    NumberAnimation { from: 0.8; to: 1.2; duration: 800; easing.type: Easing.InOutQuad }
                    NumberAnimation { from: 1.2; to: 0.8; duration: 800; easing.type: Easing.InOutQuad }
                }
            }

            // EEG icon in center
            Label {
                anchors.centerIn: parent
                text: "[~]"
                font.pixelSize: 20
                color: "white"
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
                        running: isConnecting
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
                    text: "[!] Tip"
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
