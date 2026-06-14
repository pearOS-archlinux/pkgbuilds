import QtQuick
import QtQuick.Controls
import "../components"

PageBase {
    title: "Wallpaper"
    Component.onCompleted: Wallpaper.refreshWallpapers()

    // ── Sticky header: preview + controls ──────────────────────────────
    Row {
        width: parent.width
        spacing: 16
        bottomPadding: 12

        // Current wallpaper preview (16:9)
        Rectangle {
            width: 160
            height: 90
            radius: 8
            color: Theme.bgCard
            clip: true
            Image {
                anchors.fill: parent
                source: Wallpaper.currentWallpaper ? "file://" + Wallpaper.currentWallpaper : ""
                fillMode: Image.PreserveAspectCrop
                smooth: true; mipmap: true
            }
        }

        // Controls card
        SettingsCard {
            width: parent.width - 176

            Column {
                width: parent.width; spacing: 0

                // Current wallpaper name + fill mode
                Item {
                    width: parent.width; height: 44
                    Text {
                        anchors.left: parent.left
                        anchors.right: fillCombo.left; anchors.rightMargin: 8
                        anchors.verticalCenter: parent.verticalCenter
                        text: {
                            var wp = Wallpaper.currentWallpaper
                            if (!wp) return "None"
                            return wp.split("/").pop().replace(/\.[^.]+$/, "")
                        }
                        font.pixelSize: 12; color: Theme.textPrimary; elide: Text.ElideRight
                    }
                    ComboBox {
                        id: fillCombo
                        anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter
                        model: ["Fill screen", "Fit to screen", "Stretch to Fill Screen", "Center", "Tile"]
                        width: 120; height: 28; font.pixelSize: 11
                    }
                }

                Rectangle { width: parent.width; height: 1; color: Theme.divider }

                // Load Wallpaper / Browse
                Item {
                    width: parent.width; height: 44
                    Text {
                        anchors.left: parent.left; anchors.verticalCenter: parent.verticalCenter
                        text: "Load Wallpaper"; font.pixelSize: 12; color: Theme.textPrimary
                    }
                    Rectangle {
                        anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter
                        width: browseLabel.width + 20; height: 28; radius: 6
                        color: Qt.rgba(1,1,1,0.1); border.color: Qt.rgba(1,1,1,0.2); border.width: 1
                        Text { id: browseLabel; anchors.centerIn: parent; text: "Browse…"; font.pixelSize: 12; color: Theme.textPrimary }
                        MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor
                            onClicked: {
                                // Open file dialog to pick a wallpaper
                                fileDialog.open()
                            }
                        }
                    }
                }

                Rectangle { width: parent.width; height: 1; color: Theme.divider }

                // Clock Appearance
                Item {
                    width: parent.width; height: 44
                    Text {
                        anchors.left: parent.left; anchors.verticalCenter: parent.verticalCenter
                        text: "Clock Appearance"; font.pixelSize: 12; color: Theme.textPrimary
                    }
                    Rectangle {
                        anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter
                        width: clockLabel.width + 20; height: 28; radius: 6
                        color: Qt.rgba(1,1,1,0.1); border.color: Qt.rgba(1,1,1,0.2); border.width: 1
                        Text { id: clockLabel; anchors.centerIn: parent; text: "Clock Appearance…"; font.pixelSize: 12; color: Theme.textPrimary }
                        MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor }
                    }
                }
            }
        }
    }

    // Divider
    Rectangle {
        width: parent.width; height: 1
        color: Theme.divider
    }

    Spacer {}

    // ── Wallpaper categories ────────────────────────────────────────────
    SettingsCard {
        Column {
            width: parent.width; spacing: 0

            Repeater {
                id: catRepeater
                model: Wallpaper.wallpaperCategories

                delegate: Column {
                    id: catDelegate
                    width: parent.width; spacing: 0

                    property bool showAll: false
                    property var catWallpapers: modelData.wallpapers || []

                    // Category header row
                    Item {
                        width: parent.width; height: 36

                        Text {
                            anchors.left: parent.left; anchors.leftMargin: 4
                            anchors.verticalCenter: parent.verticalCenter
                            text: modelData.name || "General"
                            font.pixelSize: 13; font.weight: Font.DemiBold; color: Theme.textPrimary
                        }

                        Text {
                            anchors.right: parent.right; anchors.rightMargin: 4
                            anchors.verticalCenter: parent.verticalCenter
                            text: catDelegate.showAll
                                ? "Show Less"
                                : "Show All (" + catDelegate.catWallpapers.length + ")"
                            font.pixelSize: 12; color: Theme.textSecondary
                            MouseArea {
                                anchors.fill: parent; cursorShape: Qt.PointingHandCursor
                                onClicked: catDelegate.showAll = !catDelegate.showAll
                            }
                        }
                    }

                    // Horizontal scrollable row (or wrapped if showAll)
                    Item {
                        width: parent.width
                        height: catDelegate.showAll ? wallFlow.implicitHeight + 12 : 108
                        clip: !catDelegate.showAll

                        Flickable {
                            anchors.fill: parent
                            contentWidth: catDelegate.showAll ? width : wallRow.implicitWidth
                            contentHeight: height
                            flickableDirection: catDelegate.showAll ? Flickable.VerticalFlick : Flickable.HorizontalFlick
                            boundsBehavior: Flickable.DragAndOvershootBounds
                            boundsMovement: Flickable.FollowBoundsBehavior
                            clip: false

                            // Normal row (horizontal scroll)
                            Row {
                                id: wallRow
                                spacing: 10; topPadding: 6; bottomPadding: 6
                                visible: !catDelegate.showAll

                                Repeater {
                                    model: catDelegate.catWallpapers
                                    delegate: WallpaperThumb { wallpaperData: modelData }
                                }
                            }

                            // Expanded flow (wrap)
                            Flow {
                                id: wallFlow
                                width: parent.width
                                spacing: 10
                                topPadding: 6; bottomPadding: 6
                                visible: catDelegate.showAll

                                Repeater {
                                    model: catDelegate.catWallpapers
                                    delegate: WallpaperThumb { wallpaperData: modelData }
                                }
                            }
                        }
                    }

                    // Divider between categories (not last)
                    Item {
                        width: parent.width; height: 9
                        visible: index < catRepeater.count - 1
                        Rectangle { anchors.centerIn: parent; width: parent.width; height: 1; color: Theme.divider }
                    }
                }
            }

            // Loading placeholder
            Item {
                width: parent.width; height: 60
                visible: Wallpaper.wallpaperCategories.length === 0
                Text {
                    anchors.centerIn: parent
                    text: "Loading wallpapers…"
                    font.pixelSize: 13; color: Theme.textSecondary
                }
            }
        }
    }

    Spacer {}

    // Inline component for wallpaper thumbnail
    component WallpaperThumb: Item {
        property var wallpaperData: ({})
        width: 100; height: 96  // 56px (16:9 of 100) + 6 gap + 14 label + 20 padding

        Rectangle {
            id: thumbWrap
            width: 100; height: 56; radius: 8; clip: true
            color: Theme.bgCard
            border.color: wallpaperData.path === Wallpaper.currentWallpaper ? "#3B82F6" : "transparent"
            border.width: wallpaperData.path === Wallpaper.currentWallpaper ? 2 : 0
            Image {
                anchors.fill: parent; anchors.margins: thumbWrap.border.width
                source: "file://" + (wallpaperData.path || "")
                fillMode: Image.PreserveAspectCrop
                smooth: true; mipmap: true; asynchronous: true
            }
            MouseArea {
                anchors.fill: parent; cursorShape: Qt.PointingHandCursor
                onClicked: Wallpaper.setWallpaper(wallpaperData.path)
            }
        }
        Text {
            anchors.top: thumbWrap.bottom; anchors.topMargin: 4
            anchors.left: parent.left; anchors.right: parent.right
            text: (wallpaperData.name || "").replace(/_/g, " ")
            font.pixelSize: 11; color: Theme.textSecondary
            horizontalAlignment: Text.AlignHCenter
            elide: Text.ElideRight
        }
    }
}
