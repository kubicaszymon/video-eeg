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
    onSelectedChannelsChanged:
    {
        if(selectedChannels.length > 0)
        {
            eegGraph.createAllSeries(selectedChannels)
        }
    }

    EegDataModel {
        id: eegData
    }

    ScrollView {
        anchors.fill: parent
        clip: true

        GraphsView {
            id: eegGraph
            width: parent.width
            height: Math.max(parent.height, selectedChannels.length * channelSpacing + 10)
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
                max: 10
                tickInterval: 2
                subTickCount: 9
                labelDecimals: 1
            }
            axisY: ValueAxis {
                min: 0
                max: selectedChannels.length * channelSpacing
                //labelsVisible: false
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

                for (var j = 0; j < count; j++){
                    var series = seriesComponent.createObject(eegGraph, {
                                                                  "name": names[j],

                                                              })

                    var mapper = mapperComponent.createObject(series, {
                                                                  "model": eegData,
                                                                  "series": series,
                                                                  "xSection": 0,
                                                                  "ySection": j+1
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
            width: 2
            color: "red"
        }
    }

    Component {
        id: mapperComponent
        XYModelMapper {}
    }
}
