import QtQuick
import QtQuick.Controls.Basic
import QtQuick.Layouts

Button {
    id: control

    Layout.fillWidth: true
    Layout.preferredHeight: 40

    property color normalColor: "transparent"
    property color hoverColor: "#2a2a3d"
    property color checkedColor: "#3d3d5c"
    property color normalTextColor: "#a0a0b0"
    property color checkedTextColor: "white"

    background: Rectangle {
        radius: 6
        color: control.checked
               ? control.checkedColor
               : (control.hovered ? control.hoverColor : control.normalColor)

        Behavior on color {
            ColorAnimation { duration: 120 }
        }
    }

    contentItem: Text {
        text: control.text
        color: control.checked ? control.checkedTextColor : control.normalTextColor
        font.pixelSize: 14
        font.bold: control.checked
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
    }
}