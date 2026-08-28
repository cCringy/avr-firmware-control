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

    Label {
        font.family: "Roboto" // Standard Windows UI Schriftart
        font.pixelSize: 12
        color: "#000000"
    }

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
            Layout.preferredWidth: 50

            tabs: [
                {name: "Pinout", icon: "qrc:/qt/qml/gui/images/pinout_icon.png"},
                {name: "ADC", icon: "qrc:/qt/qml/gui/images/adc_icon.gif"},
                {name: "Timer", icon: "qrc:/qt/qml/gui/images/timer_icon.png"},
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

                Label {
                    anchors.margins: 10
                    text: "Pinout Interface"
                    font.pixelSize: 20
                    font.bold: true
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
                Label {
                    anchors.margins: 10
                    text: "Timer"
                    font.pixelSize: 20
                    font.bold: true
                }
            }

            Rectangle {
                color: "#2c2c2c"
                Label {
                    anchors.margins: 10
                    text: "ADC"
                    font.pixelSize: 20
                    font.bold: true
                }
            }

            UartInterface{


            }
        }
         //------------------
    }
}