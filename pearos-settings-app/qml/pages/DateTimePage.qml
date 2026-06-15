import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "../components"

PageBase {
    title: "Date & Time"
    Component.onCompleted: DateTime.refresh()

    SettingsCard {
        Item {
            width: parent.width; height: 44
            Text { anchors.left: parent.left; anchors.verticalCenter: parent.verticalCenter; text: "Set time and date automatically"; font.pixelSize: 12; font.weight: Font.DemiBold; color: Theme.textPrimary }
            LiquidToggle {
                anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter
                checked: DateTime.autoTime
                onToggled: function(v) { DateTime.setAutoTime(v) }
            }
        }
    }
    Spacer {}

    SettingsCard {
        Column {
            width: parent.width; spacing: 0

            Item {
                width: parent.width; height: 44
                Text { anchors.left: parent.left; anchors.verticalCenter: parent.verticalCenter; text: "Date and time"; font.pixelSize: 12; font.weight: Font.DemiBold; color: Theme.textPrimary }
                Text {
                    anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter
                    text: Qt.formatDateTime(new Date(), DateTime.use24h ? "MMM d, yyyy HH:mm:ss" : "MMM d, yyyy h:mm:ss AP")
                    font.pixelSize: 12; color: Theme.textSecondary
                    Timer { interval: 1000; running: true; repeat: true; onTriggered: parent.text = Qt.formatDateTime(new Date(), DateTime.use24h ? "MMM d, yyyy HH:mm:ss" : "MMM d, yyyy h:mm:ss AP") }
                }
            }
            Rectangle { width: parent.width; height: 1; color: Theme.divider }

            Item {
                width: parent.width; height: 44
                Text { anchors.left: parent.left; anchors.verticalCenter: parent.verticalCenter; text: "24-hour time"; font.pixelSize: 12; font.weight: Font.DemiBold; color: Theme.textPrimary }
                LiquidToggle {
                    anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter
                    checked: DateTime.use24h
                    onToggled: function(v) { DateTime.setUse24h(v) }
                }
            }
        }
    }
    Spacer {}

    SettingsCard {
        Item {
            width: parent.width; height: 44
            Text { anchors.left: parent.left; anchors.verticalCenter: parent.verticalCenter; text: "Time Zone"; font.pixelSize: 12; font.weight: Font.DemiBold; color: Theme.textPrimary }
            SettingsComboBox {
                anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter
                model: DateTime.timezones.length > 0 ? DateTime.timezones : [DateTime.timezone || "UTC"]
                currentIndex: Math.max(0, model.indexOf(DateTime.timezone))
                width: 200; height: 28; font.pixelSize: 11
                onActivated: DateTime.setTimezone(currentText)
            }
        }
    }
    Spacer {}
}
