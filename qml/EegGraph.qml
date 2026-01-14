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
    }

    EegDataModel {
        id: eegData
    }

    Item {
        anchors.fill: parent

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

        // Channel labels - use Column with uniform distribution matching the graph's plot area
        Item {
            id: labelsContainer
            anchors.left: parent.left
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            anchors.topMargin: 16      // Same as GraphsView margins
            anchors.bottomMargin: 41   // 16 margin + ~25 for X-axis labels
            anchors.leftMargin: 20
            width: 140

            // Position each label using the actual available space
            Repeater {
                model: selectedChannels.length

                Rectangle {
                    width: 130
                    height: 18
                    radius: 3
                    color: "#ee1a2332"
                    border.color: "#2d3e50"
                    border.width: 1

                    // Calculate position: divide available height evenly among channels
                    // First channel at top, last at bottom
                    property real availableSpace: labelsContainer.height
                    property real stepSize: selectedChannels.length > 1
                        ? availableSpace / (selectedChannels.length - 1)
                        : 0

                    x: 0
                    y: (index * stepSize) - height / 2

                    Text {
                        anchors.fill: parent
                        anchors.leftMargin: 6
                        anchors.rightMargin: 6
                        verticalAlignment: Text.AlignVCenter
                        horizontalAlignment: Text.AlignLeft
                        text: channelNames[index] !== undefined ? channelNames[index] : ("Ch " + selectedChannels[index])
                        color: "#e8eef5"
                        font.pixelSize: 10
                        font.family: "monospace"
                        elide: Text.ElideRight
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
