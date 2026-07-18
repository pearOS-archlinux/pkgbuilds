/*
    SPDX-FileCopyrightText: 2014 Aleix Pol Gonzalez <aleixpol@blue-systems.com>
    SPDX-FileCopyrightText: 2024 pearOS contributors

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQml
import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import QtCore
import Qt5Compat.GraphicalEffects
import Qt.labs.platform as QtPlatform

import org.kde.plasma.components as PlasmaComponents3
import org.kde.plasma.workspace.components as PW
import org.kde.plasma.private.keyboardindicator as KeyboardIndicator
import org.kde.kirigami as Kirigami

import org.kde.plasma.private.sessions
import org.kde.breeze.components

Item {
    id: lockScreenUi

    readonly property bool softwareRendering: GraphicsInfo.api === GraphicsInfo.Software

    readonly property string homeDir: QtPlatform.StandardPaths.standardLocations(
                                          QtPlatform.StandardPaths.HomeLocation)[0]



    // Font loaders — bypass fontconfig (fonts with " - " in name not matched by fc)
    FontLoader { id: flSoft;    source: Qt.resolvedUrl("fonts/SFPro-Soft.otf") }
    FontLoader { id: flRails;   source: Qt.resolvedUrl("fonts/SFPro-Rails.otf") }
    FontLoader { id: flStencil; source: Qt.resolvedUrl("fonts/SFPro-Stencil.otf") }
    FontLoader { id: flDisplay; source: Qt.resolvedUrl("fonts/SFPro-Display.otf") }
    FontLoader { id: flNY;      source: Qt.resolvedUrl("fonts/NewYork-Semibold.otf") }
    FontLoader { id: flNYHeavy; source: Qt.resolvedUrl("fonts/NewYork-Heavy.otf") }
    FontLoader { id: flUIText;  source: Qt.resolvedUrl("fonts/SFUIText-Semibold.otf") }

    // currentFontKey is set from style.json; clockFontFamily is a JS binding
    // that references FontLoader.name directly — QML tracks name as a dependency
    // so the binding re-evaluates automatically when the loader finishes.
    property string currentFontKey: "soft"
    property int    clockFontWeight: Font.DemiBold
    property string desktopWallpaperPath: ""
    property string clockFontFamily: {
        var k = lockScreenUi.currentFontKey
        if (k === "soft")        return flSoft.name    || "SF Pro Display"
        if (k === "rails")       return flRails.name   || "SF Pro Display"
        if (k === "stencil")     return flStencil.name || "SF Pro Display"
        if (k === "new-york")    return flNY.name      || "New York Extra Large"
        if (k === "new-york-heavy") return flNYHeavy.name || "New York Extra Large"
        return flDisplay.name || "SF Pro Display"
    }

    // ── desktop wallpaper loader ──────────────────────────────────────────────
    // The wallpaper is handled natively by `wallpaperItem` (via kscreenlocker plugin).
    // The fallback path is set to a static default to prevent QML errors.
    function loadDesktopWallpaper() {
        lockScreenUi.desktopWallpaperPath = "file:///usr/share/extras/wallpapers/Default/dark-mode.jpg"
    }

    // ── style.json loader ─────────────────────────────────────────────────────
    // Maps fontFamily + fontStyle → embedded fc family name of the installed font.
    function loadStyleJson() {
        var xhr = new XMLHttpRequest()
        xhr.open("GET", "file://" + lockScreenUi.homeDir + "/.config/extras/lockscreen/style.json", true)
        xhr.onreadystatechange = function() {
            if (xhr.readyState !== XMLHttpRequest.DONE) return
            try {
                var cfg = JSON.parse(xhr.responseText).lockscreen
                if (!cfg) return

                var family = (cfg.fontFamily || "SF Pro").toLowerCase()
                var weight = (cfg.fontWeight || "Semibold").toLowerCase()
                var style  = (cfg.fontStyle  || "Soft").toLowerCase()

                // Map style.json → currentFontKey used by clockFontFamily binding
                if (family.indexOf("new york") !== -1) {
                    lockScreenUi.currentFontKey = (weight === "heavy") ? "new-york-heavy" : "new-york"
                } else {
                    if      (style === "soft")    lockScreenUi.currentFontKey = "soft"
                    else if (style === "rails")   lockScreenUi.currentFontKey = "rails"
                    else if (style === "stencil") lockScreenUi.currentFontKey = "stencil"
                    else                          lockScreenUi.currentFontKey = "display"
                }

                // Font weight — numeric (100-900) takes priority over named
                var numericWeight = cfg["font-weight"]
                if (typeof numericWeight === "number" && numericWeight >= 100 && numericWeight <= 900) {
                    lockScreenUi.clockFontWeight = numericWeight
                } else if (weight === "bold" || weight === "heavy") {
                    lockScreenUi.clockFontWeight = Font.Bold
                } else if (weight === "medium") {
                    lockScreenUi.clockFontWeight = Font.Medium
                } else if (weight === "light") {
                    lockScreenUi.clockFontWeight = Font.Light
                } else {
                    lockScreenUi.clockFontWeight = Font.DemiBold
                }
            } catch(e) {}
        }
        xhr.send()
    }

    // ── state ─────────────────────────────────────────────────────────────────
    KeyboardIndicator.KeyState {
        id: capsLockState
        key: Qt.Key_CapsLock
    }

    SessionManagement { id: sessionManagement }
    Connections {
        target: sessionManagement
        function onAboutToSuspend() { root.clearPassword() }
    }

    // ── authentication ────────────────────────────────────────────────────────
    function handleMessage(msg) {
        if (!root.notification) {
            root.notification = msg
        } else if (root.notification.includes(msg)) {
            root.notificationRepeated()
        } else {
            root.notification += "\n" + msg
        }
    }

    Connections {
        target: authenticator
        function onFailed(kind) {
            if (kind !== 0) return
            lockScreenUi.handleMessage(
                i18ndc("plasma_shell_org.kde.plasma.desktop", "@info:status", "Unlocking failed"))
            notificationRemoveTimer.restart()
            rejectAnim.start()
        }
        function onSucceeded() { Qt.quit() }
        function onInfoMessageChanged()  { lockScreenUi.handleMessage(authenticator.infoMessage) }
        function onErrorMessageChanged() { lockScreenUi.handleMessage(authenticator.errorMessage) }
        function onPromptForSecretChanged() { passwordField.forceActiveFocus() }
    }

    // ── solid black fallback — ALWAYS the first rendered item ───────────────
    // Must be the first child (no z property) so it renders behind wallpaper
    // but still fills the window → panel CANNOT bleed through X11 compositing.
    Rectangle {
        id: solidBackground
        anchors.fill: parent
        color: "#000000"
    }

    // ── wallpaper item — renders the wallpaper plugin set via System Settings ─
    // kscreenlocker injects `wallpaper` (a QQuickItem from the configured plugin).
    // We reparent it here and size via Qt.binding (anchors can't be set imperatively).
    // → Changing wallpaper in System Settings → Screen Locking works automatically.
    Item {
        id: wallpaperItem
        anchors.fill: parent
        Component.onCompleted: {
            if (typeof wallpaper !== "undefined" && wallpaper) {
                wallpaper.parent = wallpaperItem
                wallpaper.x = 0
                wallpaper.y = 0
                wallpaper.width  = Qt.binding(function() { return wallpaperItem.width })
                wallpaper.height = Qt.binding(function() { return wallpaperItem.height })
            }
        }
    }

    // ── wallpaper background image (fallback: used when desktopWallpaperPath is set) ──
    Image {
        id: wallpaperBg
        anchors.fill: parent
        source: lockScreenUi.desktopWallpaperPath
        fillMode: Image.PreserveAspectCrop
        visible: lockScreenUi.desktopWallpaperPath !== ""
    }

    // ── wallpaper source for graphical effects ─────────────────────────────
    // Used by blur and contrast effects. Prefers wallpaper plugin, falls back to Image.
    ShaderEffectSource {
        id: wallpaperSource
        sourceItem: {
            if (typeof wallpaper !== "undefined" && wallpaper) return wallpaper
            if (lockScreenUi.desktopWallpaperPath !== "") return wallpaperBg
            return null
        }
        hideSource: false; visible: false
        anchors.fill: parent
    }

    // ── BrightnessContrast masked to clock text ────────────────────────────────
    BrightnessContrast {
        anchors.fill: parent
        source: wallpaperSource.sourceItem ? wallpaperSource : null
        brightness: 0; contrast: 0.3
        visible: !softwareRendering && wallpaperSource.sourceItem !== null
        layer.enabled: true
        layer.effect: OpacityMask { maskSource: identclock }
    }

    // ── FastBlur pill behind password area ────────────────────────────────────
    // pill at bottomMargin=40: aligns with base of loginArea (pearOS-dark exact)
    Rectangle {
        id: maskOfBlur
        anchors.fill: parent; color: "transparent"; visible: false
        Rectangle {
            width: 250; height: 32; color: "#000"; radius: 15
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 40
        }
    }

    FastBlur {
        anchors.fill: parent
        source: wallpaperSource.sourceItem ? wallpaperSource : null
        radius: 32
        visible: !softwareRendering && wallpaperSource.sourceItem !== null
        layer.enabled: true
        layer.effect: OpacityMask { maskSource: maskOfBlur }
    }

    // ── clock — pearOS-dark exact layout ─────────────────────────────────────
    Item {
        id: identclock
        width: parent.width; height: parent.height
        opacity: 0.8

        Rectangle {
            height: 300; width: 400
            anchors.horizontalCenter: parent.horizontalCenter
            color: "transparent"

            ColumnLayout {
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.bottom: parent.bottom
                spacing: 2

                Text {
                    id: dateLabel
                    Layout.alignment: Qt.AlignHCenter
                    text: Qt.formatDateTime(new Date(), "ddd MMM d")
                    color: "white"; opacity: 0.5
                    font.pointSize: 20
                    font.weight: lockScreenUi.clockFontWeight
                    font.family: lockScreenUi.clockFontFamily
                    font.capitalization: Font.Capitalize
                    style: softwareRendering ? Text.Outline : Text.Normal
                }

                Text {
                    id: timeLabel
                    Layout.alignment: Qt.AlignHCenter
                    text: Qt.formatDateTime(new Date(), "hh:mm")
                    color: "white"; opacity: 0.5
                    font.pointSize: 100
                    font.weight: lockScreenUi.clockFontWeight
                    font.family: lockScreenUi.clockFontFamily
                    style: softwareRendering ? Text.Outline : Text.Normal
                }
            }

            Timer {
                interval: 1000; repeat: true; running: true
                onTriggered: {
                    timeLabel.text = Qt.formatDateTime(new Date(), "hh:mm")
                    dateLabel.text = Qt.formatDateTime(new Date(), "ddd MMM d")
                }
            }
        }
    }

    // ── top-left: battery ─────────────────────────────────────────────────────
    Battery {
        anchors.top: parent.top; anchors.topMargin: 12
        anchors.left: parent.left; anchors.leftMargin: 40
        z: 5
    }

    // ── top-right: shutdown ───────────────────────────────────────────────────
    Image {
        id: shutdownBtn
        anchors.top: parent.top; anchors.topMargin: 15
        anchors.right: parent.right; anchors.rightMargin: 40
        width: 22; height: 22
        z: 5
        source: Qt.resolvedUrl("images/system-shutdown.svg")
        fillMode: Image.PreserveAspectFit
        visible: sessionManagement.canShutdown
        MouseArea {
            anchors.fill: parent; hoverEnabled: true
            onEntered: shutdownBtn.source = Qt.resolvedUrl("images/system-shutdown-hover.svg")
            onExited:  shutdownBtn.source = Qt.resolvedUrl("images/system-shutdown.svg")
            onClicked: { shutdownBtn.source = Qt.resolvedUrl("images/system-shutdown-pressed.svg"); sessionManagement.requestShutdown() }
        }
    }

    // ── top-right: reboot ────────────────────────────────────────────────────
    Image {
        id: rebootBtn
        anchors.top: parent.top; anchors.topMargin: 15
        anchors.right: parent.right; anchors.rightMargin: 70
        width: 22; height: 22
        z: 5
        source: Qt.resolvedUrl("images/system-reboot.svg")
        fillMode: Image.PreserveAspectFit
        visible: sessionManagement.canReboot
        MouseArea {
            anchors.fill: parent; hoverEnabled: true
            onEntered: rebootBtn.source = Qt.resolvedUrl("images/system-reboot-hover.svg")
            onExited:  rebootBtn.source = Qt.resolvedUrl("images/system-reboot.svg")
            onClicked: { rebootBtn.source = Qt.resolvedUrl("images/system-reboot-pressed.svg"); sessionManagement.requestReboot() }
        }
    }

    // ── input root ────────────────────────────────────────────────────────────
    MouseArea {
        id: lockScreenRoot
        anchors.fill: parent
        hoverEnabled: true; cursorShape: Qt.ArrowCursor

        onPressed: { Window.window.requestActivate(); passwordField.forceActiveFocus() }
        Keys.onPressed: event => { event.accepted = false }

        // loginArea: direct child — NO StackView (removed to prevent height override)
        Item {
            id: loginArea

            readonly property int sizeAvatar_scaled: 72
            // pearOS-dark: sizeAvatar*.9 + 15 + 32 + 15 + hint
            height: sizeAvatar_scaled + 15 + 32 + 15 + greetingLabel.implicitHeight
            width: 150

            anchors.horizontalCenter: parent.horizontalCenter
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 40

            // ── circular avatar ───────────────────────────────────────────
            Rectangle {
                id: avatarMask
                width: loginArea.sizeAvatar_scaled; height: width
                radius: width / 2; color: "#000"; visible: false
                anchors.top: parent.top
                anchors.horizontalCenter: parent.horizontalCenter
            }

            Image {
                id: userAvatar
                // chain: ~/.face.icon → kscreenlocker_userImage → /usr/share/extras/.face.icon
                source: "file://" + lockScreenUi.homeDir + "/.face.icon"
                width: loginArea.sizeAvatar_scaled; height: width
                fillMode: Image.PreserveAspectCrop
                anchors.top: parent.top
                anchors.horizontalCenter: parent.horizontalCenter
                layer.enabled: true
                layer.effect: OpacityMask { maskSource: avatarMask }

                property int fallbackStep: 0
                onStatusChanged: {
                    if (status !== Image.Error) return
                    if (fallbackStep === 0) {
                        fallbackStep = 1
                        var ki = (typeof kscreenlocker_userImage !== "undefined" && kscreenlocker_userImage !== "")
                                 ? "file://" + kscreenlocker_userImage : ""
                        source = ki || "file:///usr/share/extras/.face.icon"
                    } else if (fallbackStep === 1) {
                        fallbackStep = 2
                        source = "file:///usr/share/extras/.face.icon"
                    }
                }
            }

            // initials circle (no image available)
            Rectangle {
                width: loginArea.sizeAvatar_scaled; height: width
                radius: width / 2; color: "#555555"
                visible: userAvatar.status !== Image.Ready
                anchors.top: parent.top
                anchors.horizontalCenter: parent.horizontalCenter
                Text {
                    anchors.centerIn: parent
                    text: {
                        var n = (typeof kscreenlocker_userName !== "undefined") ? kscreenlocker_userName : ""
                        return n.length > 0 ? n[0].toUpperCase() : "?"
                    }
                    color: "white"; font.pointSize: 26; font.bold: true
                }
            }

            // username: hidden (pearOS-dark: visible: false)

            // ── password field — FocusScope+TextInput (no Breeze styling) ──
            FocusScope {
                id: passwordField
                anchors.top: parent.top
                anchors.topMargin: loginArea.sizeAvatar_scaled + 15
                anchors.horizontalCenter: parent.horizontalCenter
                width: 150; height: 32

                enabled: !authenticator.graceLocked

                // pill background
                Rectangle {
                    anchors.fill: parent
                    color: "#ffffff"; opacity: 0.2; radius: 15
                }

                // placeholder text
                Text {
                    anchors { fill: parent; leftMargin: 10; rightMargin: 10 }
                    verticalAlignment: Text.AlignVCenter
                    text: i18ndc("plasma_shell_org.kde.plasma.desktop",
                                 "@info:placeholder in text field", "Password")
                    color: "#66FFFFFF"
                    font.weight: Font.DemiBold
                    font.family: flUIText.name || "SF UI  Text"
                    visible: passwordInput.text.length === 0
                }

                TextInput {
                    id: passwordInput
                    anchors {
                        fill: parent
                        leftMargin: 10
                        rightMargin: capsImg.opacity > 0 ? capsImg.width + 14 : 10
                        topMargin: 0; bottomMargin: 0
                    }
                    verticalAlignment: TextInput.AlignVCenter
                    echoMode: TextInput.Password
                    color: "#ffffff"
                    selectionColor: Qt.rgba(1, 1, 1, 0.4)
                    selectedTextColor: "#ffffff"
                    font.weight: Font.DemiBold
                    font.family: flUIText.name || "SF UI  Text"
                    focus: true
                    text: PasswordSync.password

                    Keys.onReturnPressed: authenticator.respond(text)
                    Keys.onEnterPressed:  authenticator.respond(text)
                    Keys.onEscapePressed: {
                        clear()
                        text = Qt.binding(() => PasswordSync.password)
                        root.clearPassword()
                    }
                }

                // caps-lock icon
                Image {
                    id: capsImg
                    width: 24; height: 24
                    sourceSize.width: 24; sourceSize.height: 24
                    source: Qt.resolvedUrl("images/capslock.svg")
                    fillMode: Image.PreserveAspectFit
                    anchors.right: parent.right; anchors.rightMargin: 10
                    anchors.verticalCenter: parent.verticalCenter
                    opacity: capsLockState.locked ? 1 : 0
                    Behavior on opacity { NumberAnimation { duration: 200 } }
                }

                // shake on wrong password
                SequentialAnimation {
                    id: rejectAnim
                    PropertyAnimation { target: passwordField; property: "x"; to:  10; duration: 50 }
                    PropertyAnimation { target: passwordField; property: "x"; to: -10; duration: 50 }
                    PropertyAnimation { target: passwordField; property: "x"; to:   6; duration: 50 }
                    PropertyAnimation { target: passwordField; property: "x"; to:  -6; duration: 50 }
                    PropertyAnimation { target: passwordField; property: "x"; to:   0; duration: 50 }
                    onFinished: {
                        passwordInput.clear()
                        passwordInput.text = Qt.binding(() => PasswordSync.password)
                    }
                }
            }

            Binding { target: PasswordSync; property: "password"; value: passwordInput.text }

            Connections {
                target: root
                function onClearPassword() {
                    passwordField.forceActiveFocus()
                    passwordInput.clear()
                    passwordInput.text = Qt.binding(() => PasswordSync.password)
                }
                function onNotificationRepeated() { rejectAnim.start() }
            }

            // ── greeting hint ─────────────────────────────────────────────
            Text {
                id: greetingLabel
                text: i18ndc("plasma_shell_org.kde.plasma.desktop", "@info", "Touch ID or Password")
                color: "#fff"; font.pointSize: 8
                anchors.top: parent.top
                anchors.topMargin: loginArea.sizeAvatar_scaled + 15 + 32 + 15
                anchors.horizontalCenter: parent.horizontalCenter
            }

            // notification / error
            Text {
                id: notifLabel
                text: root.notification || ""
                color: "white"; font.pointSize: 8
                anchors.top: greetingLabel.bottom; anchors.topMargin: 4
                anchors.horizontalCenter: parent.horizontalCenter
                visible: text.length > 0
            }
        }

        // ── timers ─────────────────────────────────────────────────────────
        Timer {
            id: notificationRemoveTimer; interval: 3000
            onTriggered: root.notification = ""
        }

        // ── fade-in ────────────────────────────────────────────────────────
        PropertyAnimation {
            id: launchAnimation; target: lockScreenRoot
            property: "opacity"; from: 0; to: 1
            duration: Kirigami.Units.veryLongDuration * 2
        }

        Component.onCompleted: {
            launchAnimation.start()
            lockScreenUi.loadDesktopWallpaper()
            lockScreenUi.loadStyleJson()
        }

        // defer activation until window is ready
        Timer {
            id: startupTimer; interval: 0; running: true; repeat: false
            onTriggered: {
                if (lockScreenUi.Window.window) lockScreenUi.Window.window.requestActivate()
                authenticator.startAuthenticating()
                passwordField.forceActiveFocus()
            }
        }

        // ── OSD ────────────────────────────────────────────────────────────
        Loader {
            z: 2; active: root.viewVisible; source: "LockOsd.qml"
            anchors { horizontalCenter: parent.horizontalCenter
                      bottom: parent.bottom; bottomMargin: Kirigami.Units.gridUnit }
        }

        // ── footer: keyboard layout only ──────────────────────────────────
        RowLayout {
            id: footer
            anchors { bottom: parent.bottom; left: parent.left; right: parent.right
                      margins: Kirigami.Units.smallSpacing }
            spacing: Kirigami.Units.smallSpacing

            PlasmaComponents3.ToolButton {
                id: keyboardButton; focusPolicy: Qt.TabFocus
                Accessible.description: i18ndc("plasma_shell_org.kde.plasma.desktop",
                                               "Button to change keyboard layout", "Switch layout")
                icon.name: "input-keyboard"
                PW.KeyboardLayoutSwitcher {
                    id: keyboardLayoutSwitcher; anchors.fill: parent
                    acceptedButtons: Qt.NoButton
                }
                text: keyboardLayoutSwitcher.layoutNames.longName
                onClicked: keyboardLayoutSwitcher.keyboardLayout.switchToNextLayout()
                visible: keyboardLayoutSwitcher.hasMultipleKeyboardLayouts
                Layout.fillHeight: true
                containmentMask: Item {
                    parent: keyboardButton; anchors.fill: parent
                    anchors.leftMargin: -footer.anchors.margins
                    anchors.bottomMargin: -footer.anchors.margins
                }
            }

            Item { Layout.fillWidth: true }
        }
    }
}