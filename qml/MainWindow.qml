import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    signal eegWindowOpen(amplifierId: string, channels: var)

    // Kolory szpitalne
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

            // LEWA SEKCJA - PACJENCI
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

                    // Header pacjentów
                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 60
                        color: sidebarColor

                        RowLayout {
                            anchors.fill: parent
                            anchors.margins: 15
                            spacing: 10

                            Label {
                                text: "👤 Pacjenci"
                                font.pixelSize: 18
                                font.bold: true
                                color: "white"
                                Layout.fillWidth: true
                            }

                            Button {
                                text: "+ Nowy"
                                font.pixelSize: 12
                                Layout.preferredWidth: 80
                                Layout.preferredHeight: 35
                                palette.button: "#2ecc71"
                                palette.buttonText: "white"
                            }
                        }
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
                                placeholderText: "Szukaj pacjenta (PESEL, nazwisko...)"
                                font.pixelSize: 12
                            }
                        }
                    }

                    // Lista pacjentów
                    ScrollView {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        clip: true

                        ListView {
                            anchors.fill: parent
                            model: 8
                            spacing: 1

                            delegate: Rectangle {
                                width: ListView.view.width
                                height: 80
                                color: mouseArea.containsMouse ? hoverColor : "white"
                                border.color: borderColor
                                border.width: 1

                                MouseArea {
                                    id: mouseArea
                                    anchors.fill: parent
                                    hoverEnabled: true
                                }

                                ColumnLayout {
                                    anchors.fill: parent
                                    anchors.margins: 12
                                    spacing: 4

                                    Label {
                                        text: "Jan Kowalski"
                                        font.pixelSize: 14
                                        font.bold: true
                                        color: textColor
                                    }

                                    Label {
                                        text: "PESEL: 85010112345"
                                        font.pixelSize: 11
                                        color: "#7f8c8d"
                                    }

                                    Label {
                                        text: "Ostatnie badanie: 10.01.2025"
                                        font.pixelSize: 11
                                        color: "#7f8c8d"
                                    }
                                }
                            }
                        }
                    }
                }
            }

            // PRAWA SEKCJA - BADANIA
            Rectangle {
                Layout.fillHeight: true
                Layout.fillWidth: true
                color: bgColor

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 0
                    spacing: 0

                    // Header badań
                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 60
                        color: sidebarColor

                        RowLayout {
                            anchors.fill: parent
                            anchors.margins: 15
                            spacing: 10

                            Label {
                                text: "📊 Badania EEG"
                                font.pixelSize: 18
                                font.bold: true
                                color: "white"
                                Layout.fillWidth: true
                            }

                            Button {
                                text: "⚡ Nowe badanie EEG"
                                font.pixelSize: 12
                                font.bold: true
                                Layout.preferredWidth: 180
                                Layout.preferredHeight: 40
                                palette.button: accentColor
                                palette.buttonText: "white"

                                onClicked: {
                                    amplifierSetupWindow.show()
                                }
                            }
                        }
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
                                text: "Filtruj:"
                                font.pixelSize: 12
                                color: textColor
                            }

                            ComboBox {
                                Layout.preferredWidth: 150
                                Layout.preferredHeight: 30
                                model: ["Wszystkie", "Dzisiaj", "Ten tydzień", "Ten miesiąc"]
                                font.pixelSize: 11
                            }

                            ComboBox {
                                Layout.preferredWidth: 150
                                Layout.preferredHeight: 30
                                model: ["Wszystkie statusy", "Ukończone", "W trakcie", "Anulowane"]
                                font.pixelSize: 11
                            }

                            Item { Layout.fillWidth: true }

                            Label {
                                text: "Sortuj:"
                                font.pixelSize: 12
                                color: textColor
                            }

                            ComboBox {
                                Layout.preferredWidth: 150
                                Layout.preferredHeight: 30
                                model: ["Najnowsze", "Najstarsze", "Wg pacjenta"]
                                font.pixelSize: 11
                            }
                        }
                    }

                    // Lista badań - POPRAWIONA
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
                                                    text: "EEG - Badanie standardowe"
                                                    font.pixelSize: 11
                                                    color: "#7f8c8d"
                                                    elide: Text.ElideRight
                                                    Layout.fillWidth: true
                                                }
                                            }

                                            Rectangle {
                                                Layout.preferredWidth: 80
                                                Layout.preferredHeight: 24
                                                color: "#d4edda"
                                                radius: 12
                                                border.color: "#c3e6cb"
                                                border.width: 1

                                                Label {
                                                    anchors.centerIn: parent
                                                    text: "Ukończone"
                                                    font.pixelSize: 9
                                                    color: "#155724"
                                                }
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
                                                text: "📅 Data:"
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
                                                text: "⏱️ Czas:"
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
                                                text: "🔌 Kanały:"
                                                font.pixelSize: 11
                                                color: "#7f8c8d"
                                            }

                                            Label {
                                                text: "14 kanałów"
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
                                                text: "👁️ Podgląd"
                                                font.pixelSize: 10
                                                Layout.fillWidth: true
                                                Layout.preferredHeight: 30
                                            }

                                            Button {
                                                text: "📄 Raport"
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
