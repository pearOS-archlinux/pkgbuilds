/*

    Copyright (C) 2013  Hong Jen Yee (PCMan) <pcman.tw@gmail.com>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

// The traffic-lights ctor block that used to live here has moved to
// titlebar_buttons.cpp -- see filer_titlebar.h's doc comment. What's left
// is just paintRoundedWindow(), cut down from Filer's real
// MainWindow::paintEvent() (originally ~line 3564-3618).

#include "filer_titlebar.h"

#include <QWidget>
#include <QPainter>
#include <QPainterPath>

namespace FilerChrome {

// Ported from MainWindow::paintEvent() (src/mainwindow.cpp, originally
// ~line 3564-3618):
//
//   const bool frameless = windowFlags() & Qt::FramelessWindowHint;
//   const qreal radius = (isMaximized() || !frameless) ? 0 : kWindowCornerRadius;
//   QPainterPath winPath;
//   winPath.addRoundedRect(rect(), radius, radius);
//   ... fillPath(winPath, ...) ...
//
// Filer's real version fills two different regions here (a frosted sidebar
// column vs. the rest); this project's MainWindow::paintEvent()
// (qt5/mainwindow.cpp) does that split itself using the real tint formula,
// then calls this helper twice (once per region) for the shared
// corner-clipping mechanic.
void paintRoundedWindow(QWidget* window, QPainter& painter, const QColor& fill, bool frameless) {
    painter.setRenderHint(QPainter::Antialiasing);
    const qreal radius = frameless ? kWindowCornerRadius : 0;
    QPainterPath winPath;
    winPath.addRoundedRect(window->rect(), radius, radius);
    painter.fillPath(winPath, fill);
}

}
