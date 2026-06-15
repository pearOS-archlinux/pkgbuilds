import QtQuick
import QtQuick.Layouts
import "../components"

PageBase {
    title: "General"
    Component.onCompleted: { SysInfo.refresh(); Startup.refresh() }

    readonly property string assetBase: "file:///usr/share/extras/system-settings/assets/"

    // Hero card
    SettingsCard {
        Column {
            width: parent.width; spacing: 8; topPadding: 12; bottomPadding: 12
            anchors.horizontalCenter: parent.horizontalCenter

            Image {
                source: assetBase + "application-default-icon.svg"
                width: 56; height: 56; fillMode: Image.PreserveAspectFit
                anchors.horizontalCenter: parent.horizontalCenter
            }
            Text {
                text: "General"
                font.pixelSize: 16; font.weight: Font.DemiBold; color: Theme.textPrimary
                anchors.horizontalCenter: parent.horizontalCenter
            }
            Text {
                text: "Manage your overall setup and preferences for Pear, such as software updates, device language, PearDrop, and more."
                font.pixelSize: 12; color: Theme.textSecondary; wrapMode: Text.WordWrap
                width: parent.width; horizontalAlignment: Text.AlignHCenter
            }
        }
    }
    Spacer {}

    component MenuCard: SettingsCard {
        id: menuCardRoot
        property var items: []
        Column {
            width: parent.width; spacing: 0
            Repeater {
                id: rep; model: menuCardRoot.items
                delegate: Column {
                    width: parent.width; spacing: 0
                    Item {
                        id: mrow; width: parent.width; height: 40
                        property bool hov: false
                        Rectangle { anchors.fill: parent; color: mrow.hov ? Theme.hoverBg : "transparent"; radius: 4 }
                        Row {
                            anchors.left: parent.left; anchors.right: parent.right
                            anchors.verticalCenter: parent.verticalCenter; spacing: 10
                            Image {
                                source: "file:///usr/share/extras/system-settings/assets/" + modelData.icon
                                width: 20; height: 20; fillMode: Image.PreserveAspectFit
                                anchors.verticalCenter: parent.verticalCenter
                            }
                            Text { text: modelData.label; font.pixelSize: 13; color: Theme.textPrimary; anchors.verticalCenter: parent.verticalCenter }
                        }
                        Text { text: "›"; anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter; font.pixelSize: 18; color: Theme.textSecondary }
                        MouseArea {
                            anchors.fill: parent; hoverEnabled: true; cursorShape: Qt.PointingHandCursor
                            onEntered: mrow.hov = true; onExited: mrow.hov = false
                            onClicked: if (modelData.idx >= 0) Navigator.navigateTo(modelData.idx)
                        }
                    }
                    Rectangle { width: parent.width; height: 1; color: Theme.divider; visible: index < rep.count - 1 }
                }
            }
        }
    }

    MenuCard {
        items: [
            { icon: "system-help.svg",              label: "About",           idx: 28 },
            { icon: "system-software-update.svg",   label: "Software Update", idx: 29 },
            { icon: "preferences-system-disks.svg", label: "Storage",         idx: 30 },
        ]
    }
    Spacer {}

    MenuCard {
        items: [
            { icon: "preferences-other.svg", label: "PearCare & Warranty", idx: -1 },
        ]
    }
    Spacer {}

    MenuCard {
        items: [
            { icon: "preferences-other.svg", label: "PearDrop & Handoff", idx: -1 },
        ]
    }
    Spacer {}

    MenuCard {
        items: [
            { icon: "preferences-desktop-user-password.svg", label: "AutoFill & Passwords",    idx: -1 },
            { icon: "preferences-system-time.svg",           label: "Date & Time",              idx: 31 },
            { icon: "cs-language.svg",                       label: "Language & Region",        idx: -1 },
            { icon: "preferences-system-login.svg",          label: "Login Items & Extensions", idx: -1 },
            { icon: "preferences-system-network-sharing.svg",label: "Sharing",                  idx: -1 },
            { icon: "preferences-system-disks.svg",          label: "Startup Disk",             idx: -1 },
            { icon: "preferences-system-backup.svg",         label: "Time Machine",             idx: -1 },
        ]
    }
    Spacer {}

    MenuCard {
        items: [
            { icon: "preferences-other.svg", label: "Device Management", idx: -1 },
        ]
    }
    Spacer {}

    MenuCard {
        items: [
            { icon: "preferences-other.svg", label: "Transfer or Reset", idx: -1 },
        ]
    }
    Spacer {}
}
