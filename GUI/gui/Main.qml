import QtQuick
import QtQuick.Layouts
import QtQuick.Controls.Basic
import gui

ApplicationWindow {
    id: window
    width: 1600
    height: 900
    minimumWidth: 200
    minimumHeight: 250
    visible: true
    title: qsTr("AVR Firmware Controlpanel")

    GridLayout {
        id: grid
        rowSpacing: 0
        columnSpacing: 0
        anchors.fill: parent

        // --- Navbar ---
        NavBar{
        }
        // --- Content-Bereich ---
        StackLayout {
            id: stack
            Layout.fillWidth: true
            Layout.fillHeight: true
            currentIndex: 0

            Rectangle {
                color: "#2c2c2c"
                Text {
                    anchors.centerIn: parent
                    text: "Pinout Panel"
                    color: "white"
                }
            }
            Rectangle {
                color: "#2c2c2c"
                Text {
                    anchors.centerIn: parent
                    text: "ADC Panel"
                    color: "white"
                }
            }
            Rectangle {
                color: "#2c2c2c"
                Text{
                    anchors.centerIn: parent
                    text: "UART Panel"
                    color: "white"
                }
            }
        }
         //------------------
    }
}