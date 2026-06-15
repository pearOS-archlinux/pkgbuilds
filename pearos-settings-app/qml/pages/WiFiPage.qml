import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "../components"

PageBase {
    title: "Wi-Fi"
    Component.onCompleted: { Network.refreshStatus(); Network.refreshNetworks() }

    // Password dialog state
    property string pendingSSID: ""
    property bool showPasswordDialog: false

    // Card: icon + description + toggle
    SettingsCard {
        RowLayout {
            width: parent.width
            height: 72
            spacing: 12

            Image {
                source: "file:///usr/share/extras/system-settings/assets/cs-network.svg"
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

        // Connected network section (Known Networks style)
        CardDivider { visible: Network.currentSSID.length > 0 }
        Item {
            width: parent.width; height: Network.currentSSID.length > 0 ? 50 : 0
            visible: Network.currentSSID.length > 0
            Row {
                anchors.left: parent.left; anchors.verticalCenter: parent.verticalCenter; spacing: 10
                // Lock icon placeholder (secure network indicator)
                Rectangle {
                    width: 8; height: 8; radius: 4; color: "#28ca42"; anchors.verticalCenter: parent.verticalCenter
                }
                Column {
                    anchors.verticalCenter: parent.verticalCenter; spacing: 3
                    Text { text: Network.currentSSID; font.pixelSize: 13; font.weight: Font.DemiBold; color: Theme.textPrimary }
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
                    width: parent.width; height: 40
                    property bool hov: false
                    Rectangle { anchors.fill: parent; color: hov ? Theme.hoverBg : "transparent"; radius: 4 }

                    Row {
                        anchors.left: parent.left; anchors.verticalCenter: parent.verticalCenter; spacing: 8
                        // Signal strength dots
                        Row {
                            spacing: 2; anchors.verticalCenter: parent.verticalCenter
                            Repeater {
                                model: 4
                                Rectangle {
                                    width: 3; height: 6 + index * 3; radius: 1
                                    anchors.bottom: parent.bottom
                                    color: index < 3 ? Theme.accent : Theme.divider
                                }
                            }
                        }
                        Text { text: modelData.ssid; font.pixelSize: 13; color: Theme.textPrimary; anchors.verticalCenter: parent.verticalCenter }
                    }

                    Text {
                        anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter
                        text: modelData.ssid === Network.currentSSID ? "Connected" : "Join"
                        font.pixelSize: 12; color: Theme.accent
                        MouseArea {
                            anchors.fill: parent; cursorShape: Qt.PointingHandCursor
                            onClicked: {
                                if (modelData.ssid !== Network.currentSSID) {
                                    pendingSSID = modelData.ssid
                                    passwordField.text = ""
                                    showPasswordDialog = true
                                }
                            }
                        }
                    }
                    Rectangle { anchors.bottom: parent.bottom; width: parent.width; height: 1; color: Theme.divider; visible: index < netRep.count - 1 }
                    MouseArea {
                        anchors.fill: parent; hoverEnabled: true
                        onEntered: parent.hov = true; onExited: parent.hov = false
                        onClicked: {
                            if (modelData.ssid !== Network.currentSSID) {
                                pendingSSID = modelData.ssid
                                passwordField.text = ""
                                showPasswordDialog = true
                            }
                        }
                    }
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

    // Password dialog — inline (like Electron's modal)
    Rectangle {
        visible: showPasswordDialog
        anchors.fill: parent
        color: Qt.rgba(0,0,0,0.45)
        z: 100

        MouseArea { anchors.fill: parent; onClicked: showPasswordDialog = false }

        Rectangle {
            anchors.centerIn: parent
            width: 320; radius: 12
            color: Theme.bgCard
            height: dialogCol.implicitHeight + 32

            Column {
                id: dialogCol
                anchors.left: parent.left; anchors.right: parent.right
                anchors.top: parent.top; anchors.margins: 16
                spacing: 14

                Text {
                    text: "Enter the password for\n\"" + pendingSSID + "\""
                    font.pixelSize: 14; font.weight: Font.DemiBold; color: Theme.textPrimary
                    anchors.horizontalCenter: parent.horizontalCenter
                    horizontalAlignment: Text.AlignHCenter
                    wrapMode: Text.WordWrap; width: parent.width
                }

                TextField {
                    id: passwordField
                    width: parent.width
                    echoMode: TextInput.Password
                    placeholderText: "Password"
                    font.pixelSize: 13
                    background: Rectangle { radius: 6; color: Qt.rgba(0,0,0,0.07); border.color: Theme.divider; border.width: 1 }
                }

                Row {
                    anchors.right: parent.right; spacing: 8

                    Rectangle {
                        height: 30; width: cancelLbl.implicitWidth + 20; radius: 6; color: Qt.rgba(0,0,0,0.08)
                        Text { id: cancelLbl; anchors.centerIn: parent; text: "Cancel"; font.pixelSize: 12; color: Theme.textPrimary }
                        MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor; onClicked: showPasswordDialog = false }
                    }
                    Rectangle {
                        height: 30; width: joinLbl.implicitWidth + 20; radius: 6; color: Theme.accent
                        Text { id: joinLbl; anchors.centerIn: parent; text: "Join"; font.pixelSize: 12; color: "white" }
                        MouseArea {
                            anchors.fill: parent; cursorShape: Qt.PointingHandCursor
                            onClicked: {
                                Network.connectToNetwork(pendingSSID, passwordField.text)
                                showPasswordDialog = false
                            }
                        }
                    }
                }
            }
        }
    }
}
