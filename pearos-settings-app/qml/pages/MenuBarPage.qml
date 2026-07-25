import QtQuick
import QtQuick.Controls
import "../components"

PageBase {
    title: "Menu Bar"
    Component.onCompleted: MenuBarCfg.refresh()

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
                label: "Automatically Hide Menu Bar"
                sublabel: "Hide the top menu bar until the pointer moves to the top edge"
                toggled: MenuBarCfg.autoHide
                onToggle: function(v) { MenuBarCfg.setAutoHide(v) }
            }
        }
    }
    Spacer {}

    SectionTitle { text: "Menu Bar Items" }
    SettingsCard {
        Column {
            width: parent.width; spacing: 0
            Repeater {
                model: MenuBarCfg.trayItems
                delegate: Column {
                    width: parent.width
                    Row2 {
                        label: modelData.label
                        toggled: modelData.visible
                        onToggle: function(v) { MenuBarCfg.setTrayItemVisible(modelData.id, v) }
                    }
                    Rectangle {
                        width: parent.width; height: 1; color: Theme.divider
                        visible: index < MenuBarCfg.trayItems.length - 1
                    }
                }
            }
        }
    }
    Spacer {}
}
