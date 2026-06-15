import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "../components"

PageBase {
    title: "Appearance"
    readonly property string ap: "file:///usr/share/extras/system-settings/assets/"
    Component.onCompleted: Appearance.refresh()

    // Theme selection card
    SettingsCard {
        Item {
            width: parent.width; height: 76

            Text {
                anchors.left: parent.left; anchors.verticalCenter: parent.verticalCenter
                text: "Appearance"; font.pixelSize: 13; font.weight: Font.DemiBold; color: Theme.textPrimary
            }

            Row {
                anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter
                spacing: 12
                Repeater {
                    model: [
                        { name: "auto",  label: "Auto",  img: ap + "auto.png" },
                        { name: "light", label: "Light", img: ap + "light.png" },
                        { name: "dark",  label: "Dark",  img: ap + "dark.png" },
                    ]
                    delegate: Column {
                        spacing: 5
                        Rectangle {
                            width: 72; height: 48; radius: 8
                            border.color: Appearance.colorScheme === modelData.name ? Theme.accent : Qt.rgba(0,0,0,0.12)
                            border.width: Appearance.colorScheme === modelData.name ? 2 : 1
                            color: "transparent"
                            clip: true
                            Image {
                                anchors.fill: parent; anchors.margins: 2
                                source: modelData.img; fillMode: Image.PreserveAspectFit
                            }
                            MouseArea {
                                anchors.fill: parent
                                cursorShape: Qt.PointingHandCursor
                                onClicked: Appearance.setColorScheme(modelData.name)
                            }
                        }
                        Text { text: modelData.label; font.pixelSize: 10; color: Theme.textPrimary; anchors.horizontalCenter: parent.horizontalCenter }
                    }
                }
            }
        }
    }
    Spacer {}

    SectionTitle { text: "Theme" }
    SettingsCard {
        Column {
            width: parent.width; spacing: 0

            Item {
                width: parent.width; height: 44
                Text { anchors.left: parent.left; anchors.verticalCenter: parent.verticalCenter; text: "Color"; font.pixelSize: 12; font.weight: Font.DemiBold; color: Theme.textPrimary }
                Row {
                    anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter; spacing: 6
                    Repeater {
                        model: [
                            { name: "purple",      hex: "#8B5CF6" },
                            { name: "magenta",     hex: "#EC4899" },
                            { name: "orange",      hex: "#F97316" },
                            { name: "yellow",      hex: "#EAB308" },
                            { name: "green",       hex: "#22C55E" },
                            { name: "azul",        hex: "#06B6D4" },
                            { name: "blue",        hex: "#3B82F6" },
                            { name: "lila",        hex: "#A855F7" },
                            { name: "dark-purple", hex: "#6B21A8" },
                            { name: "grey",        hex: "#6B7280" },
                        ]
                        delegate: Rectangle {
                            width: 20; height: 20; radius: 10; color: modelData.hex
                            border.color: "white"
                            border.width: Appearance.accent === modelData.name ? 2 : 0
                            scale: Appearance.accent === modelData.name ? 1.15 : 1.0
                            Behavior on scale { NumberAnimation { duration: 120 } }
                            MouseArea {
                                anchors.fill: parent
                                cursorShape: Qt.PointingHandCursor
                                onClicked: Appearance.setAccent(modelData.name)
                            }
                        }
                    }
                }
            }
            Rectangle { width: parent.width; height: 1; color: Theme.divider }

            Item {
                width: parent.width; height: 44
                Text { anchors.left: parent.left; anchors.verticalCenter: parent.verticalCenter; text: "Text highlight color"; font.pixelSize: 12; font.weight: Font.DemiBold; color: Theme.textPrimary }
                Text { anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter; text: "Automatic"; font.pixelSize: 12; color: Theme.textSecondary }
            }
        }
    }
    Spacer {}

    SettingsCard {
        Column {
            width: parent.width; spacing: 0

            Item {
                width: parent.width; height: 44
                Text { anchors.left: parent.left; anchors.verticalCenter: parent.verticalCenter; text: "Icon & widget style"; font.pixelSize: 12; font.weight: Font.DemiBold; color: Theme.textPrimary }
                Row {
                    anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter; spacing: 6
                    property int selected: 3
                    Repeater {
                        model: ["Default", "Dark", "Clear", "Tinted"]
                        delegate: Rectangle {
                            height: 28; width: sLbl.implicitWidth + 16; radius: 6
                            color: parent.selected === index ? Theme.accent : Qt.rgba(0,0,0,0.08)
                            Text {
                                id: sLbl; anchors.centerIn: parent; text: modelData; font.pixelSize: 12
                                color: parent.parent.selected === index ? "white" : Theme.textPrimary
                            }
                            MouseArea { anchors.fill: parent; onClicked: parent.parent.selected = index }
                        }
                    }
                }
            }
            Rectangle { width: parent.width; height: 1; color: Theme.divider }

            Item {
                width: parent.width; height: 44
                Text { anchors.left: parent.left; anchors.verticalCenter: parent.verticalCenter; text: "Folder color"; font.pixelSize: 12; font.weight: Font.DemiBold; color: Theme.textPrimary }
                Text { anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter; text: "Automatic"; font.pixelSize: 12; color: Theme.textSecondary }
            }
        }
    }
    Spacer {}

    SectionTitle { text: "Windows" }
    SettingsCard {
        Column {
            width: parent.width; spacing: 0

            Item {
                width: parent.width; height: 44
                Text { anchors.left: parent.left; anchors.verticalCenter: parent.verticalCenter; text: "Sidebar icon size"; font.pixelSize: 12; font.weight: Font.DemiBold; color: Theme.textPrimary }
                SettingsComboBox {
                    anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter
                    model: ["Small", "Medium", "Large"]
                    currentIndex: 1; width: 100; height: 28; font.pixelSize: 12
                }
            }
            Rectangle { width: parent.width; height: 1; color: Theme.divider }

            Item {
                width: parent.width; height: 44
                Text { anchors.left: parent.left; anchors.verticalCenter: parent.verticalCenter; text: "Tint window with wallpaper color"; font.pixelSize: 12; font.weight: Font.DemiBold; color: Theme.textPrimary }
                LiquidToggle {
                    anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter
                    checked: Appearance.tintEnabled
                    onToggled: function(v) { Appearance.setTintEnabled(v) }
                }
            }
        }
    }
    Spacer {}

    SettingsCard {
        Column {
            width: parent.width; spacing: 0

            Column {
                width: parent.width; spacing: 8; topPadding: 8; bottomPadding: 8
                Text { text: "Show scroll bars"; font.pixelSize: 12; font.weight: Font.DemiBold; color: Theme.textPrimary }
                Repeater {
                    model: ["Automatically based on mouse or trackpad", "When scrolling", "Always"]
                    delegate: Row {
                        spacing: 8
                        Rectangle {
                            width: 16; height: 16; radius: 8; border.color: Theme.accent; border.width: 2
                            color: index === 0 ? Theme.accent : "transparent"
                            anchors.verticalCenter: parent.verticalCenter
                        }
                        Text { text: modelData; font.pixelSize: 12; color: Theme.textPrimary }
                    }
                }
            }
            Rectangle { width: parent.width; height: 1; color: Theme.divider }

            Column {
                width: parent.width; spacing: 8; topPadding: 8; bottomPadding: 8
                Text { text: "Click in the scroll bar to"; font.pixelSize: 12; font.weight: Font.DemiBold; color: Theme.textPrimary }
                Repeater {
                    model: ["Jump to the spot that's clicked", "Jump to the next page"]
                    delegate: Row {
                        spacing: 8
                        Rectangle {
                            width: 16; height: 16; radius: 8; border.color: Theme.accent; border.width: 2
                            color: index === 0 ? Theme.accent : "transparent"
                            anchors.verticalCenter: parent.verticalCenter
                        }
                        Text { text: modelData; font.pixelSize: 12; color: Theme.textPrimary }
                    }
                }
            }
        }
    }
    Spacer {}

    SectionTitle { text: "Liquid Gel" }
    SettingsCard {
        Column {
            width: parent.width; spacing: 0

            // Enable toggle
            Item {
                width: parent.width; height: 44
                Text { anchors.left: parent.left; anchors.verticalCenter: parent.verticalCenter; text: "Enable Liquid Gel"; font.pixelSize: 12; font.weight: Font.DemiBold; color: Theme.textPrimary }
                LiquidToggle {
                    anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter
                    checked: Appearance.lgEnabled
                    onToggled: function(v) { Appearance.setLgEnabled(v) }
                }
            }
            Rectangle { width: parent.width; height: 1; color: Theme.divider }

            // Blur Strength
            Item {
                width: parent.width; height: 44
                Text { anchors.left: parent.left; anchors.verticalCenter: parent.verticalCenter; text: "Blur Strength"; font.pixelSize: 12; font.weight: Font.DemiBold; color: Theme.textPrimary }
                Row {
                    anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter; spacing: 8
                    Slider {
                        width: 140; height: 28; from: 1; to: 15; stepSize: 1
                        value: Appearance.lgBlurStrength
                        onMoved: Appearance.setLgBlurStrength(Math.round(value))
                    }
                    Text { text: Appearance.lgBlurStrength; font.pixelSize: 12; color: Theme.textSecondary; width: 22; horizontalAlignment: Text.AlignRight }
                }
            }
            Rectangle { width: parent.width; height: 1; color: Theme.divider }

            // Noise Strength
            Item {
                width: parent.width; height: 44
                Text { anchors.left: parent.left; anchors.verticalCenter: parent.verticalCenter; text: "Noise Strength"; font.pixelSize: 12; font.weight: Font.DemiBold; color: Theme.textPrimary }
                Row {
                    anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter; spacing: 8
                    Slider {
                        width: 140; height: 28; from: 0; to: 14; stepSize: 1
                        value: Appearance.lgNoiseStrength
                        onMoved: Appearance.setLgNoiseStrength(Math.round(value))
                    }
                    Text { text: Appearance.lgNoiseStrength; font.pixelSize: 12; color: Theme.textSecondary; width: 22; horizontalAlignment: Text.AlignRight }
                }
            }
            Rectangle { width: parent.width; height: 1; color: Theme.divider }

            // Refraction Strength
            Item {
                width: parent.width; height: 44
                Text { anchors.left: parent.left; anchors.verticalCenter: parent.verticalCenter; text: "Refraction Strength"; font.pixelSize: 12; font.weight: Font.DemiBold; color: Theme.textPrimary }
                Row {
                    anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter; spacing: 8
                    Slider {
                        width: 140; height: 28; from: 0; to: 20; stepSize: 1
                        value: Appearance.lgRefractionStrength
                        onMoved: Appearance.setLgRefractionStrength(Math.round(value))
                    }
                    Text { text: Appearance.lgRefractionStrength; font.pixelSize: 12; color: Theme.textSecondary; width: 22; horizontalAlignment: Text.AlignRight }
                }
            }
            Rectangle { width: parent.width; height: 1; color: Theme.divider }

            // Refraction Edge Size
            Item {
                width: parent.width; height: 44
                Text { anchors.left: parent.left; anchors.verticalCenter: parent.verticalCenter; text: "Refraction Edge Size"; font.pixelSize: 12; font.weight: Font.DemiBold; color: Theme.textPrimary }
                Row {
                    anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter; spacing: 8
                    Slider {
                        width: 140; height: 28; from: 0; to: 20; stepSize: 1
                        value: Appearance.lgRefractionEdgeSize
                        onMoved: Appearance.setLgRefractionEdgeSize(Math.round(value))
                    }
                    Text { text: Appearance.lgRefractionEdgeSize; font.pixelSize: 12; color: Theme.textSecondary; width: 22; horizontalAlignment: Text.AlignRight }
                }
            }
            Rectangle { width: parent.width; height: 1; color: Theme.divider }

            // RGB Fringing
            Item {
                width: parent.width; height: 44
                Text { anchors.left: parent.left; anchors.verticalCenter: parent.verticalCenter; text: "RGB Fringing"; font.pixelSize: 12; font.weight: Font.DemiBold; color: Theme.textPrimary }
                Row {
                    anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter; spacing: 8
                    Slider {
                        width: 140; height: 28; from: 0; to: 20; stepSize: 1
                        value: Appearance.lgRGBFringing
                        onMoved: Appearance.setLgRGBFringing(Math.round(value))
                    }
                    Text { text: Appearance.lgRGBFringing; font.pixelSize: 12; color: Theme.textSecondary; width: 22; horizontalAlignment: Text.AlignRight }
                }
            }
        }
    }
    Spacer {}
}
