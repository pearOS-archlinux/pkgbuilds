import QtQuick
import QtQuick.Layouts
import "../components"

PageBase {
    title: "Battery"
    Component.onCompleted: Battery.refresh()

    SettingsCard {
        Column {
            width: parent.width; spacing: 0
            Item { width: parent.width; height: 8 }
            Text { text: "Battery"; font.pixelSize: 14; font.weight: Font.DemiBold; color: Theme.textPrimary; bottomPadding: 12 }

            Item {
                width: parent.width; height: 36
                Text { anchors.left: parent.left; anchors.verticalCenter: parent.verticalCenter; text: "Status"; font.pixelSize: 12; color: Theme.textSecondary }
                Text {
                    anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter
                    text: !Battery.isPresent ? "No battery" : Battery.isCharging ? "Charging" : "On battery power"
                    font.pixelSize: 12; color: Theme.textPrimary
                }
            }
            Rectangle { width: parent.width; height: 1; color: Theme.divider }

            Item {
                width: parent.width; height: 36
                Text { anchors.left: parent.left; anchors.verticalCenter: parent.verticalCenter; text: "Battery Level"; font.pixelSize: 12; color: Theme.textSecondary }
                Row {
                    anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter; spacing: 8
                    Rectangle {
                        width: 60; height: 14; radius: 3; border.color: Qt.rgba(0,0,0,0.2); border.width: 1; color: "transparent"
                        anchors.verticalCenter: parent.verticalCenter
                        Rectangle {
                            width: Math.max(0, Math.min(parent.width - 2, (parent.width - 2) * Battery.percentage / 100))
                            height: parent.height - 2; x: 1; y: 1; radius: 2
                            color: Battery.percentage > 20 ? "#28ca42" : "#ff3b30"
                        }
                    }
                    Text { text: Battery.percentage + "%"; font.pixelSize: 12; color: Theme.textPrimary; anchors.verticalCenter: parent.verticalCenter }
                }
            }

            Item {
                width: parent.width; height: 36
                visible: Battery.timeRemaining.length > 0
                Text { anchors.left: parent.left; anchors.verticalCenter: parent.verticalCenter; text: "Time Remaining"; font.pixelSize: 12; color: Theme.textSecondary }
                Text { anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter; text: Battery.timeRemaining; font.pixelSize: 12; color: Theme.textPrimary }
            }

            Rectangle { width: parent.width; height: 1; color: Theme.divider; visible: Battery.designCapacity > 0 }
            Item {
                width: parent.width; height: 36; visible: Battery.designCapacity > 0
                Text { anchors.left: parent.left; anchors.verticalCenter: parent.verticalCenter; text: "Design Capacity"; font.pixelSize: 12; color: Theme.textSecondary }
                Text { anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter; text: Battery.designCapacity + " mWh"; font.pixelSize: 12; color: Theme.textPrimary }
            }

            Rectangle { width: parent.width; height: 1; color: Theme.divider; visible: Battery.currentCapacity > 0 }
            Item {
                width: parent.width; height: 36; visible: Battery.currentCapacity > 0
                Text { anchors.left: parent.left; anchors.verticalCenter: parent.verticalCenter; text: "Full Charge Capacity"; font.pixelSize: 12; color: Theme.textSecondary }
                Text { anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter; text: Battery.currentCapacity + " mWh"; font.pixelSize: 12; color: Theme.textPrimary }
            }

            Item { width: parent.width; height: 8 }
        }
    }
    Spacer {}
}
