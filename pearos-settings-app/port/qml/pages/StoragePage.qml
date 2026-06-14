import QtQuick
import QtQuick.Controls
import "../components"

PageBase {
    title: "Storage"
    Component.onCompleted: { SysInfo.refresh(); SysInfo.refreshStorage() }

    // Storage categories config
    readonly property var categories: [
        { key: "apps",      label: "Applications", color: "#d32f2f" },
        { key: "documents", label: "Documents",    color: "#ff9800" },
        { key: "photos",    label: "Photos",       color: "#fbc02d" },
        { key: "downloads", label: "Downloads",    color: "#7b1fa2" },
        { key: "desktop",   label: "Desktop",      color: "#0288d1" },
        { key: "system",    label: "System",       color: "#9e9e9e" },
    ]

    function gbForKey(key) {
        if (key === "apps")      return SysInfo.storageApps
        if (key === "documents") return SysInfo.storageDocuments
        if (key === "photos")    return SysInfo.storagePhotos
        if (key === "downloads") return SysInfo.storageDownloads
        if (key === "desktop")   return SysInfo.storageDesktop
        if (key === "system")    return SysInfo.storageSystem
        return 0
    }

    function fmtGb(gb) {
        if (gb <= 0) return "0 MB"
        if (gb < 1) return (gb * 1024).toFixed(0) + " MB"
        return gb.toFixed(2) + " GB"
    }

    // Drive card
    SettingsCard {
        Column {
            width: parent.width; spacing: 0

            // Drive header row
            Item {
                width: parent.width; height: 56
                Row {
                    anchors.left: parent.left; anchors.verticalCenter: parent.verticalCenter
                    spacing: 12
                    Image {
                        source: "file:///home/alxb421/Desktop/pkgbuilds/pearos-settings-app/port/assets/preferences-system-disks.svg"
                        width: 36; height: 36; fillMode: Image.PreserveAspectFit
                        anchors.verticalCenter: parent.verticalCenter
                    }
                    Column {
                        spacing: 3; anchors.verticalCenter: parent.verticalCenter
                        Text {
                            text: SysInfo.diskName || "Main Storage"
                            font.pixelSize: 14; font.weight: Font.Medium; color: Theme.textPrimary
                        }
                        Text {
                            text: SysInfo.totalDisk > 0
                                ? fmtGb(SysInfo.usedDisk / 1073741824) + " of "
                                  + fmtGb(SysInfo.totalDisk / 1073741824) + " used"
                                : "Calculating…"
                            font.pixelSize: 12; color: Theme.textSecondary
                        }
                    }
                }
            }

            Rectangle { width: parent.width; height: 1; color: Theme.divider }

            // Segmented bar
            Item {
                width: parent.width; height: 44

                // Background track
                Rectangle {
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.left: parent.left; anchors.right: parent.right
                    height: 20; radius: 4
                    color: Qt.rgba(0, 0, 0, 0.12)

                    // Segments — built with a Row-like approach using sequential x positions
                    Item {
                        anchors.fill: parent

                        property double total: SysInfo.totalDisk > 0 ? SysInfo.totalDisk / 1073741824 : 1

                        // Apps (red)
                        Rectangle {
                            x: 0
                            width: Math.min(parent.width, (SysInfo.storageApps / parent.parent.total) * parent.width)
                            height: parent.height; radius: 0; color: "#d32f2f"
                            Behavior on width { NumberAnimation { duration: 600; easing.type: Easing.OutCubic } }
                        }
                        // Documents (orange)
                        Rectangle {
                            x: Math.min(parent.width, (SysInfo.storageApps / parent.parent.total) * parent.width)
                            width: Math.min(parent.width - x, (SysInfo.storageDocuments / parent.parent.total) * parent.width)
                            height: parent.height; radius: 0; color: "#ff9800"
                            Behavior on width { NumberAnimation { duration: 600; easing.type: Easing.OutCubic } }
                            Behavior on x    { NumberAnimation { duration: 600; easing.type: Easing.OutCubic } }
                        }
                        // Photos (yellow)
                        Rectangle {
                            property double prevEnd: (SysInfo.storageApps + SysInfo.storageDocuments) / parent.parent.total
                            x: Math.min(parent.width, prevEnd * parent.width)
                            width: Math.min(parent.width - x, (SysInfo.storagePhotos / parent.parent.total) * parent.width)
                            height: parent.height; radius: 0; color: "#fbc02d"
                            Behavior on width { NumberAnimation { duration: 600; easing.type: Easing.OutCubic } }
                            Behavior on x    { NumberAnimation { duration: 600; easing.type: Easing.OutCubic } }
                        }
                        // Downloads (purple)
                        Rectangle {
                            property double prevEnd: (SysInfo.storageApps + SysInfo.storageDocuments + SysInfo.storagePhotos) / parent.parent.total
                            x: Math.min(parent.width, prevEnd * parent.width)
                            width: Math.min(parent.width - x, (SysInfo.storageDownloads / parent.parent.total) * parent.width)
                            height: parent.height; radius: 0; color: "#7b1fa2"
                            Behavior on width { NumberAnimation { duration: 600; easing.type: Easing.OutCubic } }
                            Behavior on x    { NumberAnimation { duration: 600; easing.type: Easing.OutCubic } }
                        }
                        // Desktop (blue)
                        Rectangle {
                            property double prevEnd: (SysInfo.storageApps + SysInfo.storageDocuments + SysInfo.storagePhotos + SysInfo.storageDownloads) / parent.parent.total
                            x: Math.min(parent.width, prevEnd * parent.width)
                            width: Math.min(parent.width - x, (SysInfo.storageDesktop / parent.parent.total) * parent.width)
                            height: parent.height; radius: 0; color: "#0288d1"
                            Behavior on width { NumberAnimation { duration: 600; easing.type: Easing.OutCubic } }
                            Behavior on x    { NumberAnimation { duration: 600; easing.type: Easing.OutCubic } }
                        }
                        // System (gray) — from right side
                        Rectangle {
                            property double pct: SysInfo.storageSystem / parent.parent.total
                            x: Math.max(0, parent.width - Math.min(parent.width, pct * parent.width))
                            width: Math.min(parent.width, pct * parent.width)
                            height: parent.height; radius: 0; color: "#9e9e9e"
                            Behavior on width { NumberAnimation { duration: 600; easing.type: Easing.OutCubic } }
                            Behavior on x    { NumberAnimation { duration: 600; easing.type: Easing.OutCubic } }
                        }

                        // Rounded caps on the filled bar as an overlay
                        Rectangle {
                            property double usedFraction: SysInfo.totalDisk > 0
                                ? (SysInfo.storageApps + SysInfo.storageDocuments + SysInfo.storagePhotos
                                   + SysInfo.storageDownloads + SysInfo.storageDesktop + SysInfo.storageSystem)
                                  / parent.parent.total
                                : 0
                            x: 0; y: 0; height: parent.height
                            width: Math.min(parent.width, usedFraction * parent.width)
                            color: "transparent"
                            radius: 4
                            border.color: "transparent"
                        }
                    }
                }
            }

            Rectangle { width: parent.width; height: 1; color: Theme.divider }

            // Legend
            Item {
                width: parent.width; height: 16
            }
            Repeater {
                model: categories
                delegate: Item {
                    width: parent.width; height: 28
                    Row {
                        anchors.left: parent.left; anchors.verticalCenter: parent.verticalCenter
                        spacing: 10
                        Rectangle {
                            width: 10; height: 10; radius: 5
                            color: modelData.color
                            anchors.verticalCenter: parent.verticalCenter
                        }
                        Text {
                            text: modelData.label
                            font.pixelSize: 13; color: Theme.textPrimary
                            anchors.verticalCenter: parent.verticalCenter
                        }
                    }
                    Text {
                        anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter
                        text: fmtGb(gbForKey(modelData.key))
                        font.pixelSize: 12; color: Theme.textSecondary
                    }
                }
            }
            Item { width: parent.width; height: 12 }
        }
    }

    Spacer {}

    // Storage categories detail cards
    SettingsCard {
        Column {
            width: parent.width; spacing: 0
            Repeater {
                id: catRep
                model: categories
                delegate: Column {
                    width: parent.width; spacing: 0
                    Item {
                        width: parent.width; height: 42
                        Row {
                            anchors.left: parent.left; anchors.verticalCenter: parent.verticalCenter
                            spacing: 10
                            Rectangle {
                                width: 8; height: 8; radius: 4
                                color: modelData.color
                                anchors.verticalCenter: parent.verticalCenter
                            }
                            Text {
                                text: modelData.label
                                font.pixelSize: 14; color: Theme.textPrimary
                                anchors.verticalCenter: parent.verticalCenter
                            }
                        }
                        Text {
                            anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter
                            text: fmtGb(gbForKey(modelData.key))
                            font.pixelSize: 13; color: Theme.textSecondary
                        }
                    }
                    Rectangle {
                        width: parent.width; height: 1; color: Theme.divider
                        visible: index < catRep.count - 1
                    }
                }
            }
        }
    }

    Spacer {}
}
