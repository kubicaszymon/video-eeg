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

    property int loading: Globals.status
    property var channelSelectionModel: []
    property int selectedCameraIndex: -1
    property string savePath: ""

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

    // ==================== INLINE COMPONENTS ====================

    // InfoBanner - information/warning banner at top of pages
    component InfoBanner: Rectangle {
        property string icon: "ℹ️"
        property string message: ""
        property color bannerColor: "#e8f4f8"
        property color bannerBorderColor: "#bee5eb"
        property color bannerTextColor: "#0c5460"

        Layout.fillWidth: true
        Layout.preferredHeight: 60
        color: bannerColor
        radius: 8
        border.color: bannerBorderColor
        border.width: 1

        RowLayout {
            anchors.fill: parent
            anchors.margins: 15
            spacing: 12

            Label {
                text: icon
                font.pixelSize: 24
            }

            Label {
                text: message
                font.pixelSize: 12
                color: bannerTextColor
                Layout.fillWidth: true
                wrapMode: Text.WordWrap
            }
        }
    }

    // SuccessBanner - green success banner with title and subtitle
    component SuccessBanner: Rectangle {
        property string title: ""
        property string subtitle: ""

        Layout.fillWidth: true
        Layout.preferredHeight: 60
        color: "#d4edda"
        radius: 8
        border.color: "#c3e6cb"
        border.width: 1

        RowLayout {
            anchors.fill: parent
            anchors.margins: 15
            spacing: 12

            Label {
                text: "✓"
                font.pixelSize: 20
                font.bold: true
                color: "#155724"
            }

            ColumnLayout {
                spacing: 2
                Layout.fillWidth: true

                Label {
                    text: title
                    font.pixelSize: 12
                    font.bold: true
                    color: "#155724"
                }

                Label {
                    text: subtitle
                    font.pixelSize: 11
                    color: "#155724"
                }
            }
        }
    }

    // SectionHeader - header with icon, title and count
    component SectionHeader: RowLayout {
        property string icon: ""
        property string title: ""
        property int count: 0

        Layout.fillWidth: true

        Label {
            text: icon + " " + title
            font.pixelSize: 16
            font.bold: true
            color: textColor
            Layout.fillWidth: true
        }

        Label {
            text: count + " found"
            font.pixelSize: 12
            color: "#7f8c8d"
        }
    }

    // DeviceCard - selectable card for devices (amplifiers, cameras)
    component DeviceCard: Rectangle {
        property string icon: "⚡"
        property string deviceName: ""
        property string deviceInfo: ""
        property bool isSelected: false

        signal clicked()

        width: ListView.view ? ListView.view.width : parent.width
        height: 80
        color: isSelected ? "#e8f4f8" : cardColor
        radius: 6
        border.color: isSelected ? accentColor : borderColor
        border.width: isSelected ? 2 : 1

        MouseArea {
            anchors.fill: parent
            hoverEnabled: true
            onEntered: parent.color = isSelected ? "#e8f4f8" : hoverColor
            onExited: parent.color = isSelected ? "#e8f4f8" : cardColor
            onClicked: parent.clicked()
        }

        RowLayout {
            anchors.fill: parent
            anchors.margins: 15
            spacing: 15

            Rectangle {
                Layout.preferredWidth: 50
                Layout.preferredHeight: 50
                radius: 25
                color: isSelected ? accentColor : "#ecf0f1"

                Label {
                    anchors.centerIn: parent
                    text: icon
                    font.pixelSize: 24
                    color: isSelected ? "white" : textColor
                }
            }

            ColumnLayout {
                Layout.fillWidth: true
                spacing: 4

                Label {
                    text: deviceName
                    font.pixelSize: 14
                    font.bold: true
                    color: textColor
                }

                Label {
                    text: deviceInfo
                    font.pixelSize: 11
                    color: "#7f8c8d"
                }
            }

            Rectangle {
                visible: isSelected
                Layout.preferredWidth: 24
                Layout.preferredHeight: 24
                radius: 12
                color: successColor

                Label {
                    anchors.centerIn: parent
                    text: "✓"
                    font.pixelSize: 14
                    font.bold: true
                    color: "white"
                }
            }
        }
    }

    // SummaryCard - card for displaying summary information
    component SummaryCard: Rectangle {
        property string icon: ""
        property string title: ""
        property alias contentItem: contentLoader.sourceComponent

        Layout.fillWidth: true
        implicitHeight: cardContent.implicitHeight + 30
        color: cardColor
        radius: 8
        border.color: borderColor
        border.width: 1

        ColumnLayout {
            id: cardContent
            anchors.fill: parent
            anchors.margins: 15
            spacing: 8

            Label {
                text: icon + " " + title
                font.pixelSize: 14
                font.bold: true
                color: textColor
            }

            Rectangle {
                Layout.fillWidth: true
                height: 1
                color: borderColor
            }

            Loader {
                id: contentLoader
                Layout.fillWidth: true
            }
        }
    }

    // NavigationBar - bottom navigation with Back, Cancel, Next buttons
    component NavigationBar: RowLayout {
        property bool showBack: true
        property bool showRefresh: false
        property string refreshText: "🔄 Refresh"
        property bool refreshEnabled: true
        property string nextText: "Next →"
        property bool nextEnabled: true
        property color nextColor: accentColor
        property string middleText: ""

        signal backClicked()
        signal cancelClicked()
        signal nextClicked()
        signal refreshClicked()

        Layout.fillWidth: true
        spacing: 15

        Button {
            visible: showRefresh
            text: refreshText
            font.pixelSize: 13
            enabled: refreshEnabled
            Layout.preferredWidth: 140
            Layout.preferredHeight: 45
            palette.button: "#95a5a6"
            palette.buttonText: "white"
            onClicked: refreshClicked()
        }

        Button {
            visible: showBack && !showRefresh
            text: "← Back"
            font.pixelSize: 13
            Layout.preferredWidth: 120
            Layout.preferredHeight: 45
            onClicked: backClicked()
        }

        Item { Layout.fillWidth: true }

        Label {
            visible: middleText !== ""
            text: middleText
            font.pixelSize: 12
            color: "#7f8c8d"
        }

        Button {
            visible: showBack && showRefresh
            text: "← Back"
            font.pixelSize: 13
            Layout.preferredWidth: 120
            Layout.preferredHeight: 45
            onClicked: backClicked()
        }

        Button {
            text: "Cancel"
            font.pixelSize: 13
            Layout.preferredWidth: 100
            Layout.preferredHeight: 45
            onClicked: cancelClicked()
        }

        Button {
            text: nextText
            font.pixelSize: 13
            font.bold: true
            enabled: nextEnabled
            Layout.preferredWidth: nextText.length > 10 ? 180 : 120
            Layout.preferredHeight: 45
            palette.button: nextColor
            palette.buttonText: "white"
            onClicked: nextClicked()
        }
    }

    // ==================== END INLINE COMPONENTS ====================

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
            if(window.loading === Globals.Loading){
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

            // LOADING OVERLAY
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
                    refreshEnabled: !window.loading
                    nextEnabled: backend.selectedAmplifierIndex !== -1

                    onRefreshClicked: {
                        Globals.status = Globals.Loading
                        timer.restart()
                        backend.refreshAmplifiersList()
                    }
                    onCancelClicked: {
                        rejected()
                        window.close()
                    }
                    onNextClicked: stackView.push(channelSelectionPage)
                }
            }
        }
    }

    // STEP 2: CHANNEL SELECTION
    Component {
        id: channelSelectionPage

        Rectangle {
            color: bgColor

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

                        Rectangle {
                            Layout.fillWidth: true
                            Layout.preferredHeight: 50
                            color: "#f8f9fa"
                            radius: 8

                            RowLayout {
                                anchors.fill: parent
                                anchors.leftMargin: 20
                                anchors.rightMargin: 20
                                spacing: 0

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

                                CheckBox {
                                    text: "Select all"
                                    font.pixelSize: 11
                                    checked: false
                                    onClicked: selectAllChannels(checked)
                                }
                            }
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
                                model: backend.currentChannels
                                spacing: 0

                                delegate: Rectangle {
                                    width: ListView.view.width
                                    height: 45
                                    color: {
                                        if (channelSelectionModel[index]) return "#e8f4f8"
                                        return index % 2 === 0 ? "white" : "#f8f9fa"
                                    }

                                    MouseArea {
                                        anchors.fill: parent
                                        onClicked: toggleChannel(index)
                                    }

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

                                        CheckBox {
                                            checked: channelSelectionModel[index] || false
                                            onClicked: toggleChannel(index)
                                        }
                                    }
                                }
                            }
                        }
                    }
                }

                NavigationBar {
                    showBack: true
                    middleText: getSelectedChannelsCount() + " / " + backend.currentChannels.length + " selected"
                    nextEnabled: getSelectedChannelsCount() > 0

                    onBackClicked: stackView.pop()
                    onCancelClicked: {
                        rejected()
                        window.close()
                    }
                    onNextClicked: stackView.push(cameraSelectionPage)
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

                    onRefreshClicked: { /* TODO: refresh cameras */ }
                    onBackClicked: stackView.pop()
                    onCancelClicked: {
                        rejected()
                        window.close()
                    }
                    onNextClicked: stackView.push(summaryPage)
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
                        SummaryCard {
                            icon: "⚡"
                            title: "Amplifier"
                            contentItem: Label {
                                text: backend.availableAmplifiers[backend.selectedAmplifierIndex]
                                font.pixelSize: 13
                                color: "#7f8c8d"
                            }
                        }

                        // Channels
                        SummaryCard {
                            icon: "🔌"
                            title: "Selected Channels (" + getSelectedChannelsCount() + ")"
                            contentItem: Label {
                                text: getSelectedChannelsList().join(", ")
                                font.pixelSize: 12
                                color: "#7f8c8d"
                                wrapMode: Text.WordWrap
                            }
                        }

                        // Camera
                        SummaryCard {
                            icon: "📹"
                            title: "Camera"
                            contentItem: Label {
                                text: selectedCameraIndex >= 0 ? availableCameras[selectedCameraIndex] : "None (EEG only)"
                                font.pixelSize: 13
                                color: "#7f8c8d"
                            }
                        }

                        // Save location
                        SummaryCard {
                            icon: "💾"
                            title: "Recording Save Location"
                            contentItem: RowLayout {
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

                NavigationBar {
                    showBack: true
                    nextText: "✓ Start Examination"
                    nextColor: successColor

                    onBackClicked: stackView.pop()
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
