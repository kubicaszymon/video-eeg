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
                    max: 1000
                    tickInterval: 100
                    subTickCount: 9
                    labelDecimals: 0
                    gridVisible: true
                }

                axisY: ValueAxis {
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
                    var names = channelList;
                    var colors = ["red", "blue", "green", "orange", "purple", "brown", "pink", "cyan"]

                    for (var j = 0; j < count; j++){
                        var series = seriesComponent.createObject(eegGraph, {
                                                                      "name": names[j],
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

            Rectangle {
                id: cursorLine
                width: 3
                height: eegGraph.height
                color: "red"
                opacity: 0.7
                z: 1000

                property real linePosition: eegData.writePosition

                visible: eegGraph.plotArea !== null

                x: {
                    if (!eegGraph.plotArea) return 0

                    var plotArea = eegGraph.plotArea
                    var samplePosition = linePosition // Użyj property zamiast bezpośrednio

                    // Linia powinna być MIĘDZY ostatnim zapisanym a następnym
                    var progress = samplePosition / 1000.0

                    return plotArea.x + progress * plotArea.width
                }

                y: eegGraph.plotArea ? eegGraph.plotArea.y : 0

                onLinePositionChanged: {
                    console.log("Line moving to sample:", linePosition)
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
