import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "../components"

PageBase {
    title: "Appearance"
    readonly property string ap: "file:///home/alxb421/Desktop/pkgbuilds/pearos-settings-app/port/assets/"
    Component.onCompleted: Appearance.refresh()

    // Theme selection card
    SettingsCard {
        Column {
            width: parent.width; spacing: 12; topPadding: 4; bottomPadding: 4

            Text { text: "Appearance"; font.pixelSize: 14; font.weight: Font.DemiBold; color: Theme.textPrimary }

            Row {
                spacing: 16
                Repeater {
                    model: [
                        { name: "auto",  label: "Auto",  img: ap + "auto.png" },
                        { name: "light", label: "Light", img: ap + "light.png" },
                        { name: "dark",  label: "Dark",  img: ap + "dark.png" },
                    ]
                    delegate: Column {
                        spacing: 6
                        Rectangle {
                            width: 72; height: 48; radius: 8
                            border.color: Appearance.colorScheme === modelData.name ? "#3B82F6" : Qt.rgba(0,0,0,0.12)
                            border.width: Appearance.colorScheme === modelData.name ? 2 : 1
                            color: "transparent"
                            clip: true
                            Image {
                                anchors.fill: parent; anchors.margins: 2
                                source: modelData.img; fillMode: Image.PreserveAspectFit
                            }
                            MouseArea { anchors.fill: parent; onClicked: Appearance.setColorScheme(modelData.name) }
                        }
                        Text { text: modelData.label; font.pixelSize: 11; color: Theme.textPrimary; anchors.horizontalCenter: parent.horizontalCenter }
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
                    anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter; spacing: 8
                    Repeater {
                        model: ["#8B5CF6","#EC4899","#F97316","#EAB308","#22C55E","#06B6D4","#3B82F6","#A855F7","#6B21A8","#6B7280"]
                        delegate: Rectangle {
                            width: 18; height: 18; radius: 9; color: modelData
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
                            color: parent.selected === index ? "#3B82F6" : Qt.rgba(0,0,0,0.08)
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
                ComboBox {
                    anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter
                    model: ["Small", "Medium", "Large"]
                    currentIndex: 1; width: 100; height: 28; font.pixelSize: 12
                }
            }
            Rectangle { width: parent.width; height: 1; color: Theme.divider }

            Item {
                width: parent.width; height: 44
                Text { anchors.left: parent.left; anchors.verticalCenter: parent.verticalCenter; text: "Tint window with wallpaper color"; font.pixelSize: 12; font.weight: Font.DemiBold; color: Theme.textPrimary }
                Rectangle {
                    id: tintToggle; width: 36; height: 18; radius: 9; property bool on: false
                    anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter
                    color: on ? "#3B82F6" : "#cccccc"
                    Behavior on color { ColorAnimation { duration: 200 } }
                    Rectangle { width: 14; height: 14; radius: 7; color: "white"; y: 2; x: tintToggle.on ? 20 : 2; Behavior on x { NumberAnimation { duration: 200 } } }
                    MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor; onClicked: tintToggle.on = !tintToggle.on }
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
                            width: 16; height: 16; radius: 8; border.color: "#3B82F6"; border.width: 2
                            color: index === 0 ? "#3B82F6" : "transparent"
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
                            width: 16; height: 16; radius: 8; border.color: "#3B82F6"; border.width: 2
                            color: index === 0 ? "#3B82F6" : "transparent"
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
                Rectangle {
                    id: lgToggle; width: 36; height: 18; radius: 9
                    anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter
                    color: Appearance.lgEnabled ? "#3B82F6" : "#cccccc"
                    Behavior on color { ColorAnimation { duration: 200 } }
                    Rectangle { width: 14; height: 14; radius: 7; color: "white"; y: 2
                        x: Appearance.lgEnabled ? 20 : 2; Behavior on x { NumberAnimation { duration: 200 } } }
                    MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor
                        onClicked: Appearance.setLgEnabled(!Appearance.lgEnabled) }
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

    SectionTitle { text: "Icon Theme" }
    SettingsCard {
        Item {
            width: parent.width; height: 44
            Text { anchors.left: parent.left; anchors.verticalCenter: parent.verticalCenter; text: "Icon Theme"; font.pixelSize: 12; font.weight: Font.DemiBold; color: Theme.textPrimary }
            ComboBox {
                anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter
                model: Appearance.iconThemes.length > 0 ? Appearance.iconThemes : [Appearance.iconTheme || "Default"]
                currentIndex: Math.max(0, model.indexOf(Appearance.iconTheme))
                width: 160; height: 28; font.pixelSize: 12
                onActivated: Appearance.setIconTheme(currentText)
            }
        }
    }
    Spacer {}
}
