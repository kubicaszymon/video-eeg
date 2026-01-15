import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: root
    width: 280
    color: panelColor

    // Required properties
    required property var backend
    required property bool isRecording
    required property bool isPaused
    required property int channelCount
    required property double dynamicChannelSpacing
    required property double timeSliderValue
    required property color panelColor
    required property color textColor
    required property color textSecondary
    required property color accentColor
    required property color successColor
    required property color warningColor
    required property color dangerColor

    // Signals
    signal startStopClicked()
    signal pauseResumeClicked()
    signal generateTestDataClicked()
    signal addMarkerClicked(string type)
    signal timeWindowChanged(double value)
    signal gainChanged(double value)
    signal endExaminationClicked()

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

                // Recording Control Section
                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 10

                    Label {
                        text: "[REC] Recording Control"
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
                        text: isRecording ? "[stop] Stop" : "[rec] Start Recording"
                        font.pixelSize: 12
                        font.bold: true
                        palette.button: isRecording ? dangerColor : successColor
                        palette.buttonText: "white"
                        onClicked: root.startStopClicked()
                    }

                    Button {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 40
                        text: isPaused ? "[>] Resume" : "[||] Pause"
                        font.pixelSize: 11
                        enabled: isRecording
                        palette.button: warningColor
                        palette.buttonText: "white"
                        onClicked: root.pauseResumeClicked()
                    }

                    Button {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 40
                        text: "[test] Generate Test Data"
                        font.pixelSize: 11
                        palette.button: accentColor
                        palette.buttonText: "white"
                        onClicked: root.generateTestDataClicked()
                    }
                }

                // Event Markers Section
                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 10

                    Label {
                        text: "[M] Event Markers"
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
                            text: "[O] Eyes Open"
                            font.pixelSize: 10
                            Layout.fillWidth: true
                            Layout.preferredHeight: 35
                            palette.button: "#3498db"
                            palette.buttonText: "white"
                            onClicked: root.addMarkerClicked("eyes_open")
                        }

                        Button {
                            text: "[-] Eyes Closed"
                            font.pixelSize: 10
                            Layout.fillWidth: true
                            Layout.preferredHeight: 35
                            palette.button: "#9b59b6"
                            palette.buttonText: "white"
                            onClicked: root.addMarkerClicked("eyes_closed")
                        }

                        Button {
                            text: "[!] Seizure Start"
                            font.pixelSize: 10
                            Layout.fillWidth: true
                            Layout.preferredHeight: 35
                            palette.button: "#e74c3c"
                            palette.buttonText: "white"
                            onClicked: root.addMarkerClicked("seizure_start")
                        }

                        Button {
                            text: "[v] Seizure Stop"
                            font.pixelSize: 10
                            Layout.fillWidth: true
                            Layout.preferredHeight: 35
                            palette.button: "#27ae60"
                            palette.buttonText: "white"
                            onClicked: root.addMarkerClicked("seizure_stop")
                        }

                        Button {
                            text: "[A] Artifact"
                            font.pixelSize: 10
                            Layout.fillWidth: true
                            Layout.preferredHeight: 35
                            palette.button: "#f39c12"
                            palette.buttonText: "white"
                            onClicked: root.addMarkerClicked("artifact")
                        }

                        Button {
                            text: "[*] Custom"
                            font.pixelSize: 10
                            Layout.fillWidth: true
                            Layout.preferredHeight: 35
                            palette.button: "#95a5a6"
                            palette.buttonText: "white"
                            onClicked: root.addMarkerClicked("custom")
                        }
                    }
                }

                // Display Parameters Section
                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 10

                    Label {
                        text: "[cfg] Display Parameters"
                        font.pixelSize: 13
                        font.bold: true
                        color: textColor
                    }

                    Rectangle {
                        Layout.fillWidth: true
                        height: 1
                        color: "#2d3e50"
                    }

                    // Time Window
                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 5

                        RowLayout {
                            Layout.fillWidth: true

                            Label {
                                text: "[t] Time Window:"
                                font.pixelSize: 11
                                color: textSecondary
                                Layout.fillWidth: true
                            }

                            Label {
                                text: timeSliderValue.toFixed(0) + "s"
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
                            value: timeSliderValue
                            stepSize: 1
                            onValueChanged: root.timeWindowChanged(value)
                        }
                    }

                    // Gain
                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 5

                        RowLayout {
                            Layout.fillWidth: true

                            Label {
                                text: "[+] Gain:"
                                font.pixelSize: 11
                                color: textSecondary
                                Layout.fillWidth: true
                            }

                            Label {
                                text: backend.gain.toFixed(2) + "x"
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
                            onValueChanged: root.gainChanged(value)
                        }

                        Label {
                            text: "Increases/decreases signal amplitude"
                            font.pixelSize: 9
                            color: textSecondary
                            Layout.fillWidth: true
                        }
                    }

                    // Channel Spacing (read-only)
                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 5

                        RowLayout {
                            Layout.fillWidth: true

                            Label {
                                text: "[=] Channel Spacing:"
                                font.pixelSize: 11
                                color: textSecondary
                                Layout.fillWidth: true
                            }

                            Label {
                                text: dynamicChannelSpacing.toFixed(0) + " (auto)"
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

                // Scale Info Section
                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 10

                    Label {
                        text: "[~] Scale Info"
                        font.pixelSize: 13
                        font.bold: true
                        color: textColor
                    }

                    Rectangle {
                        Layout.fillWidth: true
                        height: 1
                        color: "#2d3e50"
                    }

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
                                        ? "Range: " + backend.dataRangeInMicrovolts.toFixed(0) + " uV"
                                        : "Waiting for data..."
                                    font.pixelSize: 11
                                    color: backend.scaleCalibrated ? textColor : textSecondary
                                }

                                Label {
                                    text: "Scale bar: " + backend.scaleBarValue.toFixed(0) + " uV"
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

                // Actions Section
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
                        text: "[X] End Examination"
                        font.pixelSize: 11
                        Layout.preferredHeight: 40
                        palette.button: dangerColor
                        palette.buttonText: "white"
                        onClicked: root.endExaminationClicked()
                    }
                }

                Item { Layout.fillHeight: true }
            }
        }
    }
}
