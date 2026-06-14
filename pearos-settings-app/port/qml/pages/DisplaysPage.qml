import QtQuick
import QtQuick.Controls
import "../components"

PageBase {
    title: "Displays"
    Component.onCompleted: { Display.refreshDisplays(); Display.refreshBrightness("") }

    readonly property string ap: "file:///home/alxb421/Desktop/pkgbuilds/pearos-settings-app/port/assets/"

    // Connected displays list
    Repeater {
        model: Display.displays
        delegate: Column {
            width: parent.width; spacing: 0

            SettingsCard {
                Column {
                    width: parent.width; spacing: 0

                    // Display header
                    Item {
                        width: parent.width; height: 56
                        Row {
                            anchors.left: parent.left; anchors.verticalCenter: parent.verticalCenter
                            spacing: 12
                            Image {
                                source: modelData.isBuiltIn ? ap + "laptop.svg" : ap + "computer.svg"
                                width: 28; height: 28; fillMode: Image.PreserveAspectFit
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
                                        width: mainLabel.width + 10; height: 16; radius: 3
                                        color: Theme.bgCard; border.color: Theme.divider; border.width: 1
                                        anchors.verticalCenter: parent.verticalCenter
                                        Text {
                                            id: mainLabel
                                            anchors.centerIn: parent
                                            text: "main"; font.pixelSize: 10; font.weight: Font.SemiBold
                                            color: Theme.textSecondary
                                        }
                                    }
                                }
                                Text {
                                    text: {
                                        var d = modelData
                                        if (d.inches && d.resolution && d.resolution !== "")
                                            return d.inches.replace('"','') + "-inch (" + d.resolution + ")"
                                        return d.resolution || "Unknown"
                                    }
                                    font.pixelSize: 11; color: Theme.textSecondary
                                }
                            }
                        }

                        // Enable/disable + primary buttons
                        Row {
                            anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter
                            spacing: 8

                            Rectangle {
                                visible: !modelData.isPrimary && modelData.isEnabled
                                width: primaryBtn.width + 20; height: 28; radius: 6
                                color: primaryHov.containsMouse ? Theme.hoverBg : "transparent"
                                border.color: Theme.divider; border.width: 1
                                Text {
                                    id: primaryBtn
                                    anchors.centerIn: parent
                                    text: "Set Primary"; font.pixelSize: 11; color: Theme.textPrimary
                                }
                                MouseArea {
                                    id: primaryHov; anchors.fill: parent; hoverEnabled: true
                                    cursorShape: Qt.PointingHandCursor
                                    onClicked: Display.setDisplayPrimary(modelData.name)
                                }
                            }

                            Rectangle {
                                width: enableLabel.width + 20; height: 28; radius: 6
                                color: enableHov.containsMouse ? Theme.hoverBg : Theme.bgCard
                                border.color: Theme.divider; border.width: 1
                                Text {
                                    id: enableLabel; anchors.centerIn: parent
                                    text: modelData.isEnabled ? "Disable" : "Enable"
                                    font.pixelSize: 11; color: Theme.textPrimary
                                }
                                MouseArea {
                                    id: enableHov; anchors.fill: parent; hoverEnabled: true
                                    cursorShape: Qt.PointingHandCursor
                                    onClicked: Display.setDisplayEnabled(modelData.name, !modelData.isEnabled, "")
                                }
                            }
                        }
                    }

                    Rectangle { width: parent.width; height: 1; color: Theme.divider }

                    // Resolution selector
                    Item {
                        width: parent.width; height: 44
                        Text {
                            anchors.left: parent.left; anchors.verticalCenter: parent.verticalCenter
                            text: "Resolution"; font.pixelSize: 13; color: Theme.textPrimary
                        }
                        ComboBox {
                            id: resCombo
                            anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter
                            width: 180; height: 30
                            font.pixelSize: 12

                            property var resList: {
                                var list = []
                                var avail = modelData.availableResolutions || []
                                for (var i = 0; i < avail.length; i++) {
                                    list.push(avail[i].resolution || "")
                                }
                                return list
                            }

                            model: resList.length > 0 ? resList : [modelData.resolution || "Unknown"]

                            currentIndex: {
                                var cur = modelData.resolution || ""
                                var idx = resList.indexOf(cur)
                                return idx >= 0 ? idx : 0
                            }

                            onActivated: {
                                var res = resList[index]
                                if (res && res !== modelData.resolution) {
                                    Display.setDisplayEnabled(modelData.name, true, res)
                                }
                            }

                            background: Rectangle {
                                color: Theme.bgCard; radius: 6
                                border.color: Theme.divider; border.width: 1
                            }
                            contentItem: Text {
                                leftPadding: 10; rightPadding: 28
                                text: resCombo.displayText; font: resCombo.font
                                color: Theme.textPrimary; verticalAlignment: Text.AlignVCenter; elide: Text.ElideRight
                            }
                            indicator: Text {
                                x: resCombo.width - width - 8; y: (resCombo.height - height) / 2
                                text: "⌄"; font.pixelSize: 14; color: Theme.textSecondary
                            }
                            popup: Popup {
                                y: resCombo.height + 2; width: resCombo.width
                                background: Rectangle { color: Theme.bgCard; radius: 8; border.color: Theme.divider; border.width: 1 }
                                contentItem: ListView {
                                    implicitHeight: Math.min(contentHeight, 200)
                                    model: resCombo.delegateModel; clip: true
                                    ScrollIndicator.vertical: ScrollIndicator {}
                                }
                            }
                            delegate: ItemDelegate {
                                width: resCombo.width; height: 32
                                contentItem: Text {
                                    text: modelData; font.pixelSize: 12
                                    color: Theme.textPrimary; verticalAlignment: Text.AlignVCenter
                                    leftPadding: 10
                                }
                                background: Rectangle {
                                    color: hovered ? Theme.hoverBg : "transparent"
                                }
                            }
                        }
                    }

                    Rectangle { width: parent.width; height: 1; color: Theme.divider }

                    // Refresh rate
                    Item {
                        width: parent.width; height: 44
                        Text {
                            anchors.left: parent.left; anchors.verticalCenter: parent.verticalCenter
                            text: "Refresh Rate"; font.pixelSize: 13; color: Theme.textPrimary
                        }
                        Text {
                            anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter
                            text: modelData.refreshRate ? modelData.refreshRate + " Hz" : "Auto"
                            font.pixelSize: 13; color: Theme.textSecondary
                        }
                    }
                }
            }
            Spacer {}
        }
    }

    // Empty state
    Item {
        width: parent.width; height: 60
        visible: Display.displays.length === 0
        Text { anchors.centerIn: parent; text: "Loading displays…"; font.pixelSize: 13; color: Theme.textSecondary }
    }

    // Brightness card
    SettingsCard {
        Column {
            width: parent.width; spacing: 0

            Item {
                width: parent.width; height: 44
                Text {
                    anchors.left: parent.left; anchors.verticalCenter: parent.verticalCenter
                    text: "Brightness"; font.pixelSize: 13; font.weight: Font.Medium; color: Theme.textPrimary
                }
            }
            Rectangle { width: parent.width; height: 1; color: Theme.divider }

            Item {
                width: parent.width; height: 52
                Row {
                    anchors.left: parent.left; anchors.right: parent.right
                    anchors.verticalCenter: parent.verticalCenter; spacing: 10
                    Text { text: "☀"; font.pixelSize: 13; color: Theme.textSecondary; anchors.verticalCenter: parent.verticalCenter }
                    Slider {
                        id: brightnessSlider
                        from: 0; to: 100; stepSize: 1
                        value: Display.brightnessValue
                        palette.accent: "#3B82F6"
                        width: parent.width - 60
                        anchors.verticalCenter: parent.verticalCenter
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
                Rectangle {
                    id: nightTog; width: 36; height: 18; radius: 9
                    property bool on: false
                    anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter
                    color: on ? "#3B82F6" : Qt.rgba(0.5, 0.5, 0.5, 0.4)
                    Behavior on color { ColorAnimation { duration: 200 } }
                    Rectangle {
                        width: 14; height: 14; radius: 7; color: "white"; y: 2
                        x: nightTog.on ? 20 : 2
                        Behavior on x { NumberAnimation { duration: 200 } }
                    }
                    MouseArea {
                        anchors.fill: parent; cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            nightTog.on = !nightTog.on
                            Display.setNightLight(nightTog.on)
                        }
                    }
                }
            }
        }
    }

    Spacer {}
}
