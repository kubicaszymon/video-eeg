import QtQuick
import QtGraphs
import videoEeg

Rectangle {
    id: eegGraphContainer
    color: "white"
    property alias dataModel: eegData

    EegDataModel {
        id: eegData
    }

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
            max: 10
            tickInterval: 2
            subTickCount: 9
            labelDecimals: 1
        }
        axisY: ValueAxis {
            max: 10
        }

        component Marker : Rectangle {
            width: 4
            height: 4
            color: "#ffffff"
            radius: width * 0.5
            border.width: 2
            border.color: "#000000"
        }

        LineSeries {
                id: line1
                name: "Test Channel"
                XYModelMapper {
                    model: eegData
                    series: line1
                    xSection: 0
                    ySection: 1

                    Component.onCompleted: console.log("MAPPER DZIALA!")
                }
            }
        //! [linemarker]

        //Repeater {
         //   model: 5
          //  delegate: LineSeries {
          //      id: lines
          //      name: "Channel " + index

//                XYModelMapper {
//                    model: eegData
//                    series: lines
//                    xSection: 0
//                    ySection: index + 1
  //                  orientation: Qt.Vertical
//
  //                  Component.onCompleted: {
    //                        console.log("Mapper created. Model is:", model)
      //                  }
        //            onModelChanged: {
          //                  console.log("Model in mapper changed to:", model)
            //            }
             //   }
       //     }
        //}
    }

}
