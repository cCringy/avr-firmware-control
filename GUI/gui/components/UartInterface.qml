import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

// Eigenständige UART-Konfigurationsseite mit sauberem Karten-Layout.
// Jede Karte fasst Label + Control zu einer optischen Einheit zusammen,
// statt sie als getrennte Grid-Zellen zu behandeln.
Pane {
    id: root
    padding: 24

    background: Rectangle { color: "#1e1e1e" }

    property color cardColor: "#2a2a2a"
    property color cardBorder: "#3d3d3d"
    property color accent: "#4da3ff"
    property color labelColor: "#9aa0a6"
    property color valueColor: "#f0f0f0"

    // ---- Reusable "Karte": Label oben, Control darunter ----
    component SettingCard: Rectangle {
        id: card
        property alias label: cardLabel.text
        default property alias content: cardContent.data
        Layout.preferredWidth: 230
        Layout.preferredHeight: 78
        Layout.fillWidth: false
        color: root.cardColor
        border.color: root.cardBorder
        border.width: 1
        radius: 8

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 12
            spacing: 6

            Label {
                id: cardLabel
                color: root.labelColor
                font.pixelSize: 12
                font.bold: true
            }

            Item {
                id: cardContent
                Layout.fillWidth: true
                Layout.fillHeight: true
            }
        }
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 20

        Label {
            text: "UART Konfiguration"
            color: root.valueColor
            font.pixelSize: 22
            font.bold: true
        }

        Rectangle { Layout.fillWidth: true; height: 1; color: root.cardBorder }

        GridLayout {
            columns: 3
            rowSpacing: 16
            columnSpacing: 16

            // Baudrate ----------------------------------------------------
            SettingCard {
                label: "Baudrate"
                ComboBox {
                    id: baudrateBox
                    anchors.fill: parent
                    model: [9600, 19200, 38400, 57600, 115200]
                    currentIndex: 4
                }
            }

            // Mode ----------------------------------------------------------
            SettingCard {
                label: "Mode"
                ComboBox {
                    id: modeBox
                    anchors.fill: parent
                    textRole: "text"
                    valueRole: "value"
                    model: [
                        { text: "Async", value: 0 },
                        { text: "Sync",  value: 1 },
                        { text: "MSPIM", value: 2 }
                    ]
                    currentIndex: 0
                }
            }

            // Frame Size ------------------------------------------------
            SettingCard {
                label: "Frame Size"
                ComboBox {
                    id: framesizeBox
                    anchors.fill: parent
                    textRole: "text"
                    valueRole: "value"
                    model: [
                        { text: "5 Bit", value: 0x0 },
                        { text: "6 Bit", value: 0x1 },
                        { text: "7 Bit", value: 0x2 },
                        { text: "8 Bit", value: 0x3 },
                        { text: "9 Bit", value: 0x7 }
                    ]
                    currentIndex: 3
                }
            }

            // Parität ---------------------------------------------------
            SettingCard {
                label: "Parität"
                ComboBox {
                    id: parityBox
                    anchors.fill: parent
                    textRole: "text"
                    valueRole: "value"
                    model: [
                        { text: "None", value: 0x0 },
                        { text: "Even", value: 0x2 },
                        { text: "Odd",  value: 0x3 }
                    ]
                    currentIndex: 0
                }
            }

            // Stop Bits ---------------------------------------------------
            SettingCard {
                label: "Stop Bits"
                ComboBox {
                    id: stopBitsBox
                    anchors.fill: parent
                    textRole: "text"
                    valueRole: "value"
                    model: [
                        { text: "1", value: 0 },
                        { text: "2", value: 1 }
                    ]
                    currentIndex: 0
                }
            }

            // Double Speed --------------------------------------------------
            SettingCard {
                label: "Double Speed (U2X)"
                enabled: modeBox.currentValue === 0
                opacity: enabled ? 1.0 : 0.4
                RowLayout {
                    anchors.fill: parent
                    Switch {
                        id: doubleSpeedSwitch
                        checked: false
                    }
                    Label {
                        text: doubleSpeedSwitch.checked ? "Ein" : "Aus"
                        color: root.labelColor
                    }
                    Item { Layout.fillWidth: true }
                }
            }

            // SPI Mode ------------------------------------------------------
            SettingCard {
                label: "SPI Mode"
                Layout.columnSpan: 2
                Layout.preferredWidth: 476
                enabled: modeBox.currentValue === 2
                opacity: enabled ? 1.0 : 0.4
                ComboBox {
                    id: spiModeBox
                    anchors.fill: parent
                    textRole: "text"
                    valueRole: "value"
                    model: [
                        { text: "Mode 0  (CPHA=0, CPOL=0)", value: 0x0 },
                        { text: "Mode 1  (CPHA=1, CPOL=0)", value: 0x1 },
                        { text: "Mode 2  (CPHA=0, CPOL=1)", value: 0x2 },
                        { text: "Mode 3  (CPHA=1, CPOL=1)", value: 0x3 }
                    ]
                    currentIndex: 0
                }
            }
        }

        Item { Layout.fillHeight: true }
    }

    // ---- Struct aus den Controls zusammenbauen ----
    function toConfig() {
        return {
            baudrate: parseInt(baudrateBox.currentText),
            mode: modeBox.currentValue,
            framesize: framesizeBox.currentValue,
            parity: parityBox.currentValue,
            two_stop_bits: stopBitsBox.currentValue,
            async_double_speed: doubleSpeedSwitch.checked ? 1 : 0,
            spi_mode: spiModeBox.currentValue
        }
    }
}