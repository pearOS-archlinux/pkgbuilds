import QtQuick
import QtQuick.Controls
import "../components"

PageBase {
    title: "Focus"
    Component.onCompleted: Focus.refresh()

    component Toggle: LiquidToggle {
        property bool on: false
        property var onChange: null
        checked: on
        onToggled: function(v) { if (onChange) onChange(v) }
    }

    component Row2: Item {
        property string label: ""
        property string sublabel: ""
        property bool toggled: false
        property var onToggle: null
        width: parent.width; height: sublabel.length > 0 ? 52 : 44
        Column {
            anchors.left: parent.left; anchors.verticalCenter: parent.verticalCenter; spacing: 2
            Text { text: parent.parent.label; font.pixelSize: 12; font.weight: Font.DemiBold; color: Theme.textPrimary }
            Text { visible: parent.parent.sublabel.length > 0; text: parent.parent.sublabel; font.pixelSize: 11; color: Theme.textSecondary }
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
            Row2 {
                label: "Do Not Disturb"
                sublabel: "Silence notification popups and sounds until turned off"
                toggled: Focus.doNotDisturb
                onToggle: function(v) { Focus.setDoNotDisturb(v) }
            }
            Rectangle { width: parent.width; height: 1; color: Theme.divider }
            Row2 {
                label: "Allow Urgent Notifications"
                sublabel: "Still show critical notifications while Do Not Disturb is on"
                toggled: Focus.allowCriticalInDnd
                onToggle: function(v) { Focus.setAllowCriticalInDnd(v) }
            }
            Rectangle { width: parent.width; height: 1; color: Theme.divider }
            Row2 {
                label: "Mute Notification Sounds"
                sublabel: "Silence notification sounds while Do Not Disturb is on"
                toggled: Focus.muteSoundsInDnd
                onToggle: function(v) { Focus.setMuteSoundsInDnd(v) }
            }
        }
    }
    Spacer {}

    SectionTitle { text: "Turn On Automatically" }
    SettingsCard {
        Column {
            width: parent.width; spacing: 0
            Row2 {
                label: "When Screen is Mirrored"
                toggled: Focus.inhibitWhenMirrored
                onToggle: function(v) { Focus.setInhibitWhenMirrored(v) }
            }
            Rectangle { width: parent.width; height: 1; color: Theme.divider }
            Row2 {
                label: "When Sharing the Screen"
                toggled: Focus.inhibitWhenScreenSharing
                onToggle: function(v) { Focus.setInhibitWhenScreenSharing(v) }
            }
            Rectangle { width: parent.width; height: 1; color: Theme.divider }
            Row2 {
                label: "When Using a Fullscreen App"
                toggled: Focus.inhibitWhenFullscreen
                onToggle: function(v) { Focus.setInhibitWhenFullscreen(v) }
            }
        }
    }
    Spacer {}
}
