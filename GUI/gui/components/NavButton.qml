import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Button {
    id: root

    padding: 0
    leftPadding: 0
    rightPadding: 0
    topPadding: 0
    bottomPadding: 0

    property color color_normal  : "#2D3250"
    property color color_hovered : "#7077A1"
    property color color_checked : "#F6B17A"
    property color color_pressed : "#F6B17A"

    property url iconSource: ""

    property int collapsedWidth: 32
    property int expandedWidth: 160

    property bool expanded: false

    Timer{
        id: hoverTimer
        interval: 700
        onTriggered: expanded = true
    }

    HoverHandler{
        onHoveredChanged: {
            if(hovered) hoverTimer.start()
            else {
                hoverTimer.stop();
                root.expanded = false
            }
        }
    }

    implicitHeight: collapsedWidth
    implicitWidth: expanded ? expandedWidth : collapsedWidth

    z: hoverTimer.running || expanded ? 10:0

    Behavior on implicitWidth {
        NumberAnimation {
            duration: 200;
            easing.type: Easing.OutCubic
        }
    }

    background: Rectangle {
        radius: root.height/2
        color: root.checked ? color_checked
             : root.pressed ? color_pressed
             : root.hovered ? color_hovered
             : color_normal
    }

    contentItem: Item {
        Image {
            id: iconImg
            source: root.iconSource
            width: 24; height: 24
            anchors.left: parent.left
            anchors.leftMargin: (root.height - height) / 2
            anchors.verticalCenter: parent.verticalCenter
        }
        Text {
            text: root.text
            color: "white"
            opacity: root.expanded ? 1 : 0
            Behavior on opacity { NumberAnimation { duration: 150 } }
            anchors.left: iconImg.right
            anchors.leftMargin: 8
            anchors.verticalCenter: parent.verticalCenter
        }
    }
}