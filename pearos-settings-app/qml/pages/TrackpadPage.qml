import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "../components"

PageBase {
    title: "Trackpad"
    property int activeTab: 0
    readonly property string ap: "file:///usr/share/extras/system-settings/assets/"
    Component.onCompleted: Trackpad.refresh()

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

    // GIFs section
    SettingsCard {
        Row {
            width: parent.width; height: 120; spacing: 16
            AnimatedImage { source: ap + "touch.gif"; width: (parent.width - 16) / 2; height: 120; fillMode: Image.PreserveAspectFit; playing: true }
            AnimatedImage { source: ap + "touch1.gif"; width: (parent.width - 16) / 2; height: 120; fillMode: Image.PreserveAspectFit; playing: true }
        }
    }
    Spacer {}

    // Tabs
    SettingsCard {
        Row {
            width: parent.width; height: 36; spacing: 0
            Repeater {
                model: ["Point & Click", "Scroll & Zoom", "More Gestures"]
                delegate: Rectangle {
                    width: parent.width / 3; height: 36; radius: 6
                    color: activeTab === index ? Qt.rgba(0,0,0,0.1) : "transparent"
                    Text { anchors.centerIn: parent; text: modelData; font.pixelSize: 12; font.weight: Font.Medium; color: Theme.textPrimary }
                    MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor; onClicked: activeTab = index }
                }
            }
        }
    }
    Spacer {}

    // ── Point & Click ──────────────────────────────────────────────────────
    SettingsCard {
        visible: activeTab === 0
        Column {
            width: parent.width; spacing: 0

            Row2 {
                label: "Enable Trackpad"
                toggled: Trackpad.enabled
                onToggle: function(v) { Trackpad.setEnabled(v) }
            }
            Rectangle { width: parent.width; height: 1; color: Theme.divider }

            Row2 {
                label: "Tap to Click"
                toggled: Trackpad.tapToClick
                onToggle: function(v) { Trackpad.setTapToClick(v) }
            }
            Rectangle { width: parent.width; height: 1; color: Theme.divider }

            Item {
                width: parent.width; height: 44
                Text { anchors.left: parent.left; anchors.verticalCenter: parent.verticalCenter; text: "Click Method"; font.pixelSize: 12; font.weight: Font.DemiBold; color: Theme.textPrimary }
                Row {
                    anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter; spacing: 6
                    Repeater {
                        model: [{label: "Button Areas", val: 1}, {label: "Clickfinger", val: 2}]
                        delegate: Rectangle {
                            height: 28; width: cmLbl.implicitWidth + 16; radius: 6
                            color: Trackpad.clickMethod === modelData.val ? Theme.accent : Qt.rgba(0,0,0,0.08)
                            Text { id: cmLbl; anchors.centerIn: parent; text: modelData.label; font.pixelSize: 12
                                color: Trackpad.clickMethod === modelData.val ? "white" : Theme.textPrimary }
                            MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor
                                onClicked: Trackpad.setClickMethod(modelData.val) }
                        }
                    }
                }
            }
            Rectangle { width: parent.width; height: 1; color: Theme.divider }

            Row2 {
                label: "Left Handed"
                toggled: Trackpad.leftHanded
                onToggle: function(v) { Trackpad.setLeftHanded(v) }
            }
            Rectangle { width: parent.width; height: 1; color: Theme.divider }

            Column {
                width: parent.width; spacing: 6; topPadding: 10; bottomPadding: 10
                Text { text: "Pointer Acceleration"; font.pixelSize: 12; font.weight: Font.DemiBold; color: Theme.textPrimary }
                Row {
                    width: parent.width; spacing: 8
                    Text { text: "-1"; font.pixelSize: 11; color: Theme.textSecondary; anchors.verticalCenter: parent.verticalCenter }
                    Slider {
                        from: -1; to: 1; stepSize: 0.05
                        value: Trackpad.pointerAcceleration
                        palette.accent: Theme.accent; width: parent.width - 80
                        anchors.verticalCenter: parent.verticalCenter
                        onMoved: Trackpad.setPointerAcceleration(Math.round(value * 20) / 20)
                    }
                    Text { text: "+1"; font.pixelSize: 11; color: Theme.textSecondary; anchors.verticalCenter: parent.verticalCenter }
                    Text { text: Trackpad.pointerAcceleration.toFixed(2); font.pixelSize: 11; color: Theme.textSecondary; width: 36; horizontalAlignment: Text.AlignRight; anchors.verticalCenter: parent.verticalCenter }
                }
            }
            Rectangle { width: parent.width; height: 1; color: Theme.divider }

            Item {
                width: parent.width; height: 44
                Text { anchors.left: parent.left; anchors.verticalCenter: parent.verticalCenter; text: "Acceleration Profile"; font.pixelSize: 12; font.weight: Font.DemiBold; color: Theme.textPrimary }
                Row {
                    anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter; spacing: 6
                    Repeater {
                        model: [{label: "Flat", val: 1}, {label: "Adaptive", val: 2}]
                        delegate: Rectangle {
                            height: 28; width: apLbl.implicitWidth + 16; radius: 6
                            color: Trackpad.accelProfile === modelData.val ? Theme.accent : Qt.rgba(0,0,0,0.08)
                            Text { id: apLbl; anchors.centerIn: parent; text: modelData.label; font.pixelSize: 12
                                color: Trackpad.accelProfile === modelData.val ? "white" : Theme.textPrimary }
                            MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor
                                onClicked: Trackpad.setAccelProfile(modelData.val) }
                        }
                    }
                }
            }
        }
    }

    // ── Scroll & Zoom ──────────────────────────────────────────────────────
    SettingsCard {
        visible: activeTab === 1
        Column {
            width: parent.width; spacing: 0

            Row2 {
                label: "Natural Scrolling"
                toggled: Trackpad.naturalScroll
                onToggle: function(v) { Trackpad.setNaturalScroll(v) }
            }
            Rectangle { width: parent.width; height: 1; color: Theme.divider }

            Item {
                width: parent.width; height: 44
                Text { anchors.left: parent.left; anchors.verticalCenter: parent.verticalCenter; text: "Scroll Method"; font.pixelSize: 12; font.weight: Font.DemiBold; color: Theme.textPrimary }
                Row {
                    anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter; spacing: 6
                    Repeater {
                        model: [{label: "Two Finger", val: 1}, {label: "Edge", val: 2}]
                        delegate: Rectangle {
                            height: 28; width: smLbl.implicitWidth + 16; radius: 6
                            color: Trackpad.scrollMethod === modelData.val ? Theme.accent : Qt.rgba(0,0,0,0.08)
                            Text { id: smLbl; anchors.centerIn: parent; text: modelData.label; font.pixelSize: 12
                                color: Trackpad.scrollMethod === modelData.val ? "white" : Theme.textPrimary }
                            MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor
                                onClicked: Trackpad.setScrollMethod(modelData.val) }
                        }
                    }
                }
            }
            Rectangle { width: parent.width; height: 1; color: Theme.divider }

            Column {
                width: parent.width; spacing: 6; topPadding: 10; bottomPadding: 10
                Text { text: "Scroll Speed"; font.pixelSize: 12; font.weight: Font.DemiBold; color: Theme.textPrimary }
                Row {
                    width: parent.width; spacing: 8
                    Text { text: "Slow"; font.pixelSize: 11; color: Theme.textSecondary; anchors.verticalCenter: parent.verticalCenter }
                    Slider {
                        from: 0.1; to: 5.0; stepSize: 0.1
                        value: Trackpad.scrollFactor
                        palette.accent: Theme.accent; width: parent.width - 100
                        anchors.verticalCenter: parent.verticalCenter
                        onMoved: Trackpad.setScrollFactor(Math.round(value * 10) / 10)
                    }
                    Text { text: "Fast"; font.pixelSize: 11; color: Theme.textSecondary; anchors.verticalCenter: parent.verticalCenter }
                    Text { text: Trackpad.scrollFactor.toFixed(1) + "×"; font.pixelSize: 11; color: Theme.textSecondary; width: 30; horizontalAlignment: Text.AlignRight; anchors.verticalCenter: parent.verticalCenter }
                }
            }
            Rectangle { width: parent.width; height: 1; color: Theme.divider }

            Row2 {
                label: "Disable While Typing"
                toggled: Trackpad.disableWhileTyping
                onToggle: function(v) { Trackpad.setDisableWhileTyping(v) }
            }
            Rectangle { width: parent.width; height: 1; color: Theme.divider }

            Row2 {
                label: "Disable on External Mouse"
                toggled: Trackpad.disableEventsOnExtMouse
                onToggle: function(v) { Trackpad.setDisableEventsOnExtMouse(v) }
            }
        }
    }

    // ── More Gestures ──────────────────────────────────────────────────────
    SettingsCard {
        visible: activeTab === 2
        Column {
            width: parent.width; spacing: 0

            Row2 {
                label: "Tap and Drag"
                toggled: Trackpad.tapAndDrag
                onToggle: function(v) { Trackpad.setTapAndDrag(v) }
            }
            Rectangle { width: parent.width; height: 1; color: Theme.divider }

            Row2 {
                label: "Tap Drag Lock"
                toggled: Trackpad.tapDragLock
                onToggle: function(v) { Trackpad.setTapDragLock(v) }
            }
            Rectangle { width: parent.width; height: 1; color: Theme.divider }

            Row2 {
                label: "Middle Button Emulation"
                toggled: Trackpad.middleEmulation
                onToggle: function(v) { Trackpad.setMiddleEmulation(v) }
            }
            Rectangle { width: parent.width; height: 1; color: Theme.divider }

            Row2 {
                label: "Three-Finger Tap as Right Click"
                toggled: Trackpad.lmrTapButtonMap
                onToggle: function(v) { Trackpad.setLmrTapButtonMap(v) }
            }
        }
    }

    Spacer {}
}
