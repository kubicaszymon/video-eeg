import QtQuick
import QtQuick.Controls
import QtQuick.Window
import videoEeg

ApplicationWindow {
    id: root
    visible: true
    width: Screen.width
    height: Screen.height
    visibility: Window.Maximized
    title: qsTr("VideoEEG")

    Loader {
        id: contentLoader
        anchors.fill: parent
        source: "MainWindow.qml"
    }

    Connections {
       target: contentLoader.item
       ignoreUnknownSignals: true
       function onEegWindowOpen(amplifier, channels){
           contentLoader.setSource("EegWindow.qml", {
                "amplifierId": amplifier,
                "channels": channels
            })
       }
    }
}
