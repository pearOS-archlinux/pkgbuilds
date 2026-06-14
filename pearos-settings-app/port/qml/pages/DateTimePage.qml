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
            Rectangle {
                id: autoTimeToggle; width: 36; height: 18; radius: 9
                anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter
                color: DateTime.autoTime ? "#3B82F6" : "#cccccc"
                Behavior on color { ColorAnimation { duration: 200 } }
                Rectangle { width: 14; height: 14; radius: 7; color: "white"; y: 2; x: DateTime.autoTime ? 20 : 2; Behavior on x { NumberAnimation { duration: 200 } } }
                MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor; onClicked: DateTime.setAutoTime(!DateTime.autoTime) }
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
                Rectangle {
                    id: h24Toggle; width: 36; height: 18; radius: 9
                    anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter
                    color: DateTime.use24h ? "#3B82F6" : "#cccccc"
                    Behavior on color { ColorAnimation { duration: 200 } }
                    Rectangle { width: 14; height: 14; radius: 7; color: "white"; y: 2; x: DateTime.use24h ? 20 : 2; Behavior on x { NumberAnimation { duration: 200 } } }
                    MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor; onClicked: DateTime.setUse24h(!DateTime.use24h) }
                }
            }
        }
    }
    Spacer {}

    SettingsCard {
        Item {
            width: parent.width; height: 44
            Text { anchors.left: parent.left; anchors.verticalCenter: parent.verticalCenter; text: "Time Zone"; font.pixelSize: 12; font.weight: Font.DemiBold; color: Theme.textPrimary }
            ComboBox {
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
