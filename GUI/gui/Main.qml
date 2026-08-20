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
            id: navBar

            z: 50
            Layout.fillHeight: true
            Layout.preferredWidth: 70

            tabs: [
                {name: "Pinout", icon: "qrc:/qt/qml/gui/images/pinout_icon.png"},
                {name: "ADC", icon: "qrc:/qt/qml/gui/images/adc_icon.gif"},
                {name: "UART", icon: "qrc:/qt/qml/gui/images/uart_icon.png"}
            ]

            Connections {
                target: navBar

                function onItemSelected(index){
                    stack.currentIndex = index
                }
            }
        }
        // --- Content-Bereich ---
        StackLayout {
            id: stack
            Layout.fillWidth: true
            Layout.fillHeight: true
            currentIndex: navBar.selectedIndex

            Rectangle {
                color: "#343A40"
                Text {
                    anchors.centerIn: parent
                    text: "Pinout Panel"
                    color: "white"
                }

                PinoutInterface{
                    anchors.fill: parent

                    anchors.margins: 10;
                    source: "qrc:/qt/qml/gui/images/pinout.png"

                    Component.onCompleted: loadPins("C:\Users\rfick\Desktop\Embedded Systems\avr-firmware-control\pindata/pins.json")

                    onPinClicked: (pinName, pinType) => {
                        console.log("Pin geklickt:", pinName, "Typ:", pinType)
                    }
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