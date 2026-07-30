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

#include "titlebar.h"
#include "titlebar_buttons.h"

#include <QWidget>
#include <QSizePolicy>

Titlebar::Titlebar(const QString& assetsBasePath, int height,
                   std::function<void()> onClose,
                   std::function<void()> onMinimize,
                   std::function<void()> onMaximize,
                   QWidget* parent)
    : QToolBar(QStringLiteral("titlebar"), parent) {
    setObjectName(QStringLiteral("topbar"));
    setMovable(false);
    setFloatable(false);
    setFixedHeight(height);

    trafficLights_ = FilerChrome::buildTrafficLights(
        this, assetsBasePath, std::move(onClose), std::move(onMinimize), std::move(onMaximize));
    addWidget(trafficLights_);

    QWidget* spacer = new QWidget(this);
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    spacerAction_ = addWidget(spacer);
}
