import QtQuick
import QtQuick.Controls
import QtGraphs
import videoEeg

Rectangle {
    id: eegGraphContainer
    color: "white"

    property alias dataModel: eegData
    property int channelSpacing: 1
    property var selectedChannels: []

    onSelectedChannelsChanged: {
        if(selectedChannels.length > 0) {
            eegGraph.createAllSeries(selectedChannels)
        }
    }

    EegDataModel {
        id: eegData

        onWritePositionChanged: {
            // Wymuszamy przeliczenie pozycji linii
            cursorLine.updatePosition()
        }
    }

    ScrollView {
        anchors.fill: parent
        clip: true

        Item {
            width: eegGraphContainer.width
            height: Math.max(eegGraphContainer.height, selectedChannels.length * 150)

            GraphsView {
                id: eegGraph
                anchors.fill: parent
                anchors.margins: 16

                theme: GraphsTheme {
                    grid.mainColor: "darkgrey"
                    grid.subColor: "lightgrey"
                    labelTextColor: "black"
                    plotAreaBackgroundColor: "white"
                    backgroundColor: "white"
                    colorScheme: Qt.Light
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
                    min: -150
                    max: selectedChannels.length * 100 + 150
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

                    var count = channelList.length;
                    var colors = ["#e6194b", "#3cb44b", "#4363d8", "#f58231", "#911eb4",
                                  "#42d4f4", "#f032e6", "#bfef45", "#fabed4", "#469990"]

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

                onPlotAreaChanged: {
                    cursorLine.updatePosition()
                }
            }

            // Prosta czarna linia znacznika pozycji głowicy
            Rectangle {
                id: cursorLine
                width: 2
                color: "black"
                opacity: 0.9
                z: 1000

                visible: eegGraph.plotArea && eegGraph.plotArea.width > 0

                y: eegGraph.plotArea ? eegGraph.plotArea.y : 0
                height: eegGraph.plotArea ? eegGraph.plotArea.height : 0

                property real cachedX: 0
                x: cachedX

                function updatePosition() {
                    if (!eegGraph.plotArea || eegGraph.plotArea.width <= 0) {
                        return
                    }

                    var plotArea = eegGraph.plotArea
                    var headX = eegData.writePosition  // Indeks ostatniego zapisanego sample'a (0-999)

                    // Oś X: min=0, max=1000
                    // Sample o indeksie headX ma współrzędną X = headX na wykresie
                    // Musimy to przeliczyć na piksele

                    var xMin = xAxis.min  // 0
                    var xMax = xAxis.max  // 1000
                    var xRange = xMax - xMin  // 1000

                    // Normalizuj pozycję do zakresu 0-1
                    var normalizedX = (headX - xMin) / xRange

                    // Przelicz na piksele
                    var pixelX = plotArea.x + normalizedX * plotArea.width

                    console.log("Cursor: headX=" + headX + ", normalized=" + normalizedX.toFixed(3) + ", pixelX=" + pixelX.toFixed(0))

                    cachedX = pixelX
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
