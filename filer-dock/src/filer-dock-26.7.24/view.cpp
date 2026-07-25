/*
 *  SPDX-FileCopyrightText: 2013 Giorgos Tsiapaliokas <terietor@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "view.h"

#include <QGuiApplication>
#include <QScreen>

View::View(QWindow *parent)
    : QQuickView(parent)
{
}

View::~View() = default;

void View::resizeEvent(QResizeEvent *event)
{
    syncQtWindowPosition();
    QQuickView::resizeEvent(event);
}

void View::setBottomMargin(int margin)
{
    m_bottomMargin = margin;
    syncQtWindowPosition();
}

// Wayland clients can't query their own on-screen position, so Qt's idea of
// this window's position is whatever was last set client-side -- (0,0)
// unless we correct it. Everything that positions a popup relative to this
// window (PlasmaCore.Dialog's visualParent math for task tooltips and
// PearFolderArc, Qt's xdg-popup positioners for context menus) starts from
// that Qt-side position, which is why those popups used to land at the top
// of the screen: "above a dock at y=0" is exactly there. The layer-shell
// surface is anchored to the bottom edge with a fixed margin (see
// main.cpp), so the real global position is fully known; mirror it into
// QWindow so relative math comes out right. Requires
// QWaylandWindow::fixedToplevelPositions == false (main.cpp), otherwise
// QtWayland rewrites any client-set position back to the screen origin.
void View::syncQtWindowPosition()
{
    QScreen *s = screen() ? screen() : QGuiApplication::primaryScreen();
    if (!s || height() <= 0) {
        return;
    }
    const QRect geo = s->geometry();
    const QPoint pos(geo.x(), geo.y() + geo.height() - height() - m_bottomMargin);
    if (position() != pos) {
        setPosition(pos);
    }
}
