import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "../components"

PageBase {
    title: "Privacy & Security"
    readonly property string ap: "file:///home/alxb421/Desktop/pkgbuilds/pearos-settings-app/port/assets/"

    // Header card
    SettingsCard {
        Row {
            width: parent.width; height: 72; spacing: 12
            Image { source: ap + "cs-privacy.svg"; width: 40; height: 40; fillMode: Image.PreserveAspectFit; anchors.verticalCenter: parent.verticalCenter }
            Column {
                anchors.verticalCenter: parent.verticalCenter; spacing: 4; width: parent.width - 60
                Text { text: "Privacy"; font.pixelSize: 15; font.weight: Font.DemiBold; color: Theme.textPrimary }
                Text {
                    text: "Control which apps can access your data, location, camera, and microphone, and manage safety protections."
                    font.pixelSize: 12; color: Theme.textSecondary; wrapMode: Text.WordWrap; width: parent.width
                }
            }
        }
    }
    Spacer {}

    // Location Services
    SettingsCard {
        Item {
            width: parent.width; height: 44
            Row { anchors.left: parent.left; anchors.verticalCenter: parent.verticalCenter; spacing: 12
                Image { source: ap + "cs-privacy.svg"; width: 22; height: 22; fillMode: Image.PreserveAspectFit; anchors.verticalCenter: parent.verticalCenter }
                Text { text: "Location Services"; font.pixelSize: 12; font.weight: Font.DemiBold; color: Theme.textPrimary; anchors.verticalCenter: parent.verticalCenter }
            }
            Text { anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter; text: "Unavailable"; font.pixelSize: 12; color: Theme.textSecondary }
        }
    }
    Spacer {}

    // Privacy items group 1
    SettingsCard {
        Column {
            width: parent.width; spacing: 0
            Repeater {
                id: priv1Rep
                model: [
                    { icon: "preferences-system-time.svg",        label: "Calendars" },
                    { icon: "system-users.svg",                    label: "Contacts" },
                    { icon: "preferences-document.svg",            label: "Files & Folders" },
                    { icon: "preferences-system-disks.svg",        label: "Full Disk Access" },
                    { icon: "preferences-other.svg",               label: "Home Accessories" },
                    { icon: "preferences-desktop-multimedia.svg",  label: "Media & Pear Music" },
                    { icon: "preferences-web-browser-ssl.svg",     label: "Passkeys Access for Web Browsers" },
                    { icon: "preferences-desktop-wallpaper.svg",   label: "Photos" },
                    { icon: "preferences-desktop-notification-bell.svg", label: "Reminders" },
                ]
                delegate: Column {
                    width: parent.width; spacing: 0
                    Item {
                        width: parent.width; height: 44
                        Row { anchors.left: parent.left; anchors.verticalCenter: parent.verticalCenter; spacing: 12
                            Image { source: ap + modelData.icon; width: 22; height: 22; fillMode: Image.PreserveAspectFit; anchors.verticalCenter: parent.verticalCenter }
                            Text { text: modelData.label; font.pixelSize: 12; font.weight: Font.DemiBold; color: Theme.textPrimary; anchors.verticalCenter: parent.verticalCenter }
                        }
                        Text { anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter; text: "Unavailable"; font.pixelSize: 12; color: Theme.textSecondary }
                    }
                    Rectangle { width: parent.width; height: 1; color: Theme.divider; visible: index < priv1Rep.count - 1 }
                }
            }
        }
    }
    Spacer {}

    // Privacy items group 2
    SettingsCard {
        Column {
            width: parent.width; spacing: 0
            Repeater {
                id: priv2Rep
                model: [
                    { icon: "accessib.png",                           label: "Accessibility" },
                    { icon: "preferences-plugin.svg",                 label: "App Management" },
                    { icon: "preferences-other.svg",                  label: "Automation" },
                    { icon: "preferences-system-bluetooth.svg",       label: "Bluetooth" },
                    { icon: "preferences-other.svg",                  label: "Camera" },
                    { icon: "preferences-other.svg",                  label: "Developer Tools" },
                    { icon: "preferences-other.svg",                  label: "Focus" },
                    { icon: "preferences-desktop-keyboard.svg",       label: "Input Monitoring" },
                    { icon: "preferences-system-network.svg",         label: "Local Network" },
                    { icon: "preferences-sound.svg",                  label: "Microphone" },
                    { icon: "preferences-other.svg",                  label: "Motion & Fitness" },
                    { icon: "preferences-desktop-remote-desktop.svg", label: "Remote Desktop" },
                    { icon: "preferences-desktop-sound.svg",          label: "Screen & System Audio Recording" },
                    { icon: "preferences-desktop-text-to-speech.svg", label: "Speech Recognition" },
                ]
                delegate: Column {
                    width: parent.width; spacing: 0
                    Item {
                        width: parent.width; height: 44
                        Row { anchors.left: parent.left; anchors.verticalCenter: parent.verticalCenter; spacing: 12
                            Image { source: ap + modelData.icon; width: 22; height: 22; fillMode: Image.PreserveAspectFit; anchors.verticalCenter: parent.verticalCenter }
                            Text { text: modelData.label; font.pixelSize: 12; font.weight: Font.DemiBold; color: Theme.textPrimary; anchors.verticalCenter: parent.verticalCenter }
                        }
                        Text { anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter; text: "Unavailable"; font.pixelSize: 12; color: Theme.textSecondary }
                    }
                    Rectangle { width: parent.width; height: 1; color: Theme.divider; visible: index < priv2Rep.count - 1 }
                }
            }
        }
    }
    Spacer {}

    SectionTitle { text: "Security" }
    SettingsCard {
        Item {
            width: parent.width; height: 44
            Text { anchors.left: parent.left; anchors.verticalCenter: parent.verticalCenter; text: "Allow applications from"; font.pixelSize: 12; font.weight: Font.DemiBold; color: Theme.textPrimary }
            ComboBox {
                anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter
                model: ["Everyone"]
                currentIndex: 0; width: 120; height: 28; font.pixelSize: 12
            }
        }
    }
    Spacer {}
}
