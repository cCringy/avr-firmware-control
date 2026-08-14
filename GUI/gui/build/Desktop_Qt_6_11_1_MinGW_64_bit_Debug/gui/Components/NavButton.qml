import QtQuick
import QtQuick.Layouts
import QtQuick.Controls.Basic

Button {
    id: control
    Layout.fillWidth: true
    Layout.preferredHeight: 40

    checkable: true

    background: Rectangle {
        color: control.checked ? "#3a3a5c" : "#1e1e2e"
        radius: 6
    }

    contentItem: Text {
        text: control.text
        color: control.checked ? "white" : "#a0a0a0"
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
    }
}