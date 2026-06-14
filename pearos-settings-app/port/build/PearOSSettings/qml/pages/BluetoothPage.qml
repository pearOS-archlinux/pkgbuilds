import QtQuick
import QtQuick.Layouts
import "../components"

PageBase {
    title: "Bluetooth"
    Component.onCompleted: { Bluetooth.refreshStatus(); Bluetooth.refreshDevices() }

    SettingsCard {
        RowLayout {
            width: parent.width; height: 72; spacing: 12
            Image {
                source: "file:///home/alxb421/Desktop/pkgbuilds/pearos-settings-app/port/assets/cs-bluetooth.svg"
                Layout.preferredWidth: 36; Layout.preferredHeight: 36; fillMode: Image.PreserveAspectFit
            }
            Column {
                Layout.fillWidth: true; spacing: 3
                Text { text: "Bluetooth"; font.pixelSize: 15; font.weight: Font.DemiBold; color: Theme.textPrimary }
                Text {
                    text: "Set up Bluetooth to wirelessly connect your computer to Bluetooth devices."
                    font.pixelSize: 12; color: Theme.textSecondary; wrapMode: Text.WordWrap; width: parent.width
                }
            }
            SettingsToggle { labelText: ""; checked: Bluetooth.bluetoothEnabled; onToggled: v => Bluetooth.toggleBluetooth(v) }
        }
        // Discoverable info
        Item { width: parent.width; height: 10 }
        Text {
            width: parent.width
            text: 'This Computer is discoverable as "' + Bluetooth.deviceName + '"'
            font.pixelSize: 12; color: Theme.textSecondary; wrapMode: Text.WordWrap
        }
        Item { width: parent.width; height: 12 }
    }
    Spacer {}

    SectionTitle { text: "My Devices" }
    SettingsCard {
        Column {
            width: parent.width; spacing: 0
            Repeater {
                id: devRep
                model: Bluetooth.devicesModel
                delegate: Item {
                    width: parent.width; height: 44
                    Row {
                        anchors.left: parent.left; anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter
                        Column {
                            width: parent.width - 80
                            spacing: 2
                            Text { text: modelData.name; font.pixelSize: 13; color: Theme.textPrimary; elide: Text.ElideRight; width: parent.width }
                            Text {
                                text: modelData.connected ? "Connected" : modelData.type
                                font.pixelSize: 12; color: modelData.connected ? "#28ca42" : "#666"
                            }
                        }
                        Text {
                            text: modelData.connected ? "Disconnect" : "Connect"
                            font.pixelSize: 12; color: "#3B82F6"; anchors.verticalCenter: parent.verticalCenter
                            MouseArea { anchors.fill: parent; onClicked: modelData.connected ? Bluetooth.disconnectDevice(modelData.macAddress) : Bluetooth.connectDevice(modelData.macAddress) }
                        }
                    }
                    Rectangle { anchors.bottom: parent.bottom; width: parent.width; height: 1; color: Theme.divider; visible: index < devRep.count - 1 }
                }
            }
            Item {
                width: parent.width; height: 40; visible: Bluetooth.devicesModel.length === 0
                Text { anchors.centerIn: parent; text: "No devices found"; font.pixelSize: 13; color: Theme.textSecondary }
            }
        }
    }
    Spacer {}

    SectionTitle { text: "Nearby Devices" }
    SettingsCard {
        Column {
            width: parent.width; spacing: 0
            Repeater {
                id: nearRep
                model: Bluetooth.nearbyDevicesModel
                delegate: Item {
                    width: parent.width; height: 44
                    Row {
                        anchors.left: parent.left; anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter
                        Column {
                            width: parent.width - 60; spacing: 2
                            Text { text: modelData.name; font.pixelSize: 13; color: Theme.textPrimary }
                            Text { text: modelData.type; font.pixelSize: 12; color: Theme.textSecondary }
                        }
                        Text {
                            text: "Pair"; font.pixelSize: 12; color: "#3B82F6"; anchors.verticalCenter: parent.verticalCenter
                            MouseArea { anchors.fill: parent; onClicked: Bluetooth.connectDevice(modelData.macAddress) }
                        }
                    }
                    Rectangle { anchors.bottom: parent.bottom; width: parent.width; height: 1; color: Theme.divider; visible: index < nearRep.count - 1 }
                }
            }
            Item {
                width: parent.width; height: 40; visible: Bluetooth.nearbyDevicesModel.length === 0
                Text { anchors.centerIn: parent; text: "Searching..."; font.pixelSize: 13; color: Theme.textSecondary }
            }
            Item { width: parent.width; height: 8 }
            Text {
                text: "Scan for Nearby Devices"; color: "#3B82F6"; font.pixelSize: 13
                anchors.horizontalCenter: parent.horizontalCenter
                MouseArea { anchors.fill: parent; onClicked: Bluetooth.refreshNearbyDevices() }
            }
            Item { width: parent.width; height: 8 }
        }
    }
    Spacer {}
}
