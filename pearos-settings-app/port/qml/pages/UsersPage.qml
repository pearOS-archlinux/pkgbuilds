import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "../components"

PageBase {
    title: "Users & Groups"
    Component.onCompleted: Users.refresh()

    SettingsCard {
        Column {
            width: parent.width; spacing: 0
            Repeater {
                id: usersRep; model: Users.users
                delegate: Column {
                    width: parent.width; spacing: 0
                    Item {
                        width: parent.width; height: 56
                        Row {
                            anchors.left: parent.left; anchors.verticalCenter: parent.verticalCenter; spacing: 12
                            Rectangle {
                                width: 36; height: 36; radius: 18; color: Theme.divider
                                anchors.verticalCenter: parent.verticalCenter
                                Text {
                                    anchors.centerIn: parent
                                    text: (modelData.fullName || modelData.username || "?")[0].toUpperCase()
                                    font.pixelSize: 16; font.weight: Font.DemiBold; color: Theme.textPrimary
                                }
                            }
                            Column {
                                anchors.verticalCenter: parent.verticalCenter; spacing: 2
                                Text { text: modelData.fullName || modelData.username; font.pixelSize: 14; font.weight: Font.Medium; color: Theme.textPrimary }
                                Text { text: modelData.username + (modelData.isCurrent ? " · You" : ""); font.pixelSize: 12; color: Theme.textSecondary }
                            }
                        }
                    }
                    Rectangle { width: parent.width; height: 1; color: Theme.divider; visible: index < usersRep.count - 1 }
                }
            }
            Item { height: 40; width: parent.width; visible: Users.users.length === 0
                Text { anchors.centerIn: parent; text: "Loading users..."; font.pixelSize: 13; color: Theme.textSecondary }
            }
        }
    }
    Spacer {}

    SettingsCard {
        Item {
            width: parent.width; height: 44
            Text { anchors.left: parent.left; anchors.verticalCenter: parent.verticalCenter; text: "Automatically log in"; font.pixelSize: 13; font.weight: Font.Medium; color: Theme.textPrimary }
            ComboBox {
                anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter
                model: ["Off", "On"]
                currentIndex: 0
                width: 80; height: 28; font.pixelSize: 12
            }
        }
    }
    Spacer {}
}
