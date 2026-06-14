import QtQuick
import QtQuick.Controls
import "../components"

PageBase {
    title: "Desktop and Dock"
    Component.onCompleted: Dock.refresh()

    // ── Helper components ─────────────────────────────────────────────────
    component Toggle: Rectangle {
        property bool value: false
        signal toggled(bool newVal)
        width: 36; height: 18; radius: 9
        color: value ? "#3B82F6" : Qt.rgba(0.5,0.5,0.5,0.4)
        Behavior on color { ColorAnimation { duration: 150 } }
        Rectangle {
            width: 14; height: 14; radius: 7; color: "white"; y: 2
            x: parent.value ? 20 : 2
            Behavior on x { NumberAnimation { duration: 150 } }
        }
        MouseArea {
            anchors.fill: parent; cursorShape: Qt.PointingHandCursor
            onClicked: parent.toggled(!parent.value)
        }
    }

    component SettingRow: Item {
        property alias label: lbl.text
        default property alias content: contentSlot.data
        width: parent ? parent.width : 0; height: 44
        Text {
            id: lbl
            anchors.left: parent.left; anchors.verticalCenter: parent.verticalCenter
            font.pixelSize: 13; color: Theme.textPrimary
        }
        Item {
            id: contentSlot
            anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter
            width: childrenRect.width; height: parent.height
        }
    }

    component SliderRow: Item {
        property alias label: rowLabel.text
        property real from: 0; property real to: 100; property real step: 1
        property real value: 0
        property string unit: ""
        signal moved(real newVal)
        width: parent ? parent.width : 0; height: 44
        Text {
            id: rowLabel
            anchors.left: parent.left; anchors.verticalCenter: parent.verticalCenter
            font.pixelSize: 13; color: Theme.textPrimary
        }
        Row {
            anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter
            spacing: 8
            Slider {
                id: sl; from: parent.parent.from; to: parent.parent.to
                stepSize: parent.parent.step
                value: parent.parent.value
                width: 130; height: 20
                onMoved: parent.parent.moved(value)
                background: Rectangle {
                    x: sl.leftPadding; y: sl.topPadding + sl.availableHeight / 2 - height / 2
                    width: sl.availableWidth; height: 4; radius: 2
                    color: Qt.rgba(0.5,0.5,0.5,0.3)
                    Rectangle {
                        width: sl.visualPosition * parent.width; height: parent.height
                        color: "#3B82F6"; radius: 2
                    }
                }
                handle: Rectangle {
                    x: sl.leftPadding + sl.visualPosition * (sl.availableWidth - width)
                    y: sl.topPadding + sl.availableHeight / 2 - height / 2
                    width: 18; height: 18; radius: 9; color: "white"
                    layer.enabled: true
                    layer.effect: null
                    border.color: Qt.rgba(0,0,0,0.15); border.width: 1
                }
            }
            Text {
                width: 42; anchors.verticalCenter: parent.verticalCenter
                text: {
                    var v = sl.value
                    if (parent.parent.step >= 1) return Math.round(v) + parent.parent.unit
                    return v.toFixed(1) + parent.parent.unit
                }
                font.pixelSize: 12; color: Theme.textSecondary
                horizontalAlignment: Text.AlignRight
            }
        }
    }

    // ══════════════════════════════════════════════════════════════════════
    // SECTION 1: DOCK
    // ══════════════════════════════════════════════════════════════════════
    Text {
        text: "Dock"; font.pixelSize: 13; font.weight: Font.Medium
        color: Theme.textSecondary; bottomPadding: 4
    }

    SettingsCard {
        Column {
            width: parent.width; spacing: 0

            // Skin
            Item {
                width: parent.width; height: 44
                Text {
                    anchors.left: parent.left; anchors.verticalCenter: parent.verticalCenter
                    text: "Skin"; font.pixelSize: 13; color: Theme.textPrimary
                }
                ComboBox {
                    anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter
                    model: Dock.availableSkins
                    width: 150; height: 28; font.pixelSize: 12
                    currentIndex: Dock.availableSkins.indexOf(Dock.skinName)
                    onActivated: Dock.set("skinName", model[currentIndex])
                }
            }

            Rectangle { width: parent.width; height: 1; color: Theme.divider }

            // Icon Size
            SliderRow {
                label: "Size"; from: 32; to: 64; step: 2; unit: "px"
                value: Dock.iconSize
                onMoved: (v) => Dock.set("iconSize", Math.round(v))
            }

            Rectangle { width: parent.width; height: 1; color: Theme.divider }

            // Magnification toggle + strength
            Item {
                width: parent.width; height: 44
                Text {
                    anchors.left: parent.left; anchors.verticalCenter: parent.verticalCenter
                    text: "Magnification"; font.pixelSize: 13; color: Theme.textPrimary
                }
                Row {
                    anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter
                    spacing: 10
                    Text {
                        anchors.verticalCenter: parent.verticalCenter
                        text: Math.round(Dock.magnification) + "%"
                        font.pixelSize: 12; color: Theme.textSecondary; width: 36
                        horizontalAlignment: Text.AlignRight
                    }
                    Toggle {
                        value: Dock.magnification > 0
                        onToggled: (v) => Dock.set("magnification", v ? 80.0 : 0.0)
                    }
                }
            }

            Rectangle {
                width: parent.width; height: 1; color: Theme.divider
                visible: Dock.magnification > 0
            }

            SliderRow {
                visible: Dock.magnification > 0
                label: "Zoom amount"; from: 0; to: 100; step: 5; unit: "%"
                value: Dock.magnification
                onMoved: (v) => Dock.set("magnification", v)
            }

            Rectangle { width: parent.width; height: 1; color: Theme.divider }

            SliderRow {
                label: "Zoom amplitude"; from: 1.1; to: 2.5; step: 0.1
                value: Dock.amplitud
                onMoved: (v) => Dock.set("amplitud", v)
            }

            Rectangle { width: parent.width; height: 1; color: Theme.divider }

            // Icon spacing
            Item {
                width: parent.width; height: 44
                Text {
                    anchors.left: parent.left; anchors.verticalCenter: parent.verticalCenter
                    text: "Icon spacing"; font.pixelSize: 13; color: Theme.textPrimary
                }
                ComboBox {
                    anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter
                    model: ["Small", "Normal", "Large"]
                    width: 110; height: 28; font.pixelSize: 12
                    currentIndex: Dock.iconSpacing === 0 ? 0 : Dock.iconSpacing === 1 ? 1 : 2
                    onActivated: Dock.set("iconSpacing", [0, 1, 3][currentIndex])
                }
            }

            Rectangle { width: parent.width; height: 1; color: Theme.divider }

            // Show reflection
            Item {
                width: parent.width; height: 44
                Text {
                    anchors.left: parent.left; anchors.verticalCenter: parent.verticalCenter
                    text: "Show reflection"; font.pixelSize: 13; color: Theme.textPrimary
                }
                Toggle {
                    anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter
                    value: Dock.showReflection
                    onToggled: (v) => Dock.set("showReflection", v)
                }
            }
        }
    }

    Spacer {}

    // ══════════════════════════════════════════════════════════════════════
    // SECTION 2: VISUAL EFFECTS
    // ══════════════════════════════════════════════════════════════════════
    Text {
        text: "Visual Effects"; font.pixelSize: 13; font.weight: Font.Medium
        color: Theme.textSecondary; bottomPadding: 4
    }

    SettingsCard {
        Column {
            width: parent.width; spacing: 0

            Item {
                width: parent.width; height: 44
                Text {
                    anchors.left: parent.left; anchors.verticalCenter: parent.verticalCenter
                    text: "Background blur"; font.pixelSize: 13; color: Theme.textPrimary
                }
                Toggle {
                    anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter
                    value: Dock.skinBlur
                    onToggled: (v) => Dock.set("skinBlur", v)
                }
            }

            Rectangle { width: parent.width; height: 1; color: Theme.divider; visible: Dock.skinBlur }

            SliderRow {
                visible: Dock.skinBlur
                label: "Blur radius"; from: 0; to: 64; step: 1; unit: "px"
                value: Dock.skinBlurRadius
                onMoved: (v) => Dock.set("skinBlurRadius", Math.round(v))
            }

            Rectangle { width: parent.width; height: 1; color: Theme.divider }

            Item {
                width: parent.width; height: 44
                Text {
                    anchors.left: parent.left; anchors.verticalCenter: parent.verticalCenter
                    text: "Liquid gel effect"; font.pixelSize: 13; color: Theme.textPrimary
                }
                Toggle {
                    anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter
                    value: Dock.skinLiquidGelEffect
                    onToggled: (v) => Dock.set("skinLiquidGelEffect", v)
                }
            }

            Rectangle { width: parent.width; height: 1; color: Theme.divider; visible: Dock.skinLiquidGelEffect }

            SliderRow {
                visible: Dock.skinLiquidGelEffect
                label: "Refraction strength"; from: 0; to: 20; step: 0.5
                value: Dock.skinRefractionStrength
                onMoved: (v) => Dock.set("skinRefractionStrength", v)
            }

            Rectangle { width: parent.width; height: 1; color: Theme.divider; visible: Dock.skinLiquidGelEffect }

            SliderRow {
                visible: Dock.skinLiquidGelEffect
                label: "RGB fringing"; from: 0; to: 8; step: 0.1
                value: Dock.skinRgbFringing
                onMoved: (v) => Dock.set("skinRgbFringing", v)
            }

            Rectangle { width: parent.width; height: 1; color: Theme.divider }

            // Task indicator position
            Item {
                width: parent.width; height: 44
                Text {
                    anchors.left: parent.left; anchors.verticalCenter: parent.verticalCenter
                    text: "Task indicator position"; font.pixelSize: 13; color: Theme.textPrimary
                }
                ComboBox {
                    anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter
                    model: ["None", "Top", "Center", "Bottom"]
                    width: 110; height: 28; font.pixelSize: 12
                    currentIndex: Math.min(Dock.skinPositionTaskIndicator, 3)
                    onActivated: Dock.set("skinPositionTaskIndicator", currentIndex)
                }
            }
        }
    }

    Spacer {}

    // ══════════════════════════════════════════════════════════════════════
    // SECTION 3: INSIDE MARGINS
    // ══════════════════════════════════════════════════════════════════════
    Text {
        text: "Inside Margins"; font.pixelSize: 13; font.weight: Font.Medium
        color: Theme.textSecondary; bottomPadding: 4
    }

    SettingsCard {
        Column {
            width: parent.width; spacing: 0

            SliderRow { label: "Left";   from: 0; to: 60; step: 1; unit: "px"; value: Dock.skinLeftMargin   ?? 20; onMoved: (v) => Dock.set("skinLeftMargin",   Math.round(v)) }
            Rectangle { width: parent.width; height: 1; color: Theme.divider }
            SliderRow { label: "Top";    from: 0; to: 60; step: 1; unit: "px"; value: Dock.skinTopMargin    ?? 20; onMoved: (v) => Dock.set("skinTopMargin",    Math.round(v)) }
            Rectangle { width: parent.width; height: 1; color: Theme.divider }
            SliderRow { label: "Right";  from: 0; to: 60; step: 1; unit: "px"; value: Dock.skinRightMargin  ?? 20; onMoved: (v) => Dock.set("skinRightMargin",  Math.round(v)) }
            Rectangle { width: parent.width; height: 1; color: Theme.divider }
            SliderRow { label: "Bottom"; from: 0; to: 60; step: 1; unit: "px"; value: Dock.skinBottomMargin ?? 20; onMoved: (v) => Dock.set("skinBottomMargin", Math.round(v)) }
        }
    }

    Spacer {}

    // ══════════════════════════════════════════════════════════════════════
    // SECTION 4: OUTSIDE MARGINS
    // ══════════════════════════════════════════════════════════════════════
    Text {
        text: "Outside Margins"; font.pixelSize: 13; font.weight: Font.Medium
        color: Theme.textSecondary; bottomPadding: 4
    }

    SettingsCard {
        Column {
            width: parent.width; spacing: 0

            SliderRow { label: "Left";   from: -30; to: 60; step: 1; unit: "px"; value: Dock.skinOutsideLeftMargin   ?? 20; onMoved: (v) => Dock.set("skinOutsideLeftMargin",   Math.round(v)) }
            Rectangle { width: parent.width; height: 1; color: Theme.divider }
            SliderRow { label: "Top";    from: -30; to: 60; step: 1; unit: "px"; value: Dock.skinOutsideTopMargin    ?? 0;  onMoved: (v) => Dock.set("skinOutsideTopMargin",    Math.round(v)) }
            Rectangle { width: parent.width; height: 1; color: Theme.divider }
            SliderRow { label: "Right";  from: -30; to: 60; step: 1; unit: "px"; value: Dock.skinOutsideRightMargin  ?? 20; onMoved: (v) => Dock.set("skinOutsideRightMargin",  Math.round(v)) }
            Rectangle { width: parent.width; height: 1; color: Theme.divider }
            SliderRow { label: "Bottom"; from: -30; to: 60; step: 1; unit: "px"; value: Dock.skinOutsideBottomMargin ?? -10; onMoved: (v) => Dock.set("skinOutsideBottomMargin", Math.round(v)) }
        }
    }

    Spacer {}

    // ══════════════════════════════════════════════════════════════════════
    // SECTION 5: BEHAVIOR
    // ══════════════════════════════════════════════════════════════════════
    Text {
        text: "Behavior"; font.pixelSize: 13; font.weight: Font.Medium
        color: Theme.textSecondary; bottomPadding: 4
    }

    SettingsCard {
        Column {
            width: parent.width; spacing: 0

            Item {
                width: parent.width; height: 44
                Text { anchors.left: parent.left; anchors.verticalCenter: parent.verticalCenter; text: "Minimize on click"; font.pixelSize: 13; color: Theme.textPrimary }
                Toggle { anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter; value: Dock.minimizeActiveTaskOnClick; onToggled: (v) => Dock.set("minimizeActiveTaskOnClick", v) }
            }
            Rectangle { width: parent.width; height: 1; color: Theme.divider }

            Item {
                width: parent.width; height: 44
                Text { anchors.left: parent.left; anchors.verticalCenter: parent.verticalCenter; text: "Show window previews"; font.pixelSize: 13; color: Theme.textPrimary }
                Toggle { anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter; value: Dock.showToolTips; onToggled: (v) => Dock.set("showToolTips", v) }
            }
            Rectangle { width: parent.width; height: 1; color: Theme.divider }

            Item {
                width: parent.width; height: 44
                Text { anchors.left: parent.left; anchors.verticalCenter: parent.verticalCenter; text: "Highlight focused window"; font.pixelSize: 13; color: Theme.textPrimary }
                Toggle { anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter; value: Dock.highlightWindows; onToggled: (v) => Dock.set("highlightWindows", v) }
            }
            Rectangle { width: parent.width; height: 1; color: Theme.divider }

            Item {
                width: parent.width; height: 44
                Text { anchors.left: parent.left; anchors.verticalCenter: parent.verticalCenter; text: "Audio stream indicator"; font.pixelSize: 13; color: Theme.textPrimary }
                Toggle { anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter; value: Dock.indicateAudioStreams; onToggled: (v) => Dock.set("indicateAudioStreams", v) }
            }
            Rectangle { width: parent.width; height: 1; color: Theme.divider }

            Item {
                width: parent.width; height: 44
                Text { anchors.left: parent.left; anchors.verticalCenter: parent.verticalCenter; text: "Mute via audio indicator"; font.pixelSize: 13; color: Theme.textPrimary }
                Toggle { anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter; value: Dock.interactiveMute; onToggled: (v) => Dock.set("interactiveMute", v) }
            }
            Rectangle { width: parent.width; height: 1; color: Theme.divider }

            Item {
                width: parent.width; height: 44
                Text { anchors.left: parent.left; anchors.verticalCenter: parent.verticalCenter; text: "Media controls in tooltip"; font.pixelSize: 13; color: Theme.textPrimary }
                Toggle { anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter; value: Dock.tooltipControls; onToggled: (v) => Dock.set("tooltipControls", v) }
            }
            Rectangle { width: parent.width; height: 1; color: Theme.divider }

            // Middle click action
            Item {
                width: parent.width; height: 44
                Text { anchors.left: parent.left; anchors.verticalCenter: parent.verticalCenter; text: "Middle-click"; font.pixelSize: 13; color: Theme.textPrimary }
                ComboBox {
                    anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter
                    model: ["Nothing", "Close", "New window", "Minimize/Restore", "Toggle grouping", "Move to desktop"]
                    width: 150; height: 28; font.pixelSize: 12
                    currentIndex: Dock.middleClickAction
                    onActivated: Dock.set("middleClickAction", currentIndex)
                }
            }
            Rectangle { width: parent.width; height: 1; color: Theme.divider }

            // Scroll behavior
            Item {
                width: parent.width; height: 44
                Text { anchors.left: parent.left; anchors.verticalCenter: parent.verticalCenter; text: "Scroll wheel"; font.pixelSize: 13; color: Theme.textPrimary }
                ComboBox {
                    anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter
                    model: ["Nothing", "Cycle all tasks", "Cycle task windows", "Adjust volume"]
                    width: 160; height: 28; font.pixelSize: 12
                    currentIndex: Dock.wheelEnabled
                    onActivated: Dock.set("wheelEnabled", currentIndex)
                }
            }
            Rectangle { width: parent.width; height: 1; color: Theme.divider; visible: Dock.wheelEnabled > 0 }

            Item {
                visible: Dock.wheelEnabled > 0
                width: parent.width; height: 44
                Text { anchors.left: parent.left; anchors.verticalCenter: parent.verticalCenter; text: "Skip minimized on scroll"; font.pixelSize: 13; color: Theme.textPrimary }
                Toggle { anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter; value: Dock.wheelSkipMinimized; onToggled: (v) => Dock.set("wheelSkipMinimized", v) }
            }
        }
    }

    Spacer {}

    // ══════════════════════════════════════════════════════════════════════
    // SECTION 6: GROUPING & SORTING
    // ══════════════════════════════════════════════════════════════════════
    Text {
        text: "Grouping & Sorting"; font.pixelSize: 13; font.weight: Font.Medium
        color: Theme.textSecondary; bottomPadding: 4
    }

    SettingsCard {
        Column {
            width: parent.width; spacing: 0

            Item {
                width: parent.width; height: 44
                Text { anchors.left: parent.left; anchors.verticalCenter: parent.verticalCenter; text: "Group windows"; font.pixelSize: 13; color: Theme.textPrimary }
                ComboBox {
                    anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter
                    model: ["Don't group", "By program name"]
                    width: 160; height: 28; font.pixelSize: 12
                    currentIndex: Dock.groupingStrategy
                    onActivated: Dock.set("groupingStrategy", currentIndex)
                }
            }
            Rectangle { width: parent.width; height: 1; color: Theme.divider }

            Item {
                width: parent.width; height: 44
                Text { anchors.left: parent.left; anchors.verticalCenter: parent.verticalCenter; text: "Clicking grouped task"; font.pixelSize: 13; color: Theme.textPrimary }
                ComboBox {
                    anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter
                    model: ["Cycles through tasks", "Small previews", "Large previews", "Textual list"]
                    width: 160; height: 28; font.pixelSize: 12
                    currentIndex: Dock.groupedTaskVisualization
                    enabled: Dock.groupingStrategy > 0
                    onActivated: Dock.set("groupedTaskVisualization", currentIndex)
                }
            }
            Rectangle { width: parent.width; height: 1; color: Theme.divider }

            Item {
                width: parent.width; height: 44
                Text { anchors.left: parent.left; anchors.verticalCenter: parent.verticalCenter; text: "Sort tasks"; font.pixelSize: 13; color: Theme.textPrimary }
                ComboBox {
                    anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter
                    model: ["Don't sort", "Manually", "Alphabetically", "By desktop", "By activity", "By position"]
                    width: 150; height: 28; font.pixelSize: 12
                    currentIndex: Dock.sortingStrategy
                    onActivated: Dock.set("sortingStrategy", currentIndex)
                }
            }
        }
    }

    Spacer {}

    // ══════════════════════════════════════════════════════════════════════
    // SECTION 7: TASK VISIBILITY
    // ══════════════════════════════════════════════════════════════════════
    Text {
        text: "Show Tasks From"; font.pixelSize: 13; font.weight: Font.Medium
        color: Theme.textSecondary; bottomPadding: 4
    }

    SettingsCard {
        Column {
            width: parent.width; spacing: 0

            Item {
                width: parent.width; height: 44
                Text { anchors.left: parent.left; anchors.verticalCenter: parent.verticalCenter; text: "Current desktop only"; font.pixelSize: 13; color: Theme.textPrimary }
                Toggle { anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter; value: Dock.showOnlyCurrentDesktop; onToggled: (v) => Dock.set("showOnlyCurrentDesktop", v) }
            }
            Rectangle { width: parent.width; height: 1; color: Theme.divider }

            Item {
                width: parent.width; height: 44
                Text { anchors.left: parent.left; anchors.verticalCenter: parent.verticalCenter; text: "Current activity only"; font.pixelSize: 13; color: Theme.textPrimary }
                Toggle { anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter; value: Dock.showOnlyCurrentActivity; onToggled: (v) => Dock.set("showOnlyCurrentActivity", v) }
            }
            Rectangle { width: parent.width; height: 1; color: Theme.divider }

            Item {
                width: parent.width; height: 44
                Text { anchors.left: parent.left; anchors.verticalCenter: parent.verticalCenter; text: "Current screen only"; font.pixelSize: 13; color: Theme.textPrimary }
                Toggle { anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter; value: Dock.showOnlyCurrentScreen; onToggled: (v) => Dock.set("showOnlyCurrentScreen", v) }
            }
            Rectangle { width: parent.width; height: 1; color: Theme.divider }

            Item {
                width: parent.width; height: 44
                Text { anchors.left: parent.left; anchors.verticalCenter: parent.verticalCenter; text: "Minimized only"; font.pixelSize: 13; color: Theme.textPrimary }
                Toggle { anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter; value: Dock.showOnlyMinimized; onToggled: (v) => Dock.set("showOnlyMinimized", v) }
            }
            Rectangle { width: parent.width; height: 1; color: Theme.divider }

            Item {
                width: parent.width; height: 44
                Text { anchors.left: parent.left; anchors.verticalCenter: parent.verticalCenter; text: "Unhide when window needs attention"; font.pixelSize: 13; color: Theme.textPrimary }
                Toggle { anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter; value: Dock.unhideOnAttention; onToggled: (v) => Dock.set("unhideOnAttention", v) }
            }
        }
    }

    Spacer {}
}
