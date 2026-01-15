import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "components/main"

Item {
    id: root
    signal eegWindowOpen(amplifierId: string, channels: var)

    // Theme colors
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

            // Left Section - Patients
            PatientListPanel {
                Layout.fillHeight: true
                Layout.preferredWidth: parent.width * 0.35
                Layout.minimumWidth: 400
                cardColor: root.cardColor
                borderColor: root.borderColor
                sidebarColor: root.sidebarColor
                textColor: root.textColor
                hoverColor: root.hoverColor
            }

            // Right Section - Examinations
            ExaminationsPanel {
                Layout.fillHeight: true
                Layout.fillWidth: true
                bgColor: root.bgColor
                cardColor: root.cardColor
                borderColor: root.borderColor
                sidebarColor: root.sidebarColor
                accentColor: root.accentColor
                textColor: root.textColor

                onNewExaminationClicked: amplifierSetupWindow.show()
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
