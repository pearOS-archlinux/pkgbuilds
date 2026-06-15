import QtQuick
import QtQuick.Controls
import "../components"

PageBase {
    title: "Pear Intelligence & Piri"

    readonly property string ap: "file:///usr/share/extras/system-settings/assets/"

    Component.onCompleted: Piri.checkModelExists()

    // ── Hero ──────────────────────────────────────────────────────
    SettingsCard {
        Row {
            width: parent.width; spacing: 16; topPadding: 16; bottomPadding: 16
            Image {
                source: ap + "pear_intelligence.png"
                width: 64; height: 64; fillMode: Image.PreserveAspectFit
                anchors.verticalCenter: parent.verticalCenter
            }
            Column {
                spacing: 4; anchors.verticalCenter: parent.verticalCenter
                width: parent.width - 80
                Text {
                    text: "Pear Intelligence & Piri"
                    font.pixelSize: 15; font.weight: Font.DemiBold; color: Theme.textPrimary
                }
                Text {
                    text: "Piri is an intelligent assistant that helps you get things done. It can answer questions, provide information, and assist with various tasks."
                    font.pixelSize: 12; color: Theme.textSecondary
                    wrapMode: Text.WordWrap; width: parent.width
                }
            }
        }
    }

    Spacer {}

    // ── Main controls card ────────────────────────────────────────
    SettingsCard {
        Column {
            width: parent.width; spacing: 0

            // Pear Intelligence (cloud — always disabled)
            Item {
                width: parent.width; height: 44
                Text {
                    anchors.left: parent.left; anchors.verticalCenter: parent.verticalCenter
                    text: "Pear Intelligence"; font.pixelSize: 13; color: Theme.textPrimary
                }
                LiquidToggle {
                    anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter
                    checked: false; opacity: 0.4; enabled: false
                }
            }

            Rectangle { width: parent.width; height: 1; color: Theme.border; opacity: 0.5 }

            // Piri enable row
            Item {
                width: parent.width; height: Piri.downloading ? 62 : 44
                Behavior on height { NumberAnimation { duration: 200 } }

                Column {
                    anchors.left: parent.left; anchors.verticalCenter: parent.verticalCenter
                    spacing: 6

                    Text {
                        text: Piri.modelExists
                              ? "Piri is enabled"
                              : (Piri.downloading ? Piri.statusText : "Downloaded model size: 2.3 GB")
                        font.pixelSize: 13
                        color: Piri.modelExists ? "#34C759" : Theme.textPrimary
                    }

                    // Progress bar
                    Item {
                        visible: Piri.downloading
                        width: 220; height: 6
                        Rectangle {
                            anchors.fill: parent; radius: 3
                            color: Qt.rgba(0.5, 0.5, 0.5, 0.3)
                            Rectangle {
                                anchors.left: parent.left; anchors.top: parent.top; anchors.bottom: parent.bottom
                                width: parent.width * Piri.progress / 100; radius: 3; color: Theme.accent
                                Behavior on width { NumberAnimation { duration: 300 } }
                            }
                        }
                    }

                    // Error text
                    Text {
                        visible: Piri.errorText.length > 0 && !Piri.downloading
                        text: Piri.errorText
                        font.pixelSize: 11; color: "#ff5f57"; wrapMode: Text.WordWrap; width: 260
                    }
                }

                Row {
                    anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter
                    spacing: 8

                    // Enable button
                    Rectangle {
                        visible: !Piri.modelExists && !Piri.downloading
                        width: enLbl.implicitWidth + 24; height: 28; radius: 6
                        color: Qt.rgba(1,1,1,0.15); border.color: Qt.rgba(1,1,1,0.25); border.width: 1
                        Text { id: enLbl; anchors.centerIn: parent; text: "Enable Piri"; font.pixelSize: 12; color: Theme.textPrimary }
                        MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor; onClicked: Piri.downloadModel() }
                    }

                    // Stop button (while downloading)
                    Rectangle {
                        visible: Piri.downloading
                        width: stopLbl.implicitWidth + 24; height: 28; radius: 6
                        color: Qt.rgba(1,1,1,0.15); border.color: Qt.rgba(1,1,1,0.25); border.width: 1
                        Text { id: stopLbl; anchors.centerIn: parent; text: "Stop"; font.pixelSize: 12; color: Theme.textPrimary }
                        MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor; onClicked: Piri.cancelDownload() }
                    }

                    // Disable button (when installed)
                    Rectangle {
                        visible: Piri.modelExists && !Piri.downloading
                        width: disLbl.implicitWidth + 24; height: 28; radius: 6
                        color: Qt.rgba(1,1,1,0.15); border.color: Qt.rgba(1,1,1,0.25); border.width: 1
                        Text { id: disLbl; anchors.centerIn: parent; text: "Stop & Disable Piri"; font.pixelSize: 12; color: Theme.textPrimary }
                        MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor; onClicked: Piri.removeModel() }
                    }
                }
            }

            Rectangle { width: parent.width; height: 1; color: Theme.border; opacity: 0.5 }

            // Show Piri in Menu Bar
            Item {
                width: parent.width; height: 44
                Text {
                    anchors.left: parent.left; anchors.verticalCenter: parent.verticalCenter
                    text: "Show Piri in Menu Bar"; font.pixelSize: 13; color: Theme.textPrimary
                }
                LiquidToggle {
                    anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter
                    checked: Piri.showIcon
                    onToggled: function(v) { Piri.showIcon = v }
                }
            }
        }
    }

    Spacer {}

    // ── Piri Requests ─────────────────────────────────────────────
    Text {
        text: "Piri Requests"
        font.pixelSize: 13; font.weight: Font.Medium; color: Theme.textSecondary
        bottomPadding: 6
    }

    SettingsCard {
        Column {
            width: parent.width; spacing: 0

            Item {
                width: parent.width; height: 44
                Text { anchors.left: parent.left; anchors.verticalCenter: parent.verticalCenter; text: "Keyboard shortcut"; font.pixelSize: 13; color: Theme.textPrimary }
                Rectangle {
                    anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter
                    width: scLbl.implicitWidth + 20; height: 28; radius: 6
                    color: Qt.rgba(1,1,1,0.1); border.color: Qt.rgba(1,1,1,0.2); border.width: 1
                    Text { id: scLbl; anchors.centerIn: parent; text: "WIP"; font.pixelSize: 12; color: Theme.textSecondary }
                }
            }

            Rectangle { width: parent.width; height: 1; color: Theme.border; opacity: 0.5 }

            Item {
                width: parent.width; height: 44
                Text { anchors.left: parent.left; anchors.verticalCenter: parent.verticalCenter; text: "Language"; font.pixelSize: 13; color: Theme.textPrimary }
                Text { anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter; text: "English (United States)"; font.pixelSize: 13; color: Theme.textSecondary }
            }

            Rectangle { width: parent.width; height: 1; color: Theme.border; opacity: 0.5 }

            Item {
                width: parent.width; height: 44
                Text { anchors.left: parent.left; anchors.verticalCenter: parent.verticalCenter; text: "Voice"; font.pixelSize: 13; color: Theme.textPrimary }
                Row {
                    anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter; spacing: 8
                    Text { anchors.verticalCenter: parent.verticalCenter; text: "Default voice"; font.pixelSize: 13; color: Theme.textSecondary }
                    Rectangle {
                        width: selLbl.implicitWidth + 20; height: 28; radius: 6
                        color: Qt.rgba(1,1,1,0.1); border.color: Qt.rgba(1,1,1,0.2); border.width: 1
                        Text { id: selLbl; anchors.centerIn: parent; text: "Select…"; font.pixelSize: 12; color: Theme.textPrimary }
                        MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor }
                    }
                }
            }

            Rectangle { width: parent.width; height: 1; color: Theme.border; opacity: 0.5 }

            Item {
                width: parent.width; height: infoText.implicitHeight + 24
                Text {
                    id: infoText
                    anchors { left: parent.left; right: parent.right; verticalCenter: parent.verticalCenter }
                    text: "Voice input is processed on your computer, but transcripts of your requests are sent to Pear."
                    font.pixelSize: 11; color: Theme.textSecondary; wrapMode: Text.WordWrap
                }
            }
        }
    }

    Spacer {}

    Item {
        width: parent.width; height: 36
        Rectangle {
            anchors.centerIn: parent
            width: privLbl.implicitWidth + 32; height: 32; radius: 8
            color: Qt.rgba(1,1,1,0.1); border.color: Qt.rgba(1,1,1,0.2); border.width: 1
            Text { id: privLbl; anchors.centerIn: parent; text: "Piri Suggestions & Privacy"; font.pixelSize: 13; color: Theme.textPrimary }
            MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor }
        }
    }

    Spacer {}
}
