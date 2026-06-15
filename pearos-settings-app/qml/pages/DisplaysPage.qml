import QtQuick
import QtQuick.Controls
import "../components"

PageBase {
    title: "Displays"
    Component.onCompleted: { Display.refreshDisplays(); Display.refreshBrightness("") }

    readonly property string ap: "file:///usr/share/extras/system-settings/assets/"

    // Page-level selection state
    property int  selectedDispIdx: 0
    property int  previousModeId:  -1
    property bool revertVisible:   false

    property var selDisp: Display.displays.length > selectedDispIdx
                          ? Display.displays[selectedDispIdx] : null

    // Unique resolutions for selected display (sorted by pixels desc)
    property var uniqueRes: {
        if (!selDisp) return []
        var modes = selDisp.modes || []
        var sorted = modes.slice().sort(function(a, b) {
            return (b.width * b.height) - (a.width * a.height)
        })
        var seen = {}; var arr = []
        for (var i = 0; i < sorted.length; i++) {
            var key = sorted[i].width + "x" + sorted[i].height
            if (!seen[key]) { seen[key] = true; arr.push(sorted[i]) }
        }
        return arr
    }

    // Resolve preset → best mode for that resolution slot
    function modeForPreset(preset) {
        var u = uniqueRes
        if (!u || u.length === 0) return null
        var target
        if (preset === "default") target = u[0]              // highest res
        else if (preset === "larger") target = u[u.length-1] // lowest res
        else target = u[Math.floor(u.length / 2)]            // medium
        if (!target) return null
        // Find the mode ID with the highest refresh rate for this resolution
        var modes = (selDisp && selDisp.modes) || []
        var best = null
        for (var i = 0; i < modes.length; i++) {
            var m = modes[i]
            if (m.width === target.width && m.height === target.height) {
                if (!best || m.refreshRate > best.refreshRate) best = m
            }
        }
        return best
    }

    // Which preset is currently active?
    property string activePreset: {
        if (!selDisp || !uniqueRes || uniqueRes.length === 0) return "default"
        var cw = selDisp.currentWidth, ch = selDisp.currentHeight
        if (uniqueRes.length === 1) return "default"
        var u = uniqueRes
        // default = first (highest res)
        if (cw === u[0].width && ch === u[0].height) return "default"
        // larger = last (lowest res)
        if (cw === u[u.length-1].width && ch === u[u.length-1].height) return "larger"
        return "medium"
    }

    function applyPreset(preset) {
        if (!selDisp) return
        var m = modeForPreset(preset)
        if (!m) return
        previousModeId = selDisp.currentModeId
        Display.setMode(selDisp.name, m.id)
        revertVisible = true
        revertTimer.secondsLeft = 15
        revertTimer.start()
    }

    // Rate model for selected display and currently selected resolution
    property var rateModel: {
        if (!selDisp) return []
        var cw = selDisp.currentWidth, ch = selDisp.currentHeight
        var modes = selDisp.modes || []
        var arr = []
        for (var i = 0; i < modes.length; i++) {
            var m = modes[i]
            if (m.width === cw && m.height === ch) {
                arr.push(m)
            }
        }
        arr.sort(function(a, b) { return b.refreshRate - a.refreshRate })
        return arr
    }

    // ── Revert timer (wrapped in Item so it satisfies QQuickItem pageContent) ──
    Item {
        width: 0; height: 0
        Timer {
            id: revertTimer
            interval: 1000; repeat: true
            property int secondsLeft: 15
            onTriggered: {
                secondsLeft--
                if (secondsLeft <= 0) {
                    stop()
                    if (previousModeId >= 0 && selDisp)
                        Display.setMode(selDisp.name, previousModeId)
                    revertVisible = false
                }
            }
            onRunningChanged: if (!running && secondsLeft > 0) revertVisible = false
        }
    }

    // ── Display list card ─────────────────────────────────────────────────
    SettingsCard {
        Column {
            width: parent.width; spacing: 0

            Item { width: parent.width; height: 8 }
            Text {
                text: "Displays"; font.pixelSize: 14; font.weight: Font.DemiBold
                color: Theme.textPrimary; bottomPadding: 4; leftPadding: 0
            }
            Item { width: parent.width; height: 4 }

            // Loading state
            Item {
                width: parent.width; height: 48
                visible: Display.displays.length === 0
                Text { anchors.centerIn: parent; text: "Loading displays…"
                    font.pixelSize: 13; color: Theme.textSecondary }
            }

            Repeater {
                model: Display.displays
                delegate: Column {
                    width: parent.width; spacing: 0

                    // Divider before each item except first
                    Rectangle {
                        width: parent.width; height: 1; color: Theme.divider
                        visible: index > 0
                    }

                    // Display row
                    Rectangle {
                        width: parent.width; height: 56
                        radius: 6
                        color: selectedDispIdx === index ? Theme.activeBg : "transparent"
                        Behavior on color { ColorAnimation { duration: 150 } }

                        MouseArea {
                            anchors.fill: parent; cursorShape: Qt.PointingHandCursor
                            onClicked: selectedDispIdx = index
                        }

                        Row {
                            anchors.left: parent.left; anchors.leftMargin: 4
                            anchors.verticalCenter: parent.verticalCenter; spacing: 12

                            Image {
                                source: modelData.isBuiltIn ? ap + "laptop.svg" : ap + "computer.svg"
                                width: 24; height: 24; fillMode: Image.PreserveAspectFit
                                opacity: 0.8; anchors.verticalCenter: parent.verticalCenter
                            }

                            Column {
                                spacing: 3; anchors.verticalCenter: parent.verticalCenter
                                Row {
                                    spacing: 6
                                    Text {
                                        text: modelData.isBuiltIn ? "Built-in Display" : modelData.name
                                        font.pixelSize: 13; font.weight: Font.Medium; color: Theme.textPrimary
                                    }
                                    Rectangle {
                                        visible: modelData.isPrimary
                                        width: mainLbl.implicitWidth + 10; height: 16; radius: 3
                                        color: Theme.bgCard; border.color: Theme.divider; border.width: 1
                                        anchors.verticalCenter: parent.verticalCenter
                                        Text { id: mainLbl; anchors.centerIn: parent; text: "main"
                                            font.pixelSize: 9; font.weight: Font.SemiBold
                                            color: Theme.textSecondary }
                                    }
                                }
                                Text {
                                    text: {
                                        var w = modelData.currentWidth, h = modelData.currentHeight
                                        if (!w) return "Unknown"
                                        var hz = modelData.currentRefreshRate
                                        var hzStr = hz % 1 < 0.05 ? Math.round(hz) : hz.toFixed(2)
                                        return w + " × " + h + "  @" + hzStr + " Hz"
                                    }
                                    font.pixelSize: 11; color: Theme.textSecondary
                                }
                            }
                        }

                        // Enable/Disable button
                        Rectangle {
                            anchors.right: parent.right; anchors.rightMargin: 4
                            anchors.verticalCenter: parent.verticalCenter
                            width: enLbl.implicitWidth + 24; height: 28; radius: 6
                            color: enHov.containsMouse ? Theme.hoverBg : Theme.bgCard
                            border.color: Theme.divider; border.width: 1
                            Text { id: enLbl; anchors.centerIn: parent
                                text: modelData.enabled ? "Disable" : "Enable"
                                font.pixelSize: 11; color: Theme.textPrimary }
                            MouseArea {
                                id: enHov; anchors.fill: parent; hoverEnabled: true
                                cursorShape: Qt.PointingHandCursor
                                onClicked: Display.setDisplayEnabled(modelData.name, !modelData.enabled)
                            }
                        }
                    }
                }
            }
            Item { width: parent.width; height: 4 }
        }
    }

    Spacer {}

    // ── Resolution + Brightness card (visible when display selected) ───────
    SettingsCard {
        visible: selDisp !== null

        Column {
            width: parent.width; spacing: 0

            // Section title row
            Item {
                width: parent.width; height: 44
                Text {
                    anchors.left: parent.left; anchors.verticalCenter: parent.verticalCenter
                    text: "Resolution"; font.pixelSize: 14; font.weight: Font.DemiBold
                    color: Theme.textPrimary
                }
            }

            // 3 visual preset thumbnails
            Item {
                width: parent.width
                height: 110

                Row {
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.verticalCenter: parent.verticalCenter
                    spacing: 20

                    Repeater {
                        model: [
                            { preset: "larger",  label: "Larger Text" },
                            { preset: "medium",  label: "Medium" },
                            { preset: "default", label: "Default" }
                        ]

                        delegate: Item {
                            width: 90; height: 100

                            property bool isActive: activePreset === modelData.preset
                            property var mode: modeForPreset(modelData.preset)

                            Rectangle {
                                anchors.horizontalCenter: parent.horizontalCenter
                                width: 82; height: 100; radius: 8
                                color: isActive ? Qt.rgba(Theme.accent.r, Theme.accent.g, Theme.accent.b, 0.1) : "transparent"
                                border.color: isActive ? Theme.accent : "transparent"
                                border.width: isActive ? 2 : 0
                                Behavior on border.color { ColorAnimation { duration: 150 } }

                                Column {
                                    anchors.horizontalCenter: parent.horizontalCenter
                                    anchors.verticalCenter: parent.verticalCenter
                                    spacing: 8

                                    // Mini window preview
                                    Rectangle {
                                        width: 50; height: 34; radius: 4
                                        anchors.horizontalCenter: parent.horizontalCenter
                                        color: Theme.bgSearch
                                        border.color: Theme.divider; border.width: 1

                                        Column {
                                            anchors.fill: parent; spacing: 0

                                            // Traffic lights bar
                                            Rectangle {
                                                width: parent.width; height: 10; radius: 0
                                                color: "transparent"
                                                border.color: Theme.divider; border.width: 0
                                                topLeftRadius: 4; topRightRadius: 4
                                                Row {
                                                    anchors.left: parent.left; anchors.leftMargin: 4
                                                    anchors.verticalCenter: parent.verticalCenter
                                                    spacing: 3
                                                    Rectangle { width: 4; height: 4; radius: 2; color: "#ff5f57" }
                                                    Rectangle { width: 4; height: 4; radius: 2; color: "#febc2e" }
                                                    Rectangle { width: 4; height: 4; radius: 2; color: "#28ca42" }
                                                }
                                            }

                                            // Text area
                                            Item {
                                                width: parent.width
                                                height: parent.height - 10
                                                Text {
                                                    anchors.centerIn: parent
                                                    text: "Text"
                                                    font.pixelSize: modelData.preset === "larger" ? 7
                                                                   : modelData.preset === "medium" ? 5 : 4
                                                    color: Theme.textPrimary; font.weight: Font.Medium
                                                }
                                            }
                                        }
                                    }

                                    Text {
                                        anchors.horizontalCenter: parent.horizontalCenter
                                        text: modelData.label
                                        font.pixelSize: 11; font.weight: Font.Medium
                                        color: isActive ? Theme.accent : Theme.textPrimary
                                    }

                                    Text {
                                        anchors.horizontalCenter: parent.horizontalCenter
                                        text: mode ? (mode.width + "×" + mode.height) : "—"
                                        font.pixelSize: 9; color: Theme.textSecondary
                                    }
                                }

                                MouseArea {
                                    anchors.fill: parent; cursorShape: Qt.PointingHandCursor
                                    onClicked: applyPreset(modelData.preset)
                                }
                            }
                        }
                    }
                }
            }

            Rectangle { width: parent.width; height: 1; color: Theme.divider }

            // Refresh rate row
            Item {
                width: parent.width; height: 44
                Text {
                    anchors.left: parent.left; anchors.verticalCenter: parent.verticalCenter
                    text: "Refresh Rate"; font.pixelSize: 13; color: Theme.textPrimary
                }
                SettingsComboBox {
                    id: rateCombo
                    anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter
                    width: 130; height: 30; font.pixelSize: 12

                    model: rateModel.map(function(m) {
                        var hz = m.refreshRate % 1 < 0.05 ? Math.round(m.refreshRate) : m.refreshRate.toFixed(2)
                        return hz + " Hz"
                    })
                    currentIndex: {
                        var sel = selDisp ? selDisp.currentModeId : -1
                        for (var i = 0; i < rateModel.length; i++)
                            if (rateModel[i].id === sel) return i
                        return 0
                    }
                    onActivated: function(idx) {
                        if (!selDisp || idx >= rateModel.length) return
                        previousModeId = selDisp.currentModeId
                        Display.setMode(selDisp.name, rateModel[idx].id)
                        revertVisible = true
                        revertTimer.secondsLeft = 15
                        revertTimer.start()
                    }
                }
            }

            Rectangle { width: parent.width; height: 1; color: Theme.divider }

            // Scale row
            Item {
                width: parent.width; height: 44
                readonly property var opts: [
                    { label: "100%", value: 1.0 }, { label: "125%", value: 1.25 },
                    { label: "150%", value: 1.5  }, { label: "175%", value: 1.75 },
                    { label: "200%", value: 2.0  }
                ]
                Text {
                    anchors.left: parent.left; anchors.verticalCenter: parent.verticalCenter
                    text: "Display Scale"; font.pixelSize: 13; color: Theme.textPrimary
                }
                SettingsComboBox {
                    id: scaleCombo
                    anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter
                    width: 130; height: 30; font.pixelSize: 12
                    model: parent.opts.map(function(o) { return o.label })
                    currentIndex: {
                        var s = selDisp ? (selDisp.scale || 1.0) : 1.0
                        var o = parent.opts
                        for (var i = 0; i < o.length; i++)
                            if (Math.abs(o[i].value - s) < 0.1) return i
                        return 0
                    }
                    onActivated: function(idx) {
                        if (selDisp) Display.setScale(selDisp.name, parent.opts[idx].value)
                    }
                }
            }

            Rectangle { width: parent.width; height: 1; color: Theme.divider }

            // ── Brightness ──────────────────────────────────────────────
            Item {
                width: parent.width; height: 44
                Text {
                    anchors.left: parent.left; anchors.verticalCenter: parent.verticalCenter
                    text: "Brightness"; font.pixelSize: 13; font.weight: Font.Medium; color: Theme.textPrimary
                }
            }
            Item {
                width: parent.width; height: 48
                Row {
                    anchors.left: parent.left; anchors.right: parent.right
                    anchors.verticalCenter: parent.verticalCenter; spacing: 10
                    Text { text: "☀"; font.pixelSize: 13; color: Theme.textSecondary; anchors.verticalCenter: parent.verticalCenter }
                    Slider {
                        id: brightnessSlider
                        from: 0; to: 100; stepSize: 1
                        value: Display.brightnessValue
                        palette.accent: Theme.accent
                        width: parent.width - 60; anchors.verticalCenter: parent.verticalCenter
                        onMoved: Display.setBrightness(Math.round(value), "")
                    }
                    Text { text: "☀"; font.pixelSize: 20; color: Theme.textSecondary; anchors.verticalCenter: parent.verticalCenter }
                }
            }

            Rectangle { width: parent.width; height: 1; color: Theme.divider }

            // Night Light toggle
            Item {
                width: parent.width; height: 44
                Text {
                    anchors.left: parent.left; anchors.verticalCenter: parent.verticalCenter
                    text: "Night Light"; font.pixelSize: 13; color: Theme.textPrimary
                }
                LiquidToggle {
                    anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter
                    checked: Display.nightLightEnabled
                    onToggled: function(v) { Display.setNightLight(v) }
                }
            }
        }
    }

    Spacer {}

    // ── Revert card ───────────────────────────────────────────────────────
    SettingsCard {
        visible: revertVisible

        Column {
            width: parent.width; spacing: 12

            Item {
                width: parent.width; height: 24
                Text {
                    anchors.left: parent.left; anchors.verticalCenter: parent.verticalCenter
                    text: "Reverting your changes in " + revertTimer.secondsLeft + "s…"
                    font.pixelSize: 13; color: Theme.textPrimary
                }
            }

            Row {
                width: parent.width; spacing: 10

                Rectangle {
                    width: (parent.width - 10) / 2; height: 32; radius: 6
                    color: keepHov.containsMouse ? Theme.hoverBg : Theme.bgCard
                    border.color: Theme.divider; border.width: 1
                    Text { anchors.centerIn: parent; text: "Keep"; font.pixelSize: 12
                        font.weight: Font.Medium; color: Theme.textPrimary }
                    MouseArea { id: keepHov; anchors.fill: parent; hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: { revertTimer.stop(); revertTimer.secondsLeft = 0; revertVisible = false } }
                }

                Rectangle {
                    width: (parent.width - 10) / 2; height: 32; radius: 6
                    color: revHov.containsMouse ? Theme.hoverBg : Theme.bgCard
                    border.color: Theme.divider; border.width: 1
                    Text { anchors.centerIn: parent; text: "Revert"; font.pixelSize: 12
                        font.weight: Font.Medium; color: Theme.textPrimary }
                    MouseArea { id: revHov; anchors.fill: parent; hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            revertTimer.stop()
                            if (previousModeId >= 0 && selDisp)
                                Display.setMode(selDisp.name, previousModeId)
                            revertVisible = false
                        }
                    }
                }
            }
        }
    }

    Spacer {}
}
