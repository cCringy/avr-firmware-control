import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

Pane {
    id: root
    property string title: ""

    default property alias content: contentGrid.data
    property alias columns: contentGrid.columns
    property alias rowSpacing: contentGrid.rowSpacing
    property alias columnSpacing: contentGrid.columnSpacing

    ColumnLayout {
        anchors.fill: parent
        spacing: 15

        Label {
            text: root.title
            font.pixelSize: 20
            font.bold: true
        }

        GridLayout {
            id: contentGrid
            columns: 3
            rowSpacing: 15
            columnSpacing: 15
            Layout.fillWidth: true
        }
    }
}