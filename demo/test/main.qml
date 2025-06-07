import QtQuick 2.6
import QtQuick.Window 2.2

Window {
    visible: true
    width: 640
    height: 480
    title: qsTr("Inllecture_classroom")

    Column {
            anchors.centerIn: parent
            spacing: 20

            Text {
                text: "温度: " + sensor.temperature.toFixed(1) + " °C"
                font.pixelSize: 20
            }

            Text {
                text: "湿度: " + sensor.moisture.toFixed(1) + " %"
                font.pixelSize: 20
            }

            Text {
                text: "亮度: " + sensor.illumination.toFixed(0)
                font.pixelSize: 20
            }

            Text {
                text: sensor.person == true ? "当前有人" : "当前无人"
                font.pixelSize: 20
            }
        }
}
