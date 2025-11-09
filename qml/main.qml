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

    ColumnLayout {
        anchors.centerIn: parent
        spacing: 10

        Button {
            text: "Amplifier"
            Layout.preferredWidth: 100
            Layout.preferredHeight: 40
            Layout.alignment: Qt.AlignHCenter
            onClicked: {
                var component = Qt.createComponent("AmplifierSetupWindow.qml")
                var window = component.createObject(null)
                window.show()
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
