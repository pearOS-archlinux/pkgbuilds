import QtQuick
import QtQuick.Controls
import "../components"

PageBase {
    title: "Notifications"
    Component.onCompleted: Notifications.refresh()

    component Toggle: LiquidToggle {
        property bool on: false
        property var onChange: null
        checked: on
        onToggled: function(v) { if (onChange) onChange(v) }
    }

    component Row2: Item {
        property string label: ""
        property bool toggled: false
        property var onToggle: null
        width: parent.width; height: 44
        Text {
            anchors.left: parent.left; anchors.verticalCenter: parent.verticalCenter
            text: parent.label; font.pixelSize: 12; font.weight: Font.DemiBold; color: Theme.textPrimary
        }
        Toggle {
            anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter
            on: parent.toggled
            onChange: parent.onToggle
        }
    }

    SettingsCard {
        Column {
            width: parent.width; spacing: 0

            Item {
                width: parent.width; height: 44
                Text { anchors.left: parent.left; anchors.verticalCenter: parent.verticalCenter; text: "Popup Position"; font.pixelSize: 12; font.weight: Font.DemiBold; color: Theme.textPrimary }
                ComboBox {
                    anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter
                    width: 160; height: 30; font.pixelSize: 11
                    model: ["Near widget", "Top Left", "Top Center", "Top Right", "Bottom Left", "Bottom Center", "Bottom Right"]
                    currentIndex: Notifications.popupPosition
                    onActivated: function(idx) { Notifications.setPopupPosition(idx) }
                }
            }
            Rectangle { width: parent.width; height: 1; color: Theme.divider }

            Column {
                width: parent.width; spacing: 6; topPadding: 10; bottomPadding: 10
                Text { text: "Popup Duration"; font.pixelSize: 12; font.weight: Font.DemiBold; color: Theme.textPrimary }
                Row {
                    width: parent.width; spacing: 8
                    Slider {
                        from: 1000; to: 15000; stepSize: 500
                        value: Notifications.popupTimeout
                        palette.accent: Theme.accent; width: parent.width - 80
                        anchors.verticalCenter: parent.verticalCenter
                        onMoved: Notifications.setPopupTimeout(Math.round(value))
                    }
                    Text {
                        text: (Notifications.popupTimeout / 1000).toFixed(1) + "s"
                        font.pixelSize: 11; color: Theme.textSecondary; width: 44
                        horizontalAlignment: Text.AlignRight; anchors.verticalCenter: parent.verticalCenter
                    }
                }
            }
            Rectangle { width: parent.width; height: 1; color: Theme.divider }

            Row2 {
                label: "Show Countdown on Popups"
                toggled: Notifications.showPopupTimeout
                onToggle: function(v) { Notifications.setShowPopupTimeout(v) }
            }
            Rectangle { width: parent.width; height: 1; color: Theme.divider }

            Row2 {
                label: "Show Low Priority Notifications as Popups"
                toggled: Notifications.lowPriorityPopups
                onToggle: function(v) { Notifications.setLowPriorityPopups(v) }
            }
            Rectangle { width: parent.width; height: 1; color: Theme.divider }

            Row2 {
                label: "Keep Low Priority Notifications in History"
                toggled: Notifications.lowPriorityHistory
                onToggle: function(v) { Notifications.setLowPriorityHistory(v) }
            }
        }
    }
    Spacer {}

    SectionTitle { text: "Application Notifications" }
    SettingsCard {
        Column {
            width: parent.width; spacing: 0

            Item {
                width: parent.width; height: 40
                visible: Notifications.appList.length === 0
                Text { anchors.centerIn: parent; text: "No configured applications yet"; font.pixelSize: 12; color: Theme.textSecondary }
            }

            Item {
                width: parent.width; height: 26
                visible: Notifications.appList.length > 0
                Text { anchors.left: parent.left; anchors.verticalCenter: parent.verticalCenter; text: "Application"; font.pixelSize: 11; font.weight: Font.DemiBold; color: Theme.textSecondary }
                Row {
                    anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter; spacing: 40
                    Text { text: "Popups"; font.pixelSize: 11; font.weight: Font.DemiBold; color: Theme.textSecondary; width: 50; horizontalAlignment: Text.AlignHCenter }
                    Text { text: "History"; font.pixelSize: 11; font.weight: Font.DemiBold; color: Theme.textSecondary; width: 50; horizontalAlignment: Text.AlignHCenter }
                }
            }
            Rectangle { width: parent.width; height: 1; color: Theme.divider; visible: Notifications.appList.length > 0 }

            Repeater {
                model: Notifications.appList
                delegate: Column {
                    width: parent.width
                    Item {
                        width: parent.width; height: 40
                        Text {
                            anchors.left: parent.left; anchors.verticalCenter: parent.verticalCenter
                            text: modelData.label; font.pixelSize: 12; color: Theme.textPrimary
                            elide: Text.ElideRight; width: parent.width - 140
                        }
                        Row {
                            anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter; spacing: 40
                            Toggle {
                                width: 50
                                on: modelData.showPopups
                                onChange: function(v) { Notifications.setAppShowPopups(modelData.id, v) }
                            }
                            Toggle {
                                width: 50
                                on: modelData.showInHistory
                                onChange: function(v) { Notifications.setAppShowInHistory(modelData.id, v) }
                            }
                        }
                    }
                    Rectangle { width: parent.width; height: 1; color: Theme.divider; visible: index < Notifications.appList.length - 1 }
                }
            }
        }
    }
    Spacer {}
}
