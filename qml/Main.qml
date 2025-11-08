import QtQuick
import QtQuick.Controls
import QtQuick.Window
import QtQuick.Layouts

Window {
    id: mainWindow
    visible: true
    width: 250
    height: 100
    title: qsTr("VideoEEG")

    EegGraphsWindow {
        id: graphsWindow
    }

    AmplifierSetupWindow {
        id: amplifierSetupWindow
    }

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

        Button {
            text: "Camera"
            Layout.preferredWidth: 100
            Layout.preferredHeight: 40
            Layout.alignment: Qt.AlignHCenter
            onClicked: {
                // TODO: Open camera settings window
            }
        }
    }
}
