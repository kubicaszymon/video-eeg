import QtQuick
import QtQuick.Controls

Rectangle {
    id: root

    property int channelIndex: 0
    property var name: "undefined"

    color: "#1E1E1E"
    border.color: "#3E3E3E"
    border.width: 1

    Rectangle {
        id: header
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        height: 25
        color: "#2D2D2D"

        Label {
            anchors.centerIn: parent
            text: name
            color: "#FFFFFF"
            font.pixelSize: 11
        }
    }

    Canvas {
        id: canvas
        anchors.top: header.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom

        // Performance
        renderStrategy: Canvas.Threaded
        renderTarget: Canvas.FramebufferObject

        property var channelData: []
        property real minY: -100
        property real maxY: 100

        Connections {
            target: eegViewModel

            function onAllChannelsUpdated() {
                canvas.channelData = eegViewModel.getChannelData(channelIndex)
                canvas.requestPaint()
            }
        }

        onPaint: {
            var ctx = getContext("2d")

            ctx.fillStyle = "#1E1E1E"
            ctx.fillRect(0, 0, width, height)

            if (channelData.length === 0) return

            var dataMinY = Infinity
            var dataMaxY = -Infinity
            for (var i = 0; i < channelData.length; i++){
                var y = channelData[i].y
                if (y < dataMinY) dataMinY = y
                if(y > dataMaxY) dataMaxY = y
            }

            var range = dataMaxY - dataMinY
            if (range === 0) range = 1
            dataMinY -= range * 0.1
            dataMaxY += range * 0.1

            var minX = channelData[0].x
            var maxX = channelData[channelData.length - 1].x
            var timeRange = maxX - minX
            if (timeRange === 0) timeRange = 1

            ctx.strokeStyle = "#2A2A2A"
            ctx.lineWidth = 1

            for (var j = 0; j <= 4; j++){
                var gy = (height / 4) * j
                ctx.beginPath()
                ctx.moveTo(0, gy)
                ctx.lineTo(width, gy)
                ctx.stroke()
            }

            ctx.strokeStyle = "#00BCD4"
            ctx.lineWidth = 1.5
            ctx.beginPath()

            for (var k = 0; k < channelData.length; k++){
                var point = channelData[k]
                var x = ((point.x - minX) / timeRange) * width
                var y = height - ((point.y - dataMinY) / (dataMaxY - dataMinY)) * height

                if (k === 0){
                    ctx.moveTo(x, y)
                } else {
                    ctx.lineTo(x, y)
                }
            }

            ctx.stroke()
        }
    }

    Label {
        anchors.centerIn: canvas
        text: "No data"
        color: "#666666"
        visible: canvas.channelData.length === 0
    }
}
