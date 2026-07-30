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

#ifndef TITLEBAR_BUTTONS_H
#define TITLEBAR_BUTTONS_H

// Pulled out of filer_titlebar.h/.cpp (which used to hold this alongside
// paintRoundedWindow) so the close/minimize/maximize traffic-light buttons
// live in their own file, separate from titlebar.h/.cpp's Titlebar toolbar
// class and mainwindow.h/.cpp's window chrome.
#include <QString>
#include <functional>

class QWidget;

namespace FilerChrome {

// macOS traffic lights (close/minimize/maximize), built from the pearOS-dark
// GTK theme's own titlebutton assets -- ported from the
// sidebarSpacer_/makeTrafficDot block in Filer's real
// MainWindow::MainWindow() (src/mainwindow.cpp, originally ~line 701-736).
// assetsBasePath is the directory containing "titlebuttons/titlebutton-*.png"
// (this project reuses the top-level repo's assets/titlebuttons/ directory
// rather than duplicating those PNGs -- see qt5/CMakeLists.txt).
QWidget* buildTrafficLights(QWidget* parent, const QString& assetsBasePath,
                             std::function<void()> onClose,
                             std::function<void()> onMinimize,
                             std::function<void()> onMaximize);

}

#endif // TITLEBAR_BUTTONS_H
