import QtQuick
import QtQuick.Controls
import "../components"

PageBase {
    title: "Touch ID & Password"
    Component.onCompleted: TouchID.refresh()

    // ── Password ──────────────────────────────────────────────────────────
    SectionTitle { text: "Password" }
    SettingsCard {
        Item {
            width: parent.width; height: 44
            Text {
                anchors.left: parent.left; anchors.verticalCenter: parent.verticalCenter
                text: "A login password has been set for this user."
                font.pixelSize: 13; color: Theme.textPrimary
            }
            Rectangle {
                anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter
                width: changeLbl.implicitWidth + 20; height: 28; radius: 6
                color: Qt.rgba(0,0,0,0.06); border.color: Theme.divider; border.width: 1
                Text {
                    id: changeLbl; anchors.centerIn: parent
                    text: "Change…"; font.pixelSize: 12; color: Theme.textPrimary
                }
                MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor }
            }
        }
    }
    Spacer {}

    // ── Touch ID ──────────────────────────────────────────────────────────
    SectionTitle { text: "Touch ID" }

    // No device
    SettingsCard {
        visible: !TouchID.hasDevice
        Item {
            width: parent.width; height: 60
            Text {
                anchors.centerIn: parent
                text: "No Touch ID device detected"
                font.pixelSize: 13; color: Theme.textSecondary
            }
        }
    }

    // Has device
    Item {
        visible: TouchID.hasDevice
        width: parent.width
        implicitHeight: descText.implicitHeight + fingerprintRow.implicitHeight + enrollCard.implicitHeight + 16

        Text {
            id: descText
            width: parent.width
            text: "Touch ID lets you use your fingerprint to unlock your computer and make purchases within apps."
            font.pixelSize: 13; color: Theme.textSecondary
            wrapMode: Text.WordWrap
            lineHeight: 1.4
        }

        // Fingerprint grid
        Flow {
            id: fingerprintRow
            anchors.top: descText.bottom; anchors.topMargin: 16
            width: parent.width
            spacing: 20

            Repeater {
                model: TouchID.fingerprints
                delegate: Item {
                    width: 72; height: 90

                    Column {
                        anchors.horizontalCenter: parent.horizontalCenter
                        spacing: 6

                        // Fingerprint icon with delete overlay
                        Item {
                            width: 56; height: 56
                            anchors.horizontalCenter: parent.horizontalCenter

                            Rectangle {
                                anchors.fill: parent; radius: 28
                                color: Theme.bgCard
                                border.color: Theme.divider; border.width: 1
                                Image {
                                    anchors.centerIn: parent
                                    source: "file:///usr/share/extras/system-settings/assets/fingerprint.svg"
                                    width: 36; height: 36
                                    opacity: 0.8
                                }
                            }

                            // Delete button
                            Rectangle {
                                id: delBtn
                                width: 18; height: 18; radius: 9
                                color: "#6B7280"
                                anchors.top: parent.top; anchors.right: parent.right
                                visible: delHov.containsMouse
                                Text { anchors.centerIn: parent; text: "×"; color: "white"; font.pixelSize: 12 }
                                MouseArea {
                                    id: delHov
                                    anchors.fill: parent
                                    hoverEnabled: true
                                    cursorShape: Qt.PointingHandCursor
                                    onClicked: TouchID.deleteFingerprint(modelData.finger)
                                }
                            }
                            MouseArea {
                                anchors.fill: parent
                                hoverEnabled: true
                                onEntered: delBtn.visible = true
                                onExited:  delBtn.visible = !delBtn.visible ? false : delHov.containsMouse
                            }
                        }

                        Text {
                            anchors.horizontalCenter: parent.horizontalCenter
                            text: modelData.label
                            font.pixelSize: 11; color: Theme.textSecondary
                            horizontalAlignment: Text.AlignHCenter
                        }
                    }
                }
            }

            // Add Fingerprint button
            Item {
                width: 72; height: 90

                Column {
                    anchors.horizontalCenter: parent.horizontalCenter
                    spacing: 6

                    Rectangle {
                        width: 56; height: 56; radius: 28
                        anchors.horizontalCenter: parent.horizontalCenter
                        color: addHov.containsMouse ? Theme.hoverBg : Theme.bgCard
                        border.color: addHov.containsMouse ? Theme.accent : Theme.border
                        border.width: 2
                        Behavior on border.color { ColorAnimation { duration: 150 } }

                        Text {
                            anchors.centerIn: parent
                            text: "+"; font.pixelSize: 22; color: Theme.textPrimary
                        }

                        MouseArea {
                            id: addHov
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor
                            onClicked: fingerPicker.shown = !fingerPicker.shown
                        }
                    }

                    Text {
                        anchors.horizontalCenter: parent.horizontalCenter
                        text: "Add Fingerprint"
                        font.pixelSize: 11; color: Theme.textSecondary
                        horizontalAlignment: Text.AlignHCenter
                    }
                }
            }
        }

        // Enroll status card
        SettingsCard {
            id: enrollCard
            visible: TouchID.enrolling || TouchID.enrollStatus.length > 0
            anchors.top: fingerprintRow.bottom; anchors.topMargin: 16

            Column {
                width: parent.width; spacing: 8; topPadding: 4; bottomPadding: 4

                Text {
                    width: parent.width
                    text: TouchID.enrollStatus
                    font.pixelSize: 13; color: Theme.textPrimary
                    wrapMode: Text.WordWrap
                }

                Rectangle {
                    visible: TouchID.enrolling
                    width: cancelLbl.implicitWidth + 20; height: 28; radius: 6
                    color: Qt.rgba(0,0,0,0.06); border.color: Theme.divider; border.width: 1
                    Text {
                        id: cancelLbl; anchors.centerIn: parent
                        text: "Cancel"; font.pixelSize: 12; color: Theme.textPrimary
                    }
                    MouseArea {
                        anchors.fill: parent; cursorShape: Qt.PointingHandCursor
                        onClicked: TouchID.cancelEnroll()
                    }
                }
            }
        }
    }

    // Note
    Text {
        width: parent.width
        visible: TouchID.hasDevice
        text: "Most fingerprint sensors accept only one fingerprint. If enrollment fails, the sensor may already be full."
        font.pixelSize: 11; color: Theme.textSecondary
        wrapMode: Text.WordWrap; lineHeight: 1.4
        topPadding: 12
    }

    // ── Finger picker (inline, shown below the + button) ─────────────────
    SettingsCard {
        id: fingerPicker
        property bool shown: false
        visible: shown && TouchID.hasDevice && !TouchID.enrolling
        height: shown ? implicitHeight : 0

        Column {
            width: parent.width; spacing: 0

            Text {
                text: "Choose a finger to enroll"
                font.pixelSize: 13; font.weight: Font.DemiBold; color: Theme.textPrimary
                topPadding: 4; bottomPadding: 8
            }

            Repeater {
                model: [
                    "right-index-finger", "right-middle-finger", "right-ring-finger",
                    "right-little-finger", "right-thumb",
                    "left-index-finger",  "left-middle-finger",  "left-ring-finger",
                    "left-little-finger", "left-thumb"
                ]
                delegate: Item {
                    width: parent.width; height: 38
                    Text {
                        anchors.left: parent.left
                        anchors.verticalCenter: parent.verticalCenter
                        text: modelData.replace(/-/g, " ")
                        font.pixelSize: 13; color: Theme.textPrimary
                    }
                    Rectangle {
                        anchors.bottom: parent.bottom; width: parent.width; height: 1
                        color: Theme.divider; visible: index < 9
                    }
                    MouseArea {
                        anchors.fill: parent; cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            fingerPicker.shown = false
                            TouchID.enrollFingerprint(modelData)
                        }
                    }
                }
            }
        }
    }

    Spacer {}
}
