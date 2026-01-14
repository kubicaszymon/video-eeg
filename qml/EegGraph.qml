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
    readonly property int labelColumnWidth: 140

    // Graph margins and plot area calculations
    readonly property real graphMargins: 16
    readonly property real xAxisHeight: 25  // Space for X-axis labels at bottom

    // Available height for the plot area (excluding margins and X-axis)
    readonly property real plotAreaHeight: height - (2 * graphMargins) - xAxisHeight
    readonly property real plotAreaTop: graphMargins

    // Dynamically calculated spacing based on plot area height
    readonly property real calculatedSpacing: selectedChannels.length > 1
        ? plotAreaHeight / (selectedChannels.length - 1)
        : plotAreaHeight

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
        var margin = spacing * 0.4

        yAxis.min = -margin
        yAxis.max = (numChannels - 1) * spacing + margin
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

                Rectangle {
                    id: labelItem
                    width: labelColumnWidth - 4
                    height: 22
                    radius: 3
                    color: "#1a2332"
                    border.color: "#2d3e50"
                    border.width: 1

                    // Simple positioning: channel 0 at top, each subsequent channel below
                    // Using the same spacing calculation as the graph
                    y: plotAreaTop + (index * dynamicChannelSpacing) - height / 2
                    x: 2

                    Text {
                        anchors.fill: parent
                        anchors.leftMargin: 6
                        anchors.rightMargin: 6
                        verticalAlignment: Text.AlignVCenter
                        horizontalAlignment: Text.AlignRight
                        text: channelNames[index] !== undefined ? channelNames[index] : ("Ch " + selectedChannels[index])
                        color: "#c8d4e3"
                        font.pixelSize: 10
                        font.family: "monospace"
                        elide: Text.ElideMiddle
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
