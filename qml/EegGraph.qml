import QtQuick
import QtQuick.Controls
import QtGraphs
import videoEeg

Rectangle {
    id: eegGraphContainer
    color: "#0d0f12"

    property alias dataModel: eegData
    property int channelSpacing: 1
    property var selectedChannels: []
    property var channelNames: []  // Names of channels for Y-axis labels

    // Time window in seconds (X-axis range)
    property real timeWindowSeconds: 10.0

    // Width reserved for channel labels on left side
    readonly property int labelColumnWidth: 70

    // Dynamically calculated spacing based on available height
    readonly property real availableHeight: height - 100
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

        console.log("Updated Y axis for " + numChannels + " channels, spacing: " + spacing)
    }

    EegDataModel {
        id: eegData
    }

    Row {
        anchors.fill: parent
        spacing: 0

        // Left column: Channel labels
        Item {
            id: labelColumn
            width: labelColumnWidth
            height: parent.height

            // Channel name labels - positioned to align with each channel's Y position
            Repeater {
                model: selectedChannels.length

                Item {
                    id: labelItem
                    width: labelColumnWidth
                    height: 20

                    // Calculate Y position to match channel position on the graph
                    // Channel 0 is at top, channel N-1 is at bottom
                    // Y value for channel = (numChannels - 1 - index) * spacing
                    property real channelYValue: (selectedChannels.length - 1 - index) * dynamicChannelSpacing

                    // Graph plot area boundaries (accounting for margins and X-axis labels)
                    property real graphMargin: 16  // matches eegGraph anchors.margins
                    property real xAxisLabelHeight: 30  // approximate space for X-axis labels
                    property real graphTop: graphMargin
                    property real graphBottom: eegGraphContainer.height - graphMargin - xAxisLabelHeight
                    property real graphHeight: graphBottom - graphTop

                    property real yRange: yAxis.max - yAxis.min
                    property real normalizedY: yRange > 0 ? (channelYValue - yAxis.min) / yRange : 0.5
                    // Invert because screen Y increases downward, but axis Y increases upward
                    property real pixelY: graphTop + graphHeight * (1 - normalizedY) - height / 2

                    y: pixelY
                    x: 0

                    Text {
                        anchors.right: parent.right
                        anchors.rightMargin: 8
                        anchors.verticalCenter: parent.verticalCenter
                        text: channelNames[index] !== undefined ? channelNames[index] : ("Ch " + selectedChannels[index])
                        color: "#8a9cb5"
                        font.pixelSize: 11
                        font.bold: false
                        horizontalAlignment: Text.AlignRight
                    }
                }
            }
        }

        // Right column: EEG Graph
        Item {
            width: parent.width - labelColumnWidth
            height: parent.height

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
                    max: timeWindowSeconds  // X-axis in seconds (e.g., 0-10 for 10 second window)
                    tickInterval: 1         // Major tick every 1 second
                    subTickCount: 9         // Minor ticks for 0.1 second intervals
                    labelDecimals: 0        // Show whole seconds
                    gridVisible: true
                }

                axisY: ValueAxis {
                    id: yAxis
                    min: -50
                    max: 1050
                    labelsVisible: false    // Hide numeric labels, we use channel names instead
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
                    var colors = ["#e6194b", "#3cb44b", "#4363d8", "#f58231", "#911eb4",
                                  "#42d4f4", "#f032e6", "#bfef45", "#fabed4", "#469990",
                                  "#dcbeff", "#9A6324", "#fffac8", "#800000", "#aaffc3",
                                  "#808000", "#ffd8b1", "#000075", "#a9a9a9", "#ffffff"]

                    for (var j = 0; j < count; j++){
                        var series = seriesComponent.createObject(eegGraph, {
                            "name": channelNames[j] !== undefined ? channelNames[j] : ("Ch " + channelList[j]),
                            "color": colors[j % colors.length]
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
