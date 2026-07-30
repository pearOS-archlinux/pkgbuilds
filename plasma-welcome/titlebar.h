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

#ifndef TITLEBAR_H
#define TITLEBAR_H

#include <QToolBar>
#include <QString>
#include <functional>

class QWidget;

// The floating, unified titlebar/toolbar: traffic-light buttons (see
// titlebar_buttons.h) + an expanding spacer -- pulled out of MainWindow's
// old buildTitleBar() so mainwindow.h/.cpp only deal with window chrome
// plumbing (drag-to-move, hover hairline fade, blur/tint), not what's
// actually built inside the titlebar itself.
//
// Constructed as a plain floating child of MainWindow, never handed to
// QMainWindow's own addToolBar()/managed toolbar area -- so the
// sidebar/content extend to y=0 underneath it instead of losing height
// worth of their own space, see MainWindow's ctor doc comment
// (mainwindow.cpp) for why. "The toolbar is the drag handle": MainWindow
// installs itself as an event filter on this (installEventFilter(this)) to
// turn empty-area presses into startSystemMove() -- see
// MainWindow::eventFilter().
class Titlebar : public QToolBar {
public:
    // assetsBasePath/onClose/onMinimize/onMaximize are forwarded straight to
    // FilerChrome::buildTrafficLights() (titlebar_buttons.h); height is
    // MainWindow's own kToolBarHeight.
    Titlebar(const QString& assetsBasePath, int height,
             std::function<void()> onClose,
             std::function<void()> onMinimize,
             std::function<void()> onMaximize,
             QWidget* parent = nullptr);

    // The traffic-light button group -- MainWindow greyscales this via
    // QGraphicsColorizeEffect while the window is inactive (see
    // MainWindow::changeEvent()'s ActivationChange branch).
    QWidget* trafficLights() const { return trafficLights_; }

    // Inserts a widget between the traffic lights and the expanding spacer
    // (left-aligned, e.g. an app logo/title) -- QToolBar::addWidget() alone
    // would append after the spacer instead (far right), since the spacer
    // already claims all remaining space.
    void insertBeforeSpacer(QWidget* widget) { insertWidget(spacerAction_, widget); }

private:
    QWidget* trafficLights_ = nullptr;
    QAction* spacerAction_ = nullptr;
};

#endif // TITLEBAR_H
