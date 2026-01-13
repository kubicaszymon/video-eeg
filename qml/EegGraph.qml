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
            updateAxisY()
        }
    }

    // Funkcja do aktualizacji osi Y na podstawie liczby kanałów
    function updateAxisY() {
        if (selectedChannels.length === 0) return

        var numChannels = selectedChannels.length
        // CHANNEL_SPACING z EegBackend.cpp = 100.0
        var spacing = 100.0

        // Najwyższy kanał (ch=0): offset = (numChannels - 1) * spacing
        // Najniższy kanał (ch=numChannels-1): offset = 0
        // Dodaj margines dla amplitudy sygnału
        var margin = 150

        yAxis.min = -margin
        yAxis.max = (numChannels - 1) * spacing + margin

        console.log("Updated Y axis for " + numChannels + " channels: min=" + yAxis.min + ", max=" + yAxis.max)
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
                id: yAxis
                min: -150
                max: 1550  // Domyślnie dla ~15 kanałów
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
                var colors = ["red", "blue", "green", "orange", "purple", "brown", "pink", "cyan",
                              "magenta", "lime", "navy", "teal", "maroon", "olive"]

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

        // Czerwona linia - NA WIERZCHU wykresów!
        Rectangle {
            id: cursorLine
            width: 3
            color: "red"
            opacity: 0.8
            // KLUCZOWE: z większe niż GraphsView, żeby była na wierzchu
            z: 100

            visible: eegGraph.plotArea !== null

            // Pozycja X
            x: {
                if (!eegGraph.plotArea) return 0

                var plotArea = eegGraph.plotArea
                var writePos = eegData.writePosition

                // writePosition już zawiera GAP (20 sampli przesunięcia)
                var progress = writePos / 1000.0

                var calculatedX = plotArea.x + progress * plotArea.width

                // OGRANICZENIE: nie pozwól wyjść poza plotArea
                if (calculatedX < plotArea.x) {
                    return plotArea.x
                }
                if (calculatedX > plotArea.x + plotArea.width) {
                    return plotArea.x + plotArea.width
                }

                return calculatedX
            }

            // Pozycja Y i wysokość - TYLKO w granicach plotArea
            y: eegGraph.plotArea ? eegGraph.plotArea.y : 0
            height: eegGraph.plotArea ? eegGraph.plotArea.height : 0

            Connections {
                target: eegData
                function onWritePositionChanged() {
                    // Debug
                    console.log("Write position updated:", eegData.writePosition)
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
