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
    title: qsTr("⚡ Konfiguracja badania EEG")
    modality: Qt.ApplicationModal

    signal accepted(amplifierId: string, channels: var)
    signal rejected()

    property int loading: Globals.status
    property var channelSelectionModel: []
    property int selectedCameraIndex: -1
    property string savePath: ""

    // Mock kamery (zastąp prawdziwym backendem)
    property var availableCameras: ["Kamera wbudowana", "USB Camera HD", "Logitech C920"]

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
            case 1: return "Krok 1: Wybór wzmacniacza"
            case 2: return "Krok 2: Wybór kanałów"
            case 3: return "Krok 3: Wybór kamery"
            case 4: return "Krok 4: Podsumowanie"
            default: return ""
        }
    }

    function getCurrentStepDesc() {
        var step = getCurrentStepNumber()
        switch(step) {
            case 1: return "Wykryj i wybierz wzmacniacz EEG"
            case 2: return "Zaznacz kanały do rejestracji"
            case 3: return "Wybierz kamerę do nagrywania wideo"
            case 4: return "Sprawdź konfigurację przed rozpoczęciem"
            default: return ""
        }
    }

    Rectangle {
        anchors.fill: parent
        color: bgColor

        ColumnLayout {
            anchors.fill: parent
            spacing: 0

            // HEADER Z PROGRESS
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

                    // Progress indicator (4 kroki)
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
                        text: "Skanowanie urządzeń..."
                        font.pixelSize: 14
                        color: "white"
                        Layout.alignment: Qt.AlignHCenter
                    }
                }
            }
        }
    }

    // KROK 1: WYBÓR WZMACNIACZA
    Component {
        id: amplifierSelectionPage

        Rectangle {
            color: bgColor

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 30
                spacing: 20

                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 60
                    color: "#e8f4f8"
                    radius: 8
                    border.color: "#bee5eb"
                    border.width: 1

                    RowLayout {
                        anchors.fill: parent
                        anchors.margins: 15
                        spacing: 12

                        Label {
                            text: "ℹ️"
                            font.pixelSize: 24
                        }

                        Label {
                            text: "Upewnij się, że wzmacniacz jest włączony i podłączony do komputera"
                            font.pixelSize: 12
                            color: "#0c5460"
                            Layout.fillWidth: true
                            wrapMode: Text.WordWrap
                        }
                    }
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

                        RowLayout {
                            Layout.fillWidth: true

                            Label {
                                text: "🔌 Wykryte wzmacniacze"
                                font.pixelSize: 16
                                font.bold: true
                                color: textColor
                                Layout.fillWidth: true
                            }

                            Label {
                                text: backend.availableAmplifiers.length + " znaleziono"
                                font.pixelSize: 12
                                color: "#7f8c8d"
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
                                id: amplifierListView
                                model: backend.availableAmplifiers
                                spacing: 10

                                delegate: Rectangle {
                                    width: ListView.view.width
                                    height: 80
                                    color: backend.selectedAmplifierIndex === index ? "#e8f4f8" : cardColor
                                    radius: 6
                                    border.color: backend.selectedAmplifierIndex === index ? accentColor : borderColor
                                    border.width: backend.selectedAmplifierIndex === index ? 2 : 1

                                    MouseArea {
                                        anchors.fill: parent
                                        hoverEnabled: true
                                        onEntered: parent.color = backend.selectedAmplifierIndex === index ? "#e8f4f8" : hoverColor
                                        onExited: parent.color = backend.selectedAmplifierIndex === index ? "#e8f4f8" : cardColor
                                        onClicked: {
                                            backend.selectedAmplifierIndex = index
                                        }
                                    }

                                    RowLayout {
                                        anchors.fill: parent
                                        anchors.margins: 15
                                        spacing: 15

                                        Rectangle {
                                            Layout.preferredWidth: 50
                                            Layout.preferredHeight: 50
                                            radius: 25
                                            color: backend.selectedAmplifierIndex === index ? accentColor : "#ecf0f1"

                                            Label {
                                                anchors.centerIn: parent
                                                text: "⚡"
                                                font.pixelSize: 24
                                                color: backend.selectedAmplifierIndex === index ? "white" : textColor
                                            }
                                        }

                                        ColumnLayout {
                                            Layout.fillWidth: true
                                            spacing: 4

                                            Label {
                                                text: modelData
                                                font.pixelSize: 14
                                                font.bold: true
                                                color: textColor
                                            }

                                            Label {
                                                text: "Virtual Amplifier • Ready"
                                                font.pixelSize: 11
                                                color: "#7f8c8d"
                                            }
                                        }

                                        Rectangle {
                                            visible: backend.selectedAmplifierIndex === index
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

                                Label {
                                    anchors.centerIn: parent
                                    visible: amplifierListView.count === 0
                                    text: "😔 Nie znaleziono wzmacniaczy\nKliknij 'Odśwież' aby skanować ponownie"
                                    font.pixelSize: 13
                                    color: "#999999"
                                    horizontalAlignment: Text.AlignHCenter
                                }
                            }
                        }
                    }
                }

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 15

                    Button {
                        text: "🔄 Odśwież"
                        font.pixelSize: 13
                        enabled: !window.loading
                        Layout.preferredWidth: 120
                        Layout.preferredHeight: 45
                        palette.button: "#95a5a6"
                        palette.buttonText: "white"

                        onClicked: {
                            Globals.status = Globals.Loading
                            timer.restart()
                            backend.refreshAmplifiersList()
                        }
                    }

                    Item { Layout.fillWidth: true }

                    Button {
                        text: "Anuluj"
                        font.pixelSize: 13
                        Layout.preferredWidth: 100
                        Layout.preferredHeight: 45

                        onClicked: {
                            rejected()
                            window.close()
                        }
                    }

                    Button {
                        text: "Dalej →"
                        font.pixelSize: 13
                        font.bold: true
                        enabled: backend.selectedAmplifierIndex !== -1
                        Layout.preferredWidth: 120
                        Layout.preferredHeight: 45
                        palette.button: accentColor
                        palette.buttonText: "white"

                        onClicked: stackView.push(channelSelectionPage)
                    }
                }
            }
        }
    }

    // KROK 2: WYBÓR KANAŁÓW
    Component {
        id: channelSelectionPage

        Rectangle {
            color: bgColor

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 30
                spacing: 20

                Rectangle {
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
                                text: "Wybrany wzmacniacz: " + backend.availableAmplifiers[backend.selectedAmplifierIndex]
                                font.pixelSize: 12
                                font.bold: true
                                color: "#155724"
                            }

                            Label {
                                text: backend.currentChannels.length + " dostępnych kanałów"
                                font.pixelSize: 11
                                color: "#155724"
                            }
                        }
                    }
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
                                    text: "Nr"
                                    font.pixelSize: 12
                                    font.bold: true
                                    color: textColor
                                    Layout.preferredWidth: 60
                                }

                                Label {
                                    text: "Nazwa kanału"
                                    font.pixelSize: 12
                                    font.bold: true
                                    color: textColor
                                    Layout.fillWidth: true
                                }

                                CheckBox {
                                    text: "Zaznacz wszystkie"
                                    font.pixelSize: 11
                                    checked: false
                                    onClicked: {
                                        selectAllChannels(checked)
                                    }
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
                                        onClicked: {
                                            toggleChannel(index)
                                        }
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
                                            onClicked: {
                                                toggleChannel(index)
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 15

                    Button {
                        text: "← Wstecz"
                        font.pixelSize: 13
                        Layout.preferredWidth: 120
                        Layout.preferredHeight: 45
                        onClicked: stackView.pop()
                    }

                    Item { Layout.fillWidth: true }

                    Label {
                        text: getSelectedChannelsCount() + " / " + backend.currentChannels.length + " zaznaczonych"
                        font.pixelSize: 12
                        color: "#7f8c8d"
                    }

                    Button {
                        text: "Anuluj"
                        font.pixelSize: 13
                        Layout.preferredWidth: 100
                        Layout.preferredHeight: 45
                        onClicked: {
                            rejected()
                            window.close()
                        }
                    }

                    Button {
                        text: "Dalej →"
                        font.pixelSize: 13
                        font.bold: true
                        enabled: getSelectedChannelsCount() > 0
                        Layout.preferredWidth: 120
                        Layout.preferredHeight: 45
                        palette.button: accentColor
                        palette.buttonText: "white"
                        onClicked: stackView.push(cameraSelectionPage)
                    }
                }
            }
        }
    }

    // KROK 3: WYBÓR KAMERY
    Component {
        id: cameraSelectionPage

        Rectangle {
            color: bgColor

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 30
                spacing: 20

                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 60
                    color: "#e8f4f8"
                    radius: 8
                    border.color: "#bee5eb"
                    border.width: 1

                    RowLayout {
                        anchors.fill: parent
                        anchors.margins: 15
                        spacing: 12

                        Label {
                            text: "📹"
                            font.pixelSize: 24
                        }

                        Label {
                            text: "Wybierz kamerę do synchronizacji wideo z danymi EEG (opcjonalne)"
                            font.pixelSize: 12
                            color: "#0c5460"
                            Layout.fillWidth: true
                            wrapMode: Text.WordWrap
                        }
                    }
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

                        RowLayout {
                            Layout.fillWidth: true

                            Label {
                                text: "📷 Dostępne kamery"
                                font.pixelSize: 16
                                font.bold: true
                                color: textColor
                                Layout.fillWidth: true
                            }

                            Label {
                                text: availableCameras.length + " znaleziono"
                                font.pixelSize: 12
                                color: "#7f8c8d"
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
                                model: availableCameras
                                spacing: 10

                                delegate: Rectangle {
                                    width: ListView.view.width
                                    height: 80
                                    color: selectedCameraIndex === index ? "#e8f4f8" : cardColor
                                    radius: 6
                                    border.color: selectedCameraIndex === index ? accentColor : borderColor
                                    border.width: selectedCameraIndex === index ? 2 : 1

                                    MouseArea {
                                        anchors.fill: parent
                                        hoverEnabled: true
                                        onEntered: parent.color = selectedCameraIndex === index ? "#e8f4f8" : hoverColor
                                        onExited: parent.color = selectedCameraIndex === index ? "#e8f4f8" : cardColor
                                        onClicked: {
                                            selectedCameraIndex = index
                                        }
                                    }

                                    RowLayout {
                                        anchors.fill: parent
                                        anchors.margins: 15
                                        spacing: 15

                                        Rectangle {
                                            Layout.preferredWidth: 50
                                            Layout.preferredHeight: 50
                                            radius: 25
                                            color: selectedCameraIndex === index ? accentColor : "#ecf0f1"

                                            Label {
                                                anchors.centerIn: parent
                                                text: "📹"
                                                font.pixelSize: 24
                                            }
                                        }

                                        ColumnLayout {
                                            Layout.fillWidth: true
                                            spacing: 4

                                            Label {
                                                text: modelData
                                                font.pixelSize: 14
                                                font.bold: true
                                                color: textColor
                                            }

                                            Label {
                                                text: "1920x1080 • 30 FPS"
                                                font.pixelSize: 11
                                                color: "#7f8c8d"
                                            }
                                        }

                                        Rectangle {
                                            visible: selectedCameraIndex === index
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
                            }
                        }

                        // Opcja: bez kamery
                        Rectangle {
                            Layout.fillWidth: true
                            height: 60
                            color: selectedCameraIndex === -1 ? "#fff3cd" : cardColor
                            radius: 6
                            border.color: selectedCameraIndex === -1 ? "#ffc107" : borderColor
                            border.width: selectedCameraIndex === -1 ? 2 : 1

                            MouseArea {
                                anchors.fill: parent
                                onClicked: {
                                    selectedCameraIndex = -1
                                }
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
                                    text: "Kontynuuj bez kamery (tylko dane EEG)"
                                    font.pixelSize: 12
                                    font.bold: selectedCameraIndex === -1
                                    color: "#856404"
                                    Layout.fillWidth: true
                                }
                            }
                        }
                    }
                }

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 15

                    Button {
                        text: "🔄 Odśwież kamery"
                        font.pixelSize: 13
                        Layout.preferredWidth: 140
                        Layout.preferredHeight: 45
                        palette.button: "#95a5a6"
                        palette.buttonText: "white"
                    }

                    Item { Layout.fillWidth: true }

                    Button {
                        text: "← Wstecz"
                        font.pixelSize: 13
                        Layout.preferredWidth: 120
                        Layout.preferredHeight: 45
                        onClicked: stackView.pop()
                    }

                    Button {
                        text: "Anuluj"
                        font.pixelSize: 13
                        Layout.preferredWidth: 100
                        Layout.preferredHeight: 45
                        onClicked: {
                            rejected()
                            window.close()
                        }
                    }

                    Button {
                        text: "Dalej →"
                        font.pixelSize: 13
                        font.bold: true
                        Layout.preferredWidth: 120
                        Layout.preferredHeight: 45
                        palette.button: accentColor
                        palette.buttonText: "white"
                        onClicked: stackView.push(summaryPage)
                    }
                }
            }
        }
    }

    // KROK 4: PODSUMOWANIE
    Component {
        id: summaryPage

        Rectangle {
            color: bgColor

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 30
                spacing: 20

                Label {
                    text: "✅ Podsumowanie konfiguracji"
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

                        // Wzmacniacz
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
                                    text: "⚡ Wzmacniacz"
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

                        // Kanały
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
                                    text: "🔌 Wybrane kanały (" + getSelectedChannelsCount() + ")"
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

                        // Kamera
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
                                    text: "📹 Kamera"
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
                                    text: selectedCameraIndex >= 0 ? availableCameras[selectedCameraIndex] : "Brak (tylko EEG)"
                                    font.pixelSize: 13
                                    color: "#7f8c8d"
                                }
                            }
                        }

                        // Miejsce zapisu
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
                                    text: "💾 Miejsce zapisu nagrań"
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
                                        text: savePath || "Wybierz lokalizację..."
                                        readOnly: true
                                        font.pixelSize: 11
                                    }

                                    Button {
                                        text: "📁 Wybierz"
                                        font.pixelSize: 11
                                        Layout.preferredHeight: 35
                                        onClicked: folderDialog.open()
                                    }
                                }
                            }
                        }
                    }
                }

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 15

                    Button {
                        text: "← Wstecz"
                        font.pixelSize: 13
                        Layout.preferredWidth: 120
                        Layout.preferredHeight: 45
                        onClicked: stackView.pop()
                    }

                    Item { Layout.fillWidth: true }

                    Button {
                        text: "Anuluj"
                        font.pixelSize: 13
                        Layout.preferredWidth: 100
                        Layout.preferredHeight: 45
                        onClicked: {
                            rejected()
                            window.close()
                        }
                    }

                    Button {
                        text: "✓ Rozpocznij badanie"
                        font.pixelSize: 13
                        font.bold: true
                        enabled: true // Zawsze aktywny - savePath opcjonalny
                        Layout.preferredWidth: 180
                        Layout.preferredHeight: 45
                        palette.button: successColor
                        palette.buttonText: "white"

                        onClicked: {
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
}
