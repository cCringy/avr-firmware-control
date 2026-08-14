import QtQuick
import QtQuick.Controls.Basic
import QtQuick.Layouts

Rectangle {
    id: root

    property var items: []
    property int selectedIndex: 0

    signal itemSelected(int index)

    Layout.fillHeight: true
    Layout.preferredWidth: Math.max(75, window.width * 0.1)
    color: "#1e1e2e"
    z: 50

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 10
        spacing: 10

        Repeater {
            model: ["Pinout", "ADC", "UART"]

            NavButton {
                text: modelData
                checked: stack.currentIndex === index
                onClicked: stack.currentIndex = index
            }
        }
    }
}