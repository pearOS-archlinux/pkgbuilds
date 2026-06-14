import QtQuick
import QtQuick.Layouts
import "../components"

PageBase {
    title: "About"
    Component.onCompleted: SysInfo.refresh()

    // Device header
    Column {
        width: parent.width
        spacing: 4
        topPadding: 10; bottomPadding: 20

        Image {
            source: "file:///home/alxb421/Desktop/pkgbuilds/pearos-settings-app/port/assets/computer.svg"
            width: 80; height: 80
            fillMode: Image.PreserveAspectFit
            anchors.horizontalCenter: parent.horizontalCenter
        }
        Text {
            text: SysInfo.osName || "PearOS Linux"
            font.pixelSize: 18; font.weight: Font.DemiBold; color: Theme.textPrimary
            anchors.horizontalCenter: parent.horizontalCenter
        }
    }

    SettingsCard {
        Column {
            width: parent.width; spacing: 0
            Repeater {
                id: aboutRows
                model: [
                    { label: "Name",           value: SysInfo.hostName },
                    { label: "Chip",           value: SysInfo.cpuModel },
                    { label: "Memory",         value: SysInfo.totalRam > 0 ? (Math.round(SysInfo.totalRam / 107374182.4) / 10).toFixed(1) + " GB" : "" },
                    { label: "Serial Number",  value: SysInfo.serialNumber },
                ]
                delegate: Column {
                    width: parent.width; spacing: 0
                    visible: modelData.value && modelData.value.length > 0
                    Item {
                        width: parent.width; height: 32
                        Text { anchors.left: parent.left; anchors.verticalCenter: parent.verticalCenter; text: modelData.label; font.pixelSize: 12; color: Theme.textSecondary }
                        Text { anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter; text: modelData.value; font.pixelSize: 12; color: Theme.textPrimary; elide: Text.ElideRight; maximumLineCount: 1 }
                    }
                    Rectangle { width: parent.width; height: 1; color: Theme.divider; visible: index < aboutRows.count - 1 }
                }
            }
        }
    }
    Spacer {}

    SectionTitle { text: "pearOS" }
    SettingsCard {
        RowLayout {
            width: parent.width; height: 52; spacing: 12
            Column {
                Layout.fillWidth: true; spacing: 3
                Text { text: SysInfo.osName || "PearOS"; font.pixelSize: 14; font.weight: Font.DemiBold; color: Theme.textPrimary }
                Text { text: "Version  " + (SysInfo.osVersion || SysInfo.kernelVersion); font.pixelSize: 12; color: Theme.textSecondary }
            }
        }
    }
    Spacer {}

    SectionTitle { text: "Storage" }
    SettingsCard {
        Item {
            width: parent.width; height: 52
            Column {
                anchors.left: parent.left; anchors.verticalCenter: parent.verticalCenter; spacing: 6
                Text { text: "Macintosh HD"; font.pixelSize: 13; font.weight: Font.DemiBold; color: Theme.textPrimary }
                Text {
                    text: SysInfo.totalDisk > 0 ? Math.round(SysInfo.usedDisk/1073741824) + " GB of " + Math.round(SysInfo.totalDisk/1073741824) + " GB used" : ""
                    font.pixelSize: 12; color: Theme.textSecondary
                }
            }
        }
    }
    Spacer {}
}
