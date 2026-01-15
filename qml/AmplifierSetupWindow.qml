import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
import videoEeg

import "components/setup"

Window {
    id: window
    visible: false
    width: 1200
    height: 750
    title: qsTr("EEG Examination Configuration")
    modality: Qt.ApplicationModal

    signal accepted(amplifierId: string, channels: var)
    signal rejected()

    property int loading: Globals.status
    property var channelSelectionModel: []
    property int selectedCameraIndex: -1
    property string savePath: ""

    // Mock cameras (replace with real backend)
    property var availableCameras: ["Built-in Camera", "USB Camera HD", "Logitech C920"]

    // Theme colors
    readonly property color bgColor: "#f5f7fa"
    readonly property color sidebarColor: "#2c3e50"
    readonly property color cardColor: "#ffffff"
    readonly property color accentColor: "#3498db"
    readonly property color successColor: "#2ecc71"
    readonly property color textColor: "#2c3e50"
    readonly property color borderColor: "#e0e6ed"
    readonly property color hoverColor: "#ecf0f1"

    AmplifierSetupBackend {
        id: backend

        onCurrentChannelsChanged: {
            channelSelectionModel = []
            for (var i = 0; i < backend.currentChannels.length; i++) {
                channelSelectionModel.push(false)
            }
        }
    }

    Timer {
        id: timer
        running: true
        repeat: true
        onTriggered: {
            if (window.loading === Globals.Loading) {
                Globals.status = Globals.Ready
            }
        }
    }

    FileDialog {
        id: folderDialog
        fileMode: FileDialog.SaveFile
        nameFilters: ["Video files (*.mp4 *.avi)"]
        onAccepted: {
            savePath = selectedFile.toString().replace("file:///", "")
        }
    }

    // Helper functions
    function getSelectedChannelsCount() {
        var count = 0
        for (var i = 0; i < channelSelectionModel.length; i++) {
            if (channelSelectionModel[i]) count++
        }
        return count
    }

    function toggleChannel(index) {
        channelSelectionModel[index] = !channelSelectionModel[index]
        channelSelectionModel = channelSelectionModel.slice()
    }

    function selectAllChannels(checked) {
        for (var i = 0; i < channelSelectionModel.length; i++) {
            channelSelectionModel[i] = checked
        }
        channelSelectionModel = channelSelectionModel.slice()
    }

    function getSelectedChannelsList() {
        var selected = []
        for (var i = 0; i < channelSelectionModel.length; i++) {
            if (channelSelectionModel[i]) {
                selected.push(backend.currentChannels[i])
            }
        }
        return selected
    }

    function getCurrentStepNumber() {
        if (stackView.currentItem === amplifierSelectionPage) return 1
        if (stackView.currentItem === channelSelectionPage) return 2
        if (stackView.currentItem === cameraSelectionPage) return 3
        if (stackView.currentItem === summaryPage) return 4
        return 1
    }

    function getCurrentStepTitle() {
        var step = getCurrentStepNumber()
        switch (step) {
            case 1: return "Step 1: Amplifier Selection"
            case 2: return "Step 2: Channel Selection"
            case 3: return "Step 3: Camera Selection"
            case 4: return "Step 4: Summary"
            default: return ""
        }
    }

    function getCurrentStepDesc() {
        var step = getCurrentStepNumber()
        switch (step) {
            case 1: return "Detect and select EEG amplifier"
            case 2: return "Select channels to record"
            case 3: return "Select camera for video recording"
            case 4: return "Review configuration before starting"
            default: return ""
        }
    }

    function handleCancel() {
        rejected()
        window.close()
    }

    function handleRefresh() {
        Globals.status = Globals.Loading
        timer.restart()
        backend.refreshAmplifiersList()
    }

    function handleStart() {
        var selectedChannels = []
        for (var i = 0; i < channelSelectionModel.length; i++) {
            if (channelSelectionModel[i]) {
                selectedChannels.push(i)
            }
        }
        console.log("Starting examination with", selectedChannels.length, "channels")
        accepted(backend.getSelectedAmplifierId(), selectedChannels)
        window.close()
    }

    Rectangle {
        anchors.fill: parent
        color: bgColor

        ColumnLayout {
            anchors.fill: parent
            spacing: 0

            // Header with progress indicator
            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 80
                color: sidebarColor

                RowLayout {
                    anchors.fill: parent
                    anchors.margins: 20
                    spacing: 15

                    Label {
                        text: "~"
                        font.pixelSize: 32
                        color: "white"
                    }

                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 2

                        Label {
                            text: getCurrentStepTitle()
                            font.pixelSize: 18
                            font.bold: true
                            color: "white"
                        }

                        Label {
                            text: getCurrentStepDesc()
                            font.pixelSize: 12
                            color: "#95a5a6"
                        }
                    }

                    // Progress indicator (4 steps)
                    Row {
                        spacing: 10

                        Repeater {
                            model: 4

                            Row {
                                spacing: 10

                                Rectangle {
                                    width: 35
                                    height: 35
                                    radius: 17.5
                                    color: getCurrentStepNumber() > index ? successColor :
                                           getCurrentStepNumber() === index + 1 ? accentColor : "#7f8c8d"
                                    border.color: "white"
                                    border.width: 2

                                    Label {
                                        anchors.centerIn: parent
                                        text: getCurrentStepNumber() > index ? "ok" : (index + 1).toString()
                                        font.pixelSize: getCurrentStepNumber() > index ? 10 : 14
                                        font.bold: true
                                        color: "white"
                                    }
                                }

                                Rectangle {
                                    width: 40
                                    height: 2
                                    color: getCurrentStepNumber() > index + 1 ? successColor : "#7f8c8d"
                                    visible: index < 3
                                    anchors.verticalCenter: parent.verticalCenter
                                }
                            }
                        }
                    }
                }
            }

            // Content area with StackView
            StackView {
                id: stackView
                Layout.fillWidth: true
                Layout.fillHeight: true
                initialItem: amplifierSelectionPage

                pushEnter: Transition {
                    PropertyAnimation { property: "opacity"; from: 0; to: 1; duration: 200 }
                }
                pushExit: Transition {
                    PropertyAnimation { property: "opacity"; from: 1; to: 0; duration: 200 }
                }
                popEnter: Transition {
                    PropertyAnimation { property: "opacity"; from: 0; to: 1; duration: 200 }
                }
                popExit: Transition {
                    PropertyAnimation { property: "opacity"; from: 1; to: 0; duration: 200 }
                }
            }

            // Loading overlay
            Rectangle {
                Layout.fillWidth: true
                Layout.fillHeight: true
                color: "#80000000"
                visible: window.loading === Globals.Loading
                z: 1000

                ColumnLayout {
                    anchors.centerIn: parent
                    spacing: 15

                    BusyIndicator {
                        Layout.alignment: Qt.AlignHCenter
                        running: parent.parent.visible
                        Layout.preferredWidth: 60
                        Layout.preferredHeight: 60
                    }

                    Label {
                        text: "Scanning devices..."
                        font.pixelSize: 14
                        color: "white"
                        Layout.alignment: Qt.AlignHCenter
                    }
                }
            }
        }
    }

    // Step 1: Amplifier Selection
    Component {
        id: amplifierSelectionPage

        AmplifierSelectionPage {
            backend: window.backend
            bgColor: window.bgColor
            cardColor: window.cardColor
            borderColor: window.borderColor
            textColor: window.textColor
            accentColor: window.accentColor
            successColor: window.successColor
            hoverColor: window.hoverColor

            onNextClicked: stackView.push(channelSelectionPage)
            onCancelClicked: window.handleCancel()
            onRefreshClicked: window.handleRefresh()
        }
    }

    // Step 2: Channel Selection
    Component {
        id: channelSelectionPage

        ChannelSelectionPage {
            backend: window.backend
            channelSelectionModel: window.channelSelectionModel
            bgColor: window.bgColor
            cardColor: window.cardColor
            borderColor: window.borderColor
            textColor: window.textColor
            accentColor: window.accentColor

            onNextClicked: stackView.push(cameraSelectionPage)
            onBackClicked: stackView.pop()
            onCancelClicked: window.handleCancel()
            onToggleChannel: function(index) { window.toggleChannel(index) }
            onSelectAllChannels: function(checked) { window.selectAllChannels(checked) }
        }
    }

    // Step 3: Camera Selection
    Component {
        id: cameraSelectionPage

        CameraSelectionPage {
            availableCameras: window.availableCameras
            selectedCameraIndex: window.selectedCameraIndex
            bgColor: window.bgColor
            cardColor: window.cardColor
            borderColor: window.borderColor
            textColor: window.textColor
            accentColor: window.accentColor
            successColor: window.successColor
            hoverColor: window.hoverColor

            onNextClicked: stackView.push(summaryPage)
            onBackClicked: stackView.pop()
            onCancelClicked: window.handleCancel()
            onCameraSelected: function(index) { window.selectedCameraIndex = index }
            onRefreshCamerasClicked: console.log("Refresh cameras clicked")
        }
    }

    // Step 4: Summary
    Component {
        id: summaryPage

        SummaryPage {
            backend: window.backend
            availableCameras: window.availableCameras
            selectedCameraIndex: window.selectedCameraIndex
            savePath: window.savePath
            bgColor: window.bgColor
            cardColor: window.cardColor
            borderColor: window.borderColor
            textColor: window.textColor
            successColor: window.successColor

            getSelectedChannelsCount: window.getSelectedChannelsCount
            getSelectedChannelsList: window.getSelectedChannelsList

            onStartClicked: window.handleStart()
            onBackClicked: stackView.pop()
            onCancelClicked: window.handleCancel()
            onBrowseClicked: folderDialog.open()
        }
    }
}
