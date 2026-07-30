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

// Moved out of filer_titlebar.cpp (see titlebar_buttons.h) -- cut down from
// Filer's real src/mainwindow.cpp: the traffic-lights ctor block
// (originally ~line 701-736).

#include "titlebar_buttons.h"

#include <QWidget>
#include <QPushButton>
#include <QHBoxLayout>
#include <QPixmap>
#include <QIcon>
#include <QEvent>
#include <QMouseEvent>

namespace {

// QSS's `border-image: url(...)` silently fails to load any path whose
// filename matches Filer's own asset naming (`*-dark@2.png`) -- Qt's
// stylesheet image loader treats the bare "@2" as a devicePixelRatio marker
// (the convention is normally "@2x") and comes up empty, even though the
// exact same path loads fine through a plain QPixmap(path) constructor.
// Side-stepping the stylesheet image loader entirely -- QIcon/QPixmap swapped
// by hand on hover/press -- avoids the bug outright.
class TrafficDotButton : public QPushButton {
public:
    TrafficDotButton(QPixmap normal, QPixmap hover, QPixmap active, QWidget* parent)
        : QPushButton(parent), normal_(std::move(normal)), hover_(std::move(hover)), active_(std::move(active)) {
        setIcon(QIcon(normal_));
    }

protected:
    void enterEvent(QEvent* event) override {
        setIcon(QIcon(hover_));
        QPushButton::enterEvent(event);
    }
    void leaveEvent(QEvent* event) override {
        setIcon(QIcon(normal_));
        QPushButton::leaveEvent(event);
    }
    void mousePressEvent(QMouseEvent* event) override {
        setIcon(QIcon(active_));
        QPushButton::mousePressEvent(event);
    }
    void mouseReleaseEvent(QMouseEvent* event) override {
        setIcon(QIcon(underMouse() ? hover_ : normal_));
        QPushButton::mouseReleaseEvent(event);
    }

private:
    QPixmap normal_, hover_, active_;
};

}

namespace FilerChrome {

// Ported from MainWindow::MainWindow(), the sidebarSpacer_/makeTrafficDot
// block (src/mainwindow.cpp, originally ~line 701-736):
//
//   sidebarSpacer_ = new QWidget(ui.toolBar);
//   QHBoxLayout* tlLayout = new QHBoxLayout(sidebarSpacer_);
//   tlLayout->setContentsMargins(10, 0, 12, 0);
//   tlLayout->setSpacing(8);
//   // The button faces come straight from the pearOS-dark GTK theme's own
//   // titlebutton assets (see resources.qrc) -- normal state is the plain
//   // colored dot, hover shows the x/-/+ glyph, pressed shows the darker
//   // "active" face -- so Filer's chrome is pixel-identical to GTK windows
//   // decorated with the same theme.
//   auto makeTrafficDot = [&](const QString& assetBase) {
//       QPushButton* b = new QPushButton(sidebarSpacer_);
//       b->setFixedSize(16, 16);
//       b->setFlat(true);
//       b->setFocusPolicy(Qt::NoFocus);
//       b->setStyleSheet(QStringLiteral(
//           "QPushButton { border: none; background: transparent;"
//           " border-image: url(:/icons/titlebutton-%1.png) 0 0 0 0 stretch stretch; }"
//           "QPushButton:hover { border-image: url(:/icons/titlebutton-%1-hover.png) 0 0 0 0 stretch stretch; }"
//           "QPushButton:pressed { border-image: url(:/icons/titlebutton-%1-active.png) 0 0 0 0 stretch stretch; }").arg(assetBase));
//       tlLayout->addWidget(b);
//       return b;
//   };
//
// Adapted here exactly as the top-level Welcome app adapted it: assets load
// from a plain filesystem path (this project ships no Qt resource bundle,
// just PNG files on disk -- see assetsBasePath below), filenames match the
// shared repo assets/titlebuttons/ directory (copied byte-for-byte from
// Filer's src/icons/titlebuttons/), and each dot gets an #id selector
// instead of the bare "QPushButton" one Filer uses, so a stray app-wide
// QPushButton rule elsewhere can't win the cascade over it.
QWidget* buildTrafficLights(QWidget* parent, const QString& assetsBasePath,
                             std::function<void()> onClose,
                             std::function<void()> onMinimize,
                             std::function<void()> onMaximize) {
    QWidget* sidebarSpacer_ = new QWidget(parent);
    QHBoxLayout* tlLayout = new QHBoxLayout(sidebarSpacer_);
    tlLayout->setContentsMargins(10, 0, 12, 0);
    tlLayout->setSpacing(8);

    auto makeTrafficDot = [&](const QString& assetBase, std::function<void()> slot) {
        const QString base = assetsBasePath + QStringLiteral("/titlebuttons/titlebutton-%1").arg(assetBase);
        TrafficDotButton* b = new TrafficDotButton(
            QPixmap(base + QStringLiteral("-dark@2.png")),
            QPixmap(base + QStringLiteral("-hover-dark@2.png")),
            QPixmap(base + QStringLiteral("-active-dark@2.png")),
            sidebarSpacer_);
        b->setObjectName(QStringLiteral("trafficDot_%1").arg(assetBase));
        b->setFixedSize(14, 14);
        b->setIconSize(QSize(14, 14));
        b->setFlat(true);
        b->setFocusPolicy(Qt::NoFocus);
        b->setStyleSheet(QStringLiteral("QPushButton { border: none; background: transparent; }"));
        QObject::connect(b, &QPushButton::clicked, b, [slot]() { slot(); });
        tlLayout->addWidget(b);
        return b;
    };
    makeTrafficDot(QStringLiteral("close"), std::move(onClose));
    makeTrafficDot(QStringLiteral("minimize"), std::move(onMinimize));
    makeTrafficDot(QStringLiteral("maximize"), std::move(onMaximize));
    tlLayout->addStretch();
    return sidebarSpacer_;
}

}
