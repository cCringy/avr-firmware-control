import QtQuick
import QtQuick.Controls

Button{
    id : root

    property color color_normal  : "#2D3250"
    property color color_hovered : "#7077A1"
    property color color_checked : "#F6B17A"
    property color color_pressed : "#F6B17A"

    checkable : true

    background: Rectangle{
        topRightRadius: 12
        bottomRightRadius: 12

        color: root.checked ? color_checked
             : root.pressed ? color_pressed
             : root.hovered ? color_hovered
             : color_normal
    }
}