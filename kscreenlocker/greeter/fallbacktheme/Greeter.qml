/*
SPDX-FileCopyrightText: 2011 Martin Gräßlin <mgraesslin@kde.org>
SPDX-FileCopyrightText: 2023 Nate Graham <nate@kde.org>
SPDX-FileCopyrightText: 2024 pearOS contributors

SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15
import Qt5Compat.GraphicalEffects

import org.kde.kscreenlocker 1.0 as ScreenLocker

Item {
    id: root

    signal switchUserClicked()

    property bool switchUserEnabled: false
    property bool capsLockOn: false
    property bool authSucceeded: false
    property alias notification: messageLabel.text

    function resetFocus() {
        passwordField.forceActiveFocus()
    }

    // ── dark overlay on top of the Plasma wallpaper ──────────────────────────
    Rectangle {
        anchors.fill: parent
        color: "#000000"
        opacity: 0.30
    }

    // ── wallpaper capture + blur behind password pill ─────────────────────────
    ShaderEffectSource {
        id: wallpaperCapture
        sourceItem: (typeof wallpaper !== "undefined" && wallpaper !== null)
                    ? wallpaper : null
        hideSource: false
        visible: false
        anchors.fill: parent
    }

    // pill-shaped blur mask positioned at the password field location
    Item {
        id: blurMaskItem
        anchors.fill: parent
        visible: false

        Rectangle {
            width: 170
            height: 40
            radius: 20
            color: "#000"
            anchors.horizontalCenter: parent.horizontalCenter
            // matches: bottomMargin(56) + messageLabel row(~28)
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 80
        }
    }

    FastBlur {
        anchors.fill: parent
        source: wallpaperCapture.sourceItem !== null ? wallpaperCapture : null
        radius: 32
        visible: wallpaperCapture.sourceItem !== null
        layer.enabled: true
        layer.effect: OpacityMask {
            maskSource: blurMaskItem
        }
    }

    // ── clock ─────────────────────────────────────────────────────────────────
    Item {
        id: clockArea
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: parent.top
        anchors.topMargin: Math.round(parent.height * 0.18)
        width: timeLabel.implicitWidth + 20
        height: dateLabel.implicitHeight + timeLabel.implicitHeight + 6

        ColumnLayout {
            anchors.horizontalCenter: parent.horizontalCenter
            spacing: 2

            Text {
                id: dateLabel
                Layout.alignment: Qt.AlignHCenter
                text: Qt.formatDateTime(new Date(), "ddd MMM d")
                color: "white"
                opacity: 0.75
                font.pointSize: 22
                font.weight: Font.DemiBold
                font.capitalization: Font.Capitalize
            }

            Text {
                id: timeLabel
                Layout.alignment: Qt.AlignHCenter
                text: Qt.formatDateTime(new Date(), "hh:mm")
                color: "white"
                opacity: 0.75
                font.pointSize: 90
                font.bold: true
            }
        }

        Timer {
            interval: 1000
            repeat: true
            running: true
            onTriggered: {
                timeLabel.text = Qt.formatDateTime(new Date(), "hh:mm")
                dateLabel.text = Qt.formatDateTime(new Date(), "ddd MMM d")
            }
        }
    }

    // ── login area ────────────────────────────────────────────────────────────
    Item {
        id: loginArea

        readonly property int avatarSize: 72

        width: Math.max(160, usernameLabel.implicitWidth + 40)
        height: avatarSize + 10
                + usernameLabel.implicitHeight + 15
                + 32 + 8
                + messageLabel.implicitHeight + 4

        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 56

        // ── circular avatar mask ──────────────────────────────────────────────
        Rectangle {
            id: avatarMask
            width: loginArea.avatarSize
            height: loginArea.avatarSize
            radius: loginArea.avatarSize / 2
            color: "#000"
            visible: false
            anchors.top: parent.top
            anchors.horizontalCenter: parent.horizontalCenter
        }

        // user face icon
        Image {
            id: userAvatar
            source: (typeof kscreenlocker_userImage !== "undefined")
                    ? kscreenlocker_userImage : ""
            width: loginArea.avatarSize
            height: loginArea.avatarSize
            fillMode: Image.PreserveAspectCrop
            anchors.top: parent.top
            anchors.horizontalCenter: parent.horizontalCenter
            layer.enabled: true
            layer.effect: OpacityMask {
                maskSource: avatarMask
            }
        }

        // initials circle shown when no face image is available
        Rectangle {
            width: loginArea.avatarSize
            height: loginArea.avatarSize
            radius: loginArea.avatarSize / 2
            color: "#555555"
            visible: userAvatar.status !== Image.Ready
            anchors.top: parent.top
            anchors.horizontalCenter: parent.horizontalCenter

            Text {
                anchors.centerIn: parent
                text: {
                    var n = (typeof kscreenlocker_userName !== "undefined")
                            ? kscreenlocker_userName : ""
                    return n.length > 0 ? n.charAt(0).toUpperCase() : "?"
                }
                color: "white"
                font.pointSize: 26
                font.bold: true
            }
        }

        // ── username ──────────────────────────────────────────────────────────
        Text {
            id: usernameLabel
            text: (typeof kscreenlocker_userName !== "undefined")
                  ? kscreenlocker_userName : ""
            anchors.top: userAvatar.bottom
            anchors.topMargin: 10
            anchors.horizontalCenter: parent.horizontalCenter
            color: "white"
            font.pixelSize: 14
            font.bold: true
        }

        // ── password field ────────────────────────────────────────────────────
        TextField {
            id: passwordField

            anchors.top: usernameLabel.bottom
            anchors.topMargin: 15
            anchors.horizontalCenter: parent.horizontalCenter
            width: 150
            height: 32

            color: "#ffffff"
            placeholderTextColor: "#99ffffff"
            echoMode: TextInput.Password
            focus: true
            font.weight: Font.DemiBold
            placeholderText: i18nd("kscreenlocker_greet", "Password")

            enabled: !authenticator.busy
            text: PasswordSync.password

            Keys.onReturnPressed: authenticator.startAuthenticating()
            Keys.onEnterPressed:  authenticator.startAuthenticating()
            Keys.onEscapePressed: {
                text = ""
                text = Qt.binding(() => PasswordSync.password)
            }

            background: Rectangle {
                implicitWidth: parent.width
                implicitHeight: parent.height
                color: "#ffffff"
                opacity: 0.18
                radius: 15
            }

            // caps-lock indicator glyph
            Text {
                visible: root.capsLockOn
                text: "⇪"
                color: "white"
                font.pixelSize: 15
                opacity: 0.85
                anchors.right: parent.right
                anchors.rightMargin: 10
                anchors.verticalCenter: parent.verticalCenter
            }
        }

        Binding {
            target: PasswordSync
            property: "password"
            value: passwordField.text
        }

        // ── message / error label ─────────────────────────────────────────────
        Text {
            id: messageLabel
            text: ""
            color: "white"
            font.pointSize: 8
            anchors.top: passwordField.bottom
            anchors.topMargin: 8
            anchors.horizontalCenter: parent.horizontalCenter
            visible: text.length > 0
        }
    }

    // ── fingerprint hint ──────────────────────────────────────────────────────
    Text {
        visible: authenticator.authenticatorTypes & ScreenLocker.Authenticator.Fingerprint
        text: i18nd("kscreenlocker_greet", "(or place your fingerprint on the reader)")
        color: "white"
        opacity: 0.60
        font.pointSize: 8
        anchors.top: loginArea.bottom
        anchors.topMargin: 6
        anchors.horizontalCenter: parent.horizontalCenter
    }

    // ── switch-user button ────────────────────────────────────────────────────
    Rectangle {
        visible: root.switchUserEnabled
        width: switchUserLabel.implicitWidth + 24
        height: 28
        radius: 14
        color: "#ffffff"
        opacity: 0.15
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 18
        anchors.right: parent.right
        anchors.rightMargin: 24

        Text {
            id: switchUserLabel
            anchors.centerIn: parent
            text: i18nd("kscreenlocker_greet", "Switch Users")
            color: "white"
            font.pixelSize: 12
        }

        MouseArea {
            anchors.fill: parent
            onClicked: root.switchUserClicked()
        }
    }

    // ── authenticator connections ─────────────────────────────────────────────
    Connections {
        target: authenticator

        function onFailed() {
            root.notification = i18nd("kscreenlocker_greet", "Unlocking failed")
            passwordField.selectAll()
            root.resetFocus()
        }
        function onBusyChanged() {
            if (!authenticator.busy && !root.authSucceeded) {
                root.notification = ""
                passwordField.selectAll()
                root.resetFocus()
            }
        }
        function onInfoMessageChanged() {
            root.notification = Qt.binding(() => authenticator.infoMessage)
        }
        function onErrorMessageChanged() {
            root.notification = Qt.binding(() => authenticator.errorMessage)
        }
        function onPromptForSecretChanged() {
            authenticator.respond(passwordField.text)
        }
        function onSucceeded() {
            root.authSucceeded = true
            Qt.quit()
        }
    }

    Component.onCompleted: {
        root.resetFocus()
    }
}
