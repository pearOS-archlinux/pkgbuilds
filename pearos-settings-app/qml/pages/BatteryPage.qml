import QtQuick
import QtQuick.Layouts
import "../components"

PageBase {
    title: "Battery"
    Component.onCompleted: { Battery.refresh(); Battery.refreshHistory(); Battery.refreshUsage() }

    // ── Status card ───────────────────────────────────────────────────────
    SettingsCard {
        Column {
            width: parent.width; spacing: 0
            Item { width: parent.width; height: 8 }
            Text { text: "Battery"; font.pixelSize: 14; font.weight: Font.DemiBold; color: Theme.textPrimary; bottomPadding: 12 }

            Item {
                width: parent.width; height: 36
                Text { anchors.left: parent.left; anchors.verticalCenter: parent.verticalCenter; text: "Status"; font.pixelSize: 12; color: Theme.textSecondary }
                Text {
                    anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter
                    text: !Battery.isPresent ? "No battery" : Battery.isCharging ? "Charging" : "On battery power"
                    font.pixelSize: 12; color: Theme.textPrimary
                }
            }
            Rectangle { width: parent.width; height: 1; color: Theme.divider }

            Item {
                width: parent.width; height: 36
                Text { anchors.left: parent.left; anchors.verticalCenter: parent.verticalCenter; text: "Battery Level"; font.pixelSize: 12; color: Theme.textSecondary }
                Row {
                    anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter; spacing: 8
                    Rectangle {
                        width: 60; height: 14; radius: 3; border.color: Qt.rgba(0,0,0,0.2); border.width: 1; color: "transparent"
                        anchors.verticalCenter: parent.verticalCenter
                        Rectangle {
                            width: Math.max(0, Math.min(parent.width - 2, (parent.width - 2) * Battery.percentage / 100))
                            height: parent.height - 2; x: 1; y: 1; radius: 2
                            color: Battery.percentage > 20 ? "#28ca42" : "#ff3b30"
                        }
                    }
                    Text { text: Battery.percentage + "%"; font.pixelSize: 12; color: Theme.textPrimary; anchors.verticalCenter: parent.verticalCenter }
                }
            }

            Item {
                width: parent.width; height: 36
                visible: Battery.timeRemaining.length > 0
                Text { anchors.left: parent.left; anchors.verticalCenter: parent.verticalCenter; text: "Time Remaining"; font.pixelSize: 12; color: Theme.textSecondary }
                Text { anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter; text: Battery.timeRemaining; font.pixelSize: 12; color: Theme.textPrimary }
            }

            Rectangle { width: parent.width; height: 1; color: Theme.divider; visible: Battery.designCapacity > 0 }
            Item {
                width: parent.width; height: 36; visible: Battery.designCapacity > 0
                Text { anchors.left: parent.left; anchors.verticalCenter: parent.verticalCenter; text: "Design Capacity"; font.pixelSize: 12; color: Theme.textSecondary }
                Text { anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter; text: Battery.designCapacity + " mWh"; font.pixelSize: 12; color: Theme.textPrimary }
            }

            Rectangle { width: parent.width; height: 1; color: Theme.divider; visible: Battery.currentCapacity > 0 }
            Item {
                width: parent.width; height: 36; visible: Battery.currentCapacity > 0
                Text { anchors.left: parent.left; anchors.verticalCenter: parent.verticalCenter; text: "Full Charge Capacity"; font.pixelSize: 12; color: Theme.textSecondary }
                Text { anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter; text: Battery.currentCapacity + " mWh"; font.pixelSize: 12; color: Theme.textPrimary }
            }

            Item { width: parent.width; height: 8 }
        }
    }
    Spacer {}

    // ── Low Power Mode ────────────────────────────────────────────────────
    SettingsCard {
        Item {
            width: parent.width; height: 44
            Text { anchors.left: parent.left; anchors.verticalCenter: parent.verticalCenter; text: "Low power mode"; font.pixelSize: 13; font.weight: Font.Medium; color: Theme.textPrimary }
            LiquidToggle {
                anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter
                checked: Battery.lowPowerMode
                onToggled: function(v) { Battery.setLowPowerMode(v) }
            }
        }
    }
    Spacer {}

    // ── Battery Health ────────────────────────────────────────────────────
    SettingsCard {
        Column {
            width: parent.width; spacing: 0
            Item { width: parent.width; height: 8 }
            Text { text: "Battery Health"; font.pixelSize: 14; font.weight: Font.DemiBold; color: Theme.textPrimary; bottomPadding: 12 }

            Item {
                width: parent.width; height: 36; visible: Battery.healthStatus.length > 0
                Text { anchors.left: parent.left; anchors.verticalCenter: parent.verticalCenter; text: "Health Status"; font.pixelSize: 12; color: Theme.textSecondary }
                Text {
                    anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter
                    text: Battery.healthStatus; font.pixelSize: 12
                    color: Battery.healthStatus === "Good" ? "#22C55E"
                         : Battery.healthStatus === "Fair" ? "#EAB308" : "#EF4444"
                }
            }
            Rectangle { width: parent.width; height: 1; color: Theme.divider; visible: Battery.cycleCount >= 0 }
            Item {
                width: parent.width; height: 36; visible: Battery.cycleCount >= 0
                Text { anchors.left: parent.left; anchors.verticalCenter: parent.verticalCenter; text: "Cycle Count"; font.pixelSize: 12; color: Theme.textSecondary }
                Text { anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter; text: Battery.cycleCount; font.pixelSize: 12; color: Theme.textPrimary }
            }
            Rectangle { width: parent.width; height: 1; color: Theme.divider; visible: Battery.designCapacity > 0 }
            Item {
                width: parent.width; height: 36; visible: Battery.designCapacity > 0
                Text { anchors.left: parent.left; anchors.verticalCenter: parent.verticalCenter; text: "Design Capacity"; font.pixelSize: 12; color: Theme.textSecondary }
                Text { anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter; text: Battery.designCapacity + " mWh"; font.pixelSize: 12; color: Theme.textPrimary }
            }
            Rectangle { width: parent.width; height: 1; color: Theme.divider; visible: Battery.currentCapacity > 0 }
            Item {
                width: parent.width; height: 36; visible: Battery.currentCapacity > 0
                Text { anchors.left: parent.left; anchors.verticalCenter: parent.verticalCenter; text: "Full Charge Capacity"; font.pixelSize: 12; color: Theme.textSecondary }
                Text { anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter; text: Battery.currentCapacity + " mWh"; font.pixelSize: 12; color: Theme.textPrimary }
            }
            Item { width: parent.width; height: 8 }
        }
    }
    Spacer {}

    // ── Battery Level History ─────────────────────────────────────────────
    SectionTitle { text: "Battery Level History" }
    SettingsCard {
        Column {
            width: parent.width; spacing: 0

            // Header row with time-range selector
            Item {
                width: parent.width; height: 40
                Text {
                    anchors.left: parent.left; anchors.verticalCenter: parent.verticalCenter
                    text: historyCanvas.filteredData.length > 0
                          ? historyCanvas.filteredData[historyCanvas.filteredData.length-1].pct.toFixed(0) + "%"
                          : "--"
                    font.pixelSize: 20; font.weight: Font.DemiBold; color: Theme.textPrimary
                }

                Row {
                    id: rangeRow
                    anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter
                    spacing: 2
                    property int selected: 0  // index into ranges

                    readonly property var ranges: [
                        { label: "24h",  secs: 86400 },
                        { label: "3d",   secs: 259200 },
                        { label: "All",  secs: 0 }
                    ]

                    Repeater {
                        model: rangeRow.ranges
                        delegate: Rectangle {
                            height: 22; width: rangeLbl.implicitWidth + 14; radius: 4
                            color: rangeRow.selected === index ? Theme.accent : "transparent"
                            border.color: rangeRow.selected === index ? "transparent" : Theme.divider
                            border.width: 1
                            Text {
                                id: rangeLbl; anchors.centerIn: parent
                                text: modelData.label; font.pixelSize: 11
                                color: rangeRow.selected === index ? "white" : Theme.textSecondary
                            }
                            MouseArea {
                                anchors.fill: parent; cursorShape: Qt.PointingHandCursor
                                onClicked: { rangeRow.selected = index; historyCanvas.updateFilter() }
                            }
                        }
                    }
                }
            }

            Rectangle { width: parent.width; height: 1; color: Theme.divider }

            // Graph
            Canvas {
                id: historyCanvas
                width: parent.width; height: 180

                property var filteredData: []
                property color colTextSecondary: Theme.textSecondary
                property color colDivider:       Theme.divider
                property bool  isDark:           Theme.dark

                function updateFilter() {
                    var raw = Battery.chargeHistory
                    if (!raw || raw.length === 0) { filteredData = []; requestPaint(); return }
                    var secs = rangeRow.ranges[rangeRow.selected].secs
                    if (secs === 0) { filteredData = raw; requestPaint(); return }
                    var cutoff = Math.floor(Date.now() / 1000) - secs
                    var arr = []
                    for (var i = 0; i < raw.length; i++)
                        if (raw[i].ts >= cutoff) arr.push(raw[i])
                    filteredData = arr
                    requestPaint()
                }

                Connections {
                    target: Battery
                    function onHistoryChanged() { historyCanvas.updateFilter() }
                }

                onFilteredDataChanged: requestPaint()
                onColTextSecondaryChanged: requestPaint()
                onIsDarkChanged: requestPaint()

                onPaint: {
                    var ctx = getContext("2d")
                    ctx.clearRect(0, 0, width, height)

                    var data = filteredData
                    var padL = 34, padR = 12, padT = 12, padB = 22
                    var gW = width - padL - padR
                    var gH = height - padT - padB

                    // Helper text color (can't use Theme directly in onPaint)
                    var textCol  = Qt.rgba(colTextSecondary.r, colTextSecondary.g, colTextSecondary.b, colTextSecondary.a)
                    var gridCol  = Qt.rgba(colDivider.r, colDivider.g, colDivider.b, colDivider.a)

                    // No data
                    if (!data || data.length < 2) {
                        ctx.fillStyle = textCol
                        ctx.font = "12px sans-serif"
                        ctx.textAlign = "center"
                        ctx.textBaseline = "middle"
                        ctx.fillText("No battery history available", width / 2, height / 2)
                        return
                    }

                    var tMin = data[0].ts
                    var tMax = data[data.length - 1].ts
                    var tRange = Math.max(tMax - tMin, 1)

                    function xAt(ts)  { return padL + (ts - tMin) / tRange * gW }
                    function yAt(pct) { return padT + (1 - pct / 100) * gH }

                    var yBase = padT + gH

                    // Horizontal grid lines
                    ctx.setLineDash([3, 4])
                    ctx.lineWidth = 0.5
                    ctx.strokeStyle = gridCol
                    for (var g of [25, 50, 75]) {
                        ctx.beginPath()
                        ctx.moveTo(padL, yAt(g))
                        ctx.lineTo(padL + gW, yAt(g))
                        ctx.stroke()
                    }
                    ctx.setLineDash([])

                    // Y-axis labels
                    ctx.fillStyle = textCol
                    ctx.font = "9px sans-serif"
                    ctx.textAlign = "right"
                    ctx.textBaseline = "middle"
                    for (var lp of [0, 25, 50, 75, 100]) {
                        ctx.fillText(lp + "%", padL - 4, yAt(lp))
                    }

                    // Draw filled segments coloured by state
                    for (var i = 0; i < data.length - 1; i++) {
                        var x0 = xAt(data[i].ts),     y0 = yAt(data[i].pct)
                        var x1 = xAt(data[i+1].ts),   y1 = yAt(data[i+1].pct)
                        var st = data[i].state

                        if (st === "charging")
                            ctx.fillStyle = isDark ? "rgba(40,202,66,0.20)" : "rgba(40,202,66,0.15)"
                        else if (st === "discharging")
                            ctx.fillStyle = isDark ? "rgba(59,130,246,0.22)" : "rgba(59,130,246,0.12)"
                        else
                            ctx.fillStyle = isDark ? "rgba(160,160,160,0.10)" : "rgba(120,120,120,0.07)"

                        ctx.beginPath()
                        ctx.moveTo(x0, y0)
                        ctx.lineTo(x1, y1)
                        ctx.lineTo(x1, yBase)
                        ctx.lineTo(x0, yBase)
                        ctx.closePath()
                        ctx.fill()
                    }

                    // Outer line — colour-coded by state segments
                    for (var i = 0; i < data.length - 1; i++) {
                        var x0 = xAt(data[i].ts),   y0 = yAt(data[i].pct)
                        var x1 = xAt(data[i+1].ts), y1 = yAt(data[i+1].pct)
                        var st = data[i].state

                        ctx.strokeStyle = st === "charging"    ? "#28ca42"
                                        : st === "discharging" ? Theme.accentHex
                                        : (isDark ? "rgba(160,160,160,0.5)" : "rgba(120,120,120,0.4)")
                        ctx.lineWidth = 2
                        ctx.lineJoin = "round"
                        ctx.beginPath()
                        ctx.moveTo(x0, y0)
                        ctx.lineTo(x1, y1)
                        ctx.stroke()
                    }

                    // Dot for last data point
                    var lastX = xAt(data[data.length-1].ts)
                    var lastY = yAt(data[data.length-1].pct)
                    var lastSt = data[data.length-1].state
                    ctx.beginPath()
                    ctx.arc(lastX, lastY, 3.5, 0, Math.PI * 2)
                    ctx.fillStyle = lastSt === "charging" ? "#28ca42" : lastSt === "discharging" ? Theme.accentHex : "#28ca42"
                    ctx.fill()

                    // X-axis time labels
                    ctx.fillStyle = textCol
                    ctx.font = "9px sans-serif"
                    ctx.textAlign = "center"
                    ctx.textBaseline = "top"
                    var nowSec = Math.floor(Date.now() / 1000)
                    var numXLabels = 4
                    for (var j = 0; j <= numXLabels; j++) {
                        var ts  = tMin + (tRange / numXLabels) * j
                        var x   = xAt(ts)
                        var ago = nowSec - ts
                        var lbl
                        if (ago < 120)        lbl = "Now"
                        else if (ago < 3600)  lbl = Math.round(ago / 60) + "m ago"
                        else if (ago < 86400) lbl = Math.round(ago / 3600) + "h ago"
                        else                  lbl = Math.round(ago / 86400) + "d ago"
                        if (j === numXLabels) { ctx.textAlign = "right"; lbl = "Now" }
                        else if (j === 0)      ctx.textAlign = "left"
                        else                   ctx.textAlign = "center"
                        ctx.fillText(lbl, x, padT + gH + 4)
                    }
                }
            }

            // Legend
            Item {
                width: parent.width; height: 28
                Row {
                    anchors.left: parent.left; anchors.verticalCenter: parent.verticalCenter; spacing: 16
                    Repeater {
                        model: [
                            { color: "#28ca42", label: "Charging" },
                            { color: Theme.accentHex, label: "Discharging" },
                            { color: "#999999", label: "Charged" }
                        ]
                        delegate: Row {
                            spacing: 5
                            Rectangle { width: 10; height: 10; radius: 5; color: modelData.color; anchors.verticalCenter: parent.verticalCenter }
                            Text { text: modelData.label; font.pixelSize: 10; color: Theme.textSecondary; anchors.verticalCenter: parent.verticalCenter }
                        }
                    }
                }
            }
        }
    }
    Spacer {}

    // ── App Activity ──────────────────────────────────────────────────────
    SectionTitle { text: "App Activity" }
    SettingsCard {
        Column {
            width: parent.width; spacing: 0

            // Column headers
            Item {
                width: parent.width; height: 30
                Text {
                    anchors.left: parent.left; anchors.verticalCenter: parent.verticalCenter
                    text: "Application"
                    font.pixelSize: 11; font.weight: Font.DemiBold; color: Theme.textSecondary
                }
                Text {
                    anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter
                    text: "Energy Impact"
                    font.pixelSize: 11; font.weight: Font.DemiBold; color: Theme.textSecondary
                }
            }
            Rectangle { width: parent.width; height: 1; color: Theme.divider }

            // Empty state
            Item {
                width: parent.width; height: 48
                visible: Battery.appUsage.length === 0
                Text {
                    anchors.centerIn: parent
                    text: "No significant activity"; font.pixelSize: 12; color: Theme.textSecondary
                }
            }

            // Process rows
            Repeater {
                model: Battery.appUsage
                delegate: Column {
                    width: parent.width; spacing: 0
                    Item {
                        width: parent.width; height: 40
                        Row {
                            anchors.left: parent.left; anchors.verticalCenter: parent.verticalCenter; spacing: 8
                            // Rank badge
                            Rectangle {
                                width: 18; height: 18; radius: 4
                                color: index === 0 ? "#ff9500"
                                     : index === 1 ? Theme.bgCard
                                     : index === 2 ? Theme.bgCard
                                     : "transparent"
                                border.color: index < 3 ? Theme.divider : "transparent"
                                border.width: index > 0 ? 1 : 0
                                anchors.verticalCenter: parent.verticalCenter
                                Text {
                                    anchors.centerIn: parent
                                    text: index + 1
                                    font.pixelSize: 9; font.weight: Font.Bold
                                    color: index === 0 ? "white" : Theme.textSecondary
                                }
                            }
                            Text {
                                text: modelData.name; font.pixelSize: 12; color: Theme.textPrimary
                                anchors.verticalCenter: parent.verticalCenter
                                elide: Text.ElideRight
                                width: Math.min(implicitWidth, 160)
                            }
                        }
                        Row {
                            anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter; spacing: 8
                            // Bar
                            Rectangle {
                                width: 100; height: 6; radius: 3
                                color: Theme.bgSearch
                                anchors.verticalCenter: parent.verticalCenter
                                Rectangle {
                                    width: Math.max(4, parent.width * modelData.rel)
                                    height: parent.height; radius: 3
                                    color: index === 0 ? "#ff9500" : Theme.accent
                                    Behavior on width { NumberAnimation { duration: 400; easing.type: Easing.OutCubic } }
                                }
                            }
                            Text {
                                text: modelData.cpu + "%"
                                font.pixelSize: 11; color: Theme.textSecondary
                                width: 38; horizontalAlignment: Text.AlignRight
                                anchors.verticalCenter: parent.verticalCenter
                            }
                        }
                    }
                    Rectangle { width: parent.width; height: 1; color: Theme.divider; visible: index < Battery.appUsage.length - 1 }
                }
            }

            // Refresh button
            Item {
                width: parent.width; height: 40
                Rectangle {
                    anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter
                    width: refreshLbl.implicitWidth + 20; height: 26; radius: 6
                    color: refHov.containsMouse ? Theme.hoverBg : "transparent"
                    border.color: Theme.divider; border.width: 1
                    Text {
                        id: refreshLbl; anchors.centerIn: parent
                        text: "Refresh"; font.pixelSize: 11; color: Theme.textPrimary
                    }
                    MouseArea {
                        id: refHov; anchors.fill: parent
                        hoverEnabled: true; cursorShape: Qt.PointingHandCursor
                        onClicked: Battery.refreshUsage()
                    }
                }
            }
        }
    }
    Spacer {}
}
