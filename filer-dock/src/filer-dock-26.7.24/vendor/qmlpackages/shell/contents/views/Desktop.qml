/*
 *  Adapted from plasma-sdk's plasmoidviewer shell Desktop.qml
 *  (SPDX-FileCopyrightText: 2012 Marco Martin <mart@kde.org>,
 *   SPDX-FileCopyrightText: 2023 ivan tkachenko <me@ratijas.tk>)
 *  for filer-dock: transparent background (no black canvas), no SDK
 *  dev toolbar, no stayBehind/fillScreen (this is a small panel-sized
 *  window, not a full desktop background).
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick

Item {
    id: root

    property Item containment

    onContainmentChanged: {
        containment.visible = true
        containment.anchors.fill = root
    }
}
