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
                max: 1000
                tickInterval: 100
                subTickCount: 9
                labelDecimals: 0
                gridVisible: true
            }

            axisY: ValueAxis {
                id: yAxis
                min: -50
                max: 1050
                labelsVisible: true
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
                        "name": "Ch " + channelList[j],
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
