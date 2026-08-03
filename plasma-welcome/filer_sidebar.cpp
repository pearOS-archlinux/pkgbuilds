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


// Same body as ../filer_sidebar.cpp, itself cut down from Filer's real
// src/sidepane.cpp: the removed includes (placesview.h/dirtreeview.h/
// dirtreemodel.h/path.h/filemenu.h/settings.h, all libfm-qt-only) and the
// ItemStyle class + scrollbar-mode helpers belonged to Fm::SidePane, the
// actual places/dirtree file-browser widget -- this qt5/ shell has no
// folders to browse, so that widget isn't dragged in here either. What's
// left below is the part that doesn't touch libfm-qt at all: the sidebar's
// pure-Qt stylesheet formula and item font, used as-is by this project's own
// QListWidget sidebar (see MainWindow::buildSidebar() in qt5/mainwindow.cpp,
// which calls these with real values read from
// ~/.config/filer/default/settings.conf's [FolderView] SidePane* keys).
#include "filer_sidebar.h"
#include <QApplication>
#include <QPalette>

namespace Fm {

// Finder sidebar look: no frame/border, and a rounded selection pill
// instead of Qt's default square highlight rectangle. The background is
// fully transparent: MainWindow::paintEvent() paints the frosted
// (translucent, KWin-blurred) full-height column behind the sidebar --
// painting any fill here as well would just double-darken that column.
// Row dimensions (24px tall, 16px icon, 11px label, no vertical padding/
// margin -- items sit flush per spec's "Items" container having no gap)
// match the uikit design reference's actual assembled example, which is
// the authoritative one (~/Desktop/uikit-svg/examples/fullwindow.css,
// "Item"/"Leading"/"Folder"/"Label" blocks), not the larger "expanded"
// variant in sidebars/sidebar_light_example.css used previously.
// Label color used to be hardcoded to the uikit dark-variant swatch
// (~/Desktop/uikit-svg/sidebars/sidebar_dark_example.css, #F5F5F5) on the
// assumption the system only ever ran pearOS-dark -- it stayed white-on-dark
// even after switching to a light color scheme, since MainWindow::paintEvent()
// now derives the sidebar's actual background from the live palette (see the
// comment there) but this stylesheet didn't follow. QPalette::WindowText is
// the same live, theme-aware signal, computed fresh on every call.
// Selection pill: rgba(153, 153, 153, 0.17) i.e. rgba(153, 153, 153, 43).
// "Mutern VF" (pearos-muternvf, already installed system-wide -- see
// DesktopItemDelegate's identical use for desktop icon labels) is a
// variable font exposing weight-mapped named instances (Text Medium, Text
// SemiBold, Text Bold, ...); fontconfig auto-picks the nearest instance for
// a requested numeric weight. Applied via QWidget::setFont() rather than
// the stylesheet's font-family/font-weight: a QSS font-family declaration
// on ::item wasn't reliably picked up by the item delegate's rendering
// path, while the widget-level QFont always is (same reason
// styleSectionHeader() in placesmodel.cpp sets it via QFont, not QSS).
QFont sidePaneItemFont(int fontWeight, int fontSize) {
  QFont f(QStringLiteral("Mutern VF"));
  f.setPixelSize(fontSize);
  f.setWeight(QFont::Weight(qBound(1, fontWeight, 1000)));
  return f;
}

QString buildSidePaneStyleSheet(int itemSpacing, int horizontalPadding, int fontWeight,
                                 const QColor& selectorColor, int selectorOpacity, int fontSize) {
  Q_UNUSED(fontWeight);
  Q_UNUSED(selectorColor);
  const QPalette pal = qApp->palette();
  // Used to be a flat rgba(selectorColor, selectorOpacity) painted straight
  // over the sidebar -- the configured grey blended toward whatever's
  // underneath by the same fixed ratio regardless of theme. That ratio was
  // tuned against a dark sidebar (a fixed-distance-toward-mid-grey pill
  // reads as a subtle lightening there), but the *same* fixed distance from
  // a light background overshoots into a heavy, dark-looking block instead
  // of the pale, barely-there pill macOS actually uses in light mode --
  // it's not that the two themes need different math, it's that "blend a
  // fixed amount toward one absolute grey" was never going to land as the
  // same relative contrast against both a light and a dark base.
  // Blending toward a per-theme target derived *from the live background*
  // instead (a bit lighter in dark mode, a bit darker in light mode) keeps
  // the pill a small, constant perceptual step away from its own
  // background either way. selectorOpacity still controls how strong that
  // step reads, same knob as before (Settings::sidePaneSelectorOpacity()).
  const QColor sidebarBg = pal.color(QPalette::Window);
  const bool isDark = sidebarBg.lightness() < 128;
  const QColor themedTarget = isDark ? sidebarBg.lighter(180) : sidebarBg.darker(112);
  const qreal blend = qBound(0, selectorOpacity, 255) / 255.0;
  const QColor pillColor = QColor::fromRgbF(
      sidebarBg.redF()   + (themedTarget.redF()   - sidebarBg.redF())   * blend,
      sidebarBg.greenF() + (themedTarget.greenF() - sidebarBg.greenF()) * blend,
      sidebarBg.blueF()  + (themedTarget.blueF()  - sidebarBg.blueF())  * blend);
  QString selectorRgba = QStringLiteral("rgba(%1, %2, %3, 255)")
      .arg(pillColor.red()).arg(pillColor.green()).arg(pillColor.blue());
  const QColor text = pal.color(QPalette::Active, QPalette::WindowText);
  QColor disabledText = pal.color(QPalette::Disabled, QPalette::WindowText);
  disabledText.setAlpha(100); // same ~39% the old rgba(245, 245, 245, 100) carried
  QString textRgba = QStringLiteral("rgba(%1, %2, %3, %4)")
      .arg(text.red()).arg(text.green()).arg(text.blue()).arg(text.alpha());
  QString disabledTextRgba = QStringLiteral("rgba(%1, %2, %3, %4)")
      .arg(disabledText.red()).arg(disabledText.green()).arg(disabledText.blue()).arg(disabledText.alpha());
  return QStringLiteral(
    // Horizontal margin is 0 here -- the selection pill spans the full row
    // width, only inset by horizontalPadding; this rule supplies the
    // pill's vertical margin (item spacing) and its rounded shape.
    // (Filer's original targets QTreeView::item, QListView::item -- both
    // its Places and DirTree sidebar modes; this project's sidebar is
    // always a QListWidget, so only that selector is kept.)
    "QListWidget::item { border-radius: 8px; padding: 0px %2px; margin: %1px 0px; color: %4; font-size: %6px; }"
    // macOS selects sidebar rows with a neutral grey pill (the accent color
    // only tints the row's icon), not a filled accent-blue bar.
    "QListWidget::item:selected:enabled { background-color: %3; color: %4; }"
    // No hover highlight in the sidebar -- only the selection pill above
    // should ever show a background.
    "QListWidget::item:hover:!selected:enabled { background-color: transparent; }"
    "QListWidget::item:disabled { color: %5; background-color: transparent; }"
  ).arg(itemSpacing).arg(horizontalPadding).arg(selectorRgba).arg(textRgba).arg(disabledTextRgba).arg(fontSize);
}

}
