import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: mainWindow
    signal eegWindowOpen(amplifierId: string, channels: var)

    // Hospital colors
    readonly property color bgColor: "#f5f7fa"
    readonly property color sidebarColor: "#2c3e50"
    readonly property color cardColor: "#ffffff"
    readonly property color accentColor: "#3498db"
    readonly property color textColor: "#2c3e50"
    readonly property color borderColor: "#e0e6ed"
    readonly property color hoverColor: "#ecf0f1"

    Rectangle {
        anchors.fill: parent
        color: bgColor

        RowLayout {
            anchors.fill: parent
            anchors.margins: 0
            spacing: 0

            // LEFT SECTION - PATIENTS
            Rectangle {
                Layout.fillHeight: true
                Layout.preferredWidth: parent.width * 0.35
                Layout.minimumWidth: 400
                color: cardColor
                border.color: borderColor
                border.width: 1

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 0
                    spacing: 0

                    PanelHeader {
                        icon: "👤"
                        title: "Patients"
                        buttonText: "+ New"
                        buttonColor: "#2ecc71"
                        headerColor: sidebarColor
                    }

                    // Search bar
                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 50
                        color: "white"
                        border.color: borderColor
                        border.width: 1

                        RowLayout {
                            anchors.fill: parent
                            anchors.margins: 10
                            spacing: 5

                            Label {
                                text: "🔍"
                                font.pixelSize: 16
                            }

                            TextField {
                                Layout.fillWidth: true
                                placeholderText: "Search patient (ID, surname...)"
                                font.pixelSize: 12
                            }
                        }
                    }

                    // Patient list
                    ScrollView {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        clip: true

                        ListView {
                            anchors.fill: parent
                            model: 8
                            spacing: 1

                            delegate: PatientListItem {
                                patientName: "Jan Kowalski"
                                patientId: "85010112345"
                                lastExamination: "10.01.2025"
                                textColor: mainWindow.textColor
                                hoverColor: mainWindow.hoverColor
                                borderColor: mainWindow.borderColor
                            }
                        }
                    }
                }
            }

            // RIGHT SECTION - EXAMINATIONS
            Rectangle {
                Layout.fillHeight: true
                Layout.fillWidth: true
                color: bgColor

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 0
                    spacing: 0

                    PanelHeader {
                        icon: "📊"
                        title: "EEG Examinations"
                        buttonText: "⚡ New EEG Examination"
                        buttonColor: accentColor
                        headerColor: sidebarColor
                        onButtonClicked: amplifierSetupWindow.show()
                    }

                    // Filters bar
                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 50
                        color: "white"
                        border.color: borderColor
                        border.width: 1

                        RowLayout {
                            anchors.fill: parent
                            anchors.leftMargin: 15
                            anchors.rightMargin: 15
                            spacing: 15

                            Label {
                                text: "Filter:"
                                font.pixelSize: 12
                                color: textColor
                            }

                            ComboBox {
                                Layout.preferredWidth: 150
                                Layout.preferredHeight: 30
                                model: ["All", "Today", "This week", "This month"]
                                font.pixelSize: 11
                            }

                            ComboBox {
                                Layout.preferredWidth: 150
                                Layout.preferredHeight: 30
                                model: ["All statuses", "Completed", "In progress", "Cancelled"]
                                font.pixelSize: 11
                            }

                            Item { Layout.fillWidth: true }

                            Label {
                                text: "Sort:"
                                font.pixelSize: 12
                                color: textColor
                            }

                            ComboBox {
                                Layout.preferredWidth: 150
                                Layout.preferredHeight: 30
                                model: ["Newest", "Oldest", "By patient"]
                                font.pixelSize: 11
                            }
                        }
                    }

                    // Examination list
                    ScrollView {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        clip: true
                        contentWidth: availableWidth

                        Flow {
                            width: parent.width
                            padding: 15
                            spacing: 15

                            Repeater {
                                model: 6

                                Rectangle {
                                    width: (parent.width - parent.padding * 2 - parent.spacing) / 2
                                    height: 200
                                    color: cardColor
                                    radius: 8
                                    border.color: cardMouseArea.containsMouse ? accentColor : borderColor
                                    border.width: cardMouseArea.containsMouse ? 2 : 1

                                    MouseArea {
                                        id: cardMouseArea
                                        anchors.fill: parent
                                        hoverEnabled: true
                                    }

                                    ColumnLayout {
                                        anchors.fill: parent
                                        anchors.margins: 15
                                        spacing: 8

                                        RowLayout {
                                            Layout.fillWidth: true
                                            spacing: 10

                                            Rectangle {
                                                Layout.preferredWidth: 50
                                                Layout.preferredHeight: 50
                                                color: "#e8f4f8"
                                                radius: 25

                                                Label {
                                                    anchors.centerIn: parent
                                                    text: "📈"
                                                    font.pixelSize: 24
                                                }
                                            }

                                            ColumnLayout {
                                                Layout.fillWidth: true
                                                spacing: 2

                                                Label {
                                                    text: "Jan Kowalski"
                                                    font.pixelSize: 14
                                                    font.bold: true
                                                    color: textColor
                                                    elide: Text.ElideRight
                                                    Layout.fillWidth: true
                                                }

                                                Label {
                                                    text: "EEG - Standard examination"
                                                    font.pixelSize: 11
                                                    color: "#7f8c8d"
                                                    elide: Text.ElideRight
                                                    Layout.fillWidth: true
                                                }
                                            }

                                            StatusBadge {
                                                status: "Completed"
                                            }
                                        }

                                        Rectangle {
                                            Layout.fillWidth: true
                                            Layout.preferredHeight: 1
                                            color: borderColor
                                        }

                                        GridLayout {
                                            Layout.fillWidth: true
                                            columns: 2
                                            columnSpacing: 10
                                            rowSpacing: 5

                                            Label {
                                                text: "📅 Date:"
                                                font.pixelSize: 11
                                                color: "#7f8c8d"
                                            }

                                            Label {
                                                text: "14.01.2025 10:30"
                                                font.pixelSize: 11
                                                color: textColor
                                                Layout.fillWidth: true
                                            }

                                            Label {
                                                text: "⏱️ Duration:"
                                                font.pixelSize: 11
                                                color: "#7f8c8d"
                                            }

                                            Label {
                                                text: "45 min"
                                                font.pixelSize: 11
                                                color: textColor
                                                Layout.fillWidth: true
                                            }

                                            Label {
                                                text: "🔌 Channels:"
                                                font.pixelSize: 11
                                                color: "#7f8c8d"
                                            }

                                            Label {
                                                text: "14 channels"
                                                font.pixelSize: 11
                                                color: textColor
                                                Layout.fillWidth: true
                                            }
                                        }

                                        Item { Layout.fillHeight: true }

                                        RowLayout {
                                            Layout.fillWidth: true
                                            spacing: 8

                                            Button {
                                                text: "👁️ Preview"
                                                font.pixelSize: 10
                                                Layout.fillWidth: true
                                                Layout.preferredHeight: 30
                                            }

                                            Button {
                                                text: "📄 Report"
                                                font.pixelSize: 10
                                                Layout.fillWidth: true
                                                Layout.preferredHeight: 30
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    AmplifierSetupWindow {
        id: amplifierSetupWindow
        onAccepted: function(amplifier, channels) {
            eegWindowOpen(amplifier, channels)
        }

        onRejected: {
            console.log("Setup cancelled")
        }
    }
}
