import QtQuick
import QtQuick.Layouts
import "../components"

PageBase {
    title: "General"
    Component.onCompleted: { SysInfo.refresh(); Startup.refresh() }

    // Hero card like Electron's general-hero-card
    SettingsCard {
        Column {
            width: parent.width; spacing: 8; topPadding: 12; bottomPadding: 12
            anchors.horizontalCenter: parent.horizontalCenter

            Image {
                source: "file:///home/alxb421/Desktop/pkgbuilds/pearos-settings-app/port/assets/application-default-icon.svg"
                width: 56; height: 56; fillMode: Image.PreserveAspectFit
                anchors.horizontalCenter: parent.horizontalCenter
            }
            Text {
                text: "General"
                font.pixelSize: 16; font.weight: Font.DemiBold; color: Theme.textPrimary
                anchors.horizontalCenter: parent.horizontalCenter
            }
            Text {
                text: "Manage your overall setup and preferences for Pear, such as software updates, device language, and more."
                font.pixelSize: 12; color: Theme.textSecondary; wrapMode: Text.WordWrap
                width: parent.width; horizontalAlignment: Text.AlignHCenter
            }
        }
    }
    Spacer {}

    // Menu items (with chevron)
    SettingsCard {
        Column {
            width: parent.width; spacing: 0
            Repeater {
                id: menuRep1
                model: [
                    { icon: "system-help.svg",              label: "About",           idx: 28 },
                    { icon: "system-software-update.svg",   label: "Software Update", idx: 29 },
                    { icon: "preferences-system-disks.svg", label: "Storage",         idx: 30 },
                ]
                delegate: Column {
                    width: parent.width; spacing: 0
                    Item {
                        id: row1
                        width: parent.width; height: 40
                        property bool hov: false
                        Rectangle { anchors.fill: parent; color: row1.hov ? Theme.hoverBg : "transparent"; radius: 4 }
                        Row {
                            anchors.left: parent.left; anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter
                            spacing: 10
                            Image {
                                source: "file:///home/alxb421/Desktop/pkgbuilds/pearos-settings-app/port/assets/" + modelData.icon
                                width: 20; height: 20; fillMode: Image.PreserveAspectFit
                                anchors.verticalCenter: parent.verticalCenter
                            }
                            Text { text: modelData.label; font.pixelSize: 13; color: Theme.textPrimary; anchors.verticalCenter: parent.verticalCenter }
                        }
                        Text { text: "›"; anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter; font.pixelSize: 18; color: Theme.textSecondary }
                        MouseArea {
                            anchors.fill: parent; hoverEnabled: true; cursorShape: Qt.PointingHandCursor
                            onEntered: row1.hov = true; onExited: row1.hov = false
                            onClicked: Navigator.currentIdx = modelData.idx
                        }
                    }
                    Rectangle { width: parent.width; height: 1; color: Theme.divider; visible: index < menuRep1.count - 1 }
                }
            }
        }
    }
    Spacer {}

    SettingsCard {
        Column {
            width: parent.width; spacing: 0
            Repeater {
                id: menuRep2
                model: [
                    { icon: "preferences-system-time.svg",  label: "Date & Time",              idx: 31 },
                    { icon: "cs-language.svg",              label: "Language & Region",         idx: -1 },
                    { icon: "preferences-system-login.svg", label: "Login Items & Extensions",  idx: -1 },
                ]
                delegate: Column {
                    width: parent.width; spacing: 0
                    Item {
                        id: row2
                        width: parent.width; height: 40
                        property bool hov: false
                        Rectangle { anchors.fill: parent; color: row2.hov ? Theme.hoverBg : "transparent"; radius: 4 }
                        Row {
                            anchors.left: parent.left; anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter
                            spacing: 10
                            Image {
                                source: "file:///home/alxb421/Desktop/pkgbuilds/pearos-settings-app/port/assets/" + modelData.icon
                                width: 20; height: 20; fillMode: Image.PreserveAspectFit
                                anchors.verticalCenter: parent.verticalCenter
                            }
                            Text { text: modelData.label; font.pixelSize: 13; color: Theme.textPrimary; anchors.verticalCenter: parent.verticalCenter }
                        }
                        Text { text: "›"; anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter; font.pixelSize: 18; color: Theme.textSecondary }
                        MouseArea {
                            anchors.fill: parent; hoverEnabled: true; cursorShape: Qt.PointingHandCursor
                            onEntered: row2.hov = true; onExited: row2.hov = false
                            onClicked: if (modelData.idx >= 0) Navigator.currentIdx = modelData.idx
                        }
                    }
                    Rectangle { width: parent.width; height: 1; color: Theme.divider; visible: index < menuRep2.count - 1 }
                }
            }
        }
    }
    Spacer {}
}
