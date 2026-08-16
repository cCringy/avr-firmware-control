import QtQuick

Item {
    id: root

    property String imageSource
    property var pins : []
    signal pinClicked()

    Image{
        id: pinOutImage
        anchors.fill: parent
        source: root.imageSource
        fillMode: Image.PreserveAspectFit
        smooth: true
        mipmap: true
    }

    Item{
        id: pinOverlay
        Repeater{
            model: root.pins
            MouseArea{
                onClicked: root.pinClicked(modelData.name)
            }
        }
    }
}
