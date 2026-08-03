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


#ifndef FM_SIDEPANE_H
#define FM_SIDEPANE_H

// Same file as ../filer_sidebar.h (the top-level Welcome app's cut-down copy
// of Filer's real src/sidepane.h) -- the Fm::SidePane widget declaration
// (the actual places/dirtree file-browser widget, needing FmPath/libfm-qt)
// is gone there and stays gone here; this qt5/ shell has no folders to
// browse either. What's left is the pair of free functions that don't touch
// libfm-qt at all.
//
// Unlike the top-level Welcome app (which calls buildSidePaneStyleSheet()
// with hardcoded literals at its one call site), qt5/mainwindow.cpp reads
// the real [FolderView] SidePane* keys out of
// ~/.config/filer/default/settings.conf and passes those through instead --
// see MainWindow::buildSidebar() there.
#include <QString>
#include <QFont>
#include <QColor>

namespace Fm {

// itemSpacing is the vertical gap (px) between item rows (the pill's
// top/bottom margin); horizontalPadding is the left/right padding (px)
// inside each pill, between its edge and the icon/text content; fontWeight
// is a CSS-style numeric weight (100-900, e.g. 400 normal, 500 medium,
// 600 semibold); selectorColor/selectorOpacity (0-255) are the selection
// pill's fill color.
QString buildSidePaneStyleSheet(int itemSpacing, int horizontalPadding, int fontWeight,
                                 const QColor& selectorColor, int selectorOpacity, int fontSize = 11);

// The "Mutern VF" font (family + numeric weight, 100-900-ish per
// Settings::sidePaneFontWeight(), pixel size per sidePaneItemFontSize())
// applied to sidebar item rows -- see the comment on its definition for
// why this is a QFont rather than a QSS font-family/font-weight declaration.
QFont sidePaneItemFont(int fontWeight, int fontSize);

}

#endif // FM_SIDEPANE_H
