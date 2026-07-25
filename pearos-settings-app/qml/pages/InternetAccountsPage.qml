import QtQuick
import QtQuick.Controls
import "../components"

PageBase {
    title: "Internet Accounts"
    Component.onCompleted: InetAccounts.refresh()

    SettingsCard {
        Column {
            width: parent.width; spacing: 0

            Item {
                width: parent.width; height: 48
                visible: InetAccounts.accounts.length === 0
                Text { anchors.centerIn: parent; text: "No internet accounts configured"; font.pixelSize: 12; color: Theme.textSecondary }
            }

            Repeater {
                model: InetAccounts.accounts
                delegate: Column {
                    width: parent.width
                    Item {
                        width: parent.width; height: 48
                        Column {
                            anchors.left: parent.left; anchors.verticalCenter: parent.verticalCenter; spacing: 2
                            Text { text: modelData.name; font.pixelSize: 13; font.weight: Font.DemiBold; color: Theme.textPrimary }
                            Text { text: modelData.provider + (modelData.enabled ? "" : " · Disabled"); font.pixelSize: 11; color: Theme.textSecondary }
                        }
                        Rectangle {
                            anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter
                            height: 26; width: 64; radius: 6; color: Qt.rgba(239,68,68,0.15)
                            Text { anchors.centerIn: parent; text: "Remove"; font.pixelSize: 11; color: "#EF4444" }
                            MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor
                                onClicked: InetAccounts.removeAccount(modelData.id) }
                        }
                    }
                    Rectangle { width: parent.width; height: 1; color: Theme.divider; visible: index < InetAccounts.accounts.length - 1 }
                }
            }

            Rectangle { width: parent.width; height: 1; color: Theme.divider; visible: InetAccounts.accounts.length > 0 }

            Item {
                width: parent.width; height: 44
                Text { anchors.left: parent.left; anchors.verticalCenter: parent.verticalCenter; text: "Add Account…"; font.pixelSize: 12; color: Theme.accent }
                MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor; onClicked: InetAccounts.openAddAccount() }
            }
        }
    }
    Spacer {}

    Text {
        width: parent.width; wrapMode: Text.WordWrap
        text: "Supports Nextcloud, ownCloud, and WebDAV. Adding an account opens KDE's Internet Accounts panel to sign in."
        font.pixelSize: 11; color: Theme.textSecondary
    }
    Spacer {}
}
