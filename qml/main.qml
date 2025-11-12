import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Window
import QtQuick.Layouts

Window {
    id: root
    visible: true
    width: 250
    height: 100
    title: qsTr("VideoEEG")

    property var amplifierSetupWindow: null
    property var eegWindow: null

    ColumnLayout {
        anchors.centerIn: parent
        spacing: 10

        Button {
            text: "Amplifier"
            Layout.preferredWidth: 100
            Layout.preferredHeight: 40
            Layout.alignment: Qt.AlignHCenter
            onClicked: {
                if(!amplifierSetupWindow)
                {
                    var component = Qt.createComponent("AmplifierSetupWindow.qml")
                    amplifierSetupWindow = component.createObject(root, {"mainWindow": root})
                }
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
