import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
import videoEeg

Window {
    id: window
    visible: false
    width: 1200
    height: 750
    title: qsTr("⚡ EEG Examination Configuration")
    modality: Qt.ApplicationModal

    signal accepted(amplifierId: string, channels: var)
    signal rejected()

    property var channelSelectionModel: []
    property int selectedCameraIndex: -1
    property string savePath: ""
    property int currentStep: 1
    property int channelUpdateCounter: 0  // Trigger do wymuszenia aktualizacji UI

    // Mock cameras (replace with real backend)
    property var availableCameras: ["Built-in Camera", "USB Camera HD", "Logitech C920"]

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

    FileDialog {
        id: folderDialog
        fileMode: FileDialog.SaveFile
        nameFilters: ["Video files (*.mp4 *.avi)"]
        onAccepted: {
            savePath = selectedFile.toString().replace("file:///", "")
        }
    }

    function isChannelSelected(idx) {
        void(channelUpdateCounter)  // Dependency na uupdatecounter
        return channelSelectionModel[idx] === true
    }

    function getSelectedChannelsCount() {
        void(channelUpdateCounter)
        var count = 0
        for (var i = 0; i < channelSelectionModel.length; i++) {
            if (channelSelectionModel[i]) count++
        }
        return count
    }

    function areAllChannelsSelected() {
        void(channelUpdateCounter)
        if (channelSelectionModel.length === 0) return false
        for (var i = 0; i < channelSelectionModel.length; i++) {
            if (!channelSelectionModel[i]) return false
        }
        return true
    }

    function toggleChannel(index) {
        var newModel = channelSelectionModel.slice()
        newModel[index] = !newModel[index]
        channelSelectionModel = newModel
        channelUpdateCounter++
        console.log("Toggle channel", index, "->", newModel[index], "counter:", channelUpdateCounter)
    }

    function selectAllChannels(selectAll) {
        var newModel = []
        for (var i = 0; i < channelSelectionModel.length; i++) {
            newModel.push(selectAll)
        }
        channelSelectionModel = newModel
        channelUpdateCounter++
        console.log("Select all:", selectAll, "counter:", channelUpdateCounter)
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
        return currentStep
    }

    function getCurrentStepTitle() {
        var step = getCurrentStepNumber()
        switch(step) {
            case 1: return "Step 1: Amplifier Selection"
            case 2: return "Step 2: Channel Selection"
            case 3: return "Step 3: Camera Selection"
            case 4: return "Step 4: Summary"
            default: return ""
        }
    }

    function getCurrentStepDesc() {
        var step = getCurrentStepNumber()
        switch(step) {
            case 1: return "Detect and select EEG amplifier"
            case 2: return "Select channels to record"
            case 3: return "Select camera for video recording"
            case 4: return "Review configuration before starting"
            default: return ""
        }
    }

    Rectangle {
        anchors.fill: parent
        color: bgColor

        ColumnLayout {
            anchors.fill: parent
            spacing: 0

            // HEADER WITH PROGRESS
            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 80
                color: sidebarColor

                RowLayout {
                    anchors.fill: parent
                    anchors.margins: 20
                    spacing: 15

                    Label {
                        text: "⚡"
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
                                        text: getCurrentStepNumber() > index ? "✓" : (index + 1).toString()
                                        font.pixelSize: getCurrentStepNumber() > index ? 16 : 14
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

            // CONTENT
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
        }

        // LOADING OVERLAY - positioned over entire window content
        Rectangle {
            anchors.fill: parent
            color: "#cc1a1a2e"
            visible: backend.isLoading
            opacity: backend.isLoading ? 1 : 0
            z: 1000

            Behavior on opacity {
                NumberAnimation { duration: 200 }
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {} // Block clicks
            }

            Rectangle {
                anchors.centerIn: parent
                width: 280
                height: 200
                radius: 16
                color: cardColor
                border.color: accentColor
                border.width: 2

                layer.enabled: true

                ColumnLayout {
                    anchors.centerIn: parent
                    spacing: 20

                    // Custom spinning loader
                    Item {
                        Layout.alignment: Qt.AlignHCenter
                        Layout.preferredWidth: 60
                        Layout.preferredHeight: 60

                        Rectangle {
                            id: spinnerOuter
                            anchors.centerIn: parent
                            width: 60
                            height: 60
                            radius: 30
                            color: "transparent"
                            border.color: borderColor
                            border.width: 4
                        }

                        Rectangle {
                            id: spinnerArc
                            anchors.centerIn: parent
                            width: 60
                            height: 60
                            radius: 30
                            color: "transparent"
                            border.color: accentColor
                            border.width: 4

                            Rectangle {
                                width: 32
                                height: 32
                                color: cardColor
                                anchors.right: parent.right
                                anchors.verticalCenter: parent.verticalCenter
                            }

                            Rectangle {
                                width: 32
                                height: 32
                                color: cardColor
                                anchors.bottom: parent.bottom
                                anchors.horizontalCenter: parent.horizontalCenter
                            }

                            RotationAnimation on rotation {
                                from: 0
                                to: 360
                                duration: 1000
                                loops: Animation.Infinite
                                running: backend.isLoading
                            }
                        }

                        Label {
                            anchors.centerIn: parent
                            text: "🔍"
                            font.pixelSize: 20
                        }
                    }

                    ColumnLayout {
                        Layout.alignment: Qt.AlignHCenter
                        spacing: 6

                        Label {
                            text: "Scanning for devices..."
                            font.pixelSize: 15
                            font.bold: true
                            color: textColor
                            Layout.alignment: Qt.AlignHCenter
                        }

                        Label {
                            text: "Looking for EEG amplifiers"
                            font.pixelSize: 12
                            color: "#7f8c8d"
                            Layout.alignment: Qt.AlignHCenter
                        }
                    }

                    // Animated dots
                    Row {
                        Layout.alignment: Qt.AlignHCenter
                        spacing: 6

                        Repeater {
                            model: 3

                            Rectangle {
                                width: 8
                                height: 8
                                radius: 4
                                color: accentColor

                                SequentialAnimation on opacity {
                                    loops: Animation.Infinite
                                    running: backend.isLoading

                                    PauseAnimation { duration: index * 200 }
                                    NumberAnimation { from: 0.3; to: 1; duration: 300 }
                                    NumberAnimation { from: 1; to: 0.3; duration: 300 }
                                    PauseAnimation { duration: (2 - index) * 200 }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    // STEP 1: AMPLIFIER SELECTION
    Component {
        id: amplifierSelectionPage

        Rectangle {
            color: bgColor

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 30
                spacing: 20

                InfoBanner {
                    icon: "ℹ️"
                    message: "Make sure the amplifier is turned on and connected to the computer"
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    color: cardColor
                    radius: 8
                    border.color: borderColor
                    border.width: 1

                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: 20
                        spacing: 15

                        SectionHeader {
                            icon: "🔌"
                            title: "Detected Amplifiers"
                            count: backend.availableAmplifiers.length
                            textColor: window.textColor
                        }

                        Rectangle {
                            Layout.fillWidth: true
                            Layout.preferredHeight: 1
                            color: borderColor
                        }

                        ScrollView {
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            clip: true

                            ListView {
                                id: amplifierListView
                                model: backend.availableAmplifiers
                                spacing: 10

                                delegate: DeviceCard {
                                    icon: "⚡"
                                    deviceName: modelData
                                    deviceInfo: "Virtual Amplifier • Ready"
                                    isSelected: backend.selectedAmplifierIndex === index
                                    accentColor: window.accentColor
                                    cardColor: window.cardColor
                                    hoverColor: window.hoverColor
                                    textColor: window.textColor
                                    borderColor: window.borderColor
                                    successColor: window.successColor
                                    onClicked: backend.selectedAmplifierIndex = index
                                }

                                Label {
                                    anchors.centerIn: parent
                                    visible: amplifierListView.count === 0
                                    text: "😔 No amplifiers found\nClick 'Refresh' to scan again"
                                    font.pixelSize: 13
                                    color: "#999999"
                                    horizontalAlignment: Text.AlignHCenter
                                }
                            }
                        }
                    }
                }

                NavigationBar {
                    showBack: false
                    showRefresh: true
                    refreshText: "🔄 Refresh"
                    refreshEnabled: !backend.isLoading
                    nextEnabled: backend.selectedAmplifierIndex !== -1
                    nextColor: accentColor

                    onRefreshClicked: {
                        backend.refreshAmplifiersList()
                    }
                    onCancelClicked: {
                        rejected()
                        window.close()
                    }
                    onNextClicked: {
                        currentStep = 2
                        stackView.push(channelSelectionPage)
                    }
                }
            }
        }
    }

    // STEP 2: CHANNEL SELECTION
    Component {
        id: channelSelectionPage

        Rectangle {
            color: bgColor

            // Local properties that react to channelUpdateCounter
            property bool allSelected: {
                void(channelUpdateCounter)
                return areAllChannelsSelected()
            }
            property int selectedCount: {
                void(channelUpdateCounter)
                return getSelectedChannelsCount()
            }

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 30
                spacing: 20

                SuccessBanner {
                    title: "Selected amplifier: " + backend.availableAmplifiers[backend.selectedAmplifierIndex]
                    subtitle: backend.currentChannels.length + " available channels"
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    color: cardColor
                    radius: 8
                    border.color: borderColor
                    border.width: 1

                    ColumnLayout {
                        anchors.fill: parent
                        spacing: 0

                        // Header row
                        Rectangle {
                            Layout.fillWidth: true
                            Layout.preferredHeight: 50
                            color: "#f8f9fa"
                            radius: 8

                            RowLayout {
                                anchors.fill: parent
                                anchors.leftMargin: 20
                                anchors.rightMargin: 20

                                Label {
                                    text: "No."
                                    font.pixelSize: 12
                                    font.bold: true
                                    color: textColor
                                    Layout.preferredWidth: 60
                                }

                                Label {
                                    text: "Channel Name"
                                    font.pixelSize: 12
                                    font.bold: true
                                    color: textColor
                                    Layout.fillWidth: true
                                }

                                // Select All checkbox
                                Rectangle {
                                    width: selectAllRow.width + 16
                                    height: 30
                                    radius: 4
                                    color: selectAllMouse.containsMouse ? hoverColor : "transparent"

                                    Row {
                                        id: selectAllRow
                                        anchors.centerIn: parent
                                        spacing: 8

                                        Rectangle {
                                            width: 18
                                            height: 18
                                            radius: 3
                                            border.color: allSelected ? accentColor : "#bdc3c7"
                                            border.width: 2
                                            color: allSelected ? accentColor : "white"

                                            Text {
                                                anchors.centerIn: parent
                                                text: "✓"
                                                font.pixelSize: 14
                                                font.bold: true
                                                color: "white"
                                                visible: allSelected
                                            }
                                        }

                                        Text {
                                            text: "Select all"
                                            font.pixelSize: 12
                                            color: textColor
                                            anchors.verticalCenter: parent.verticalCenter
                                        }
                                    }

                                    MouseArea {
                                        id: selectAllMouse
                                        anchors.fill: parent
                                        hoverEnabled: true
                                        cursorShape: Qt.PointingHandCursor
                                        onClicked: {
                                            selectAllChannels(!allSelected)
                                        }
                                    }
                                }
                            }
                        }

                        Rectangle {
                            Layout.fillWidth: true
                            Layout.preferredHeight: 1
                            color: borderColor
                        }

                        // Channel list
                        ScrollView {
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            clip: true

                            ListView {
                                id: channelListView
                                model: backend.currentChannels
                                spacing: 0

                                delegate: Rectangle {
                                    id: channelRow
                                    width: channelListView.width
                                    height: 45

                                    property bool isSelected: {
                                        void(channelUpdateCounter)
                                        return isChannelSelected(index)
                                    }

                                    color: isSelected ? "#e8f4f8" : (index % 2 === 0 ? "white" : "#f8f9fa")

                                    RowLayout {
                                        anchors.fill: parent
                                        anchors.leftMargin: 20
                                        anchors.rightMargin: 20
                                        spacing: 15

                                        Label {
                                            text: (index + 1).toString()
                                            font.pixelSize: 12
                                            color: textColor
                                            Layout.preferredWidth: 60
                                        }

                                        Label {
                                            text: modelData
                                            font.pixelSize: 12
                                            color: textColor
                                            Layout.fillWidth: true
                                        }

                                        // Channel checkbox
                                        Rectangle {
                                            width: 18
                                            height: 18
                                            radius: 3
                                            border.color: channelRow.isSelected ? accentColor : "#bdc3c7"
                                            border.width: 2
                                            color: channelRow.isSelected ? accentColor : "white"

                                            Text {
                                                anchors.centerIn: parent
                                                text: "✓"
                                                font.pixelSize: 14
                                                font.bold: true
                                                color: "white"
                                                visible: channelRow.isSelected
                                            }

                                            MouseArea {
                                                anchors.fill: parent
                                                cursorShape: Qt.PointingHandCursor
                                                onClicked: {
                                                    toggleChannel(index)
                                                }
                                            }
                                        }
                                    }

                                    // Make whole row clickable
                                    MouseArea {
                                        anchors.fill: parent
                                        z: -1
                                        onClicked: {
                                            toggleChannel(index)
                                        }
                                    }
                                }
                            }
                        }
                    }
                }

                NavigationBar {
                    showBack: true
                    middleText: selectedCount + " / " + backend.currentChannels.length + " selected"
                    nextEnabled: selectedCount > 0
                    nextColor: accentColor

                    onBackClicked: {
                        currentStep = 1
                        stackView.pop()
                    }
                    onCancelClicked: {
                        rejected()
                        window.close()
                    }
                    onNextClicked: {
                        currentStep = 3
                        stackView.push(cameraSelectionPage)
                    }
                }
            }
        }
    }

    // STEP 3: CAMERA SELECTION
    Component {
        id: cameraSelectionPage

        Rectangle {
            color: bgColor

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 30
                spacing: 20

                InfoBanner {
                    icon: "📹"
                    message: "Select a camera to synchronize video with EEG data (optional)"
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    color: cardColor
                    radius: 8
                    border.color: borderColor
                    border.width: 1

                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: 20
                        spacing: 15

                        SectionHeader {
                            icon: "📷"
                            title: "Available Cameras"
                            count: availableCameras.length
                            textColor: window.textColor
                        }

                        Rectangle {
                            Layout.fillWidth: true
                            Layout.preferredHeight: 1
                            color: borderColor
                        }

                        ScrollView {
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            clip: true

                            ListView {
                                model: availableCameras
                                spacing: 10

                                delegate: DeviceCard {
                                    icon: "📹"
                                    deviceName: modelData
                                    deviceInfo: "1920x1080 • 30 FPS"
                                    isSelected: selectedCameraIndex === index
                                    accentColor: window.accentColor
                                    cardColor: window.cardColor
                                    hoverColor: window.hoverColor
                                    textColor: window.textColor
                                    borderColor: window.borderColor
                                    successColor: window.successColor
                                    onClicked: selectedCameraIndex = index
                                }
                            }
                        }

                        // Option: no camera
                        Rectangle {
                            Layout.fillWidth: true
                            height: 60
                            color: selectedCameraIndex === -1 ? "#fff3cd" : cardColor
                            radius: 6
                            border.color: selectedCameraIndex === -1 ? "#ffc107" : borderColor
                            border.width: selectedCameraIndex === -1 ? 2 : 1

                            MouseArea {
                                anchors.fill: parent
                                onClicked: selectedCameraIndex = -1
                            }

                            RowLayout {
                                anchors.fill: parent
                                anchors.margins: 15
                                spacing: 12

                                Label {
                                    text: "⊘"
                                    font.pixelSize: 20
                                    color: "#856404"
                                }

                                Label {
                                    text: "Continue without camera (EEG data only)"
                                    font.pixelSize: 12
                                    font.bold: selectedCameraIndex === -1
                                    color: "#856404"
                                    Layout.fillWidth: true
                                }
                            }
                        }
                    }
                }

                NavigationBar {
                    showBack: true
                    showRefresh: true
                    refreshText: "🔄 Refresh cameras"
                    nextColor: accentColor

                    onRefreshClicked: { /* TODO: refresh cameras */ }
                    onBackClicked: {
                        currentStep = 2
                        stackView.pop()
                    }
                    onCancelClicked: {
                        rejected()
                        window.close()
                    }
                    onNextClicked: {
                        currentStep = 4
                        stackView.push(summaryPage)
                    }
                }
            }
        }
    }

    // STEP 4: SUMMARY
    Component {
        id: summaryPage

        Rectangle {
            color: bgColor

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 30
                spacing: 20

                Label {
                    text: "✅ Configuration Summary"
                    font.pixelSize: 18
                    font.bold: true
                    color: textColor
                }

                ScrollView {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    clip: true

                    ColumnLayout {
                        width: parent.width
                        spacing: 15

                        // Amplifier
                        Rectangle {
                            Layout.fillWidth: true
                            height: contentCol1.implicitHeight + 30
                            color: cardColor
                            radius: 8
                            border.color: borderColor
                            border.width: 1

                            ColumnLayout {
                                id: contentCol1
                                anchors.fill: parent
                                anchors.margins: 15
                                spacing: 8

                                Label {
                                    text: "⚡ Amplifier"
                                    font.pixelSize: 14
                                    font.bold: true
                                    color: textColor
                                }

                                Rectangle {
                                    Layout.fillWidth: true
                                    height: 1
                                    color: borderColor
                                }

                                Label {
                                    text: backend.availableAmplifiers[backend.selectedAmplifierIndex]
                                    font.pixelSize: 13
                                    color: "#7f8c8d"
                                }
                            }
                        }

                        // Channels
                        Rectangle {
                            Layout.fillWidth: true
                            height: contentCol2.implicitHeight + 30
                            color: cardColor
                            radius: 8
                            border.color: borderColor
                            border.width: 1

                            ColumnLayout {
                                id: contentCol2
                                anchors.fill: parent
                                anchors.margins: 15
                                spacing: 8

                                Label {
                                    text: "🔌 Selected Channels (" + getSelectedChannelsCount() + ")"
                                    font.pixelSize: 14
                                    font.bold: true
                                    color: textColor
                                }

                                Rectangle {
                                    Layout.fillWidth: true
                                    height: 1
                                    color: borderColor
                                }

                                Label {
                                    text: getSelectedChannelsList().join(", ")
                                    font.pixelSize: 12
                                    color: "#7f8c8d"
                                    wrapMode: Text.WordWrap
                                    Layout.fillWidth: true
                                }
                            }
                        }

                        // Camera
                        Rectangle {
                            Layout.fillWidth: true
                            height: contentCol3.implicitHeight + 30
                            color: cardColor
                            radius: 8
                            border.color: borderColor
                            border.width: 1

                            ColumnLayout {
                                id: contentCol3
                                anchors.fill: parent
                                anchors.margins: 15
                                spacing: 8

                                Label {
                                    text: "📹 Camera"
                                    font.pixelSize: 14
                                    font.bold: true
                                    color: textColor
                                }

                                Rectangle {
                                    Layout.fillWidth: true
                                    height: 1
                                    color: borderColor
                                }

                                Label {
                                    text: selectedCameraIndex >= 0 ? availableCameras[selectedCameraIndex] : "None (EEG only)"
                                    font.pixelSize: 13
                                    color: "#7f8c8d"
                                }
                            }
                        }

                        // Save location
                        Rectangle {
                            Layout.fillWidth: true
                            height: contentCol4.implicitHeight + 30
                            color: cardColor
                            radius: 8
                            border.color: borderColor
                            border.width: 1

                            ColumnLayout {
                                id: contentCol4
                                anchors.fill: parent
                                anchors.margins: 15
                                spacing: 8

                                Label {
                                    text: "💾 Recording Save Location"
                                    font.pixelSize: 14
                                    font.bold: true
                                    color: textColor
                                }

                                Rectangle {
                                    Layout.fillWidth: true
                                    height: 1
                                    color: borderColor
                                }

                                RowLayout {
                                    Layout.fillWidth: true
                                    spacing: 10

                                    TextField {
                                        Layout.fillWidth: true
                                        text: savePath || "Select location..."
                                        readOnly: true
                                        font.pixelSize: 11
                                    }

                                    Button {
                                        text: "📁 Browse"
                                        font.pixelSize: 11
                                        Layout.preferredHeight: 35
                                        onClicked: folderDialog.open()
                                    }
                                }
                            }
                        }
                    }
                }

                NavigationBar {
                    showBack: true
                    nextText: "✓ Start Examination"
                    nextColor: successColor

                    onBackClicked: {
                        currentStep = 3
                        stackView.pop()
                    }
                    onCancelClicked: {
                        rejected()
                        window.close()
                    }
                    onNextClicked: {
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
                }
            }
        }
    }
}
