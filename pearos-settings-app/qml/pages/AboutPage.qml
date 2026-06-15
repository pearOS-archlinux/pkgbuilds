import QtQuick
import QtQuick.Layouts
import "../components"

PageBase {
    title: "About"
    Component.onCompleted: { SysInfo.refresh(); Display.refreshDisplays() }

    // Device header
    Column {
        width: parent.width
        spacing: 4
        topPadding: 10; bottomPadding: 20

        Image {
            source: "file:///usr/share/extras/system-settings/assets/computer.svg"
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
                    { label: "Coverage Status", value: "Not registered" },
                    { label: "Parts & Service", value: "Standard coverage" },
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
            Image {
                source: "file:///usr/share/extras/system-settings/assets/pear.svg"
                width: 36; height: 36; fillMode: Image.PreserveAspectFit
                Layout.preferredWidth: 36; Layout.preferredHeight: 36
            }
            Column {
                Layout.fillWidth: true; spacing: 3
                Text { text: SysInfo.osName || "PearOS"; font.pixelSize: 14; font.weight: Font.DemiBold; color: Theme.textPrimary }
                Text { text: "Version  " + (SysInfo.osVersion || SysInfo.kernelVersion); font.pixelSize: 12; color: Theme.textSecondary }
            }
        }
    }
    Spacer {}

    SectionTitle { text: "Displays" }
    SettingsCard {
        Column {
            width: parent.width; spacing: 0

            Item {
                width: parent.width; height: 52
                Row {
                    anchors.left: parent.left; anchors.verticalCenter: parent.verticalCenter; spacing: 12
                    Image {
                        source: "file:///usr/share/extras/system-settings/assets/cs-display.svg"
                        width: 32; height: 32; fillMode: Image.PreserveAspectFit
                        anchors.verticalCenter: parent.verticalCenter
                    }
                    Column {
                        anchors.verticalCenter: parent.verticalCenter; spacing: 3
                        Text {
                            text: Display.displays.length > 0 ? (Display.displays[0].name || "Built-in Display") : "Built-in Display"
                            font.pixelSize: 13; font.weight: Font.Medium; color: Theme.textPrimary
                        }
                        Text {
                            text: Display.displays.length > 0
                                ? (Display.displays[0].currentWidth || "") + " × " + (Display.displays[0].currentHeight || "") + " @ " + Math.round(Display.displays[0].currentRefreshRate || 0) + "Hz"
                                : ""
                            font.pixelSize: 12; color: Theme.textSecondary
                        }
                    }
                }
            }
            Rectangle { width: parent.width; height: 1; color: Theme.divider }
            Item {
                width: parent.width; height: 44
                Rectangle {
                    anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter
                    height: 28; width: dsLbl.implicitWidth + 20; radius: 6
                    color: Qt.rgba(0,0,0,0.08)
                    Text { id: dsLbl; anchors.centerIn: parent; text: "Display Settings…"; font.pixelSize: 12; color: Theme.textPrimary }
                    MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor; onClicked: Navigator.navigateTo(32) }
                }
            }
        }
    }
    Spacer {}

    SectionTitle { text: "Storage" }
    SettingsCard {
        Column {
            width: parent.width; spacing: 0

            Item {
                width: parent.width; height: 52
                Row {
                    anchors.left: parent.left; anchors.verticalCenter: parent.verticalCenter; spacing: 12
                    Image {
                        source: "file:///usr/share/extras/system-settings/assets/preferences-system-disks.svg"
                        width: 32; height: 32; fillMode: Image.PreserveAspectFit
                        anchors.verticalCenter: parent.verticalCenter
                    }
                    Column {
                        anchors.verticalCenter: parent.verticalCenter; spacing: 3
                        Text { text: SysInfo.diskName || "Main Storage"; font.pixelSize: 13; font.weight: Font.Medium; color: Theme.textPrimary }
                        Text {
                            text: SysInfo.totalDisk > 0 ? Math.round(SysInfo.usedDisk/1073741824) + " GB of " + Math.round(SysInfo.totalDisk/1073741824) + " GB used" : ""
                            font.pixelSize: 12; color: Theme.textSecondary
                        }
                    }
                }
            }
            Rectangle { width: parent.width; height: 1; color: Theme.divider }
            Item {
                width: parent.width; height: 44
                Rectangle {
                    anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter
                    height: 28; width: ssLbl.implicitWidth + 20; radius: 6
                    color: Qt.rgba(0,0,0,0.08)
                    Text { id: ssLbl; anchors.centerIn: parent; text: "Storage Settings…"; font.pixelSize: 12; color: Theme.textPrimary }
                    MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor; onClicked: Navigator.navigateTo(30) }
                }
            }
        }
    }
    Spacer {}

    // Footer
    Column {
        width: parent.width; spacing: 12; bottomPadding: 16

        Rectangle {
            anchors.horizontalCenter: parent.horizontalCenter
            height: 28; width: srLbl.implicitWidth + 20; radius: 6
            color: Qt.rgba(0,0,0,0.08)
            Text { id: srLbl; anchors.centerIn: parent; text: "System Report…"; font.pixelSize: 12; color: Theme.textPrimary }
            MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor
                onClicked: Qt.openUrlExternally("file:///usr/bin/hardinfo") }
        }
        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            text: "© 2017-2026 Pear Software and Services L.T.D. All Rights Reserved"
            font.pixelSize: 11; color: Theme.textSecondary
        }
    }
}
