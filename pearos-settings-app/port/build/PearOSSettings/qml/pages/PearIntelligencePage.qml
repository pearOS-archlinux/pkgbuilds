import QtQuick
import QtQuick.Controls
import "../components"

PageBase {
    title: "Pear Intelligence & Piri"

    readonly property string ap: "file:///home/alxb421/Desktop/pkgbuilds/pearos-settings-app/port/assets/"

    property string piriStatus: "not_installed"  // not_installed | installing | installed | disabled
    property int    downloadPct: 0
    property bool   showInMenuBar: false
    property bool   piriEnabled: false

    Component.onCompleted: piriStatus = "not_installed"

    // Hero card — image + description
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
                    text: "Pear Intelligence & Piri is an intelligent assistant that helps you get things done. It can answer questions, provide information, and assist with various tasks."
                    font.pixelSize: 12; color: Theme.textSecondary
                    wrapMode: Text.WordWrap; width: parent.width
                }
            }
        }
    }

    Spacer {}

    // Main controls card
    SettingsCard {
        Column {
            width: parent.width; spacing: 0

            // Pear Intelligence toggle (always disabled — cloud feature)
            Item {
                width: parent.width; height: 44
                Text {
                    anchors.left: parent.left; anchors.verticalCenter: parent.verticalCenter
                    text: "Pear Intelligence"; font.pixelSize: 13; color: Theme.textPrimary
                }
                Rectangle {
                    width: 36; height: 18; radius: 9
                    anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter
                    color: Qt.rgba(0.5, 0.5, 0.5, 0.3)
                    opacity: 0.5
                    Rectangle {
                        width: 14; height: 14; radius: 7; color: "white"; y: 2; x: 2
                    }
                }
            }

            Rectangle { width: parent.width; height: 1; color: Theme.divider }

            // Piri enable section
            Item {
                width: parent.width; height: 54
                Column {
                    anchors.left: parent.left; anchors.verticalCenter: parent.verticalCenter
                    spacing: 2
                    Text {
                        text: "Downloaded model size: 2.3 GB"
                        font.pixelSize: 13; color: Theme.textPrimary
                    }
                    // Progress bar (shown while installing)
                    Item {
                        width: 200; height: 8
                        visible: piriStatus === "installing"
                        Rectangle {
                            anchors.fill: parent; radius: 4
                            color: Qt.rgba(0.5, 0.5, 0.5, 0.3)
                            Rectangle {
                                anchors.left: parent.left; anchors.top: parent.top; anchors.bottom: parent.bottom
                                width: parent.width * downloadPct / 100; radius: 4; color: "#3B82F6"
                                Behavior on width { NumberAnimation { duration: 300 } }
                            }
                        }
                    }
                }
                // Enable/Disable buttons
                Row {
                    anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter
                    spacing: 8

                    Rectangle {
                        visible: piriStatus === "not_installed" || piriStatus === "disabled"
                        width: enableLabel.width + 24; height: 28; radius: 6
                        color: Qt.rgba(1,1,1,0.15); border.color: Qt.rgba(1,1,1,0.25); border.width: 1
                        Text {
                            id: enableLabel; anchors.centerIn: parent
                            text: "Enable Piri"
                            font.pixelSize: 12; color: Theme.textPrimary
                        }
                        MouseArea {
                            anchors.fill: parent; cursorShape: Qt.PointingHandCursor
                            onClicked: { piriStatus = "installing"; downloadPct = 0 }
                        }
                    }

                    Rectangle {
                        visible: piriStatus === "installing"
                        width: stopLabel.width + 24; height: 28; radius: 6
                        color: Qt.rgba(1,1,1,0.15); border.color: Qt.rgba(1,1,1,0.25); border.width: 1
                        Text {
                            id: stopLabel; anchors.centerIn: parent
                            text: "Stop"; font.pixelSize: 12; color: Theme.textPrimary
                        }
                        MouseArea {
                            anchors.fill: parent; cursorShape: Qt.PointingHandCursor
                            onClicked: piriStatus = "not_installed"
                        }
                    }

                    Rectangle {
                        visible: piriStatus === "installed"
                        width: disableLabel.width + 24; height: 28; radius: 6
                        color: Qt.rgba(1,1,1,0.15); border.color: Qt.rgba(1,1,1,0.25); border.width: 1
                        Text {
                            id: disableLabel; anchors.centerIn: parent
                            text: "Stop & Disable Piri"; font.pixelSize: 12; color: Theme.textPrimary
                        }
                        MouseArea {
                            anchors.fill: parent; cursorShape: Qt.PointingHandCursor
                            onClicked: piriStatus = "disabled"
                        }
                    }
                }
            }

            Rectangle { width: parent.width; height: 1; color: Theme.divider }

            // Show Piri in Menu Bar
            Item {
                width: parent.width; height: 44
                Text {
                    anchors.left: parent.left; anchors.verticalCenter: parent.verticalCenter
                    text: "Show Piri in Menu Bar"; font.pixelSize: 13; color: Theme.textPrimary
                }
                Rectangle {
                    id: menuBarTog; width: 36; height: 18; radius: 9
                    anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter
                    color: showInMenuBar ? "#3B82F6" : Qt.rgba(0.5,0.5,0.5,0.4)
                    Behavior on color { ColorAnimation { duration: 200 } }
                    Rectangle {
                        width: 14; height: 14; radius: 7; color: "white"; y: 2
                        x: showInMenuBar ? 20 : 2
                        Behavior on x { NumberAnimation { duration: 200 } }
                    }
                    MouseArea {
                        anchors.fill: parent; cursorShape: Qt.PointingHandCursor
                        onClicked: showInMenuBar = !showInMenuBar
                    }
                }
            }
        }
    }

    Spacer {}

    // Piri Requests section
    Text {
        text: "Piri Requests"
        font.pixelSize: 13; font.weight: Font.Medium; color: Theme.textSecondary
        bottomPadding: 6
    }

    SettingsCard {
        Column {
            width: parent.width; spacing: 0

            // Keyboard shortcut
            Item {
                width: parent.width; height: 44
                Text {
                    anchors.left: parent.left; anchors.verticalCenter: parent.verticalCenter
                    text: "Keyboard shortcut"; font.pixelSize: 13; color: Theme.textPrimary
                }
                Rectangle {
                    anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter
                    width: shortcutText.width + 20; height: 28; radius: 6
                    color: Qt.rgba(1,1,1,0.1); border.color: Qt.rgba(1,1,1,0.2); border.width: 1
                    Text {
                        id: shortcutText; anchors.centerIn: parent
                        text: "WIP"; font.pixelSize: 12; color: Theme.textSecondary
                    }
                }
            }

            Rectangle { width: parent.width; height: 1; color: Theme.divider }

            // Language
            Item {
                width: parent.width; height: 44
                Text {
                    anchors.left: parent.left; anchors.verticalCenter: parent.verticalCenter
                    text: "Language"; font.pixelSize: 13; color: Theme.textPrimary
                }
                Text {
                    anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter
                    text: "English (United States)"; font.pixelSize: 13; color: Theme.textSecondary
                }
            }

            Rectangle { width: parent.width; height: 1; color: Theme.divider }

            // Voice
            Item {
                width: parent.width; height: 44
                Text {
                    anchors.left: parent.left; anchors.verticalCenter: parent.verticalCenter
                    text: "Voice"; font.pixelSize: 13; color: Theme.textPrimary
                }
                Row {
                    anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter
                    spacing: 8
                    Text {
                        anchors.verticalCenter: parent.verticalCenter
                        text: "Default voice"; font.pixelSize: 13; color: Theme.textSecondary
                    }
                    Rectangle {
                        width: selectLabel.width + 20; height: 28; radius: 6
                        color: Qt.rgba(1,1,1,0.1); border.color: Qt.rgba(1,1,1,0.2); border.width: 1
                        Text {
                            id: selectLabel; anchors.centerIn: parent
                            text: "Select…"; font.pixelSize: 12; color: Theme.textPrimary
                        }
                        MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor }
                    }
                }
            }

            Rectangle { width: parent.width; height: 1; color: Theme.divider }

            // Info text
            Item {
                width: parent.width; height: infoCol.implicitHeight + 24
                Column {
                    id: infoCol
                    anchors.left: parent.left; anchors.right: parent.right
                    anchors.verticalCenter: parent.verticalCenter
                    spacing: 0
                    Text {
                        width: parent.width
                        text: "Voice input is processed on your computer, but transcripts of your requests are sent to Pear."
                        font.pixelSize: 11; color: Theme.textSecondary; wrapMode: Text.WordWrap
                    }
                }
            }
        }
    }

    Spacer {}

    // Piri Suggestions & Privacy button
    Item {
        width: parent.width; height: 36
        Rectangle {
            anchors.centerIn: parent
            width: privacyLabel.width + 32; height: 32; radius: 8
            color: Qt.rgba(1,1,1,0.1); border.color: Qt.rgba(1,1,1,0.2); border.width: 1
            Text {
                id: privacyLabel; anchors.centerIn: parent
                text: "Piri Suggestions & Privacy"
                font.pixelSize: 13; color: Theme.textPrimary
            }
            MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor }
        }
    }

    Spacer {}
}
