import QtQuick
import QtQuick.Controls
import QtQuick.Window
import videoEeg

ApplicationWindow {
    id: root
    visible: true
    width: 250
    height: 100
    title: qsTr("VideoEEG main.qml")

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
