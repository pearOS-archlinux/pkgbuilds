import QtQuick
import QtQuick.Layouts
import "../components"

PageBase {
    title: "Network"
    Component.onCompleted: Network.refreshActiveConnections()

    SettingsCard {
        Column {
            width: parent.width; spacing: 0
            Item { width: parent.width; height: 8 }
            Text { text: "Active Connections"; font.pixelSize: 14; font.weight: Font.DemiBold; color: Theme.textPrimary; bottomPadding: 8 }

            Repeater {
                id: connRep; model: Network.activeConnections
                delegate: Column {
                    width: parent.width; spacing: 0
                    Item {
                        width: parent.width; height: 48
                        Column {
                            anchors.left: parent.left; anchors.verticalCenter: parent.verticalCenter; spacing: 2
                            Text { text: modelData.name; font.pixelSize: 13; color: Theme.textPrimary }
                            Text {
                                text: (modelData.type || "") + (modelData.ip4 ? " · " + modelData.ip4 : "")
                                font.pixelSize: 11; color: Theme.textSecondary
                            }
                        }
                        Rectangle {
                            width: 8; height: 8; radius: 4
                            anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter
                            color: modelData.state === "activated" ? "#28ca42" : "#ff9500"
                        }
                    }
                    Rectangle { width: parent.width; height: 1; color: Theme.divider; visible: index < connRep.count - 1 }
                }
            }

            Item {
                width: parent.width; height: 40; visible: Network.activeConnections.length === 0
                Text { anchors.centerIn: parent; text: "Loading connections..."; font.pixelSize: 13; color: Theme.textSecondary }
            }
            Item { width: parent.width; height: 8 }
        }
    }
    Spacer {}
}
