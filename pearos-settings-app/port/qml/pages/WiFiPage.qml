import QtQuick
import QtQuick.Layouts
import "../components"

PageBase {
    title: "Wi-Fi"
    Component.onCompleted: { Network.refreshStatus(); Network.refreshNetworks() }

    // Card: icon + description + toggle
    SettingsCard {
        // card-section-image layout
        RowLayout {
            width: parent.width
            height: 72
            spacing: 12

            Image {
                source: "file:///home/alxb421/Desktop/pkgbuilds/pearos-settings-app/port/assets/cs-network.svg"
                Layout.preferredWidth: 36; Layout.preferredHeight: 36
                fillMode: Image.PreserveAspectFit
            }

            Column {
                Layout.fillWidth: true
                spacing: 3
                Text { text: "Wi-Fi"; font.pixelSize: 15; font.weight: Font.DemiBold; color: Theme.textPrimary }
                Text {
                    text: "Set up Wi-Fi to wirelessly connect your computer to the internet. Turn on Wi-Fi, then choose a network to join."
                    font.pixelSize: 12; color: Theme.textSecondary; wrapMode: Text.WordWrap
                    width: parent.width
                }
            }

            SettingsToggle {
                labelText: ""; checked: Network.wifiEnabled
                onToggled: v => Network.toggleWifi(v)
            }
        }

        // Connected network section
        CardDivider { visible: Network.currentSSID.length > 0 }
        Item {
            width: parent.width; height: Network.currentSSID.length > 0 ? 50 : 0
            visible: Network.currentSSID.length > 0
            Column {
                anchors.left: parent.left; anchors.verticalCenter: parent.verticalCenter
                spacing: 4
                Text { text: Network.currentSSID; font.pixelSize: 12; font.weight: Font.DemiBold; color: Theme.textPrimary }
                Row {
                    spacing: 6
                    Rectangle { width: 8; height: 8; radius: 4; color: "#28ca42"; y: 4 }
                    Text { text: "Connected"; font.pixelSize: 12; color: Theme.textSecondary }
                }
            }
        }
    }
    Spacer {}

    SectionTitle { text: "Other Networks" }

    SettingsCard {
        Column {
            width: parent.width
            spacing: 0

            Repeater {
                id: netRep
                model: Network.networksModel
                delegate: Item {
                    width: parent.width; height: 36
                    Row {
                        anchors.left: parent.left; anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter
                        Text { text: modelData.ssid; font.pixelSize: 13; color: Theme.textPrimary; width: parent.width - 60; elide: Text.ElideRight }
                        Text {
                            text: modelData.ssid === Network.currentSSID ? "Connected" : "Join"
                            font.pixelSize: 12; color: "#3B82F6"
                            MouseArea { anchors.fill: parent; onClicked: Network.connectToNetwork(modelData.ssid, "") }
                        }
                    }
                    Rectangle { anchors.bottom: parent.bottom; width: parent.width; height: 1; color: Theme.divider; visible: index < netRep.count - 1 }
                }
            }
            Item {
                width: parent.width; height: 40
                visible: Network.networksModel.length === 0
                Text { anchors.centerIn: parent; text: "Scanning for networks..."; font.pixelSize: 13; color: Theme.textSecondary }
            }
        }
    }
    Spacer {}
}
