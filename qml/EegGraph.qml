import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtGraphs
import videoEeg

Rectangle {
    id: eegGraphContainer
    color: "#0d0f12"

    property alias dataModel: eegData
    property int channelSpacing: 1
    property var selectedChannels: []
    property var channelNames: []

    // Time window in seconds (X-axis range)
    property real timeWindowSeconds: 10.0

    // Marker manager reference (set from parent)
    property var markerManager: null

    // Display scaler reference (for calibration bar)
    property var scaler: null

    // Calibration bar value in μV (configurable)
    property real calibrationValue: 50.0

    // Channel colors - shared between legend and graph
    readonly property var channelColors: [
        "#e6194b", "#3cb44b", "#4363d8", "#f58231", "#911eb4",
        "#42d4f4", "#f032e6", "#bfef45", "#fabed4", "#469990",
        "#dcbeff", "#9A6324", "#fffac8", "#800000", "#aaffc3",
        "#808000", "#ffd8b1", "#000075", "#a9a9a9", "#ffffff"
    ]

    // Dynamically calculated spacing based on available height
    readonly property real availableHeight: graphArea.height - 60
    readonly property real calculatedSpacing: selectedChannels.length > 1
        ? availableHeight / (selectedChannels.length - 1)
        : availableHeight

    property real dynamicChannelSpacing: Math.max(calculatedSpacing, 20)

    onSelectedChannelsChanged: {
        if(selectedChannels.length > 0) {
            updateAxisY()
            eegGraph.createAllSeries(selectedChannels)
        }
    }

    onHeightChanged: {
        if(selectedChannels.length > 0) {
            updateAxisY()
        }
    }

    function updateAxisY() {
        if (selectedChannels.length === 0) return

        var numChannels = selectedChannels.length
        var spacing = dynamicChannelSpacing
        var margin = spacing * 0.5

        yAxis.min = -margin
        yAxis.max = (numChannels - 1) * spacing + margin
    }

    function getChannelName(index) {
        if (channelNames[index] !== undefined) {
            return channelNames[index]
        }
        return "Ch " + selectedChannels[index]
    }

    function getChannelColor(index) {
        return channelColors[index % channelColors.length]
    }

    EegDataModel {
        id: eegData
    }

    RowLayout {
        anchors.fill: parent
        spacing: 0

        // Left panel: Channel legend
        Rectangle {
            Layout.preferredWidth: 160
            Layout.fillHeight: true
            color: "#141a24"
            border.color: "#2d3e50"
            border.width: 1

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 8
                spacing: 4

                // Header
                Label {
                    text: "Channels"
                    font.pixelSize: 11
                    font.bold: true
                    color: "#8a9cb5"
                    Layout.fillWidth: true
                }

                Rectangle {
                    Layout.fillWidth: true
                    height: 1
                    color: "#2d3e50"
                }

                // Scrollable channel list
                ScrollView {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    clip: true

                    ListView {
                        id: channelListView
                        model: selectedChannels.length
                        spacing: 2

                        delegate: Rectangle {
                            width: channelListView.width - 4
                            height: 24
                            radius: 4
                            color: "#1a2332"
                            border.color: "#2d3e50"
                            border.width: 1

                            RowLayout {
                                anchors.fill: parent
                                anchors.leftMargin: 6
                                anchors.rightMargin: 6
                                spacing: 8

                                // Color indicator
                                Rectangle {
                                    Layout.preferredWidth: 14
                                    Layout.preferredHeight: 14
                                    radius: 3
                                    color: getChannelColor(index)
                                    border.color: Qt.lighter(getChannelColor(index), 1.3)
                                    border.width: 1
                                }

                                // Channel name
                                Label {
                                    Layout.fillWidth: true
                                    text: getChannelName(index)
                                    color: "#e8eef5"
                                    font.pixelSize: 10
                                    font.family: "monospace"
                                    elide: Text.ElideRight
                                }
                            }
                        }
                    }
                }

                // Channel count footer
                Label {
                    text: selectedChannels.length + " channels"
                    font.pixelSize: 9
                    color: "#5a6a7a"
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignRight
                }
            }
        }

        // Right side: Graph
        Item {
            id: graphArea
            Layout.fillWidth: true
            Layout.fillHeight: true

            GraphsView {
                id: eegGraph
                anchors.fill: parent
                anchors.margins: 16

                theme: GraphsTheme {
                    grid.mainColor: "#2d3e50"
                    grid.subColor: "#1a2332"
                    labelTextColor: "#8a9cb5"
                    plotAreaBackgroundColor: "#0d0f12"
                    backgroundColor: "#0d0f12"
                    colorScheme: Qt.Dark
                }

                axisX: ValueAxis {
                    id: xAxis
                    min: 0
                    max: timeWindowSeconds
                    tickInterval: 1
                    subTickCount: 9
                    labelDecimals: 0
                    gridVisible: true
                }

                axisY: ValueAxis {
                    id: yAxis
                    min: -50
                    max: 1050
                    labelsVisible: false
                    gridVisible: true
                }

                property var activeSeries: []

                function createAllSeries(channelList) {
                    for (var i = 0; i < activeSeries.length; i++){
                        eegGraph.removeSeries(activeSeries[i])
                        activeSeries[i].destroy()
                    }
                    activeSeries = []

                    var count = channelList.length

                    for (var j = 0; j < count; j++){
                        var series = seriesComponent.createObject(eegGraph, {
                            "name": getChannelName(j),
                            "color": getChannelColor(j)
                        })

                        var mapper = mapperComponent.createObject(series, {
                            "model": eegData,
                            "series": series,
                            "xSection": 0,
                            "ySection": j + 1
                        })

                        eegGraph.addSeries(series)
                        activeSeries.push(series)
                    }
                }
            }

            // Marker overlay - displays markers as vertical lines with labels
            Item {
                id: markerOverlay
                anchors.fill: eegGraph
                anchors.margins: 16

                // Plot area dimensions (accounting for axis labels)
                readonly property real plotLeftMargin: 0
                readonly property real plotTopMargin: 10
                readonly property real plotRightMargin: 10
                readonly property real plotBottomMargin: 30
                readonly property real plotWidth: width - plotLeftMargin - plotRightMargin
                readonly property real plotHeight: height - plotTopMargin - plotBottomMargin

                Repeater {
                    model: markerManager ? markerManager.markers : []

                    Item {
                        id: markerItem

                        // Calculate X position based on xPosition (0 to timeWindowSeconds)
                        readonly property real xPos: markerOverlay.plotLeftMargin +
                            (modelData.xPosition / timeWindowSeconds) * markerOverlay.plotWidth

                        x: xPos
                        y: markerOverlay.plotTopMargin
                        width: 2
                        height: markerOverlay.plotHeight

                        // Vertical line
                        Rectangle {
                            id: markerLine
                            width: 2
                            height: parent.height
                            color: modelData.color
                            opacity: 0.85
                        }

                        // Label background
                        Rectangle {
                            id: labelBackground
                            anchors.bottom: markerLine.top
                            anchors.bottomMargin: 2
                            anchors.horizontalCenter: markerLine.horizontalCenter
                            width: markerLabel.width + 8
                            height: markerLabel.height + 4
                            radius: 3
                            color: modelData.color
                            opacity: 0.95

                            Label {
                                id: markerLabel
                                anchors.centerIn: parent
                                text: modelData.label
                                font.pixelSize: 9
                                font.bold: true
                                color: "white"
                            }
                        }
                    }
                }
            }

            // Calibration bar - professional "step" calibrator at the start of the graph
            // Shows exact voltage scale based on current sensitivity setting
            Item {
                id: calibrationBar
                anchors.left: eegGraph.left
                anchors.top: eegGraph.top
                anchors.bottom: eegGraph.bottom
                anchors.margins: 16
                width: 60
                visible: scaler !== null && selectedChannels.length > 0

                // Calculate calibration step height in pixels
                // Height = calibrationValue[μV] × displayGain[px/μV]
                readonly property real stepHeight: scaler ? calibrationValue * scaler.displayGain : 50

                // Draw calibration step using Canvas for precise rendering
                Canvas {
                    id: calibrationCanvas
                    anchors.fill: parent
                    anchors.topMargin: 10
                    anchors.bottomMargin: 30

                    onPaint: {
                        var ctx = getContext("2d")
                        ctx.reset()

                        var stepH = calibrationBar.stepHeight
                        var centerY = height / 2

                        // Step waveform: baseline -> up -> plateau -> down -> baseline
                        var lineWidth = 2
                        var plateauWidth = 20
                        var baselineWidth = 10

                        ctx.strokeStyle = "#4a90e2"
                        ctx.lineWidth = lineWidth
                        ctx.lineCap = "square"
                        ctx.lineJoin = "miter"

                        ctx.beginPath()

                        // Starting baseline (left)
                        ctx.moveTo(5, centerY)
                        ctx.lineTo(5 + baselineWidth, centerY)

                        // Step up (positive μV goes UP on screen)
                        ctx.lineTo(5 + baselineWidth, centerY - stepH)

                        // Plateau at top
                        ctx.lineTo(5 + baselineWidth + plateauWidth, centerY - stepH)

                        // Step down
                        ctx.lineTo(5 + baselineWidth + plateauWidth, centerY)

                        // Ending baseline (right)
                        ctx.lineTo(5 + baselineWidth + plateauWidth + baselineWidth, centerY)

                        ctx.stroke()

                        // Draw height indicator line (vertical with caps)
                        var indicatorX = 5 + baselineWidth + plateauWidth + baselineWidth + 8
                        ctx.strokeStyle = "#8a9cb5"
                        ctx.lineWidth = 1

                        ctx.beginPath()
                        // Vertical line
                        ctx.moveTo(indicatorX, centerY)
                        ctx.lineTo(indicatorX, centerY - stepH)
                        // Top cap
                        ctx.moveTo(indicatorX - 3, centerY - stepH)
                        ctx.lineTo(indicatorX + 3, centerY - stepH)
                        // Bottom cap
                        ctx.moveTo(indicatorX - 3, centerY)
                        ctx.lineTo(indicatorX + 3, centerY)
                        ctx.stroke()
                    }

                    // Repaint when displayGain changes
                    Connections {
                        target: scaler
                        function onDisplayGainChanged() {
                            calibrationCanvas.requestPaint()
                        }
                    }

                    Component.onCompleted: requestPaint()
                }

                // Calibration value label
                Rectangle {
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.bottom: calibrationCanvas.bottom
                    anchors.bottomMargin: -25
                    width: calibLabel.width + 10
                    height: calibLabel.height + 6
                    radius: 3
                    color: "#1a2332"
                    border.color: "#2d3e50"
                    border.width: 1

                    Label {
                        id: calibLabel
                        anchors.centerIn: parent
                        text: calibrationValue.toFixed(0) + " μV"
                        font.pixelSize: 10
                        font.bold: true
                        color: "#4a90e2"
                    }
                }
            }
        }
    }

    Component {
        id: seriesComponent
        LineSeries {
            width: 1
        }
    }

    Component {
        id: mapperComponent
        XYModelMapper {}
    }
}
