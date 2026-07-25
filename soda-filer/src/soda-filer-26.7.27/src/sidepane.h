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

#include "libfmqtglobals.h"
#include <libfm/fm.h>
#include <QWidget>
#include <QString>
#include <QFont>
#include <QColor>

class QComboBox;
class QVBoxLayout;
class QWidget;

namespace Filer {
class Settings;
}

namespace Fm {

class FileMenu;

// Shared with OpenDialog's own places sidebar so both look identical.
// The background is fully transparent by design: the containing window's
// paintEvent() paints the frosted (translucent, KWin-blurred) full-height
// column behind the sidebar -- painting any fill here as well would just
// double-darken that column.
// itemSpacing is the vertical gap (px) between item rows (the pill's
// top/bottom margin); horizontalPadding is the left/right padding (px)
// inside each pill, between its edge and the icon/text content; fontWeight
// is a CSS-style numeric weight (100-900, e.g. 400 normal, 500 medium,
// 600 semibold); selectorColor/selectorOpacity (0-255) are the selection
// pill's fill color.
LIBFM_QT_API QString buildSidePaneStyleSheet(int itemSpacing, int horizontalPadding, int fontWeight,
                                              const QColor& selectorColor, int selectorOpacity);

// The "Mutern VF" font (family + numeric weight, 100-900-ish per
// Settings::sidePaneFontWeight(), pixel size per sidePaneItemFontSize())
// applied to sidebar item rows -- see the comment on its definition for
// why this is a QFont rather than a QSS font-family/font-weight declaration.
LIBFM_QT_API QFont sidePaneItemFont(int fontWeight, int fontSize);

class LIBFM_QT_API SidePane : public QWidget {
  Q_OBJECT

public:
  enum Mode {
      ModeNone = -1,
      ModePlaces = 0,
      ModeDirTree,
      NumModes
  };

public:
  explicit SidePane(QWidget* parent = 0);
  virtual ~SidePane();

  QSize iconSize() {
    return iconSize_;
  }

  void setIconSize(QSize size);

  // Re-applies all sidebar layout/appearance settings (item spacing/
  // padding, font weight/sizes, icon-text gap, category header spacing/
  // padding, selector color/opacity/reach, scrollbar mode) to whichever
  // view is currently active -- called both at startup and live (see
  // Application::onTintSettingsFileChanged()). Values are cached as plain
  // members (rather than holding onto the Settings reference) so
  // setCurrentPath()'s mode switches (Places/DirTree) and setMode()'s view
  // (re)creation can keep re-applying the current settings without needing
  // a live Settings object at that point.
  void applySettings(Filer::Settings& settings);

  // Re-tints the Places sidebar's accent-colored SVG icons after a system
  // accent-color change -- see PlacesView::updateAccentIcons()/
  // PlacesModel::updateIcons(). No-op in DirTree mode (plain folder icons,
  // not accent-tinted).
  void updateAccentIcons();

  FmPath* currentPath() {
    return currentPath_;
  }

  void setCurrentPath(FmPath* path);

  void setMode(Mode mode);

  Mode mode() {
    return mode_;
  }

  QWidget* view() {
    return view_;
  }

  const char *modeName(Mode mode);

  Mode modeByName(const char *str);

#if 0 // FIXME: are these APIs from libfm-qt needed?
  int modeCount(void) {
    return NumModes;
  }

  QString modeLabel(Mode mode);

  QString modeTooltip(Mode mode);
#endif

  void setShowHidden(bool show_hidden);

  bool showHidden() {
    return showHidden_;
  }

  bool setHomeDir(const char *home_dir);

  // libfm-gtk compatible alias
  FmPath* getCwd() {
    return currentPath();
  }

  void chdir(FmPath* path) {
    setCurrentPath(path);
  }

Q_SIGNALS:
  void chdirRequested(int type, FmPath* path);
  void openFolderInNewWindowRequested(FmPath* path);
  void openFolderInNewTabRequested(FmPath* path);
  void openFolderInTerminalRequested(FmPath* path);
  void createNewFolderRequested(FmPath* path);
  void modeChanged(Fm::SidePane::Mode mode);

  void prepareFileMenu(Fm::FileMenu* menu); // emit before showing a Fm::FileMenu

protected Q_SLOTS:
  void onPlacesViewChdirRequested(int type, FmPath* path);
  void onDirTreeViewChdirRequested(int type, FmPath* path);
  void onComboCurrentIndexChanged(int current);

private:
  void initDirTree();

private:
  FmPath* currentPath_;
  QWidget* view_;
  QComboBox* combo_;
  QVBoxLayout* verticalLayout;
  QSize iconSize_;
  Mode mode_;
  bool showHidden_;
  int itemSpacing_ = 0;
  int horizontalPadding_ = 6;
  int fontWeight_ = 500;
  int itemFontSize_ = 11;
  int categoryFontSize_ = 11;
  int categoryLeftPadding_ = 4;
  int iconTextSpacing_ = 6;
  int categorySpacingTop_ = 17;
  int categorySpacingBottom_ = 8;
  int selectorRightInset_ = 8;
  QColor selectorColor_ = QColor(66, 66, 66);
  int selectorOpacity_ = 255;
  int scrollBarMode_ = 0;
  class ItemStyle* itemStyle_ = nullptr;
};

}

#endif // FM_SIDEPANE_H
