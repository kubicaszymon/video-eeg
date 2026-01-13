import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    signal eegWindowOpen(amplifierId: string, channels: var)

    ColumnLayout {
        anchors.centerIn: parent
        spacing: 10

        Button {
            text: "Amplifier"
            Layout.preferredWidth: 100
            Layout.preferredHeight: 40
            Layout.alignment: Qt.AlignHCenter
            onClicked: {
                amplifierSetupWindow.show()
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
