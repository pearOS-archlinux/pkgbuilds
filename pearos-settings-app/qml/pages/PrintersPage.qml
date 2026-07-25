import QtQuick
import QtQuick.Controls
import "../components"

PageBase {
    title: "Printers & Scanners"
    Component.onCompleted: { Printers.refresh(); Printers.refreshJobs() }

    property string addUri: ""
    property string addName: ""

    SettingsCard {
        Column {
            width: parent.width; spacing: 0

            Item {
                width: parent.width; height: 40
                visible: Printers.printers.length === 0
                Text { anchors.centerIn: parent; text: "No printers configured"; font.pixelSize: 12; color: Theme.textSecondary }
            }

            Repeater {
                model: Printers.printers
                delegate: Column {
                    width: parent.width
                    Item {
                        width: parent.width; height: 52
                        Column {
                            anchors.left: parent.left; anchors.verticalCenter: parent.verticalCenter; spacing: 2
                            Row {
                                spacing: 6
                                Text { text: modelData.name; font.pixelSize: 13; font.weight: Font.DemiBold; color: Theme.textPrimary }
                                Text { visible: modelData.isDefault; text: "Default"; font.pixelSize: 10; color: Theme.accent }
                            }
                            Text { text: modelData.status; font.pixelSize: 11; color: Theme.textSecondary }
                        }
                        Row {
                            anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter; spacing: 6

                            Rectangle {
                                visible: !modelData.isDefault
                                height: 26; width: setDefLbl.implicitWidth + 14; radius: 6
                                color: Qt.rgba(0,0,0,0.08)
                                Text { id: setDefLbl; anchors.centerIn: parent; text: "Set Default"; font.pixelSize: 11; color: Theme.textPrimary }
                                MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor
                                    onClicked: Printers.setDefaultPrinter(modelData.name) }
                            }
                            Rectangle {
                                height: 26; width: toggleLbl.implicitWidth + 14; radius: 6
                                color: Qt.rgba(0,0,0,0.08)
                                Text { id: toggleLbl; anchors.centerIn: parent
                                    text: modelData.enabled ? "Pause" : "Resume"; font.pixelSize: 11; color: Theme.textPrimary }
                                MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor
                                    onClicked: Printers.setPrinterEnabled(modelData.name, !modelData.enabled) }
                            }
                            Rectangle {
                                height: 26; width: removeLbl.implicitWidth + 14; radius: 6
                                color: Qt.rgba(239,68,68,0.15)
                                Text { id: removeLbl; anchors.centerIn: parent; text: "Remove"; font.pixelSize: 11; color: "#EF4444" }
                                MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor
                                    onClicked: Printers.removePrinter(modelData.name) }
                            }
                        }
                    }
                    Rectangle { width: parent.width; height: 1; color: Theme.divider; visible: index < Printers.printers.length - 1 }
                }
            }
        }
    }
    Spacer {}

    SectionTitle { text: "Add Printer" }
    SettingsCard {
        Column {
            width: parent.width; spacing: 8; topPadding: 8; bottomPadding: 8

            Text { text: "Add a network printer by its IPP/driverless URI (e.g. ipp://192.168.1.50/ipp/print)."
                   font.pixelSize: 11; color: Theme.textSecondary; wrapMode: Text.WordWrap; width: parent.width }

            Row {
                width: parent.width; spacing: 8
                TextField {
                    width: (parent.width - 8) * 0.4; height: 32; font.pixelSize: 12
                    placeholderText: "Name"
                    onTextChanged: addName = text
                }
                TextField {
                    width: (parent.width - 8) * 0.6 - 90; height: 32; font.pixelSize: 12
                    placeholderText: "ipp://host/ipp/print"
                    onTextChanged: addUri = text
                }
                Rectangle {
                    height: 32; width: 80; radius: 6
                    color: (addName.length > 0 && addUri.length > 0) ? Theme.accent : Qt.rgba(0,0,0,0.08)
                    Text { anchors.centerIn: parent; text: "Add"; font.pixelSize: 12
                        color: (addName.length > 0 && addUri.length > 0) ? "white" : Theme.textSecondary }
                    MouseArea {
                        anchors.fill: parent; cursorShape: Qt.PointingHandCursor
                        onClicked: if (addName.length > 0 && addUri.length > 0) Printers.addPrinterByUri(addName, addUri)
                    }
                }
            }
        }
    }
    Spacer {}

    SectionTitle { text: "Print Queue" }
    SettingsCard {
        Column {
            width: parent.width; spacing: 0
            Item {
                width: parent.width; height: 40
                visible: Printers.jobs.length === 0
                Text { anchors.centerIn: parent; text: "No active print jobs"; font.pixelSize: 12; color: Theme.textSecondary }
            }
            Repeater {
                model: Printers.jobs
                delegate: Column {
                    width: parent.width
                    Item {
                        width: parent.width; height: 40
                        Column {
                            anchors.left: parent.left; anchors.verticalCenter: parent.verticalCenter
                            Text { text: modelData.printer + " — " + modelData.user; font.pixelSize: 12; color: Theme.textPrimary }
                            Text { text: modelData.date; font.pixelSize: 10; color: Theme.textSecondary }
                        }
                        Rectangle {
                            anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter
                            height: 24; width: cancelLbl.implicitWidth + 14; radius: 6
                            color: Qt.rgba(239,68,68,0.15)
                            Text { id: cancelLbl; anchors.centerIn: parent; text: "Cancel"; font.pixelSize: 11; color: "#EF4444" }
                            MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor
                                onClicked: Printers.cancelJob(modelData.id) }
                        }
                    }
                    Rectangle { width: parent.width; height: 1; color: Theme.divider; visible: index < Printers.jobs.length - 1 }
                }
            }
        }
    }
    Spacer {}
}
