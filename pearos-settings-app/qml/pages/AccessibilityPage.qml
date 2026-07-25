import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "../components"

PageBase {
    title: "Accessibility"
    property int activeTab: 0
    Component.onCompleted: Accessibility.refresh()

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

    component SliderRow: Column {
        property string label: ""
        property real from: 0
        property real to: 1
        property real val: 0
        property string suffix: ""
        property var onSet: null
        width: parent.width; spacing: 6; topPadding: 10; bottomPadding: 10
        Text { text: parent.label; font.pixelSize: 12; font.weight: Font.DemiBold; color: Theme.textPrimary }
        Row {
            width: parent.width; spacing: 8
            Slider {
                from: parent.parent.from; to: parent.parent.to
                value: parent.parent.val
                palette.accent: Theme.accent; width: parent.parent.width - 60
                anchors.verticalCenter: parent.verticalCenter
                onMoved: parent.parent.onSet(value)
            }
            Text {
                text: Math.round(parent.parent.val) + parent.parent.suffix
                font.pixelSize: 11; color: Theme.textSecondary; width: 44
                horizontalAlignment: Text.AlignRight; anchors.verticalCenter: parent.verticalCenter
            }
        }
    }

    // Tabs
    SettingsCard {
        Row {
            width: parent.width; height: 36; spacing: 0
            Repeater {
                model: ["Keyboard", "Pointer", "Display", "Audio"]
                delegate: Rectangle {
                    width: parent.width / 4; height: 36; radius: 6
                    color: activeTab === index ? Qt.rgba(0,0,0,0.1) : "transparent"
                    Text { anchors.centerIn: parent; text: modelData; font.pixelSize: 12; font.weight: Font.Medium; color: Theme.textPrimary }
                    MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor; onClicked: activeTab = index }
                }
            }
        }
    }
    Spacer {}

    // ── Keyboard ─────────────────────────────────────────────────────────
    SettingsCard {
        visible: activeTab === 0
        Column {
            width: parent.width; spacing: 0

            Row2 {
                label: "Sticky Keys"
                sublabel: "Press modifier keys (Shift, Ctrl, Alt) one at a time instead of holding them"
                toggled: Accessibility.stickyKeys
                onToggle: function(v) { Accessibility.setStickyKeys(v) }
            }
            Rectangle { width: parent.width; height: 1; color: Theme.divider }

            Row2 {
                label: "Slow Keys"
                sublabel: "Require keys to be held briefly before they register"
                toggled: Accessibility.slowKeys
                onToggle: function(v) { Accessibility.setSlowKeys(v) }
            }
            Rectangle { width: parent.width; height: 1; color: Theme.divider }

            SliderRow {
                label: "Slow Keys Delay"
                from: 100; to: 1000; val: Accessibility.slowKeysDelay; suffix: " ms"
                onSet: function(v) { Accessibility.setSlowKeysDelay(Math.round(v)) }
            }
            Rectangle { width: parent.width; height: 1; color: Theme.divider }

            Row2 {
                label: "Bounce Keys"
                sublabel: "Ignore rapid repeated key presses from an unsteady hand"
                toggled: Accessibility.bounceKeys
                onToggle: function(v) { Accessibility.setBounceKeys(v) }
            }
            Rectangle { width: parent.width; height: 1; color: Theme.divider }

            SliderRow {
                label: "Bounce Keys Delay"
                from: 100; to: 1000; val: Accessibility.bounceKeysDelay; suffix: " ms"
                onSet: function(v) { Accessibility.setBounceKeysDelay(Math.round(v)) }
            }
        }
    }

    // ── Pointer ──────────────────────────────────────────────────────────
    SettingsCard {
        visible: activeTab === 1
        Column {
            width: parent.width; spacing: 0

            Row2 {
                label: "Mouse Keys"
                sublabel: "Move the pointer and click using the numeric keypad"
                toggled: Accessibility.mouseKeys
                onToggle: function(v) { Accessibility.setMouseKeys(v) }
            }
            Rectangle { width: parent.width; height: 1; color: Theme.divider }

            SliderRow {
                label: "Mouse Keys Max Speed"
                from: 10; to: 1000; val: Accessibility.mouseKeysMaxSpeed; suffix: " px/s"
                onSet: function(v) { Accessibility.setMouseKeysMaxSpeed(Math.round(v)) }
            }
            Rectangle { width: parent.width; height: 1; color: Theme.divider }

            Item {
                width: parent.width; height: 44
                Text { anchors.left: parent.left; anchors.verticalCenter: parent.verticalCenter; text: "Cursor Size"; font.pixelSize: 12; font.weight: Font.DemiBold; color: Theme.textPrimary }
                Row {
                    anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter; spacing: 6
                    Repeater {
                        model: [{label: "Default", val: 24}, {label: "Large", val: 36}, {label: "Extra Large", val: 48}, {label: "Huge", val: 64}]
                        delegate: Rectangle {
                            height: 28; width: csLbl.implicitWidth + 16; radius: 6
                            color: Accessibility.cursorSize === modelData.val ? Theme.accent : Qt.rgba(0,0,0,0.08)
                            Text { id: csLbl; anchors.centerIn: parent; text: modelData.label; font.pixelSize: 12
                                color: Accessibility.cursorSize === modelData.val ? "white" : Theme.textPrimary }
                            MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor
                                onClicked: Accessibility.setCursorSize(modelData.val) }
                        }
                    }
                }
            }
        }
    }

    // ── Display ──────────────────────────────────────────────────────────
    SettingsCard {
        visible: activeTab === 2
        Column {
            width: parent.width; spacing: 0

            Row2 {
                label: "Reduce Motion"
                sublabel: "Turn off window and effect animations"
                toggled: Accessibility.reduceMotion
                onToggle: function(v) { Accessibility.setReduceMotion(v) }
            }
            Rectangle { width: parent.width; height: 1; color: Theme.divider }

            SliderRow {
                label: "Increase Contrast"
                from: 0; to: 10; val: Accessibility.contrast; suffix: ""
                onSet: function(v) { Accessibility.setContrast(Math.round(v)) }
            }
        }
    }

    // ── Audio ────────────────────────────────────────────────────────────
    SettingsCard {
        visible: activeTab === 3
        Column {
            width: parent.width; spacing: 0

            Row2 {
                label: "Audible Bell"
                sublabel: "Play a system sound for alerts instead of staying silent"
                toggled: Accessibility.audibleBell
                onToggle: function(v) { Accessibility.setAudibleBell(v) }
            }
        }
    }

    Spacer {}
}
