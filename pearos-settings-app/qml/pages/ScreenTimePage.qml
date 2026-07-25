import QtQuick
import QtQuick.Controls
import "../components"

PageBase {
    title: "Screen Time"
    Component.onCompleted: { ScreenTime.ensureTrackerRunning(); ScreenTime.refresh() }

    function fmtDuration(secs) {
        var h = Math.floor(secs / 3600)
        var m = Math.floor((secs % 3600) / 60)
        if (h > 0) return h + "h " + m + "m"
        return m + "m"
    }

    // ── Today total + weekly chart ──────────────────────────────────────
    SettingsCard {
        Column {
            width: parent.width; spacing: 0
            Item { width: parent.width; height: 8 }

            Text { text: "Today"; font.pixelSize: 12; color: Theme.textSecondary }
            Text {
                text: fmtDuration(ScreenTime.todayTotalSecs)
                font.pixelSize: 28; font.weight: Font.DemiBold; color: Theme.textPrimary
                bottomPadding: 12
            }

            Canvas {
                id: weekCanvas
                width: parent.width; height: 140

                property var weekData: ScreenTime.weekTotals
                property color barCol: Theme.accent
                property color textCol: Theme.textSecondary

                Connections {
                    target: ScreenTime
                    function onChanged() { weekCanvas.weekData = ScreenTime.weekTotals; weekCanvas.requestPaint() }
                }
                onBarColChanged: requestPaint()

                onPaint: {
                    var ctx = getContext("2d")
                    ctx.clearRect(0, 0, width, height)
                    var data = weekData
                    if (!data || data.length === 0) return

                    var padB = 24, padT = 10
                    var gH = height - padT - padB
                    var barW = Math.min(36, (width / data.length) * 0.5)
                    var slot = width / data.length

                    var maxSecs = 1
                    for (var i = 0; i < data.length; i++) maxSecs = Math.max(maxSecs, data[i].seconds)

                    var accentCol = Qt.rgba(barCol.r, barCol.g, barCol.b, 1)
                    var textColRgba = Qt.rgba(textCol.r, textCol.g, textCol.b, textCol.a)

                    for (var j = 0; j < data.length; j++) {
                        var h = data[j].seconds > 0 ? Math.max(4, (data[j].seconds / maxSecs) * gH) : 0
                        var x = j * slot + (slot - barW) / 2
                        var y = padT + (gH - h)
                        var isToday = j === data.length - 1

                        ctx.fillStyle = isToday ? accentCol : Qt.rgba(barCol.r, barCol.g, barCol.b, 0.35)
                        ctx.beginPath()
                        ctx.roundedRect ? ctx.roundedRect(x, y, barW, h, 4) : ctx.rect(x, y, barW, h)
                        ctx.fill()

                        ctx.fillStyle = textColRgba
                        ctx.font = "10px sans-serif"
                        ctx.textAlign = "center"
                        ctx.textBaseline = "top"
                        ctx.fillText(data[j].label, x + barW / 2, padT + gH + 6)
                    }
                }
            }
            Item { width: parent.width; height: 8 }
        }
    }
    Spacer {}

    // ── Tracking status ──────────────────────────────────────────────────
    SettingsCard {
        Item {
            width: parent.width; height: 32
            Row {
                anchors.left: parent.left; anchors.verticalCenter: parent.verticalCenter; spacing: 6
                Rectangle {
                    width: 8; height: 8; radius: 4; anchors.verticalCenter: parent.verticalCenter
                    color: ScreenTime.trackingActive ? "#22C55E" : "#EF4444"
                }
                Text {
                    text: ScreenTime.trackingActive ? "Tracking active" : "Tracker not running"
                    font.pixelSize: 11; color: Theme.textSecondary
                }
            }
        }
    }
    Spacer {}

    // ── Per-app breakdown ─────────────────────────────────────────────────
    SectionTitle { text: "Most Used Today" }
    SettingsCard {
        Column {
            width: parent.width; spacing: 0

            Item {
                width: parent.width; height: 48
                visible: ScreenTime.todayApps.length === 0
                Text { anchors.centerIn: parent; text: "No activity recorded yet today"; font.pixelSize: 12; color: Theme.textSecondary }
            }

            Repeater {
                model: ScreenTime.todayApps
                delegate: Column {
                    width: parent.width
                    Item {
                        width: parent.width; height: 40
                        Text {
                            anchors.left: parent.left; anchors.verticalCenter: parent.verticalCenter
                            text: modelData.label; font.pixelSize: 12; color: Theme.textPrimary
                            elide: Text.ElideRight; width: 150
                        }
                        Row {
                            anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter; spacing: 8
                            Rectangle {
                                width: 140; height: 8; radius: 4
                                color: Theme.bgSearch
                                anchors.verticalCenter: parent.verticalCenter
                                Rectangle {
                                    width: Math.max(4, parent.width * modelData.rel)
                                    height: parent.height; radius: 4
                                    color: Theme.accent
                                    Behavior on width { NumberAnimation { duration: 300; easing.type: Easing.OutCubic } }
                                }
                            }
                            Text {
                                text: fmtDuration(modelData.seconds)
                                font.pixelSize: 11; color: Theme.textSecondary
                                width: 50; horizontalAlignment: Text.AlignRight
                                anchors.verticalCenter: parent.verticalCenter
                            }
                        }
                    }
                    Rectangle { width: parent.width; height: 1; color: Theme.divider; visible: index < ScreenTime.todayApps.length - 1 }
                }
            }
        }
    }
    Spacer {}
}
