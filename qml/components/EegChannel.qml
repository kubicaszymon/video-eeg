import QtQuick
import QtQuick.Controls

Rectangle {
    id: root

    property var viewModel: null

    property int channelIndex: 0
    property string name: "undefined"

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

        property var renderData: null

        Connections {
            target: viewModel

            function onChannelDataChanged(chanIndex) {
                //console.log("Channel", chanIndex, "updating")
                if (chanIndex === channelIndex) {
                    canvas.renderData = viewModel.getChannelRenderData(channelIndex)
                    canvas.requestPaint()
                }
            }
        }

        onPaint: {
            var ctx = getContext("2d")

            // Clear background
            ctx.fillStyle = "#1E1E1E"
            ctx.fillRect(0, 0, width, height)

            // Check if we have data
            if (!renderData || renderData.isEmpty) {
                return
            }

            var points = renderData.points

            // Draw grid lines
            ctx.strokeStyle = "#2A2A2A"
            ctx.lineWidth = 1

            for (var j = 0; j <= 4; j++){
                var gy = (height / 4) * j
                ctx.beginPath()
                ctx.moveTo(0, gy)
                ctx.lineTo(width, gy)
                ctx.stroke()
            }

            // Draw signal - points are already normalized to 0-1
            ctx.strokeStyle = "#00BCD4"
            ctx.lineWidth = 1.5
            ctx.beginPath()

            for (var k = 0; k < points.length; k++){
                var point = points[k]

                // Map normalized coordinates to canvas pixels
                var x = point.x * width
                var y = height - (point.y * height)  // Flip Y axis

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
        visible: !canvas.renderData || canvas.renderData.isEmpty
    }
}
