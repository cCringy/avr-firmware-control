import QtQuick
import QtQuick.Controls
import gui

Item {
    id: root
    property alias source: image.source
    property var pins: []
    property string selectedPin: ""

    signal pinClicked(string pinName, string pinType)
    signal pinHovered(string pinName)

    implicitWidth: 800
    implicitHeight: 600

    PinLoader{id:pinLoader}

    function loadPins(url){
        root.pins = pinLoader.loadDefaultPins(image.sourceSize.width, image.sourceSize.height)
    }
    Image {
        id: image
        anchors.fill: parent
        fillMode: Image.PreserveAspectFit
        smooth: true
        mipmap: true
    }

    Item {
        id: overlay
        x: image.x + (image.width - image.paintedWidth) / 2
        y: image.y + (image.height - image.paintedHeight) / 2
        width: image.paintedWidth
        height: image.paintedHeight

        Repeater {
            model: root.pins
            delegate: Rectangle {
                id: hotspot
                required property var modelData

                x: modelData.relX * overlay.width - width / 2
                y: modelData.relY * overlay.height - height / 2
                width: (modelData.relW ?? 0.025) * overlay.width
                height: (modelData.relH ?? 0.025) * overlay.height
                radius: width / 2
                color: mouseArea.containsMouse
                       ? Qt.rgba(1, 0.5, 0, 0.5)
                       : Qt.rgba(1, 0, 0, 0.15)
                border.color: root.selectedPin === modelData.name ? "orange" : "red"
                border.width: root.selectedPin === modelData.name ? 3 : 1

                MouseArea {
                    id: mouseArea
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor

                    onClicked: {
                        root.selectedPin = hotspot.modelData.name
                        root.pinClicked(hotspot.modelData.name, hotspot.modelData.type)
                    }
                    onEntered: root.pinHovered(hotspot.modelData.name)
                }

                ToolTip.visible: mouseArea.containsMouse
                ToolTip.delay: 300
                ToolTip.text: hotspot.modelData.name +
                              (hotspot.modelData.info ? "\n" + hotspot.modelData.info : "")
            }
        }
    }
}