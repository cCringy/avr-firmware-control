import QtQuick
import QtQuick.Controls.Basic
import QtQuick.Layouts

Rectangle {
    id: root

    property alias tabs: buttons.model

    default property alias content: extraArea.data

    property int selectedIndex: 0

    signal itemSelected(int index)

    color: "#212529"

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 5
        spacing: 5

        Rectangle {
            id: tabsCard
            radius: 20
            Layout.fillWidth: true
            Layout.preferredHeight: tabsWrapper.height + 5
            color: "#3A3F5C"

            Item {
                id: tabsWrapper
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.top: parent.top
                anchors.topMargin: 10
                width: 32          // = collapsedWidth der Buttons, fix
                height: buttonsColumn.implicitHeight

                ColumnLayout {
                    id: buttonsColumn
                    anchors.left: parent.left
                    spacing: 8
                    Repeater {
                        id: buttons
                        NavButton {
                            Layout.fillWidth: false
                            text: modelData.name
                            iconSource: modelData.icon
                            icon.color: "transparent"
                            checked: root.selectedIndex === index
                            onClicked: {
                                root.selectedIndex = index
                                root.itemSelected(index)
                            }
                        }
                    }
                }
            }
        }

        ColumnLayout {
            id: extraArea
            Layout.fillWidth: true
            // hier landen später Settings-Button, Text etc. (default property)
        }
    }
}




