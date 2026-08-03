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

// Originally the same file as ../filer_titlebar.h (the top-level pearOS
// Welcome project's Qt6 extraction of Filer's window chrome); the
// close/minimize/maximize traffic-light buttons have since been split out
// into their own titlebar_buttons.h/.cpp (and the toolbar that hosts them
// into titlebar.h/.cpp's Titlebar class), so this file no longer matches
// ../filer_titlebar.h -- what's left here is just the shared corner-radius
// constant and the unused paintRoundedWindow() helper (see its own doc
// comment).

#ifndef FILER_TITLEBAR_H
#define FILER_TITLEBAR_H

#include <QColor>

class QWidget;
class QPainter;

namespace FilerChrome {

// Matches Filer's own kWindowCornerRadius (mainwindow.cpp:95) exactly --
// "matches the Tahoe reference's corner rounding".
inline constexpr qreal kWindowCornerRadius = 16;

// Rounded-corner window fill -- simplified from MainWindow::paintEvent()
// (which additionally splits the fill into a frosted sidebar column vs.
// main-area tint; this project's mainwindow.cpp reimplements that split
// itself using the real Filer tint formula -- see MainWindow::paintEvent()
// there -- so only the corner-clipping mechanic is reused here, same as the
// top-level extraction).
void paintRoundedWindow(QWidget* window, QPainter& painter, const QColor& fill, bool frameless);

}

#endif // FILER_TITLEBAR_H
