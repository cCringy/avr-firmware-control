import QtQuick
import QtQuick.Controls.Basic
import QtQuick.Layouts

Rectangle {
    id: root

    property alias tabs: buttons.model
    property int selectedIndex: 0

    signal itemSelected(int index)

    Layout.fillHeight: true
    Layout.preferredWidth: Math.max(75, window.width * 0.1)
    z: 50

    color: "#2c2c2c"

    ColumnLayout {
        anchors.fill: parent
        anchors.rightMargin: 5
        spacing: 1

        Repeater {
            id : buttons

            NavButton {
                Layout.fillWidth: true
                Layout.fillHeight: true
                text: modelData
                checked: root.selectedIndex === index
                onClicked: {
                    root.selectedIndex = index
                    root.itemSelected(index)
                }
            }
        }
    }
}