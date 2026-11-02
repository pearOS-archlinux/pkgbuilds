/*
SPDX-FileCopyrightText: 2011 Martin Gräßlin <mgraesslin@kde.org>
SPDX-FileCopyrightText: 2023 Nate Graham <nate@kde.org>
SPDX-FileCopyrightText: 2024 pearOS contributors

SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.15

import org.kde.plasma.private.sessions 2.0

Item {
    id: lockScreen

    property alias capsLockOn: unlockUI.capsLockOn
    property bool locked: false

    signal unlockRequested()

    SessionManagement {
        id: sessionManagment
    }

    Greeter {
        id: unlockUI
        anchors.fill: parent
        visible: lockScreen.locked
        switchUserEnabled: sessionManagment.canSwitchUser
        onSwitchUserClicked: sessionManagment.switchUser()
    }

    function returnToLogin() {
        unlockUI.resetFocus()
    }
}
