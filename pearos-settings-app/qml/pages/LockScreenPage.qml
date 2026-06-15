import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "../components"

PageBase {
    title: "Lock Screen"
    Component.onCompleted: LockScreen.refresh()

    SettingsCard {
        Item {
            width: parent.width; height: 44
            Text { anchors.left: parent.left; anchors.verticalCenter: parent.verticalCenter; text: "Enable automatic lock"; font.pixelSize: 12; font.weight: Font.DemiBold; color: Theme.textPrimary }
            LiquidToggle {
                anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter
                checked: LockScreen.autoLock
                onToggled: function(v) { LockScreen.setAutoLock(v) }
            }
        }
    }
    Spacer {}

    SettingsCard {
        Column {
            width: parent.width; spacing: 0

            Item {
                width: parent.width; height: 44
                Text { anchors.left: parent.left; anchors.verticalCenter: parent.verticalCenter; text: "Lock after"; font.pixelSize: 12; font.weight: Font.DemiBold; color: Theme.textPrimary }
                ComboBox {
                    anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter
                    model: ["1 minute", "2 minutes", "5 minutes", "10 minutes", "30 minutes", "Never"]
                    currentIndex: {
                        var v = LockScreen.lockTimeout
                        if (v <= 1) return 0
                        if (v <= 2) return 1
                        if (v <= 5) return 2
                        if (v <= 10) return 3
                        if (v <= 30) return 4
                        return 5
                    }
                    width: 130; height: 28; font.pixelSize: 12
                    onActivated: {
                        var vals = [1, 2, 5, 10, 30, 0]
                        LockScreen.setLockTimeout(vals[currentIndex])
                    }
                }
            }
            Rectangle { width: parent.width; height: 1; color: Theme.divider }

            Item {
                width: parent.width; height: 44
                Text { anchors.left: parent.left; anchors.verticalCenter: parent.verticalCenter; text: "Require password to unlock"; font.pixelSize: 12; font.weight: Font.DemiBold; color: Theme.textPrimary }
                LiquidToggle {
                    anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter
                    checked: LockScreen.requirePassword
                    onToggled: function(v) { LockScreen.setRequirePassword(v) }
                }
            }
        }
    }
    Spacer {}

    SettingsCard {
        Item {
            width: parent.width; height: 44
            Rectangle {
                anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter
                width: 120; height: 30; radius: 6; color: Qt.rgba(0,0,0,0.06); border.color: Qt.rgba(0,0,0,0.15); border.width: 1
                Text { anchors.centerIn: parent; text: "Lock Screen Now"; font.pixelSize: 12; color: Theme.textPrimary }
                MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor; onClicked: LockScreen.lockNow() }
            }
        }
    }
    Spacer {}
}
