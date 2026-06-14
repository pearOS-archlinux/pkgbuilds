import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "../components"

PageBase {
    title: "Software Update"
    Component.onCompleted: Updates.checkUpdates()

    // Updates container
    SettingsCard {
        Column {
            width: parent.width; spacing: 0
            Item { width: parent.width; height: 8 }

            Item {
                width: parent.width; height: 52
                Column {
                    anchors.left: parent.left; anchors.verticalCenter: parent.verticalCenter; spacing: 3
                    Text {
                        text: Updates.checking ? "Checking for updates..."
                            : Updates.updateCount > 0 ? Updates.updateCount + " update(s) available"
                            : "Your software is up to date."
                        font.pixelSize: 14; font.weight: Font.DemiBold; color: Theme.textPrimary
                    }
                    Text {
                        text: Updates.updating ? "Installing updates..." : ""
                        font.pixelSize: 12; color: Theme.textSecondary; visible: text.length > 0
                    }
                }
                Row {
                    anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter; spacing: 8
                    visible: Updates.updateCount > 0 && !Updates.updating

                    Rectangle {
                        height: 30; width: upNowLabel.implicitWidth + 20; radius: 6; color: "#3B82F6"
                        Text { id: upNowLabel; anchors.centerIn: parent; text: "Update Now"; font.pixelSize: 12; color: "white" }
                        MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor; onClicked: Updates.applyUpdates() }
                    }
                }
            }

            // List of available updates
            Column {
                width: parent.width; spacing: 0; visible: Updates.updateCount > 0
                Rectangle { width: parent.width; height: 1; color: Theme.divider }
                Repeater {
                    id: updRep; model: Updates.updates
                    delegate: Column {
                        width: parent.width; spacing: 0
                        Item {
                            width: parent.width; height: 40
                            Text { anchors.left: parent.left; anchors.verticalCenter: parent.verticalCenter; text: modelData.name; font.pixelSize: 13; color: Theme.textPrimary }
                            Text {
                                anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter
                                text: (modelData.current || "") + " → " + (modelData.latest || "")
                                font.pixelSize: 11; color: Theme.textSecondary
                            }
                        }
                        Rectangle { width: parent.width; height: 1; color: Theme.divider; visible: index < updRep.count - 1 }
                    }
                }
            }

            Item { width: parent.width; height: 8 }
        }
    }
    Spacer {}

    SettingsCard {
        Item {
            width: parent.width; height: 44
            Text { anchors.left: parent.left; anchors.verticalCenter: parent.verticalCenter; text: "Beta Updates"; font.pixelSize: 12; font.weight: Font.DemiBold; color: Theme.textPrimary }
            Rectangle {
                id: betaToggle; width: 36; height: 18; radius: 9; property bool on: false
                anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter
                color: on ? "#3B82F6" : "#cccccc"
                Behavior on color { ColorAnimation { duration: 200 } }
                Rectangle { width: 14; height: 14; radius: 7; color: "white"; y: 2; x: betaToggle.on ? 20 : 2; Behavior on x { NumberAnimation { duration: 200 } } }
                MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor; onClicked: betaToggle.on = !betaToggle.on }
            }
        }
    }
    Spacer {}

    SettingsCard {
        Item {
            width: parent.width; height: 44
            Text {
                anchors.left: parent.left; anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter
                text: "Use of this software is subject to the original license agreement that accompanied the software being updated"
                font.pixelSize: 12; color: Theme.textSecondary; wrapMode: Text.WordWrap
            }
        }
    }
    Spacer {}
}
