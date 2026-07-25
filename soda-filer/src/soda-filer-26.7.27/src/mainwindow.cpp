#include <QDialog>
#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QDateTime>
#include <QVariantAnimation>
#include <QEasingCurve>
#include <cmath>
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

#include "mainwindow.h"

#include <QAction>
#include <QCompleter>
#include <QDebug>
#include <QFileSystemModel>
#include <QKeySequence>
#include <QLabel>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QProcess>
#include <QShortcut>
#include <QSplitter>
#include <QStandardPaths>
#include <QToolButton>
#include <QStorageInfo>
#include <QImage>
#include <QPalette>
#include <QVBoxLayout>
#include <QPainter>
#include <QPainterPath>
#include <QPushButton>
#include <QMouseEvent>
#include <QTimer>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QGraphicsBlurEffect>
#include <QScrollBar>
#include <KWindowEffects>

#include "quicklookdialog.h"
#include <QX11Info>
#include <cstring>
#include <xcb/xcb.h>

#include "application.h"
#include "bookmarkaction.h"
#include "desktopwindow.h"
#include "filelauncher.h"
#include <QScreen>
#include <QGuiApplication>
#include "filemenu.h"
#include "tags.h"
#include "fileoperation.h"
#include "filepropsdialog.h"
#include "gotofolderwindow.h"
#include "folderview_p.h"
#include <QtEndian>
#include "metadata.h"
#include "path.h"
#include "icontheme.h"
#include "pathedit.h"
#include "tabpage.h"
#include "trash.h"
#include "ui_about.h"
#include "utilities.h"
#include "windowregistry.h"

// #include "qmodeltest/modeltest.h"

#include <X11/Xlib.h>

using namespace Fm;

namespace Filer {

static constexpr qreal kWindowCornerRadius = 16; // matches the Tahoe reference's corner rounding

// Forward declaration -- defined further down (near updateSidebarBlurRegion()),
// but needed earlier by eventFilter()'s statusBarOverlay_ paint handling too.
static int sidebarColumnRight(const QWidget* sidePane, const QWidget* window);

QWidget* MainWindow::createToolbarButtonGroup(const QList<QAction*>& actions, const QString& groupToolTip) {
  QWidget* group = new QWidget(ui.toolBar);
  QHBoxLayout* layout = new QHBoxLayout(group);
  layout->setContentsMargins(4, 3, 4, 3);
  layout->setSpacing(0);
  for(QAction* action : actions) {
    QToolButton* button = new QToolButton(group);
    button->setDefaultAction(action);
    button->setAutoRaise(true);
    if(!groupToolTip.isEmpty())
      button->setToolTip(QString()); // tooltip lives on the group instead
    layout->addWidget(button);
  }
  // "Liquid Glass" backdrop (see paintLiquidGlassPanel()) instead of the
  // previous flat rgba(127,127,127,20) fill. No "glassRadius" override --
  // the default (half the widget's height) gives a true pill/stadium
  // shape, not the old fixed-14px rounded rect.
  group->setAttribute(Qt::WA_TranslucentBackground);
  group->installEventFilter(this);
  glassPanels_.append(group);
  if(!groupToolTip.isEmpty())
    group->setToolTip(groupToolTip);
  return group;
}

// Left/right margin (layout contentsMargins) + explicit height/width for a
// naturally-sized segmented pill (BackForward/ViewOptions/ShareTagsMore):
// 0 means "auto", i.e. leave that dimension to the layout's own sizeHint
// instead of forcing it, since these groups' natural width already depends
// on how many buttons they hold.
//
// When width IS given (>0), it's the pill's real, exact total width --
// margin no longer folds into it (an earlier attempt made a negative
// margin shrink the pill from this end instead of moving it, which wasn't
// what a negative *margin* should do -- see backForwardGroup_'s own
// position offset in applyScrollInsets(), which is the actual fix for
// "move/overlap, don't resize"). With Width explicit, margin here now only
// controls how tightly the buttons sit against the pill's own fixed edges.
static void applyToolbarPillGroupSettings(QWidget* group, int marginLeft, int marginRight, int height, int width) {
  if(!group)
    return;
  if(QHBoxLayout* layout = qobject_cast<QHBoxLayout*>(group->layout())) {
    QMargins m = layout->contentsMargins();
    m.setLeft(marginLeft);
    m.setRight(marginRight);
    layout->setContentsMargins(m);
  }
  QSizePolicy policy = group->sizePolicy();
  policy.setHorizontalPolicy(width > 0 ? QSizePolicy::Fixed : QSizePolicy::Preferred);
  policy.setVerticalPolicy(height > 0 ? QSizePolicy::Fixed : QSizePolicy::Preferred);
  group->setSizePolicy(policy);
  group->setMinimumWidth(width > 0 ? width : 0);
  group->setMaximumWidth(width > 0 ? width : QWIDGETSIZE_MAX);
  group->setMinimumHeight(height > 0 ? height : 0);
  group->setMaximumHeight(height > 0 ? height : QWIDGETSIZE_MAX);
  group->updateGeometry();
  group->update();
}

// Same, but for a round glass button (IconGrouping/Search): these are never
// "naturally" sized -- 0 falls back to autoSize (Settings::
// toolbarIconOuterSize() + the wider-composite-icon bump both already used)
// instead of the layout's own sizeHint. innerButton, when given, is kept in
// sync with the button's own size (searchButton_ has its own fixed size
// separate from searchButtonGroup_'s wrapping layout, unlike groupByGroup_'s
// button which already fills its group via the layout) and its icon
// re-sized to iconSize -- meaningful for searchButton_'s plain SVG icon,
// unlike groupByGroup_/viewModeCollapsedButton_'s baked composite pixmaps,
// which don't re-render at a new size after construction.
//
// The GROUP -- not the button -- is the glass panel (see glassPanels_):
// sizing the group to exactly the button's own w/h, as before, meant margin
// only ever repositioned the icon *inside* an unchanged-size circle instead
// of growing the circle itself, the same bug applyToolbarPillGroupSettings()
// had before its own fix. Margins now pad the group beyond the button's
// size instead, so the glass grows into a stadium/pill (paintLiquidGlassPanel()
// already supports non-square panels for exactly this) as margin increases,
// matching how the three pill groups already behave.
static void applyToolbarRoundGroupSettings(QWidget* group, QAbstractButton* innerButton, int marginLeft, int marginRight, int height, int width, int autoSize, int iconSize) {
  if(!group)
    return;
  if(QHBoxLayout* layout = qobject_cast<QHBoxLayout*>(group->layout())) {
    QMargins m = layout->contentsMargins();
    m.setLeft(marginLeft);
    m.setRight(marginRight);
    layout->setContentsMargins(m);
  }
  int buttonW = width > 0 ? width : autoSize;
  int buttonH = height > 0 ? height : autoSize;
  group->setFixedSize(buttonW + marginLeft + marginRight, buttonH);
  if(innerButton) {
    innerButton->setFixedSize(buttonW, buttonH);
    innerButton->setIconSize(QSize(iconSize, iconSize));
  }
  group->updateGeometry();
  group->update();
}

void MainWindow::applyToolbarGroupSettings(Settings& settings) {
  int innerSize = settings.toolbarIconInnerSize();
  int outerSize = settings.toolbarIconOuterSize();
  int autoRoundSize = outerSize + 10; // matches groupByGroup_/searchButtonGroup_'s existing wide-composite bump

  // Drives the back/forward and view-mode segmented control icons (neither
  // calls setIconSize() individually -- both rely on this toolbar-wide
  // default, same as before this setting existed).
  ui.toolBar->setIconSize(QSize(innerSize, innerSize));

  if(backForwardLeftSpacer_)
    backForwardLeftSpacer_->setFixedWidth(settings.backForwardGroupLeftOffset());

  // Neither backForwardGroupMarginLeft() nor backForwardGroupMarginRight()
  // is passed as this pill's own internal padding here -- both move things
  // externally instead (the group itself, and folderNameLabel_ / the
  // "Desktop -- Local" title, respectively -- see the position offsets in
  // applyScrollInsets()), so applying them here too would shift the
  // buttons/pill a second time on top of that. Both internal edges keep
  // createToolbarButtonGroup()'s own default (4) regardless of the
  // settings' values.
  applyToolbarPillGroupSettings(backForwardGroup_, 4, 4, settings.backForwardGroupHeight(), settings.backForwardGroupWidth());
  applyToolbarPillGroupSettings(viewModeGroup_, settings.viewOptionsGroupMarginLeft(), settings.viewOptionsGroupMarginRight(), settings.viewOptionsGroupHeight(), settings.viewOptionsGroupWidth());
  applyToolbarPillGroupSettings(shareTagsMoreGroup_, settings.shareTagsMoreGroupMarginLeft(), settings.shareTagsMoreGroupMarginRight(), settings.shareTagsMoreGroupHeight(), settings.shareTagsMoreGroupWidth());
  applyToolbarRoundGroupSettings(groupByGroup_, nullptr, settings.iconGroupingGroupMarginLeft(), settings.iconGroupingGroupMarginRight(), settings.iconGroupingGroupHeight(), settings.iconGroupingGroupWidth(), autoRoundSize, innerSize);
  applyToolbarRoundGroupSettings(searchButtonGroup_, searchButton_, settings.searchButtonGroupMarginLeft(), settings.searchButtonGroupMarginRight(), settings.searchButtonGroupHeight(), settings.searchButtonGroupWidth(), autoRoundSize, innerSize);

  // Share/Tags/More's own three buttons still get their icons resized
  // individually (the group itself isn't round, so there's no single
  // "circle size" to derive them from) -- now driven by the shared
  // innerSize instead of the old per-group ShareMoreButtonSize.
  if(shareTagsMoreGroup_) {
    const QList<QToolButton*> buttons = shareTagsMoreGroup_->findChildren<QToolButton*>();
    for(QToolButton* button : buttons)
      button->setIconSize(QSize(innerSize, innerSize));
  }

  // Each group's own updateGeometry() above only invalidates *its* cached
  // sizeHint -- QToolBarLayout (ui.toolBar's own layout, wrapping each group
  // as a QWidgetAction) doesn't listen for that and keeps handing the group
  // its old slot geometry until something invalidates the toolbar layout
  // itself. Without this, a margin/height/width change only ever moved the
  // icon *inside* the group's unchanged-size glass pill instead of actually
  // resizing the pill -- this is what forces the toolbar to re-measure every
  // item and give each group its real, current-settings size.
  if(QLayout* toolBarLayout = ui.toolBar->layout()) {
    toolBarLayout->invalidate();
    toolBarLayout->activate();
  }
  ui.toolBar->update();
}

// Animates searchHolder_'s width between fromWidth and toWidth via a
// temporary setFixedWidth() override (searchHolder_ normally just has
// min/max width, letting the toolbar layout size it -- a fixed width lets
// each animation frame force an exact, smoothly interpolated size instead).
// collapseAtEnd: when true (closing), an OutBack bounce would overshoot
// past the collapsed button's own size, so InCubic is used instead, and
// searchButton_ is swapped back in once the animation finishes.
void MainWindow::animateSearchBar(int fromWidth, int toWidth, bool collapseAtEnd) {
  if(!searchAnim_ || !searchHolder_)
    return;
  searchAnim_->stop();
  searchAnim_->disconnect();
  connect(searchAnim_, &QVariantAnimation::valueChanged, this, [this](const QVariant& v) {
    if(searchHolder_)
      searchHolder_->setFixedWidth(v.toInt());
  });
  if(collapseAtEnd) {
    connect(searchAnim_, &QVariantAnimation::finished, this, [this]() {
      if(!searchHolderAction_ || !searchButtonAction_)
        return;
      searchHolderAction_->setVisible(false);
      searchButtonAction_->setVisible(true);
    });
  }
  searchAnim_->setEasingCurve(collapseAtEnd ? QEasingCurve::InCubic : QEasingCurve::OutBack);
  searchAnim_->setStartValue(fromWidth);
  searchAnim_->setEndValue(toWidth);
  searchAnim_->start();
}

// Settings::searchBarExpandWidth(): at/above that window width, the search
// field stays permanently expanded (searchButton_ hidden, no animation --
// this isn't a user interaction, just the toolbar reflowing); below it,
// falls back to the collapsed button unless the user already clicked it
// open (searchManuallyExpanded_, left alone here -- the FocusOut handler
// in eventFilter() is what closes that case). Called from resizeEvent()
// and updateFromSettings() (the threshold itself is live-reloadable).
void MainWindow::updateSearchBarMode() {
  if(!searchButton_ || !searchHolder_ || !searchHolderAction_ || !searchButtonAction_)
    return;
  Settings& settings = static_cast<Application*>(qApp)->settings();
  bool wide = width() >= settings.searchBarExpandWidth();
  if(wide) {
    // QAbstractAnimation::stop() does NOT emit finished() -- an in-flight
    // collapse animation's finished handler (which does this same
    // hide-holder/show-button swap) would silently never run, leaving
    // both widgets visible at once. Every branch here sets both widgets'
    // visibility explicitly and unconditionally instead of trusting an
    // animation callback to have done it, so this can never end up
    // ambiguous regardless of what was interrupted.
    if(searchAnim_)
      searchAnim_->stop();
    searchManuallyExpanded_ = false;
    searchHolder_->setMinimumWidth(140);
    searchHolder_->setMaximumWidth(220);
    searchHolderAction_->setVisible(true);
    searchButtonAction_->setVisible(false);
  }
  else if(searchManuallyExpanded_) {
    // Mid-interaction below the threshold (the user clicked searchButton_
    // open, or is about to focus-out and collapse it) -- leave
    // searchHolder_'s width/animation alone, but searchButton_ must stay
    // hidden regardless of how this state was reached.
    searchButtonAction_->setVisible(false);
  }
  else {
    if(searchAnim_)
      searchAnim_->stop();
    searchHolderAction_->setVisible(false);
    searchButtonAction_->setVisible(true);
  }
  // See the matching comment in updateViewModeCollapseMode(): swapping the
  // search field for the round button (or back) changes ui.toolBar's own
  // natural height, so its real geometry needs recomputing here too --
  // not just from resizeEvent() -- or a settings-reload-triggered swap
  // leaves ui.toolBar->height() stale for applyScrollInsets()' sidebar
  // inset until the next real resize.
  updateToolbarOverlayGeometry();
}

// See the declaration in mainwindow.h.
void MainWindow::updateToolbarStyleSheet() {
  Settings& settings = static_cast<Application*>(qApp)->settings();
  // Hover/pressed/checked feedback used to be a fixed white wash
  // (rgba(255,255,255,...)) -- a visible lightening against a dark toolbar,
  // but close to invisible against a light one (white washed over
  // near-white). Same fix as the sidebar selector pill: wash toward
  // whichever direction actually contrasts with the live background
  // (lighten on dark, darken on light) instead of a fixed color.
  const bool isDark = palette().color(QPalette::Window).lightness() < 128;
  const char* wash = isDark ? "255, 255, 255" : "0, 0, 0";
  // Taller bar with round (circular-hover) buttons, like the Tahoe
  // reference toolbar. QLineEdit into a quiet, pill-shaped "current
  // folder" field, closer to Finder's understated toolbar than a
  // browser-style URL bar.
  ui.toolBar->setStyleSheet(QStringLiteral(
      "QToolBar { border: none; background: transparent; spacing: 8px; padding: 8px 10px; }"
      "QToolBar QToolButton { border: none; border-radius: 14px; padding: %1px; }"
      "QToolBar QToolButton:hover { background-color: rgba(%2, 25); }"
      "QToolBar QToolButton:pressed { background-color: rgba(%2, 45); }"
      // The view-mode segmented control's checked/filled state (see its
      // setup further down in the constructor) -- kept here, rebuilt
      // alongside everything else above, rather than appended once at
      // startup: this whole stylesheet gets replaced wholesale on every
      // live settings/theme reload (Application::onSystemThemeConfigChanged()
      // et al. -> updateFromSettings() -> here), which was silently
      // dropping a rule appended only once after construction.
      "QToolBar QToolButton:checked { background-color: rgba(%2, 45); border-radius: 14px; }")
      .arg(settings.toolbarIconPadding()).arg(QLatin1String(wash)));
}

// See the declaration in mainwindow.h.
void MainWindow::updateScrollBarStyleSheet() {
  Settings& settings = static_cast<Application*>(qApp)->settings();
  // Only the width/height metric is overridden -- no color/border/margin
  // rules -- so the handle, groove, and arrows stay whatever the active
  // system theme draws them as; this cascades to every QScrollBar under
  // `this` (folder view, sidebar, column view...) via Qt's own stylesheet
  // inheritance, so it doesn't need to be set on each one individually.
  // Plain set, not append: nothing else sets a stylesheet on `this`, so
  // there's nothing to preserve -- appending here instead would accumulate
  // one more duplicate copy of this same rule on every settings reload.
  setStyleSheet(QStringLiteral(
      "QScrollBar:vertical { width: %1px; }"
      "QScrollBar:horizontal { height: %1px; }")
      .arg(settings.scrollBarWidth()));
  // A wide enough scrollbar inflates the sidebar QTreeView's own
  // minimumSizeHint() (Qt reserves room for its scrollbars there) past
  // Settings::splitterPos() -- QSplitter::setSizes() (see
  // updateFromSettings()) silently clamps up to a child's minimum size
  // instead of honoring the requested size below it, so a large enough
  // ScrollBarWidth widened the whole sidebar column instead of just its
  // scrollbar. Overriding the minimum here removes that floor so the
  // splitter can actually hold the sidebar at its configured width; the
  // scrollbar itself still renders at the requested width regardless,
  // since that comes from the stylesheet above, not from this minimum.
  if(ui.sidePane)
    ui.sidePane->setMinimumWidth(1);
}

// Settings::viewModeCollapseWidth(): below that window width, the 4-button
// segmented control doesn't fit comfortably, so it's replaced with
// viewModeCollapsedButton_ (see the ctor). Also forced regardless of width
// while the search field is manually expanded (searchManuallyExpanded_,
// see the searchButton_ click handler and the filterBar FocusOut handler in
// eventFilter()) -- the segmented control plus an expanded 180px search
// field was routinely wider than the toolbar had room for, which pushed
// filterBar (or something else in the layout) out far enough that it lost
// focus and immediately re-collapsed itself right back, making the search
// button look broken/unclickable. Freeing that space by collapsing the
// segmented control first is the same trick already used for a narrow
// window, just triggered by a different condition.
void MainWindow::updateViewModeCollapseMode() {
  if(!viewModeGroupAction_ || !viewModeCollapsedButtonAction_)
    return;
  Settings& settings = static_cast<Application*>(qApp)->settings();
  bool collapse = width() < settings.viewModeCollapseWidth() || searchManuallyExpanded_;
  viewModeGroupAction_->setVisible(!collapse);
  viewModeCollapsedButtonAction_->setVisible(collapse);
  // Swapping the 4-button segmented control for the single round button
  // (or back) changes ui.toolBar's own natural height -- they're not the
  // same size. updateToolbarOverlayGeometry() (which reads that height via
  // sizeHint() and re-applies ui.toolBar's real geometry) used to only run
  // from resizeEvent(), so a collapse triggered by a live settings reload
  // (not a resize) left ui.toolBar->height() stale until the next real
  // resize -- and applyScrollInsets() reads exactly that height for the
  // sidebar's pass-through inset, so the sidebar's item offset broke until
  // then too.
  updateToolbarOverlayGeometry();
}

void MainWindow::updateBreadcrumb(const QString& pathName) {
  QLayoutItem* child;
  while((child = breadcrumbLayout_->takeAt(0)) != nullptr) {
    delete child->widget();
    delete child;
  }
  if(pathName.isEmpty())
    return;

  // QStorageInfo(pathName) picks whatever mount point the path happens to
  // live under -- on this system /home is its own subvolume/mount, so a
  // path like /home/alxb421/Desktop anchored the trail at "home" instead
  // of the actual disk. Only anchor at that inner mount if it's a genuinely
  // different physical device (an external USB, a DMG's tmp mount, etc.);
  // otherwise always start from the root filesystem's own volume label
  // ("SSD"), matching Finder, which never stops at internal mount
  // boundaries.
  QStorageInfo rootStorage(QStringLiteral("/"));
  QStorageInfo pathStorage(pathName);
  bool sameDevice = !pathStorage.isValid() || !rootStorage.isValid()
      || pathStorage.device() == rootStorage.device();
  const QStorageInfo& anchorStorage = sameDevice ? rootStorage : pathStorage;
  QString rootPath = sameDevice ? QStringLiteral("/")
      : (anchorStorage.isValid() ? anchorStorage.rootPath() : QStringLiteral("/"));
  QString volumeName = (anchorStorage.isValid() && !anchorStorage.displayName().isEmpty())
      ? anchorStorage.displayName() : rootPath;

  QString relative = pathName.mid(rootPath.length());
  QStringList segments = relative.split(QLatin1Char('/'), Qt::SkipEmptyParts);

  QStringList crumbLabels;
  QStringList crumbPaths;
  crumbLabels << volumeName;
  crumbPaths << rootPath;
  QString accum = rootPath;
  for(const QString& seg : segments) {
    accum = accum.endsWith(QLatin1Char('/')) ? (accum + seg) : (accum + QLatin1Char('/') + seg);
    crumbLabels << seg;
    crumbPaths << accum;
  }

  for(int i = 0; i < crumbLabels.size(); ++i) {
    if(i > 0) {
      QLabel* sep = new QLabel(QStringLiteral("›"), breadcrumbBar_);
      sep->setStyleSheet(QStringLiteral("color: rgba(127, 127, 127, 150);"));
      // Fixed: a QHBoxLayout with no stretch factors set still splits any
      // leftover width equally among all Preferred-policy children (their
      // default), which was stretching the gap after every "›" instead of
      // leaving it as trailing space. Fixed pins each item to its sizeHint.
      sep->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
      breadcrumbLayout_->addWidget(sep);
    }
    QToolButton* button = new QToolButton(breadcrumbBar_);
    button->setText(crumbLabels.at(i));
    button->setAutoRaise(true);
    // Settings::breadcrumbIconSize(): the segment's real per-item colored
    // icon (whatever the active icon theme gives that specific folder/
    // volume -- the same green folder, external-drive glyph, etc. shown
    // everywhere else in the app), not a generic themed glyph. 0 hides the
    // icon entirely (falls back to the old text-only look).
    int iconSize = static_cast<Application*>(qApp)->settings().breadcrumbIconSize();
    if(iconSize > 0) {
      FmPath* crumbPath = fm_path_new_for_str(crumbPaths.at(i).toUtf8().constData());
      GError* err = nullptr;
      FmFileInfo* crumbInfo = fm_file_info_new_from_native_file(crumbPath, crumbPaths.at(i).toUtf8().constData(), &err);
      if(err)
        g_error_free(err);
      if(crumbInfo) {
        // Rendered at 64px and downscaled, not requested directly at
        // breadcrumbIconSize() -- most icon themes ship a flatter,
        // near-monochrome glyph for their small (16px) declared size and
        // save the fully colored artwork (the green folder shown
        // everywhere else in the app) for their larger ones. Asking for
        // the small size directly got that plain glyph instead of the
        // real per-item icon this is supposed to show.
        QPixmap pixmap = IconTheme::icon(fm_file_info_get_icon(crumbInfo)).pixmap(64, 64);
        button->setIcon(QIcon(pixmap));
        fm_file_info_unref(crumbInfo);
      }
      fm_path_unref(crumbPath);
      button->setIconSize(QSize(iconSize, iconSize));
      button->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    }
    else {
      button->setToolButtonStyle(Qt::ToolButtonTextOnly);
    }
    button->setStyleSheet(QStringLiteral("QToolButton { border: none; }"));
    button->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    // Guarantee the button is at least as wide as its own text (+ icon, if
    // shown) needs, in case the style's sizeHint() under-measures it --
    // without this, Qt was eliding each segment's label ("home" -> "ho...")
    // even when the bar had plenty of unused width to its right.
    int textWidth = button->fontMetrics().horizontalAdvance(crumbLabels.at(i));
    int iconAllowance = iconSize > 0 ? iconSize + 4 : 0;
    button->setMinimumWidth(textWidth + iconAllowance + 16);
    QString targetPath = crumbPaths.at(i);
    connect(button, &QToolButton::clicked, this, [this, targetPath]() {
      FmPath* fmPath = fm_path_new_for_str(targetPath.toUtf8().constData());
      chdir(fmPath);
      fm_path_unref(fmPath);
    });
    breadcrumbLayout_->addWidget(button);
  }
  // Absorb all remaining width here instead of it being split between
  // segments -- the bar can still use the full available width (segments
  // never truncate), it just doesn't have to occupy it.
  breadcrumbLayout_->addStretch(1);

  // Navigating resets scroll to a position the blur-refresh timer may have
  // already seen for a different folder (e.g. both start at 0), which would
  // otherwise skip repainting the toolbar's blurred-content background for
  // the newly loaded folder until the user scrolls.
  if(ui.toolBar)
    ui.toolBar->update();
}

MainWindow::MainWindow(FmPath *path) : QMainWindow(), fileLauncher_(this), sidebarTint_(28, 28, 28) {

  Settings &settings = static_cast<Application *>(qApp)->settings();
  setAttribute(Qt::WA_DeleteOnClose);
  // Drop the server-side (KWin) decoration entirely -- the whole window
  // chrome (rounded corners via paintEvent, traffic-light buttons in the
  // toolbar, drag-to-move on the toolbar) is drawn by us, like macOS
  // Finder's unified toolbar/titlebar window. Trade-off: no decoration
  // means no KWin resize edges either; KWin's Meta+right-drag resize (and
  // the Alt+F3 window menu) still work as fallbacks.
  setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
  // setup user interface
  ui.setupUi(this);

  // Shared size for the toolbar's round, single-icon glass buttons
  // (searchButton_, viewModeCollapsedButton_, the Group By button) -- these
  // used to be hardcoded, then derived from ui.toolBar->iconSize() (which
  // turned out to return something smaller than the toolbar's other
  // buttons actually render at, so they still looked visibly undersized).
  // Now driven by Settings::toolbarIconInnerSize()/toolbarIconOuterSize()
  // (see applyToolbarGroupSettings(), which live-reloads both) instead of
  // the old per-group ShareMoreButtonSize.
  const int kRoundToolbarButtonIconSize = settings.toolbarIconInnerSize();
  const int kRoundToolbarButtonSize = settings.toolbarIconOuterSize();

  // Frameless = no KWin resize edges, so reserve a thin strip around the
  // content that belongs to the MainWindow itself (children don't cover
  // it); presses landing there reach mousePressEvent(), which hands off to
  // the compositor via startSystemResize(). Top stays 0 -- the toolbar
  // already handles moving, and a top resize strip would fight the
  // traffic lights.
  setContentsMargins(5, 0, 5, 5);

  // The generic file-manager icon, resolved by the active icon theme (same
  // name pinder.desktop declares, so taskbar and window agree).
  this->setWindowIcon(QIcon::fromTheme("system-file-manager"));

  this->raise();

  // hide menu items that are not usable
  // if(!uriExists("computer:///"))
  //  ui.actionComputer->setVisible(false);
  if (!settings.supportTrash())
    ui.actionTrash->setVisible(false);

  // Dedicated gap between the sidebar column and the Back/Forward group
  // (Settings::backForwardGroupLeftOffset(), applied live in
  // applyToolbarGroupSettings()) -- inserted first so it lands to the left
  // of backForwardGroup_ below (QToolBar::insertWidget() slots each new
  // widget immediately before the given action; actionGoUp already sits
  // right after sidebarSpacer_/the invisible actionNewTab at this point).
  backForwardLeftSpacer_ = new QWidget(ui.toolBar);
  ui.toolBar->insertWidget(ui.actionGoUp, backForwardLeftSpacer_);

  // Back/forward belong to the same navigation action, so they're grouped
  // into one pill-shaped control instead of two separate toolbar buttons.
  backForwardGroup_ =
      createToolbarButtonGroup({ui.actionGoBack, ui.actionGoForward});
  ui.toolBar->insertWidget(ui.actionGoUp, backForwardGroup_);

  // Hairline divider between the two buttons, like the segmented view-mode
  // control's own internal seams.
  {
    QFrame* sep = new QFrame(backForwardGroup_);
    sep->setFrameShape(QFrame::VLine);
    sep->setFixedSize(1, 16);
    sep->setStyleSheet(QStringLiteral("background-color: rgba(255, 255, 255, 40); border: none;"));
    QHBoxLayout* bfLayout = qobject_cast<QHBoxLayout*>(backForwardGroup_->layout());
    bfLayout->insertWidget(1, sep);
    bfLayout->setAlignment(sep, Qt::AlignVCenter);
  }

  // add a context menu for showing browse history to back and forward buttons
  const QList<QToolButton *> backForwardButtons =
      backForwardGroup_->findChildren<QToolButton *>();
  QToolButton *backButton = backForwardButtons.value(0);
  QToolButton *forwardButton = backForwardButtons.value(1);
  backButton->setContextMenuPolicy(Qt::CustomContextMenu);
  connect(backButton, &QToolButton::customContextMenuRequested, this,
          &MainWindow::onBackForwardContextMenu);
  forwardButton->setContextMenuPolicy(Qt::CustomContextMenu);
  connect(forwardButton, &QToolButton::customContextMenuRequested, this,
          &MainWindow::onBackForwardContextMenu);

  // tabbed browsing interface
  ui.tabBar->setDocumentMode(true);
  ui.tabBar->setTabsClosable(true);
  ui.tabBar->setElideMode(Qt::ElideRight);
  ui.tabBar->setExpanding(false);
  ui.tabBar->setMovable(true); // reorder the tabs by dragging

#if QT_VERSION >= QT_VERSION_CHECK(5, 4, 0)
  // switch to the tab under the cursor during dnd.
  ui.tabBar->setChangeCurrentOnDrag(true);
  ui.tabBar->setAcceptDrops(true);
#endif

  connect(ui.tabBar, &QTabBar::currentChanged, this,
          &MainWindow::onTabBarCurrentChanged);
  connect(ui.tabBar, &QTabBar::tabCloseRequested, this,
          &MainWindow::onTabBarCloseRequested);
  connect(ui.tabBar, &QTabBar::tabMoved, this, &MainWindow::onTabBarTabMoved);
  connect(ui.stackedWidget, &QStackedWidget::widgetRemoved, this,
          &MainWindow::onStackedWidgetWidgetRemoved);

  // Search field: a pill-shaped QLineEdit living directly in the toolbar
  // (see the styling/positioning further down, where it replaces the old
  // "Filter" icon-button slot) rather than a hidden bar at the bottom of
  // the window that a toggle button had to reveal.
  filterBar = new QLineEdit(this);
  filterBar->setPlaceholderText(tr("Search"));
  filterBar->setClearButtonEnabled(true);
  connect(filterBar, &QLineEdit::textChanged, this,
          &MainWindow::onFilterStringChanged);

  // side pane
  ui.sidePane->setIconSize(
      QSize(settings.sidePaneIconSize(), settings.sidePaneIconSize()));
  ui.sidePane->applySettings(settings);
  ui.sidePane->setMode(settings.sidePaneMode());
  ui.sidePane->installEventFilter(this);
  connect(ui.sidePane, &Fm::SidePane::chdirRequested, this,
          &MainWindow::onSidePaneChdirRequested);
  connect(ui.sidePane, &Fm::SidePane::openFolderInNewWindowRequested, this,
          &MainWindow::onSidePaneOpenFolderInNewWindowRequested);
  connect(ui.sidePane, &Fm::SidePane::openFolderInNewTabRequested, this,
          &MainWindow::onSidePaneOpenFolderInNewTabRequested);
  connect(ui.sidePane, &Fm::SidePane::openFolderInTerminalRequested, this,
          &MainWindow::onSidePaneOpenFolderInTerminalRequested);
  connect(ui.sidePane, &Fm::SidePane::createNewFolderRequested, this,
          &MainWindow::onSidePaneCreateNewFolderRequested);
  connect(ui.sidePane, &Fm::SidePane::modeChanged, this,
          &MainWindow::onSidePaneModeChanged);

  ui.splitter->setHandleWidth(
      0); // probono: No handles between side bar and main window content

  // detect change of splitter position
  connect(ui.splitter, &QSplitter::splitterMoved, this,
          &MainWindow::onSplitterMoved);

  // path bar
  pathEntry = new Fm::PathEdit(this);
  connect(pathEntry, &Fm::PathEdit::returnPressed, this,
          &MainWindow::onPathEntryReturnPressed);
  pathEntryAction_ = ui.toolBar->insertWidget(ui.actionGo, pathEntry);

  // Finder-style vibrancy: only the sidebar is frosted (matching the
  // reference), not the whole window -- the content area stays fully
  // opaque. WA_TranslucentBackground is still needed on the whole native
  // surface (that's what gives it an alpha channel at all; it's also what
  // makes the rounded corners painted in paintEvent() actually transparent
  // outside the radius). paintEvent() paints the frosted translucent column
  // for the full sidebar strip (top to bottom, like Finder -- the sidebar
  // widget's own background is transparent, see sidepane.cpp) and an opaque
  // fill for everything right of it; updateSidebarBlurRegion() keeps KWin's
  // blur restricted to that same column. This is a no-op with no visual
  // effect if the user doesn't have a blur-behind effect (e.g. better-blur)
  // active.
  setAttribute(Qt::WA_TranslucentBackground);
  winId(); // realize the native window now so windowHandle() is valid
  updateSidebarBlurRegion();

  // macOS traffic lights, leftmost in the toolbar (the toolbar doubles as
  // the title bar now that the window is frameless).
  sidebarSpacer_ = new QWidget(ui.toolBar);
  QHBoxLayout* tlLayout = new QHBoxLayout(sidebarSpacer_);
  tlLayout->setContentsMargins(10, 0, 12, 0);
  tlLayout->setSpacing(8);
  // The button faces come straight from the pearOS-dark GTK theme's own
  // titlebutton assets (see resources.qrc) -- normal state is the plain
  // colored dot, hover shows the x/-/+ glyph, pressed shows the darker
  // "active" face -- so Filer's chrome is pixel-identical to GTK windows
  // decorated with the same theme.
  auto makeTrafficDot = [&](const QString& assetBase) {
      QPushButton* b = new QPushButton(sidebarSpacer_);
      b->setFixedSize(16, 16);
      b->setFlat(true);
      b->setFocusPolicy(Qt::NoFocus);
      b->setStyleSheet(QStringLiteral(
          "QPushButton { border: none; background: transparent;"
          " border-image: url(:/icons/titlebutton-%1.png) 0 0 0 0 stretch stretch; }"
          "QPushButton:hover { border-image: url(:/icons/titlebutton-%1-hover.png) 0 0 0 0 stretch stretch; }"
          "QPushButton:pressed { border-image: url(:/icons/titlebutton-%1-active.png) 0 0 0 0 stretch stretch; }").arg(assetBase));
      tlLayout->addWidget(b);
      return b;
  };
  QPushButton* tlClose = makeTrafficDot(QStringLiteral("close"));
  QPushButton* tlMin = makeTrafficDot(QStringLiteral("minimize"));
  QPushButton* tlMax = makeTrafficDot(QStringLiteral("maximize"));
  connect(tlClose, &QPushButton::clicked, this, &QWidget::close);
  connect(tlMin, &QPushButton::clicked, this, &QWidget::showMinimized);
  connect(tlMax, &QPushButton::clicked, this, [this]() {
      isMaximized() ? showNormal() : showMaximized();
  });
  tlLayout->addStretch();
  sidebarSpacer_->setFixedWidth(settings.splitterPos());

  ui.toolBar->insertWidget(ui.actionNewTab, sidebarSpacer_);
  ui.toolBar->removeAction(ui.actionGoUp);

  // Frameless window: the toolbar is the drag handle (see eventFilter()).
  ui.toolBar->installEventFilter(this);

  // Finder-like chrome: real Finder windows have no in-window menu bar at
  // all -- only the always-on-screen global menu bar (which filer-topbar's
  // org.kde.plasma.appmenu applet already provides for Filer's windows).
  // Keeping the QMenuBar object itself (just hidden) preserves every menu
  // action's keyboard shortcut and lets a global-menu export still find it.
  if(menuBar()) {
    QMenuBar* freshProbe = new QMenuBar();
    qDebug() << "GLOBALMENU-DEBUG: uiMenuBarNative=" << menuBar()->isNativeMenuBar()
             << "freshMenuBarNative=" << freshProbe->isNativeMenuBar()
             << "theme env=" << qgetenv("QT_QPA_PLATFORMTHEME");
    delete freshProbe;
    menuBar()->setVisible(false);
  }

  // Flatten the toolbar into the title bar (no frame/border, blends with the
  // window background) and turn the path field from a raw address-bar
  // QLineEdit into a quiet, pill-shaped "current folder" field, closer to
  // Finder's understated toolbar than a browser-style URL bar.
  // Taller bar with round (circular-hover) buttons, like the Tahoe
  // reference toolbar.
  updateToolbarStyleSheet();
  updateScrollBarStyleSheet();
  // Without this, QToolBar's native style can still paint its own opaque
  // panel underneath the QSS "background: transparent" rule above, hiding
  // MainWindow::paintEvent()'s wallpaper-tinted fill behind it.
  ui.toolBar->setAttribute(Qt::WA_StyledBackground, true);
  ui.toolBar->setAutoFillBackground(false);
  ui.statusbar->setStyleSheet(QStringLiteral(
      "QStatusBar { border: none; background: transparent; } QStatusBar::item { border: none; }"));
  // Without this, QStatusBar's native style can still paint its own opaque
  // panel underneath the QSS "background: transparent" rule above (same
  // issue as ui.toolBar, see its own WA_StyledBackground comment) -- that
  // opaque panel spans the statusbar's full width, including the strip
  // under the sidebar column, where it visibly clashes with the sidebar's
  // frosted fill (painted by MainWindow::paintEvent(), which already
  // extends the full window height) as a separate-looking band right at
  // the bottom of the sidebar.
  ui.statusbar->setAttribute(Qt::WA_StyledBackground, true);
  ui.statusbar->setAutoFillBackground(false);
  // The separator hairlines below the toolbar and above the breadcrumb bar
  // are painted in paintEvent() instead of as a QSS border, so they can be
  // clipped to stop at the sidebar column instead of running underneath it
  // (a QSS border on the toolbar/statusbar spans their full width, which
  // includes the sidebar-column strip).
  pathEntry->setStyleSheet(QStringLiteral(
      "QLineEdit { border: none; border-radius: 8px; padding: 4px 10px;"
      " background-color: rgba(127, 127, 127, 30); }"
      "QLineEdit:focus { background-color: rgba(127, 127, 127, 45); }"));

  // Finder shows the current folder's name as plain bold toolbar text, not
  // an editable address field -- the field only appears (Finder: Cmd+Shift+G
  // "Go to Folder"; here: Ctrl+L, already wired below) to type a path
  // directly, then reverts to the label once it loses focus.
  folderNameLabel_ = new QLabel(ui.toolBar);
  QFont boldFont = folderNameLabel_->font();
  boldFont.setBold(true);
  boldFont.setPointSize(boldFont.pointSize() + 1);
  folderNameLabel_->setFont(boldFont);
  folderNameLabelAction_ = ui.toolBar->insertWidget(pathEntryAction_, folderNameLabel_);
  pathEntryAction_->setVisible(false);
  pathEntry->installEventFilter(this);

  // Finder-style view-mode segmented control, pinned to the toolbar's right
  // edge by an expanding spacer (matching the reference layout: nav +
  // folder name on the left, view modes + search on the right). Reuses the
  // existing checkable/exclusive actionIconView/actionDetailedList/
  // actionThumbnailView (already grouped into a QActionGroup further down)
  // so the View menu and toolbar always agree. Only four modes remain --
  // Icons, Detailed List, Column, Gallery -- Compact View was removed.
  QWidget* toolbarSpacer = new QWidget(ui.toolBar);
  toolbarSpacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
  ui.toolBar->addWidget(toolbarSpacer);

  ui.actionIconView->setIcon(QIcon(QStringLiteral(":/icons/square-grid-2x2.svg")));
  ui.actionDetailedList->setIcon(QIcon(QStringLiteral(":/icons/list-bullet.svg")));
  ui.actionThumbnailView->setIcon(QIcon::fromTheme(QStringLiteral("view-preview")));

  // Column (Miller) and Gallery views (Finder's remaining two view modes).
  // Neither is one of Fm::FolderView's ViewModes -- TabPage swaps in its
  // own widget for each (see TabPage::setColumnMode()/setGalleryMode()) --
  // but they're presented in the same exclusive group so the segmented
  // control reads as one control.
  actionColumnView_ = new QAction(QIcon(QStringLiteral(":/icons/rectangle-split-3x1.svg")), tr("Column View"), this);
  actionColumnView_->setCheckable(true);
  connect(actionColumnView_, &QAction::triggered, this, [this]() {
      if(TabPage* page = currentPage())
          page->setColumnMode(true);
  });
  ui.menu_View->insertAction(ui.actionThumbnailView, actionColumnView_);

  actionGalleryView_ = new QAction(QIcon(QStringLiteral(":/icons/squares-below-rectangle.svg")), tr("Gallery View"), this);
  actionGalleryView_->setCheckable(true);
  connect(actionGalleryView_, &QAction::triggered, this, [this]() {
      if(TabPage* page = currentPage())
          page->setGalleryMode(true);
  });
  ui.menu_View->insertAction(ui.actionThumbnailView, actionGalleryView_);

  // One segmented control, one tooltip -- not four separate button
  // tooltips -- describing what the whole group does.
  viewModeGroup_ = createToolbarButtonGroup(
      {ui.actionIconView, ui.actionDetailedList, actionColumnView_, actionGalleryView_},
      tr("Show items as icons, in a list, in columns or in a gallery"));
  viewModeGroupAction_ = ui.toolBar->addWidget(viewModeGroup_);
  // A pill-shaped, segmented look: flat buttons that fill solid when
  // checked, like macOS's view-mode control -- the ":checked" rule itself
  // lives in updateToolbarStyleSheet() (see its doc comment for why it's
  // not appended here anymore).

  // Collapsed fallback (Settings::viewModeCollapseWidth()): a single round
  // button showing the gallery-view icon + a chevron composited into one
  // pixmap (there's no room for two separate icons in a 24px circle), with
  // a popup menu holding the same 4 checkable/exclusive actions --
  // reusing them directly (not copies) keeps their checked state in sync
  // with the full segmented control and the View menu automatically.
  viewModeCollapsedButton_ = new QToolButton(ui.toolBar);
  QSize viewModeCollapsedIconSize;
  {
    // Proportions tuned at a 16px icon size, scaled to whatever
    // kRoundToolbarButtonIconSize actually is so this composite grows/
    // shrinks along with the rest of the toolbar's buttons.
    qreal s = kRoundToolbarButtonIconSize / 16.0;
    QPixmap galleryPix = QIcon(QStringLiteral(":/icons/squares-below-rectangle.svg")).pixmap(qRound(14 * s), qRound(14 * s));
    QPixmap chevronPix = QIcon(QStringLiteral(":/icons/chevron-up-chevron-down.svg")).pixmap(qRound(8 * s), qRound(12 * s));
    viewModeCollapsedIconSize = QSize(qRound(24 * s), qRound(14 * s));
    QPixmap combined(viewModeCollapsedIconSize);
    combined.fill(Qt::transparent);
    QPainter cp(&combined);
    cp.drawPixmap(0, 0, galleryPix);
    cp.drawPixmap(qRound(16 * s), qRound(1 * s), chevronPix);
    cp.end();
    viewModeCollapsedButton_->setIcon(QIcon(combined));
  }
  viewModeCollapsedButton_->setIconSize(viewModeCollapsedIconSize);
  // Same reasoning as groupByGroup's kGroupByButtonSize below: this
  // button's icon is also a wider composite (gallery glyph + chevron),
  // which read as cramped filling the plain kRoundToolbarButtonSize circle
  // searchButton_ uses for a single square icon.
  const int kViewModeCollapsedButtonSize = kRoundToolbarButtonSize + 10;
  viewModeCollapsedButton_->setFixedSize(kViewModeCollapsedButtonSize, kViewModeCollapsedButtonSize);
  // Fixed size policy on top of setFixedSize(): when the toolbar's total
  // content is wider than the window (a narrow window, or the group-by/
  // search buttons appearing right as the segmented control collapses),
  // QToolBarLayout visibly compressed this button below its fixed size
  // regardless -- min==max from setFixedSize() alone wasn't a strong
  // enough signal for it. An explicit Fixed/Fixed policy is.
  viewModeCollapsedButton_->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  viewModeCollapsedButton_->setAutoRaise(true);
  viewModeCollapsedButton_->setAttribute(Qt::WA_TranslucentBackground);
  viewModeCollapsedButton_->setToolTip(tr("Show items as icons, in a list, in columns or in a gallery"));
  viewModeCollapsedButton_->installEventFilter(this);
  glassPanels_.append(viewModeCollapsedButton_);
  QMenu* viewModeMenu = new QMenu(this);
  viewModeMenu->addAction(ui.actionIconView);
  viewModeMenu->addAction(ui.actionDetailedList);
  viewModeMenu->addAction(actionColumnView_);
  viewModeMenu->addAction(actionGalleryView_);
  viewModeCollapsedButton_->setMenu(viewModeMenu);
  viewModeCollapsedButton_->setPopupMode(QToolButton::InstantPopup);
  viewModeCollapsedButton_->setStyleSheet(QStringLiteral("QToolButton::menu-indicator { image: none; width: 0px; height: 0px; }"));
  viewModeCollapsedButtonAction_ = ui.toolBar->addWidget(viewModeCollapsedButton_);
  updateViewModeCollapseMode();

  // No addSeparator() here (or before Share/Tags/More and Search, below)
  // anymore -- a QToolBar separator reserves its own fixed, unconfigurable
  // width on top of whatever a group's own MarginLeft/Right says, so no
  // amount of shrinking a group's margins could actually bring adjacent
  // groups closer together than the separator's own gap allowed. Each
  // group's MarginLeft/Right (applyToolbarGroupSettings()) is the only
  // thing controlling the space between groups now.

  // Group By: same structure as Finder's own "Group By" menu (None, then a
  // separator, then the actual grouping criteria). Sits alone in its own
  // pill between the view-mode segmented control and the Share/Tags/More
  // group. NOTE: this only wires up the menu/checked-state UI for now --
  // there's no grouped-section rendering in FolderView/FolderModel yet to
  // actually reorganize items by these criteria, so selecting anything
  // other than None doesn't yet change what's shown.
  // Composited icon (grid glyph + a small chevron-down to its right,
  // signaling "this opens a menu") -- same technique as
  // viewModeCollapsedButton_'s combined icon above. The button itself
  // stays kRoundToolbarButtonSize/round; only the icon content inside it
  // is wider than tall.
  QIcon groupByIcon;
  QSize groupByIconSize;
  {
    // Same 16px-tuned-then-scaled approach as viewModeCollapsedButton_'s
    // composite above.
    qreal s = kRoundToolbarButtonIconSize / 16.0;
    // Grid glyph and chevron both scaled down from their previous 14px/8x5px
    // (~0.8x) -- at full size they filled the composite canvas edge-to-edge
    // and read as oversized/cramped inside the round button compared to the
    // other toolbar icons, which all have real breathing room around them.
    QPixmap gridPix = QIcon(QStringLiteral(":/icons/square-grid-3x1-below-line-grid-1x2.svg")).pixmap(qRound(11 * s), qRound(11 * s));
    QPixmap chevronPix = QIcon(QStringLiteral(":/icons/chevron-down.svg")).pixmap(qRound(6 * s), qRound(4 * s));
    groupByIconSize = QSize(qRound(24 * s), qRound(14 * s));
    QPixmap combined(groupByIconSize);
    combined.fill(Qt::transparent);
    QPainter cp(&combined);
    cp.drawPixmap(qRound(1 * s), qRound(1.5 * s), gridPix);
    cp.drawPixmap(qRound(16 * s), qRound(5 * s), chevronPix);
    cp.end();
    groupByIcon = QIcon(combined);
  }
  QAction* groupByAction = new QAction(groupByIcon, tr("Group By"), this);
  groupByAction->setToolTip(tr("Change the icon grouping"));
  QMenu* groupByMenu = new QMenu(this);
  QActionGroup* groupByActionGroup = new QActionGroup(groupByMenu);
  groupByActionGroup->setExclusive(true);
  auto addGroupByOption = [&](const QString& label) {
    QAction* a = groupByMenu->addAction(label);
    a->setCheckable(true);
    groupByActionGroup->addAction(a);
    return a;
  };
  addGroupByOption(tr("None"))->setChecked(true);
  groupByMenu->addSeparator();
  addGroupByOption(tr("Name"));
  addGroupByOption(tr("Kind"));
  addGroupByOption(tr("Application"));
  addGroupByOption(tr("Date Last Opened"));
  addGroupByOption(tr("Date Added"));
  addGroupByOption(tr("Date Modified"));
  addGroupByOption(tr("Date Created"));
  addGroupByOption(tr("Size"));
  addGroupByOption(tr("Tags"));
  groupByAction->setMenu(groupByMenu);
  groupByGroup_ = createToolbarButtonGroup({groupByAction}, tr("Change the icon grouping"));
  ui.toolBar->addWidget(groupByGroup_);
  const QList<QToolButton*> groupByButtons = groupByGroup_->findChildren<QToolButton*>();
  if(!groupByButtons.isEmpty()) {
    groupByButtons.first()->setPopupMode(QToolButton::InstantPopup);
    // The button's own QSS padding (6px, from "QToolBar QToolButton {
    // padding: 6px }") is meant for buttons living inside the pill-shaped
    // *segmented* groups, sized around that padding by construction. This
    // group is different -- a single round button whose surrounding
    // circle already comes from the group's own fixed kRoundToolbarButtonSize
    // (below) -- so that same padding, stacked on top of
    // createToolbarButtonGroup()'s own 4/3px layout margins, squeezed the
    // icon into a much smaller area than searchButton_/
    // viewModeCollapsedButton_ get (they have neither). Zeroing both here
    // lets the icon use the full circle, matching those two.
    groupByButtons.first()->setStyleSheet(QStringLiteral(
        "QToolButton { padding: 0px; } QToolButton::menu-indicator { image: none; width: 0px; height: 0px; }"));
    // groupByIcon is a wider-than-tall composite (grid glyph + chevron, see
    // above), so its icon size isn't the square kRoundToolbarButtonIconSize
    // used elsewhere -- only the button's own outer size (below) needs to
    // match those for the round shape to read consistently.
    groupByButtons.first()->setIconSize(groupByIconSize);
  }
  if(QHBoxLayout* groupByLayout = qobject_cast<QHBoxLayout*>(groupByGroup_->layout()))
    groupByLayout->setContentsMargins(0, 0, 0, 0);
  // Bigger glass circle than the other two round buttons -- its icon is a
  // wider composite (grid + chevron) that read as cramped filling the same
  // kRoundToolbarButtonSize circle those use, so this one gets extra room
  // around it instead. (Settings::iconGroupingGroupHeight()/Width(), applied
  // in applyToolbarGroupSettings(), can override this default outright.)
  const int kGroupByButtonSize = kRoundToolbarButtonSize + 10;
  groupByGroup_->setFixedSize(kGroupByButtonSize, kGroupByButtonSize);
  // See the matching comment on viewModeCollapsedButton_'s size policy.
  groupByGroup_->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

  // Share: Finder's share-sheet has no equivalent infrastructure here, so
  // this is the nearest useful substitute -- a menu of actions on the
  // current selection, reusing the same QActions the Edit menu already has.
  QAction* shareAction = new QAction(QIcon(QStringLiteral(":/icons/square-and-arrow-up.svg")), tr("Share"), this);
  QMenu* shareMenu = new QMenu(this);
  shareMenu->addAction(ui.actionCopy);
  shareMenu->addAction(ui.actionDuplicate);
  shareMenu->addSeparator();
  shareMenu->addAction(ui.actionOpenWith);
  shareAction->setMenu(shareMenu);

  // Tag: reuses the same fixed color-swatch palette and xattr-backed
  // read/write helpers (Fm::tagPalette()/Fm::setFileTag()/Fm::clearFileTag())
  // that the desktop's own right-click "Tags" submenu uses -- see
  // filemenu.cpp and tags.h. Applies to the current tab's selected file(s).
  QAction* tagAction = new QAction(QIcon(QStringLiteral(":/icons/tag-outline.svg")), tr("Tags"), this);
  QMenu* tagMenu = new QMenu(this);
  for(const Fm::TagInfo& t : Fm::tagPalette()) {
      QAction* swatchAction = tagMenu->addAction(t.name);
      QPixmap swatch(12, 12);
      swatch.fill(t.color);
      swatchAction->setIcon(QIcon(swatch));
      QString tagName = t.name;
      connect(swatchAction, &QAction::triggered, this, [this, tagName]() {
          TabPage* page = currentPage();
          if(!page)
              return;
          FmFileInfoList* files = page->selectedFiles();
          if(!files)
              return;
          for(GList* l = fm_file_info_list_peek_head_link(files); l; l = l->next) {
              QString path = QString::fromUtf8(fm_path_to_str(fm_file_info_get_path(FM_FILE_INFO(l->data))));
              Fm::setFileTag(path, tagName);
          }
      });
  }
  tagMenu->addSeparator();
  QAction* noTagAction = tagMenu->addAction(tr("None"));
  connect(noTagAction, &QAction::triggered, this, [this]() {
      TabPage* page = currentPage();
      if(!page)
          return;
      FmFileInfoList* files = page->selectedFiles();
      if(!files)
          return;
      for(GList* l = fm_file_info_list_peek_head_link(files); l; l = l->next) {
          QString path = QString::fromUtf8(fm_path_to_str(fm_file_info_get_path(FM_FILE_INFO(l->data))));
          Fm::clearFileTag(path);
      }
  });
  tagAction->setMenu(tagMenu);

  // More (...): the odds and ends that don't have their own icon slot --
  // reuses existing actions/menus (Show Hidden, the Sorting submenu, Get
  // Info) rather than duplicating their logic.
  QAction* moreAction = new QAction(QIcon(QStringLiteral(":/icons/ellipsis.svg")), tr("More"), this);
  QMenu* moreMenu = new QMenu(this);
  moreMenu->addAction(ui.actionShowHidden);
  moreMenu->addMenu(ui.menuSorting);
  moreMenu->addSeparator();
  moreMenu->addAction(ui.actionFileProperties);
  moreAction->setMenu(moreMenu);

  // Share/Tags/More act on the same current-selection context, so they're
  // grouped into one pill-shaped control like the back/forward and
  // view-mode groups.
  shareTagsMoreGroup_ =
      createToolbarButtonGroup({shareAction, tagAction, moreAction});
  ui.toolBar->addWidget(shareTagsMoreGroup_);
  const QList<QToolButton*> shareTagsMoreButtons = shareTagsMoreGroup_->findChildren<QToolButton*>();
  for(QToolButton* button : shareTagsMoreButtons) {
      button->setPopupMode(QToolButton::InstantPopup);
      // InstantPopup still draws a small down-arrow indicator by default
      // for a button with a menu -- not wanted here, these already read as
      // buttons via their own icon.
      button->setStyleSheet(QStringLiteral("QToolButton::menu-indicator { image: none; width: 0px; height: 0px; }"));
  }

  // Search: a small pill-shaped field sitting directly in the toolbar,
  // Finder/Spotlight-style, instead of a button that reveals a bar hidden
  // at the bottom of the window.
  filterBar->addAction(QIcon(QStringLiteral(":/icons/magnifyingglass.svg")), QLineEdit::LeadingPosition);
  // 24px height, fully pill-shaped (radius = height/2), 0px/8px padding:
  // uikit design reference, ~/Desktop/uikit-svg/search_fields/search_field.css.
  filterBar->setFixedHeight(24);
  filterBar->setMinimumWidth(140);
  filterBar->setMaximumWidth(220);
  // "Liquid Glass" backdrop (see paintLiquidGlassPanel()) instead of the
  // previous flat rgba(127,127,127,30) pill fill -- filterBar itself goes
  // transparent and sits in a holder that paints the glass behind it, same
  // pattern as the back/forward, view-mode, and share/tag/more groups
  // (see createToolbarButtonGroup()).
  filterBar->setStyleSheet(QStringLiteral(
      "QLineEdit { border: none; border-radius: 12px; padding: 0px 8px; background-color: transparent; }"));
  filterBar->installEventFilter(this); // QEvent::FocusOut -- see updateSearchBarMode()
  searchHolder_ = new QWidget(ui.toolBar);
  QHBoxLayout* searchHolderLayout = new QHBoxLayout(searchHolder_);
  searchHolderLayout->setContentsMargins(0, 0, 0, 0);
  searchHolderLayout->addWidget(filterBar);
  searchHolder_->setAttribute(Qt::WA_TranslucentBackground);
  searchHolder_->installEventFilter(this);
  glassPanels_.append(searchHolder_);
  searchHolderAction_ = ui.toolBar->addWidget(searchHolder_);

  // Collapsed state: a lone glass-circle search button, shown instead of
  // searchHolder_ below Settings::searchBarExpandWidth() until clicked.
  // Same slot in the toolbar layout as searchHolder_ (added right after
  // it) -- toggling visibility between the two keeps it from jumping to a
  // different position.
  searchButton_ = new QToolButton(ui.toolBar);
  searchButton_->setIcon(QIcon(QStringLiteral(":/icons/magnifyingglass.svg")));
  searchButton_->setIconSize(QSize(kRoundToolbarButtonIconSize, kRoundToolbarButtonIconSize));
  // +10, matching groupByGroup/viewModeCollapsedButton_'s own circle size
  // (see their comments) -- all three round buttons read as the same size
  // now, regardless of each one's own icon shape/aspect.
  const int kSearchButtonSize = kRoundToolbarButtonSize + 10;
  searchButton_->setFixedSize(kSearchButtonSize, kSearchButtonSize);
  // See the matching comment on viewModeCollapsedButton_'s size policy.
  searchButton_->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  searchButton_->setAutoRaise(true);
  searchButton_->setAttribute(Qt::WA_TranslucentBackground);
  searchButton_->installEventFilter(this);
  glassPanels_.append(searchButton_);
  // Thin margin-only wrapper (see applyToolbarGroupSettings()/Settings::
  // searchButtonGroupMarginLeft()/Right()) -- searchButton_ itself keeps its
  // own fixed size, glass paint, and event filter untouched; this just adds
  // configurable breathing room around it, the same way the other four
  // groups already get theirs from createToolbarButtonGroup()'s layout.
  searchButtonGroup_ = new QWidget(ui.toolBar);
  QHBoxLayout* searchButtonGroupLayout = new QHBoxLayout(searchButtonGroup_);
  searchButtonGroupLayout->setContentsMargins(0, 0, 0, 0);
  searchButtonGroupLayout->setSpacing(0);
  searchButtonGroupLayout->addWidget(searchButton_);
  searchButtonGroup_->setAttribute(Qt::WA_TranslucentBackground);
  searchButtonAction_ = ui.toolBar->addWidget(searchButtonGroup_);
  connect(searchButton_, &QToolButton::clicked, this, [this]() {
    searchManuallyExpanded_ = true;
    // Frees up the space the segmented view-mode control was holding
    // before the search field itself tries to grow into it -- see the doc
    // comment on updateViewModeCollapseMode().
    updateViewModeCollapseMode();
    searchButtonAction_->setVisible(false);
    searchHolder_->setMinimumWidth(0);
    searchHolder_->setMaximumWidth(QWIDGETSIZE_MAX);
    searchHolderAction_->setVisible(true);
    animateSearchBar(searchButton_->width(), 180, false);
    filterBar->setFocus();
    filterBar->selectAll();
  });

  searchAnim_ = new QVariantAnimation(this);
  searchAnim_->setDuration(280);
  updateSearchBarMode();

  // All five toolbar groups now exist -- apply their margin/height/width
  // settings plus the two global icon-size settings once, here (see
  // updateFromSettings() for the live-reload counterpart).
  applyToolbarGroupSettings(settings);

  // Breadcrumb trail (volume name, then each path segment), replacing the
  // old free-space "available" label -- sits on the left of the status
  // bar, right where the content area begins next to the sidebar (same x
  // as the toolbar's back/forward group above it). breadcrumbSpacer_'s
  // width is kept in sync with the sidebar's width by
  // updateSidebarSpacerWidth(), same as sidebarSpacer_ in the toolbar.
  breadcrumbSpacer_ = new QWidget(ui.statusbar);
  ui.statusbar->addWidget(breadcrumbSpacer_);
  breadcrumbBar_ = new QWidget(ui.statusbar);
  breadcrumbBar_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
  breadcrumbLayout_ = new QHBoxLayout(breadcrumbBar_);
  breadcrumbLayout_->setContentsMargins(0, 0, 0, 0);
  breadcrumbLayout_->setSpacing(2);
  // Nonzero stretch so this claims all leftover status-bar width instead of
  // QStatusBar sizing it to (and, when tight, shrinking it below) its
  // sizeHint(), which was eliding segment text ("ho...", "alxb4...").
  ui.statusbar->addWidget(breadcrumbBar_, 1);

  // Unified/vibrancy toolbar: pull ui.toolBar out of QMainWindow's managed
  // top toolbar area (which reserves a non-overlapping strip for it) and
  // float it as a raised overlay instead, so the folder view's content
  // extends up underneath it and is visible (blurred) through it while
  // scrolling -- see the QEvent::Paint handling for ui.toolBar in
  // eventFilter(). removeToolBar() frees the reserved
  // strip, which centralWidget()/ui.splitter automatically reclaim.
  removeToolBar(ui.toolBar);
  ui.toolBar->setParent(this);
  ui.toolBar->show();
  ui.toolBar->raise();
  updateToolbarOverlayGeometry();

  // Same treatment, bottom edge -- see the long comment on
  // updateStatusBarOverlayGeometry() in mainwindow.h for why the real
  // ui.statusbar isn't reused as the overlay itself the way ui.toolBar is.
  ui.statusbar->setFixedHeight(0);
  ui.statusbar->removeWidget(breadcrumbSpacer_);
  ui.statusbar->removeWidget(breadcrumbBar_);
  statusBarOverlay_ = new QWidget(this);
  statusBarOverlay_->installEventFilter(this);
  QHBoxLayout* statusBarLayout = new QHBoxLayout(statusBarOverlay_);
  statusBarLayout->setContentsMargins(0, 0, 0, 0);
  statusBarLayout->setSpacing(0);
  // setParent() implicitly hides a widget that was previously visible --
  // both were visible under ui.statusbar, so both need an explicit show()
  // again now, or they -- and the whole breadcrumb trail with them --
  // silently never render under their new parent despite the layout
  // still sizing/positioning them correctly.
  breadcrumbSpacer_->setParent(statusBarOverlay_);
  breadcrumbSpacer_->show();
  statusBarLayout->addWidget(breadcrumbSpacer_);
  breadcrumbBar_->setParent(statusBarOverlay_);
  breadcrumbBar_->show();
  statusBarLayout->addWidget(breadcrumbBar_, 1);
  statusBarOverlay_->show();
  statusBarOverlay_->raise();
  updateStatusBarOverlayGeometry();

  // See the declaration in mainwindow.h: a dedicated widget so the top
  // blur band can be stacked above ui.splitter's items but below
  // ui.toolBar's own children, instead of painted in MainWindow::paintEvent()
  // (below everything).
  topBlurOverlay_ = new QWidget(this);
  topBlurOverlay_->setAttribute(Qt::WA_TranslucentBackground);
  topBlurOverlay_->setAttribute(Qt::WA_TransparentForMouseEvents);
  topBlurOverlay_->installEventFilter(this);
  topBlurOverlay_->show();
  topBlurOverlay_->raise();
  ui.toolBar->raise(); // re-assert on top of topBlurOverlay_, not just ui.splitter
  updateTopBlurOverlayGeometry();

  // See the declaration in mainwindow.h: kept raised above every other
  // child so the window's own outline stroke always wins, regardless of
  // what else just got raised (ui.toolBar above, statusBarOverlay_, ...).
  windowStrokeOverlay_ = new QWidget(this);
  windowStrokeOverlay_->setAttribute(Qt::WA_TranslucentBackground);
  windowStrokeOverlay_->setAttribute(Qt::WA_TransparentForMouseEvents);
  windowStrokeOverlay_->installEventFilter(this);
  windowStrokeOverlay_->show();
  updateWindowStrokeOverlayGeometry();

  toolbarBlurLastSidebarView_ = nullptr;
  toolbarBlurTimer_ = new QTimer(this);
  connect(toolbarBlurTimer_, &QTimer::timeout, this, &MainWindow::applyScrollInsets);
  toolbarBlurTimer_->start(80);

  // Toolbar/titlebar hairline: invisible at rest, fades in on a 1.5s
  // titlebar hover or on scrolling the current folder view (see
  // eventFilter()'s ui.toolBar Enter/Leave handling and
  // applyScrollInsets()'s scroll-value polling), fades back out once
  // neither is true.
  toolbarHairlineAnim_ = new QVariantAnimation(this);
  toolbarHairlineAnim_->setDuration(200);
  connect(toolbarHairlineAnim_, &QVariantAnimation::valueChanged, this, [this](const QVariant& value) {
    toolbarHairlineOpacity_ = value.toReal();
    update();
  });
  toolbarHairlineHoverTimer_ = new QTimer(this);
  toolbarHairlineHoverTimer_->setSingleShot(true);
  toolbarHairlineHoverTimer_->setInterval(1500);
  connect(toolbarHairlineHoverTimer_, &QTimer::timeout, this, [this]() { animateToolbarHairlineTo(1.0); });
  toolbarHairlineIdleTimer_ = new QTimer(this);
  toolbarHairlineIdleTimer_->setSingleShot(true);
  toolbarHairlineIdleTimer_->setInterval(1000);
  connect(toolbarHairlineIdleTimer_, &QTimer::timeout, this, [this]() { animateToolbarHairlineTo(0.0); });

  // setup the splitter
  ui.splitter->setStretchFactor(1, 1); // only the right pane can be stretched
  QList<int> sizes;
  sizes.append(settings.splitterPos());
  sizes.append(300);
  ui.splitter->setSizes(sizes);

  // load bookmark menu
  bookmarks = fm_bookmarks_dup();
  g_signal_connect(bookmarks, "changed", G_CALLBACK(onBookmarksChanged), this);
  loadBookmarksMenu();

  // Fix the menu groups which is not done by Qt designer
  // To my suprise, this was supported in Qt designer 3 :-(
  QActionGroup *group = new QActionGroup(ui.menu_View);
  group->setExclusive(true);
  group->addAction(ui.actionIconView);
  group->addAction(ui.actionThumbnailView);
  group->addAction(ui.actionDetailedList);
  group->addAction(actionColumnView_);
  group->addAction(actionGalleryView_);

  group = new QActionGroup(ui.menuSorting);
  group->setExclusive(true);
  group->addAction(ui.actionByFileName);
  group->addAction(ui.actionByMTime);
  group->addAction(ui.actionByFileSize);
  group->addAction(ui.actionByFileType);
  group->addAction(ui.actionByOwner);

  group = new QActionGroup(ui.menuSorting);
  group->setExclusive(true);
  group->addAction(ui.actionAscending);
  group->addAction(ui.actionDescending);

  // create shortcuts
  QShortcut *shortcut;
  shortcut = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_L), this);
  connect(shortcut, &QShortcut::activated, this, [this]() {
      folderNameLabelAction_->setVisible(false);
      pathEntryAction_->setVisible(true);
      pathEntry->setFocus();
      pathEntry->selectAll();
  });

  shortcut = new QShortcut(Qt::ALT + Qt::Key_D, this);
  connect(shortcut, &QShortcut::activated, this, [this]() {
      folderNameLabelAction_->setVisible(false);
      pathEntryAction_->setVisible(true);
      pathEntry->setFocus();
      pathEntry->selectAll();
  });

  shortcut = new QShortcut(Qt::CTRL + Qt::Key_Tab, this);
  connect(shortcut, &QShortcut::activated, this,
          &MainWindow::onShortcutNextTab);

  shortcut = new QShortcut(Qt::CTRL + Qt::SHIFT + Qt::Key_Tab, this);
  connect(shortcut, &QShortcut::activated, this,
          &MainWindow::onShortcutPrevTab);

  int i;
  for (i = 0; i < 10; ++i) {
    shortcut = new QShortcut(QKeySequence(Qt::ALT + Qt::Key_0 + i), this);
    connect(shortcut, &QShortcut::activated, this,
            &MainWindow::onShortcutJumpToTab);

    shortcut = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_0 + i), this);
    connect(shortcut, &QShortcut::activated, this,
            &MainWindow::onShortcutJumpToTab);
  }

  shortcut = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_Down),
                           this); // pronono: open
  connect(shortcut, &QShortcut::activated, this,
          &MainWindow::on_actionOpen_triggered); // probono

  shortcut = new QShortcut(QKeySequence(Qt::ALT + Qt::CTRL + Qt::Key_Down),
                           this); // pronono: open
  connect(shortcut, &QShortcut::activated, this,
          &MainWindow::on_actionOpenWith_triggered); // probono

  shortcut = new QShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_Down),
                           this); // pronono: open and close current window
  connect(shortcut, &QShortcut::activated, this,
          &MainWindow::on_actionOpenAndCloseCurrentWindow_triggered); // probono

  // Deliberately no shortcut on the plain Delete key: trashing is
  // Cmd+Backspace (actionDelete in main-win.ui) and permanent deletion is
  // Cmd+Option+Backspace, macOS-style (Cmd = Meta, Option = Alt).
  shortcut =
      new QShortcut(QKeySequence(Qt::META + Qt::ALT + Qt::Key_Backspace),
                    this); // force delete
  connect(shortcut, &QShortcut::activated, this,
          &MainWindow::on_actionDeleteWithoutTrash_triggered);

  // The menu bar is hidden (global menu via filer-topbar), and its actions'
  // shortcuts do not fire in this window -- register them explicitly, the
  // same way DesktopWindow gets them via addActions().
  shortcut = new QShortcut(QKeySequence(Qt::META + Qt::Key_Backspace),
                           this); // move to trash
  connect(shortcut, &QShortcut::activated, this,
          &MainWindow::on_actionDelete_triggered);

  shortcut = new QShortcut(QKeySequence(Qt::META + Qt::SHIFT + Qt::Key_Backspace),
                           this); // empty trash
  connect(shortcut, &QShortcut::activated, this,
          &MainWindow::on_actionEmptyTrash_triggered);

  // Deliberately no separate QShortcut for Qt::Key_Space here: Space is
  // already handled by FolderViewListView::keyPressEvent() (folderview.cpp),
  // which emits quickLookRequested() -> on_actionQuickLook_triggered() below.
  // A second, redundant WindowShortcut on the same key fired for the same
  // keypress (QShortcut's default WindowShortcut context matches as soon as
  // any child of this window has focus, not just this widget itself),
  // causing Quick Look to open-then-immediately-close (or close-then-reopen)
  // on a single Space press -- see the identical fix/comment in
  // DesktopWindow's constructor (desktopwindow.cpp) for the first place this
  // was diagnosed; this window had the same bug and was just never updated
  // to match.

  shortcut = new QShortcut(QKeySequence(Qt::Key_Return), this); // rename shortcut
  connect(shortcut, &QShortcut::activated, this,
          &MainWindow::on_actionRename_triggered);

  shortcut = new QShortcut(QKeySequence(Qt::Key_Enter), this); // rename shortcut
  connect(shortcut, &QShortcut::activated, this,
          &MainWindow::on_actionRename_triggered);

  if (QToolButton *clearButton = filterBar->findChild<QToolButton *>()) {
    clearButton->setToolTip(tr("Clear text (Ctrl+K)"));
    shortcut = new QShortcut(Qt::CTRL + Qt::Key_K, this);
    connect(shortcut, &QShortcut::activated, filterBar, &QLineEdit::clear);
  }

  if (path)
    addTab(path);

  // Will be reflected in menubar when window is opened next time;
  // hence also treating this in MainWindow::updateFromSettings for immediate
  // effect
  ui.actionNewWin->setVisible(!settings.spatialMode()); // probono
  ui.actionGoUpAndCloseCurrentWindow->setVisible(
      settings.spatialMode()); // probono

  // size from spatial mode or from settings
  if (settings.spatialMode()) {

    // hide the things we don't want in spatial mode
    ui.tabBar->hide();
    ui.sidePane->hide();
    ui.toolBar->hide();
    ui.frame->layout()->setContentsMargins(0, 0, 0, 0);

    // Set the window position and size
    MetaData metaData(fm_path_to_str(path));
    int x, y, width, height;
    bool ok;
    x = metaData.getWindowOriginX(ok);
    if (ok)
      y = metaData.getWindowOriginY(ok);
    if (ok)
      width = metaData.getWindowWidth(ok);
    if (ok)
      height = metaData.getWindowHeight(ok);
    if (ok)
      setGeometry(x, y, width, height);

    // Set the window view mode
    MetaData::FolderView view = metaData.getWindowView(ok);
    if (ok) {
      switch (view) {
      case MetaData::Icons:
        ui.actionIconView->trigger();
        break;
      case MetaData::Compact:
        // Compact View was removed; fall back to Detailed List.
        ui.actionDetailedList->trigger();
        break;
      case MetaData::List:
        ui.actionDetailedList->trigger();
        break;
      case MetaData::Thumbnail:
        ui.actionThumbnailView->trigger();
        break;
      case MetaData::Columns:
        // not implemented
        ui.actionDetailedList->trigger();
        break;
      }
    }

    // Set the window sort item
    MetaData::SortItem sortItem = metaData.getWindowSortItem(ok);
    if (ok) {
      switch (sortItem) {
      case MetaData::FileName:
        ui.actionByFileName->trigger();
        break;
      case MetaData::FileType:
        ui.actionByFileType->trigger();
        break;
      case MetaData::FileSize:
        ui.actionByFileSize->trigger();
        break;
      case MetaData::ModifiedTime:
        ui.actionByMTime->trigger();
        break;
      case MetaData::Owner:
        ui.actionByOwner->trigger();
        break;
      }
    }

    // Set the window sort order
    MetaData::SortOrder sortOrder = metaData.getWindowSortOrder(ok);
    if (ok) {
      switch (sortOrder) {
      case MetaData::Ascending:
        ui.actionAscending->trigger();
        break;
      case MetaData::Descending:
        ui.actionDescending->trigger();
        break;
      }
    }

    // Set the window sort case sensitivity
    MetaData::SortCase sortCase = metaData.getWindowSortCase(ok);
    if (ok) {
      switch (sortCase) {
      case MetaData::CaseSensitive:
        ui.actionCaseSensitive->setChecked(true);
        break;
      case MetaData::NotCaseSensitive:
        ui.actionCaseSensitive->setChecked(false);
        break;
      }
    }

    // Set the window 'folder first' sort option
    MetaData::SortFolderFirst folderFirst =
        metaData.getWindowSortFolderFirst(ok);
    if (ok) {
      switch (folderFirst) {
      case MetaData::FoldersFirst:
        ui.actionFolderFirst->setChecked(true);
        break;
      case MetaData::NotFoldersFirst:
        ui.actionFolderFirst->setChecked(false);
        break;
      }
    }

  } else if (settings.rememberWindowSize()) {
    resize(settings.windowWidth(), settings.windowHeight());
    if (settings.windowMaximized())
      setWindowState(windowState() | Qt::WindowMaximized);
  }

  // addTab() above already started the folder loading that populates the
  // view -- for a small/fast directory, that can finish (and lay icons out)
  // before the resize()/setGeometry() calls just above ever ran, so it's
  // laid out for whatever narrow default width the window started at. Icon
  // mode's Free movement + Fixed resize mode (see FolderView::setViewMode())
  // means it won't reflow itself once the window reaches its real size, so
  // it's stuck looking like a single stacked column. A relayout now, with
  // the size finally settled, re-flows it into a proper grid.
  if (TabPage *page = currentPage()) {
    if (QAbstractItemView *view = page->folderView()->childView())
      view->doItemsLayout();
  }

  connect(&WindowRegistry::instance(), &WindowRegistry::raiseWindow, this,
          &MainWindow::onRaiseWindow);
  connect(&WindowRegistry::instance(),
          &WindowRegistry::raiseWindowAndSelectItems, this,
          &MainWindow::onRaiseWindowAndSelectItems);
  connect(&WindowRegistry::instance(), &WindowRegistry::closeWindow, this,
          &MainWindow::onCloseWindow);

  if (QX11Info::isPlatformX11()) {
    const char *atomName = "_FILER_PATH";
    const char *atomValue = fm_path_to_str(path);
    WId x11Window = this->winId();
    xcb_connection_t *connection = QX11Info::connection();
    xcb_intern_atom_cookie_t cookie =
        xcb_intern_atom(connection, 1, strlen(atomName), atomName);
    xcb_intern_atom_reply_t *reply =
        xcb_intern_atom_reply(connection, cookie, NULL);
    if (reply) {
      xcb_atom_t atom = reply->atom;
      xcb_change_property(connection, XCB_PROP_MODE_REPLACE, x11Window, atom,
                          XCB_ATOM_STRING, 8, strlen(atomValue), atomValue);
      xcb_flush(connection);
    }
  }
  updateSidebarSpacerWidth();
}

MainWindow::~MainWindow() {
  // update registry
  TabPage *page = currentPage();
  if (bookmarks)
    g_object_unref(bookmarks);
}

void MainWindow::chdir(FmPath *path) {
  if (isSpatialMode()) {
    if (!WindowRegistry::instance().checkPathAndRaise(fm_path_to_str(path))) {
      (new MainWindow(path))->show();
    }
  } else {
    TabPage *page = currentPage();

    if (page) {
      filterBar->clear();
      page->chdir(path, true);
      updateUIForCurrentPage();
    }
  }
}

// add a new tab
void MainWindow::addTab(FmPath *path) {
  Settings &settings = static_cast<Application *>(qApp)->settings();

  TabPage *newPage = new TabPage(path, this);
  newPage->setFileLauncher(&fileLauncher_);
  int index = ui.stackedWidget->addWidget(newPage);
  connect(newPage, &TabPage::titleChanged, this,
          &MainWindow::onTabPageTitleChanged);
  connect(newPage, &TabPage::statusChanged, this,
          &MainWindow::onTabPageStatusChanged);
  connect(newPage, &TabPage::openDirRequested, this,
          &MainWindow::onTabPageOpenDirRequested);
  connect(newPage, &TabPage::sortFilterChanged, this,
          &MainWindow::onTabPageSortFilterChanged);
  connect(newPage->folderView(), &Fm::FolderView::quickLookRequested, this,
          &MainWindow::on_actionQuickLook_triggered);
  // Auto-update Quick Look when selection changes while it's open
  connect(newPage->folderView(), &Fm::FolderView::selChanged, this, [this](int nSel) {
      // Selection changed while Quick Look happens to be open: just update
      // the preview to the new file, never close -- this is not a user
      // request to dismiss Quick Look (see showOrRefreshQuickLook()).
      if (QuickLookDialog::activeInstance() && nSel > 0)
          showOrRefreshQuickLook(false);
  });
  connect(newPage, &TabPage::backwardRequested, this,
          &MainWindow::on_actionGoBack_triggered);
  connect(newPage, &TabPage::forwardRequested, this,
          &MainWindow::on_actionGoForward_triggered);

  ui.tabBar->insertTab(index, newPage->title());

  if (!settings.alwaysShowTabs()) {
    ui.tabBar->setVisible(ui.tabBar->count() > 1);
    // Tab bar visibility just changed -- it shares its top margin with the
    // toolbar-height reservation (see updateToolbarOverlayGeometry()), so
    // that reservation needs recomputing now too.
    updateToolbarOverlayGeometry();
  }

  if (QX11Info::isPlatformX11()) {
    const char *atomName = "_FILER_PATH";
    const char *atomValue = fm_path_to_str(path);
    WId x11Window = this->winId();
    xcb_connection_t *connection = QX11Info::connection();
    xcb_intern_atom_cookie_t cookie =
        xcb_intern_atom(connection, 1, strlen(atomName), atomName);
    xcb_intern_atom_reply_t *reply =
        xcb_intern_atom_reply(connection, cookie, NULL);
    if (reply) {
      xcb_atom_t atom = reply->atom;
      xcb_change_property(connection, XCB_PROP_MODE_REPLACE, x11Window, atom,
                          XCB_ATOM_STRING, 8, strlen(atomValue), atomValue);
      xcb_flush(connection);
      free(reply);
    }
  }
}

void MainWindow::addWindow(FmPath *path) { (new MainWindow(path))->show(); }

void MainWindow::onPathEntryReturnPressed() {
  QString text = pathEntry->text();
  QByteArray utext = text.toUtf8();
  FmPath *path = fm_path_new_for_display_name(utext);
  chdir(path);
  fm_path_unref(path);
  // Match Finder: the path field is a transient "Go to Folder" overlay, not
  // a permanent address bar -- revert to the bold folder-name label once the
  // user has navigated.
  pathEntry->clearFocus();
}

#undef FocusOut // X11/Xlib.h #defines this, clashing with QEvent::FocusOut
bool MainWindow::eventFilter(QObject* watched, QEvent* event) {
  if (watched == ui.sidePane && event->type() == QEvent::Resize) {
    QTimer::singleShot(0, this, &MainWindow::updateSidebarSpacerWidth);
  }

  if(watched == pathEntry && event->type() == QEvent::FocusOut) {
    pathEntryAction_->setVisible(false);
    folderNameLabelAction_->setVisible(true);
  }
  else if(watched == filterBar && event->type() == QEvent::FocusOut) {
    // Collapse back to the search button -- but only if it was opened by
    // a click in the first place (searchManuallyExpanded_) and the window
    // is still narrower than the threshold (updateSearchBarMode() already
    // owns the expanded case above it).
    Settings& settings = static_cast<Application*>(qApp)->settings();
    if(searchManuallyExpanded_ && width() < settings.searchBarExpandWidth()) {
      searchManuallyExpanded_ = false;
      // Restores the segmented view-mode control now that the search field
      // no longer needs the room it borrowed -- see updateViewModeCollapseMode().
      updateViewModeCollapseMode();
      animateSearchBar(searchHolder_->width(), searchButton_->width(), true);
    }
  }
  else if(glassPanels_.contains(static_cast<QWidget*>(watched)) && event->type() == QEvent::Paint) {
    paintLiquidGlassPanel(static_cast<QWidget*>(watched));
    // Every other glass panel is a plain container whose only content IS
    // the glass background -- its buttons/field are separate child
    // widgets that paint themselves afterward regardless, so consuming
    // this event is fine. searchButton_/viewModeCollapsedButton_ differ:
    // they ARE the widget that needs an icon drawn on top in this same
    // paint pass, so their own QToolButton::paintEvent() must still run
    // after ours -- consuming it here left the button a blank glass
    // circle, no icon.
    return watched != searchButton_ && watched != viewModeCollapsedButton_;
  }
  else if(watched == topBlurOverlay_ && event->type() == QEvent::Paint) {
    // See the declaration in mainwindow.h: painted here (its own widget,
    // raised above ui.splitter but below ui.toolBar) instead of in
    // MainWindow::paintEvent(), so it actually reads as a frosted veil
    // over the sidebar/content items rather than sitting invisibly
    // underneath them.
    Settings& settings = static_cast<Application*>(qApp)->settings();
    int topBlurHeight = settings.windowTopBlurHeight();
    if(topBlurHeight > 0) {
      QPainter p(topBlurOverlay_);
      p.setRenderHint(QPainter::Antialiasing);
      const bool frameless = windowFlags() & Qt::FramelessWindowHint;
      const qreal radius = (isMaximized() || !frameless) ? 0 : kWindowCornerRadius;
      paintBlurBand(p, topBlurOverlay_, QRect(0, 0, topBlurOverlay_->width(), topBlurHeight), ui.splitter, true, true,
                    palette().color(QPalette::Window), settings.windowTopBlurPower(), radius,
                    settings.windowTopBlurSaturation() / 100.0, settings.windowTopBlurBrightness() / 100.0,
                    qBound(0, settings.windowTopBlurOpacity(), 255) / 255.0, sidebarTint_,
                    settings.windowTopBlurKeyColor(), settings.windowTopBlurKeyTolerance());
    }
    return true;
  }
  else if(watched == windowStrokeOverlay_ && event->type() == QEvent::Paint) {
    // See the declaration in mainwindow.h -- kept raised above every other
    // sibling so this always wins, unlike when it was drawn directly in
    // MainWindow::paintEvent() (still first to paint, no matter where in
    // that function the drawing happened -- children paint after their
    // parent regardless).
    QPainter p(windowStrokeOverlay_);
    p.setRenderHint(QPainter::Antialiasing);
    const bool frameless = windowFlags() & Qt::FramelessWindowHint;
    const qreal radius = (isMaximized() || !frameless) ? 0 : kWindowCornerRadius;
    // Inset by half the pen width so the 1px line itself isn't clipped by
    // the widget's own bounds (a stroke drawn exactly on rect()'s edge
    // would have half its width fall outside what this widget can paint).
    QPainterPath strokePath;
    strokePath.addRoundedRect(QRectF(windowStrokeOverlay_->rect()).adjusted(0.5, 0.5, -0.5, -0.5), radius, radius);
    QPen strokePen(QColor(255, 255, 255, 51)); // 51/255 ~= 20% opacity
    strokePen.setWidthF(1.0);
    p.setPen(strokePen);
    p.setBrush(Qt::NoBrush);
    p.drawPath(strokePath);
    return true;
  }
  else if(watched == statusBarOverlay_ && event->type() == QEvent::Paint) {
    // Unlike ui.toolBar (deliberately left see-through so scrolled content
    // peeks up, blurred, underneath it -- the whole point of the vibrancy
    // toolbar), this bar sits over whatever ui.splitter now paints in the
    // space it reclaimed from the old docked status bar (real folder/
    // sidebar items, not a blank margin) -- so with no fill of its own it
    // read as a plain see-through strip with those items showing through
    // right behind the breadcrumb text, unreadable. Filled here, first,
    // with the same tint MainWindow::paintEvent() gives the rest of the
    // window's chrome, so it reads as part of that chrome again instead of
    // a transparent gap -- breadcrumbSpacer_/breadcrumbBar_ are separate
    // child widgets and still paint their own actual content on top of
    // this afterward, same reasoning as ui.toolBar's children above.
    //
    // Only the content-column portion, though: statusBarOverlay_ itself
    // spans the full window width (its left portion is breadcrumbSpacer_,
    // an invisible placeholder the same width as the sidebar, keeping
    // breadcrumbBar_ aligned with the content column above it -- see the
    // ctor) -- filling the sidebar-column portion too painted over the
    // sidebar's own last row of items with this opaque color, and (since
    // that's exactly the strip KWin's blur-behind is keyed to, see
    // updateSidebarBlurRegion()) opaquely blocked the blur there as well.
    QPainter p(statusBarOverlay_);
    p.setRenderHint(QPainter::Antialiasing);
    // Clipped to the window's own rounded-corner outline (translated into
    // this widget's local coordinates -- it sits flush against the
    // window's bottom edge, so its own bottom corners coincide with the
    // window's): a plain fillRect() has square corners of its own, which
    // poked out past the window's rounded bottom-right corner as a visible
    // square notch.
    const bool frameless = windowFlags() & Qt::FramelessWindowHint;
    const qreal radius = (isMaximized() || !frameless) ? 0 : kWindowCornerRadius;
    QPainterPath windowPath;
    windowPath.addRoundedRect(QRectF(0, -(height() - statusBarOverlay_->height()), width(), height()), radius, radius);
    p.setClipPath(windowPath);
    int colRight = sidebarColumnRight(ui.sidePane, this);
    p.fillRect(QRect(colRight, 0, statusBarOverlay_->width() - colRight, statusBarOverlay_->height()), mainAreaTintColor());
    // The hairline above the breadcrumb bar: drawn here, in the overlay's
    // own paint handler, rather than in MainWindow::paintEvent() like the
    // toolbar's matching hairline -- painted there, it sat exactly on this
    // widget's own top row, and since children always paint after their
    // parent, the fillRect() just above covered it completely on every
    // repaint. Always drawn (unlike the toolbar's hairline, which will
    // fade in/out with hover/scroll) -- see the request that it stay
    // always visible regardless of that.
    QColor hairline(127, 127, 127, 60);
    p.fillRect(QRect(colRight, 0, statusBarOverlay_->width() - colRight, 1), hairline);
    return false;
  }
  // The frameless window's toolbar doubles as the title bar: empty-area
  // presses reach the toolbar itself (children like buttons/pathEntry
  // swallow their own clicks first), so a press here means "drag the
  // window" -- startSystemMove() delegates the actual move to the
  // compositor, which is the only reliable way to move a window on
  // Wayland. Double-click toggles maximize, like a real title bar.
  else if(watched == ui.toolBar) {
    if(event->type() == QEvent::Paint) {
      // Consume the toolbar's own (opaque, native) background paint
      // entirely rather than letting Qt draw it -- since nothing is drawn
      // here to replace it, whatever MainWindow::paintEvent() already
      // painted underneath (parents always paint before their children,
      // both sharing this window's one backing store) simply shows
      // through unobstructed, including the top blur band there. Child
      // widgets (buttons, labels, ...) still receive and handle their own
      // separate QEvent::Paint independently of this, since they're
      // distinct QWidgets, so they still draw normally on top.
      return true;
    }
    if(event->type() == QEvent::MouseButtonPress) {
      QMouseEvent* me = static_cast<QMouseEvent*>(event);
      if(me->button() == Qt::LeftButton && windowHandle()) {
        windowHandle()->startSystemMove();
        return true;
      }
    }
    else if(event->type() == QEvent::MouseButtonDblClick) {
      isMaximized() ? showNormal() : showMaximized();
      return true;
    }
    else if(event->type() == QEvent::Enter) {
      // Only starts the 1.5s countdown -- doesn't fade in immediately, so
      // a cursor merely passing over the titlebar on its way elsewhere
      // doesn't trigger it.
      toolbarHairlineHoverTimer_->start();
    }
    else if(event->type() == QEvent::Leave) {
      // Cancel a countdown that hadn't fired yet, and fade out on whatever
      // delay covers "the pointer just left" the same as a scroll going
      // idle -- if the timer HAD already fired (hairline visible), this
      // still applies and starts the idle countdown toward fading back out.
      toolbarHairlineHoverTimer_->stop();
      scheduleToolbarHairlineFadeOut();
    }
  }
  return QMainWindow::eventFilter(watched, event);
}

void MainWindow::on_actionGoUp_triggered() {
  TabPage *page = currentPage();

  if (page) {
    filterBar->clear();
    FmPath *parent = fm_path_get_parent(page->path());
    if (parent)
      chdir(parent);
    updateUIForCurrentPage();
  }
}

// probono
void MainWindow::on_actionGoUpAndCloseCurrentWindow_triggered() {
  // Do not blindly close the window, but only when the path has a parent (= is
  // not /)
  TabPage *page = currentPage();

  if (page) {
    filterBar->clear();
    FmPath *parent = fm_path_get_parent(page->path());
    if (parent) {
      MainWindow::on_actionGoUp_triggered();
      close();
    }
  }
}

void MainWindow::on_actionGoBack_triggered() {
  TabPage *page = currentPage();

  if (page) {
    filterBar->clear();
    page->backward();
    updateUIForCurrentPage();
  }
}

void MainWindow::on_actionGoForward_triggered() {
  TabPage *page = currentPage();

  if (page) {
    filterBar->clear();
    page->forward();
    updateUIForCurrentPage();
  }
}

void MainWindow::on_actionHome_triggered() { chdir(fm_path_get_home()); }

void MainWindow::on_actionReload_triggered() { currentPage()->reload(); }

void MainWindow::on_actionGo_triggered() { onPathEntryReturnPressed(); }

void MainWindow::on_actionNewTab_triggered() {
  FmPath *path = currentPage()->path();
  addTab(path);
}

void MainWindow::on_actionNewWin_triggered() {
  FmPath *path = currentPage()->path();
  (new MainWindow(path))->show();
}

void MainWindow::on_actionNewFolder_triggered() {
  if (TabPage *tabPage = currentPage()) {
    FmPath *dirPath = tabPage->folderView()->path();

    if (dirPath)
      createFileOrFolder(CreateNewFolder, dirPath);
  }
}

void MainWindow::on_actionNewBlankFile_triggered() {
  if (TabPage *tabPage = currentPage()) {
    FmPath *dirPath = tabPage->folderView()->path();

    if (dirPath)
      createFileOrFolder(CreateNewTextFile, dirPath);
  }
}

void MainWindow::on_actionCloseTab_triggered() {
  closeTab(ui.tabBar->currentIndex());
}

void MainWindow::on_actionCloseWindow_triggered() {
  // FIXME: should we save state here?
  close();
  // the window will be deleted automatically on close
}

// probono
void MainWindow::on_actionOpen_triggered() {
  TabPage *page = currentPage();

  if (page) {
    FmFileInfoList *files = page->selectedFiles();

    if (files) {
      if (page->fileLauncher()) {
        page->fileLauncher()->launchFiles(NULL, files);
      } else { // use the default launcher
        Fm::FileLauncher launcher;
        launcher.launchFiles(NULL, files);
      }
    }
  }
}

// probono
void MainWindow::on_actionOpenWith_triggered() {
  TabPage *page = currentPage();

  if (page) {
    FmFileInfoList *files = page->selectedFiles();

    if (files) {

      FmPathList *paths = fm_path_list_new_from_file_info_list(files);
      for (GList *l = fm_path_list_peek_head_link(paths); l; l = l->next) {
        FmPath *path = FM_PATH(l->data);
        QString sourcePathStr = QString(fm_path_to_str(path));
        QProcess p;
        p.setProgram("open");
        p.setArguments({"--chooser", sourcePathStr});
        qDebug() << p.program() << p.arguments();
        p.startDetached();
      }
      fm_path_list_unref(paths);
    }
  }
}

// probono
void MainWindow::on_actionOpenAndCloseCurrentWindow_triggered() {
  // Do not blindly do this, but only if files are selected
  TabPage *page = currentPage();

  if (page) {
    FmFileInfoList *files = page->selectedFiles();

    if (files) {
      MainWindow::on_actionOpen_triggered();
      close();
    }
  }
}

// probono
void MainWindow::on_actionShowContents_triggered() {
  TabPage *page = currentPage();

  if (page) {
    FmFileInfoList *files = page->selectedFiles();

    if (files) {
      if (page->fileLauncher()) {
        page->fileLauncher()->launchFiles(NULL, files, true);
      } else { // use the default launcher
        Fm::FileLauncher launcher;
        launcher.launchFiles(NULL, files, true);
      }
    }
  }
}

void MainWindow::on_actionFileProperties_triggered() {
  TabPage *page = currentPage();

  if (page) {
    FmFileInfoList *files = page->selectedFiles();

    if (files) {
      Fm::FilePropsDialog::showForFiles(files);
      fm_file_info_list_unref(files);
    }
  }
}

void MainWindow::on_actionFolderProperties_triggered() {
  TabPage *page = currentPage();

  if (page) {
    FmFolder *folder = page->folder();

    if (folder) {
      FmFileInfo *info = fm_folder_get_info(folder);

      if (info)
        Fm::FilePropsDialog::showForFile(info);
    }
  }
}

void MainWindow::on_actionShowHidden_triggered(bool checked) {
  TabPage *tabPage = currentPage();
  tabPage->setShowHidden(checked);
  ui.sidePane->setShowHidden(checked);
  static_cast<Application *>(qApp)->settings().setShowHidden(checked);
}

void MainWindow::on_actionByFileName_triggered(bool checked) {
  currentPage()->sort(Fm::FolderModel::ColumnFileName,
                      currentPage()->sortOrder());
  if (isSpatialMode()) {
    QString path = currentPage()->pathName();
    MetaData metaData(path);
    metaData.setWindowSortItem(MetaData::SortItem::FileName);
  }
}

void MainWindow::on_actionByMTime_triggered(bool checked) {
  currentPage()->sort(Fm::FolderModel::ColumnFileMTime,
                      currentPage()->sortOrder());
  if (isSpatialMode()) {
    QString path = currentPage()->pathName();
    MetaData metaData(path);
    metaData.setWindowSortItem(MetaData::SortItem::ModifiedTime);
  }
}

void MainWindow::on_actionByOwner_triggered(bool checked) {
  currentPage()->sort(Fm::FolderModel::ColumnFileOwner,
                      currentPage()->sortOrder());
  if (isSpatialMode()) {
    QString path = currentPage()->pathName();
    MetaData metaData(path);
    metaData.setWindowSortItem(MetaData::SortItem::Owner);
  }
}

void MainWindow::on_actionByFileSize_triggered(bool checked) {
  currentPage()->sort(Fm::FolderModel::ColumnFileSize,
                      currentPage()->sortOrder());
  if (isSpatialMode()) {
    QString path = currentPage()->pathName();
    MetaData metaData(path);
    metaData.setWindowSortItem(MetaData::SortItem::FileSize);
  }
}

void MainWindow::on_actionByFileType_triggered(bool checked) {
  currentPage()->sort(Fm::FolderModel::ColumnFileType,
                      currentPage()->sortOrder());
  if (isSpatialMode()) {
    QString path = currentPage()->pathName();
    MetaData metaData(path);
    metaData.setWindowSortItem(MetaData::SortItem::FileType);
  }
}

void MainWindow::on_actionAscending_triggered(bool checked) {
  currentPage()->sort(currentPage()->sortColumn(), Qt::AscendingOrder);
  if (isSpatialMode()) {
    QString path = currentPage()->pathName();
    MetaData metaData(path);
    metaData.setWindowSortOrder(MetaData::SortOrder::Ascending);
  }
}

void MainWindow::on_actionDescending_triggered(bool checked) {
  currentPage()->sort(currentPage()->sortColumn(), Qt::DescendingOrder);
  if (isSpatialMode()) {
    QString path = currentPage()->pathName();
    MetaData metaData(path);
    metaData.setWindowSortOrder(MetaData::SortOrder::Descending);
  }
}

void MainWindow::on_actionCaseSensitive_triggered(bool checked) {
  currentPage()->setSortCaseSensitive(checked);
  if (isSpatialMode()) {
    QString path = currentPage()->pathName();
    MetaData metaData(path);
    metaData.setWindowSortCase(checked ? MetaData::SortCase::CaseSensitive
                                       : MetaData::SortCase::NotCaseSensitive);
  }
}

void MainWindow::on_actionFolderFirst_triggered(bool checked) {
  currentPage()->setSortFolderFirst(checked);
  if (isSpatialMode()) {
    QString path = currentPage()->pathName();
    MetaData metaData(path);
    metaData.setWindowSortFolderFirst(
        checked ? MetaData::SortFolderFirst::FoldersFirst
                : MetaData::SortFolderFirst::NotFoldersFirst);
  }
}

void MainWindow::on_actionFilter_triggered(bool checked) {
  // The search field now lives permanently in the toolbar (see the ctor),
  // so this action just focuses it instead of toggling visibility.
  Q_UNUSED(checked);
  filterBar->setFocus();
  filterBar->selectAll();
}

void MainWindow::on_actionComputer_triggered() {
  FmPath *path = fm_path_new_for_uri("file:/");
  chdir(path);
  fm_path_unref(path);
}

void MainWindow::on_actionApplications_triggered() {
  // chdir(fm_path_get_apps_menu());
  // probono: Use hardcoded /Applications for now
  FmPath *path = fm_path_new_for_uri("file:///Applications");
  chdir(path);
  fm_path_unref(path);
}

void MainWindow::on_actionUtilities_triggered() {
  FmPath *path = fm_path_new_for_uri("file:///Applications/Utilities");
  chdir(path);
  fm_path_unref(path);
}

void MainWindow::on_actionDocuments_triggered() {
  // chdir(fm_path_get_apps_menu());
  FmPath *path;
  path = fm_path_new_for_str(
      QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)
          .toLocal8Bit()
          .data());
  chdir(path);
  fm_path_unref(path);
}

void MainWindow::on_actionDownloads_triggered() {
  // chdir(fm_path_get_apps_menu());
  FmPath *path;
  path = fm_path_new_for_str(
      QStandardPaths::writableLocation(QStandardPaths::DownloadLocation)
          .toLocal8Bit()
          .data());
  chdir(path);
  fm_path_unref(path);
}

void MainWindow::on_actionTrash_triggered() {
  // chdir(fm_path_get_trash()); // Do not use trash://
  FmPath *path;
  path = fm_path_new_for_str(QString(QStandardPaths::writableLocation(
                                         QStandardPaths::GenericDataLocation) +
                                     "/Trash/files")
                                 .toUtf8());
  chdir(path);
  fm_path_unref(path);
}

void MainWindow::on_actionNetwork_triggered() {
  qDebug() << "Launching Zeroconf.app using the 'launch' command";
  QProcess::startDetached("launch", {"Zeroconf"});
}

void MainWindow::on_actionDesktop_triggered() { chdir(fm_path_get_desktop()); }

void MainWindow::on_actionAddToBookmarks_triggered() {
  TabPage *page = currentPage();

  if (page) {
    FmPath *cwd = page->path();

    if (cwd) {
      char *dispName = fm_path_display_basename(cwd);
      fm_bookmarks_insert(bookmarks, cwd, dispName, -1);
      g_free(dispName);
    }
  }
}

void MainWindow::on_actionEditBookmarks_triggered() {
  Application *app = static_cast<Application *>(qApp);
  app->editBookmarks();
}

void MainWindow::on_actionAbout_triggered() {
  // the about dialog
  class AboutDialog : public QDialog {
  public:
    explicit AboutDialog(QWidget *parent = 0, Qt::WindowFlags f = 0) {
      ui.setupUi(this);
      ui.version->setText(tr("Version: %1").arg(FILER_VERSION));
    }

  private:
    Ui::AboutDialog ui;
  };
  AboutDialog dialog(this);
  dialog.exec();
}

void MainWindow::on_actionIconView_triggered() {
  currentPage()->setViewMode(Fm::FolderView::IconMode);
  if (isSpatialMode()) {
    QString path = currentPage()->pathName();
    MetaData metaData(path);
    metaData.setWindowView(MetaData::Icons);
  }
}

void MainWindow::on_actionDetailedList_triggered() {
  currentPage()->setViewMode(Fm::FolderView::DetailedListMode);
  if (isSpatialMode()) {
    QString path = currentPage()->pathName();
    MetaData metaData(path);
    metaData.setWindowView(MetaData::List);
  }
}

void MainWindow::on_actionThumbnailView_triggered() {
  currentPage()->setViewMode(Fm::FolderView::ThumbnailMode);
  if (isSpatialMode()) {
    QString path = currentPage()->pathName();
    MetaData metaData(path);
    metaData.setWindowView(MetaData::Thumbnail);
  }
}

void MainWindow::on_actionGoToFolder_triggered() {
  GotoFolderDialog *gotoFolderDialog = new GotoFolderDialog(this);
  int code = gotoFolderDialog->exec();
  if (code == QDialog::Accepted) {
    FmPath *path =
        fm_path_new_for_path(gotoFolderDialog->getPath().toLatin1().data());
    chdir(path);
    fm_path_unref(path);
  }
}

void MainWindow::onTabBarCloseRequested(int index) { closeTab(index); }

void MainWindow::onTabBarTabMoved(int from, int to) {
  // a tab in the tab bar is moved by the user, so we have to move the
  //  corredponding tab page in the stacked widget to the new position, too.
  QWidget *page = ui.stackedWidget->widget(from);
  if (page) {
    // we're not going to delete the tab page, so here we block signals
    // to avoid calling the slot onStackedWidgetWidgetRemoved() before
    // removing the page. Otherwise the page widget will be destroyed.
    ui.stackedWidget->blockSignals(true);
    ui.stackedWidget->removeWidget(page);
    ui.stackedWidget->insertWidget(to,
                                   page); // insert the page to the new position
    ui.stackedWidget->blockSignals(false); // unblock signals
    ui.stackedWidget->setCurrentWidget(page);
  }
}

void MainWindow::onFilterStringChanged(QString str) {
  if (TabPage *tabPage = currentPage()) {
    // appy filter only if needed (not if tab is changed)
    if (str != tabPage->getFilterStr()) {
      tabPage->setFilterStr(str);
      tabPage->applyFilter();
    }
  }
}

void MainWindow::closeTab(int index) {
  QWidget *page = ui.stackedWidget->widget(index);
  if (page) {
    ui.stackedWidget->removeWidget(
        page); // this does not delete the page widget
    delete page;
    // NOTE: we do not remove the tab here.
    // it'll be donoe in onStackedWidgetWidgetRemoved()
  }
}

void MainWindow::resizeEvent(QResizeEvent *event) {
  QMainWindow::resizeEvent(event);
  qDebug() << "MainWindow resize: width =" << width() << "height =" << height();
  updateSidebarBlurRegion();
  updateSidebarSpacerWidth();
  updateToolbarOverlayGeometry();
  updateStatusBarOverlayGeometry();
  updateTopBlurOverlayGeometry();
  updateWindowStrokeOverlayGeometry();
  updateSearchBarMode();
  updateViewModeCollapseMode();
  applyScrollInsets();
  // On X11 our own frameGeometry() is authoritative, so drive the wallpaper
  // tint directly. On Wayland it's fed by WindowGeometryTracker instead --
  // see updateSidebarWallpaperTint()'s doc comment in mainwindow.h.
  if(QX11Info::isPlatformX11()) {
    updateSidebarWallpaperTint(frameGeometry());
  }
  Settings &settings = static_cast<Application *>(qApp)->settings();
  if (settings.spatialMode()) {
    TabPage *page = currentPage();
    if (page) {
      QString path = page->pathName();
      MetaData metaData(path);
      metaData.setWindowHeight(height());
      metaData.setWindowWidth(width());
    }
  } else if (settings.rememberWindowSize()) {
    settings.setLastWindowMaximized(isMaximized());

    if (!isMaximized()) {
      settings.setLastWindowWidth(width());
      settings.setLastWindowHeight(height());
    }
  }
}

void MainWindow::moveEvent(QMoveEvent *event) {
  QMainWindow::moveEvent(event);
  if(QX11Info::isPlatformX11()) {
    updateSidebarWallpaperTint(frameGeometry());
  }
  if (isSpatialMode()) {
    TabPage *page = currentPage();
    if (page) {
      QString path = page->pathName();
      MetaData metaData(path);
      metaData.setWindowOriginX(geometry().x());
      metaData.setWindowOriginY(geometry().y());
    }
  }
}

void MainWindow::closeEvent(QCloseEvent *event) {
  QWidget::closeEvent(event);
  Settings &settings = static_cast<Application *>(qApp)->settings();
  if (settings.rememberWindowSize()) {
    settings.setLastWindowMaximized(isMaximized());

    if (!isMaximized()) {
      settings.setLastWindowWidth(width());
      settings.setLastWindowHeight(height());
    }
  }
}

void MainWindow::onTabBarCurrentChanged(int index) {
  ui.stackedWidget->setCurrentIndex(index);
  if (TabPage *page = static_cast<TabPage *>(ui.stackedWidget->widget(index)))
    filterBar->setText(page->getFilterStr());
  updateUIForCurrentPage();
}

void MainWindow::updateStatusBarForCurrentPage() {
  TabPage *tabPage = currentPage();
  updateBreadcrumb(tabPage->pathName());
}

bool MainWindow::isSpatialMode() const {
  Settings &settings = static_cast<Application *>(qApp)->settings();
  return settings.spatialMode();
}

void MainWindow::updateViewMenuForCurrentPage() {
  TabPage *tabPage = currentPage();

  if (tabPage) {
    // update menus. FIXME: should we move this to another method?
    ui.actionShowHidden->setChecked(tabPage->showHidden());

    // view mode
    QAction *modeAction = NULL;

    switch (tabPage->viewMode()) {
    case Fm::FolderView::IconMode:
      modeAction = ui.actionIconView;
      break;

    case Fm::FolderView::CompactMode:
      // Compact View was removed from the UI; treat it like Detailed List.
      modeAction = ui.actionDetailedList;
      break;

    case Fm::FolderView::DetailedListMode:
      modeAction = ui.actionDetailedList;
      break;

    case Fm::FolderView::ThumbnailMode:
      modeAction = ui.actionThumbnailView;
      break;
    }

    // Gallery/Column replace the folder view without changing its ViewMode,
    // so they take precedence over whatever mode the hidden view reports.
    if (tabPage->galleryMode())
      modeAction = actionGalleryView_;
    else if (tabPage->columnMode())
      modeAction = actionColumnView_;

    Q_ASSERT(modeAction != NULL);
    modeAction->setChecked(true);

    // sort menu
    QAction *sortActions[Fm::FolderModel::NumOfColumns] = {};
    sortActions[Fm::FolderModel::ColumnFileName] = ui.actionByFileName;
    sortActions[Fm::FolderModel::ColumnFileMTime] = ui.actionByMTime;
    sortActions[Fm::FolderModel::ColumnFileSize] = ui.actionByFileSize;
    sortActions[Fm::FolderModel::ColumnFileType] = ui.actionByFileType;
    sortActions[Fm::FolderModel::ColumnFileOwner] = ui.actionByOwner;
    int sortCol = tabPage->sortColumn();
    if(sortCol >= 0 && sortCol < Fm::FolderModel::NumOfColumns && sortActions[sortCol])
        sortActions[sortCol]->setChecked(true);

    if (tabPage->sortOrder() == Qt::AscendingOrder)
      ui.actionAscending->setChecked(true);
    else
      ui.actionDescending->setChecked(true);

    ui.actionCaseSensitive->setChecked(tabPage->sortCaseSensitive());
    ui.actionFolderFirst->setChecked(tabPage->sortFolderFirst());
  }
}

// See the declaration in mainwindow.h.
QString MainWindow::titleBarTextFor(TabPage* tabPage) const {
  if(!tabPage)
    return QString();
  QString title = tabPage->title();
  if(title == tr("Recents"))
    return title;
  FmPath* path = tabPage->path();
  if(path && fm_path_is_native(path))
    return title + QStringLiteral(" — ") + tr("Local");
  return title;
}

void MainWindow::updateUIForCurrentPage() {

  TabPage *tabPage = currentPage();

  if (tabPage) {
    // probono: Whenever we switch to a tab, see whether the user has items
    // selected and enable/disable menu items accordingly
    if (currentPage()->selectedFiles() == 0x0)
      disableMenuItems();
    else
      enableMenuItems();

    QString titleBarText = titleBarTextFor(tabPage);
    setWindowTitle(titleBarText);
    pathEntry->setText(tabPage->pathName());
    folderNameLabel_->setText(titleBarText);
    updateBreadcrumb(tabPage->pathName());
    tabPage->folderView()->childView()->setFocus();

    QString path = tabPage->pathName();
    if(path.contains("/pearos_dmg_mounts/")) {
        // Set DMG view mode
        ui.sidePane->setVisible(false);
        ui.toolBar->setVisible(false);
        if(statusBarOverlay_) statusBarOverlay_->setVisible(false);
        if(menuBar()) menuBar()->setVisible(false);

        // The DMG window hides the toolbar -- which, on the frameless
        // Finder-style windows, is the only titlebar/close-button/drag
        // handle there is. Hand this window back to KWin for decoration
        // instead (real macOS DMG windows have a plain decorated titlebar
        // too). Changing flags recreates the native window, so re-show.
        if(windowFlags() & Qt::FramelessWindowHint) {
            setWindowFlags(windowFlags() & ~Qt::FramelessWindowHint);
            show();
        }
        // The 5px frameless-resize margins (see the constructor) would
        // inset the fixed-pixel background/icon layout; decorated windows
        // resize via KWin edges, so drop them entirely here.
        setContentsMargins(0, 0, 0, 0);
        
        QDir bgDir(path + "/.background");
        QSize bgSize(640, 480);
        if(bgDir.exists()) {
            QStringList bgFiles = bgDir.entryList(QStringList() << "*.png" << "*.jpg" << "*.jpeg", QDir::Files);
            if(bgFiles.isEmpty()) {
                bgFiles = bgDir.entryList(QStringList() << "*.tiff" << "*.tif", QDir::Files);
            }
            if(!bgFiles.isEmpty()) {
                QString bgPath = bgDir.absoluteFilePath(bgFiles.first());
                QPixmap pixmap(bgPath);
                if(!pixmap.isNull()) {
                    bgSize = pixmap.size();

                    // Icon labels must read against whatever this DMG's
                    // background image happens to be (could be dark or
                    // light) -- sample the image's average luminance and
                    // force plain white or black text, whichever contrasts,
                    // instead of leaving the normal (fixed) theme text color
                    // that can end up unreadable against some backgrounds.
                    QImage bgImage = pixmap.toImage();
                    qint64 totalLum = 0;
                    int samples = 0;
                    int stepX = qMax(1, bgImage.width() / 40);
                    int stepY = qMax(1, bgImage.height() / 40);
                    for(int y = 0; y < bgImage.height(); y += stepY) {
                        for(int x = 0; x < bgImage.width(); x += stepX) {
                            QColor c = bgImage.pixelColor(x, y);
                            totalLum += qRound(0.299 * c.red() + 0.587 * c.green() + 0.114 * c.blue());
                            ++samples;
                        }
                    }
                    double avgLum = samples ? (double)totalLum / samples : 255.0;
                    QColor textColor = (avgLum < 128.0) ? Qt::white : Qt::black;
                    QWidget* dmgView = tabPage->folderView()->childView();
                    QPalette pal = dmgView->palette();
                    pal.setColor(QPalette::Text, textColor);
                    pal.setColor(QPalette::WindowText, textColor);
                    dmgView->setPalette(pal);
                }
                tabPage->folderView()->childView()->setStyleSheet(
                    "QListView { border-image: url('" + bgPath + "') 0 0 0 0 stretch stretch; }"
                );
            }
        }
        
        // Ensure icon mode and large icons
        tabPage->setViewMode(Fm::FolderView::IconMode);
        tabPage->folderView()->setIconSize(Fm::FolderView::IconMode, QSize(128, 128));
        
        // Real macOS DMG windows are never resizable -- their background
        // image and icon positions (from .DS_Store, both parsed below) are
        // both fixed pixel layouts with no notion of "relative" position,
        // so letting the user resize this window stretched the background
        // (via the border-image stylesheet) while the icons stayed at their
        // original absolute coordinates, visibly drifting out of place.
        // Matching the non-resizable behavior sidesteps that entirely.
        setFixedSize(bgSize.width(), bgSize.height());
        
        // Parse .DS_Store for custom positions
        QMap<QString, QPoint> dmgPositions;
        QFile dsFile(path + "/.DS_Store");
        if(dsFile.open(QIODevice::ReadOnly)) {
            QByteArray data = dsFile.readAll();
            int idx = 0;
            QByteArray searchPattern = QByteArray("Ilocblob\0\0\0\x10", 12);
            while((idx = data.indexOf(searchPattern, idx)) != -1) {
                if(idx + 20 <= data.length()) {
                    uint32_t x = qFromBigEndian<uint32_t>(data.mid(idx + 12, 4).constData());
                    uint32_t y = qFromBigEndian<uint32_t>(data.mid(idx + 16, 4).constData());
                    QString foundName;
                    for(int len = 1; len < 255; ++len) {
                        int startPos = idx - (len * 2) - 4;
                        if(startPos >= 0) {
                            uint32_t possibleLen = qFromBigEndian<uint32_t>(data.mid(startPos, 4).constData());
                            if(possibleLen == (uint32_t)len) {
                                bool valid = true;
                                QString name;
                                for(int i = 0; i < len; ++i) {
                                    uint16_t c = qFromBigEndian<uint16_t>(data.mid(startPos + 4 + i*2, 2).constData());
                                    if(c == 0 || c > 0x10FFFF) { valid = false; break; }
                                    name.append(QChar(c));
                                }
                                if(valid) {
                                    foundName = name;
                                    break;
                                }
                            }
                        }
                    }
                    if(!foundName.isEmpty()) {
                        dmgPositions[foundName] = QPoint(x, y);
                    }
                }
                idx += 12;
            }
        }
        
        // Apply custom positions when items are loaded
        Fm::FolderViewListView* listView = static_cast<Fm::FolderViewListView*>(tabPage->folderView()->childView());
        listView->setMovement(QListView::Free);
        QAbstractItemModel* model = listView->model();
        if(model && !dmgPositions.isEmpty()) {
            auto applyPositions = [listView, model, dmgPositions]() {
                for(int i = 0; i < model->rowCount(); ++i) {
                    QModelIndex index = model->index(i, 0);
                    QString name = index.data(Qt::DisplayRole).toString();
                    if(dmgPositions.contains(name)) {
                        listView->setPositionForIndex(dmgPositions[name], index);
                    } else if(dmgPositions.contains(name + ".app")) {
                        listView->setPositionForIndex(dmgPositions[name + ".app"], index);
                    } else if(dmgPositions.contains(name + ".desktop")) {
                        listView->setPositionForIndex(dmgPositions[name + ".desktop"], index);
                    }
                }
            };
            connect(model, &QAbstractItemModel::rowsInserted, listView, [applyPositions](const QModelIndex&, int, int) {
                applyPositions();
            });
            // Apply now if the folder is already loaded, and re-apply after
            // the event loop settles: QListView silently re-lays-out (in
            // default grid order, discarding setPositionForIndex()) on any
            // resize, and setFixedSize()/setContentsMargins()/window-flag
            // changes above all queue resizes that land AFTER this code.
            applyPositions();
            QTimer::singleShot(0, listView, applyPositions);
            QTimer::singleShot(300, listView, applyPositions);
        }

    } else {
        // Clear stylesheet and any DMG-forced text color/fixed size in case
        // it's not a DMG (or a DMG tab was closed and another tab in this
        // same window is now current).
        QWidget* view = tabPage->folderView()->childView();
        view->setStyleSheet("");
        view->setPalette(QPalette());
        setMinimumSize(0, 0);
        setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
        // Restore the frameless-resize margins the DMG branch removes.
        // (A window that went DMG-decorated keeps its KWin decoration until
        // closed -- flags are not flipped back mid-life -- so only frameless
        // windows get their grip margins back.)
        if(windowFlags() & Qt::FramelessWindowHint)
            setContentsMargins(5, 0, 5, 5);
    }

    // update side pane
    ui.sidePane->setCurrentPath(tabPage->path());
    ui.sidePane->setShowHidden(tabPage->showHidden());

    // update back/forward/up toolbar buttons
    ui.actionGoUp->setEnabled(tabPage->canUp());
    ui.actionGoUpAndCloseCurrentWindow->setEnabled(tabPage->canUp());
    ui.actionGoBack->setEnabled(tabPage->canBackward());
    ui.actionGoForward->setEnabled(tabPage->canForward());

    updateViewMenuForCurrentPage();
    updateStatusBarForCurrentPage();
  }
}

void MainWindow::onStackedWidgetWidgetRemoved(int index) {
  // qDebug("onStackedWidgetWidgetRemoved: %d", index);
  // need to remove associated tab from tabBar
  ui.tabBar->removeTab(index);
  if (ui.tabBar->count() == 0) { // this is the last one
    deleteLater();               // destroy the whole window
    // qDebug("delete window");
  } else {
    Settings &settings = static_cast<Application *>(qApp)->settings();
    if (!settings.alwaysShowTabs() && ui.tabBar->count() == 1) {
      ui.tabBar->setVisible(false);
      // See the matching call in addTab(): the toolbar-height top margin
      // is only reserved while the tab bar is visible.
      updateToolbarOverlayGeometry();
    }
  }
}

void MainWindow::onTabPageTitleChanged(QString title) {
  TabPage *tabPage = static_cast<TabPage *>(sender());
  int index = ui.stackedWidget->indexOf(tabPage);

  if (index >= 0)
    ui.tabBar->setTabText(index, title);

  if (tabPage == currentPage()) {
    QString titleBarText = titleBarTextFor(tabPage);
    setWindowTitle(titleBarText);
    if(folderNameLabel_)
      folderNameLabel_->setText(titleBarText);
  }
}

void Filer::MainWindow::disableMenuItems() {
  // probono: No object has been selected by the user, so disable the actions
  // that work on filesystem objects qDebug() << "probono: disableMenuItems";
  ui.actionOpen->setEnabled(false);
  ui.actionOpenWith->setEnabled(false);
  ui.actionFileProperties->setEnabled(false);
  ui.actionCut->setEnabled(false);
  ui.actionCopy->setEnabled(false);
  ui.actionDuplicate->setEnabled(false);
  ui.actionRename->setEnabled(false);
  ui.actionTrash->setEnabled(false);
  ui.actionShowContents->setEnabled(false);
}

void Filer::MainWindow::enableMenuItems() {
  // probono: At least one object has been selected, so enable the actions that
  // work on filesystem objects qDebug() << "probono: enableMenuItems";
  ui.actionOpen->setEnabled(true);
  ui.actionOpenWith->setEnabled(true);
  ui.actionFileProperties->setEnabled(true);
  ui.actionCut->setEnabled(true);
  ui.actionCopy->setEnabled(true);
  ui.actionDuplicate->setEnabled(true);
  ui.actionRename->setEnabled(true);
  ui.actionTrash->setEnabled(true);
  ui.actionShowContents->setEnabled(true);
}

void MainWindow::onTabPageStatusChanged(int type, QString statusText) {
  TabPage *tabPage = static_cast<TabPage *>(sender());
  if (tabPage == currentPage()) {
    switch (type) {
    case TabPage::StatusTextNormal:
    case TabPage::StatusTextSelectedFiles: {
      // The "<name>" Kind / item-count text used to fight the free-space
      // label for the same status bar row; both are gone now in favor of
      // the breadcrumb trail (see updateBreadcrumb()), but menu items still
      // need to reflect whether anything is selected.
      QString text = tabPage->statusText(TabPage::StatusTextSelectedFiles);
      if (text.isEmpty())
        disableMenuItems(); // probono
      else
        enableMenuItems(); // probono
      break;
    }
    case TabPage::StatusTextFSInfo:
      break;
    }
  }
}

void MainWindow::onTabPageOpenDirRequested(FmPath *path, int target) {
  switch (target) {
  case OpenInCurrentTab:
    chdir(path);
    break;

  case OpenInNewTab:
    addTab(path);
    break;

  case OpenInNewWindow: {
    (new MainWindow(path))->show();
    break;
  }
  }
}

void MainWindow::onTabPageSortFilterChanged() {
  TabPage *tabPage = static_cast<TabPage *>(sender());

  if (tabPage == currentPage()) {
    updateViewMenuForCurrentPage();
    Settings &settings = static_cast<Application *>(qApp)->settings();
    settings.setSortColumn(
        static_cast<Fm::FolderModel::ColumnId>(tabPage->sortColumn()));
    settings.setSortOrder(tabPage->sortOrder());
    settings.setSortFolderFirst(tabPage->sortFolderFirst());

    // Update metadata - this is when the user click on column headings to
    // sort, rather than selecting items to sort in the menu
    if (isSpatialMode()) {

      QString path = currentPage()->pathName();
      MetaData metaData(path);

      switch (static_cast<Fm::FolderModel::ColumnId>(tabPage->sortColumn())) {
      case Fm::FolderModel::ColumnFileName:
        metaData.setWindowSortItem(MetaData::SortItem::FileName);
        break;
      case Fm::FolderModel::ColumnFileType:
        metaData.setWindowSortItem(MetaData::SortItem::FileType);
        break;
      case Fm::FolderModel::ColumnFileSize:
        metaData.setWindowSortItem(MetaData::SortItem::FileSize);
        break;
      case Fm::FolderModel::ColumnFileMTime:
        metaData.setWindowSortItem(MetaData::SortItem::ModifiedTime);
        break;
      case Fm::FolderModel::ColumnFileOwner:
        metaData.setWindowSortItem(MetaData::SortItem::Owner);
        break;
      case Fm::FolderModel::NumOfColumns:
        break;
      }

      switch (tabPage->sortOrder()) {
      case Qt::AscendingOrder:
        metaData.setWindowSortOrder(MetaData::SortOrder::Ascending);
        break;
      case Qt::DescendingOrder:
        metaData.setWindowSortOrder(MetaData::SortOrder::Descending);
        break;
      }

      metaData.setWindowSortFolderFirst(
          tabPage->sortFolderFirst()
              ? MetaData::SortFolderFirst::FoldersFirst
              : MetaData::SortFolderFirst::NotFoldersFirst);
    }
  }
}

void MainWindow::onSidePaneChdirRequested(int type, FmPath *path) {
  // FIXME: use enum for type value or change it to button.
  if (type == 0) // left button (default)
    chdir(path);
  else if (type == 1) // middle button
    addTab(path);
  else if (type == 2) // new window
    (new MainWindow(path))->show();
}

void MainWindow::onSidePaneOpenFolderInNewWindowRequested(FmPath *path) {
  (new MainWindow(path))->show();
}

void MainWindow::onSidePaneOpenFolderInNewTabRequested(FmPath *path) {
  addTab(path);
}

void MainWindow::onSidePaneOpenFolderInTerminalRequested(FmPath *path) {
  Application *app = static_cast<Application *>(qApp);
  app->openFolderInTerminal(path);
}

void MainWindow::onSidePaneCreateNewFolderRequested(FmPath *path) {
  createFileOrFolder(CreateNewFolder, path);
}

void MainWindow::onSidePaneModeChanged(Fm::SidePane::Mode mode) {
  static_cast<Application *>(qApp)->settings().setSidePaneMode(mode);
}

void MainWindow::onSplitterMoved(int pos, int index) {
  Application *app = static_cast<Application *>(qApp);
  app->settings().setSplitterPos(pos);
  updateSidebarBlurRegion();
  updateSidebarSpacerWidth();
  qDebug() << "Sidebar size dragged to width:" << ui.sidePane->width() << "px (pos:" << pos << ")";
}

// The full-height sidebar column strip (x = 0 .. sidebar's right edge),
// like Finder, where the frosted column runs from the very top of the
// window (behind the traffic lights) down to the bottom -- not just the
// sidebar widget's own rect. Shared by paintEvent() (which paints the
// translucent fill) and updateSidebarBlurRegion() (which tells KWin to
// blur exactly the same strip). Width 0 when the sidebar is hidden
// (e.g. DMG view mode).
static int sidebarColumnRight(const QWidget* sidePane, const QWidget* window) {
  if(!sidePane->isVisible())
    return 0;
  return sidePane->mapTo(const_cast<QWidget*>(window), QPoint(sidePane->width(), 0)).x();
}

// Restricts KWin's blur-behind to just the sidebar column (see the
// vibrancy comment in the constructor for why the rest of the window is
// deliberately excluded), re-run whenever the sidebar's geometry could
// have changed.
void MainWindow::updateSidebarBlurRegion() {
  int colRight = sidebarColumnRight(ui.sidePane, this);
  if(colRight <= 0) {
    KWindowEffects::enableBlurBehind(windowHandle(), false);
  } else {
    // Clip the strip to the rounded window outline so the blur doesn't
    // poke out square at the top-left/bottom-left corners.
    QPainterPath path;
    path.addRoundedRect(rect(), kWindowCornerRadius, kWindowCornerRadius);
    QRegion region = QRegion(path.toFillPolygon().toPolygon()) & QRegion(0, 0, colRight, height());
    KWindowEffects::enableBlurBehind(windowHandle(), true, region);
  }
  update(); // repaint the custom chrome for the new geometry
}

// Retints the sidebar column from the wallpaper directly behind it, like
// macOS's translucent sidebar vibrancy. windowGlobalRect is this window's
// own frame geometry in screen coordinates -- on X11 that comes straight
// from frameGeometry() (see moveEvent()/resizeEvent()); on Wayland, where a
// client can't query its own global position, it's relayed here by
// WindowGeometryTracker via a KWin script + D-Bus (see application.cpp).
void MainWindow::updateSidebarWallpaperTint(const QRect& windowGlobalRect) {
  if(!static_cast<Application*>(qApp)->settings().tintWindow())
    return;
  // The sidebar is always anchored to the window's left edge, so pick the
  // screen from a point there -- not windowGlobalRect.center(), which for a
  // wide window drifts off-screen (and starts returning a null QScreen)
  // well before the sidebar itself does, snapping the tint back to gray
  // while the sidebar is still fully visible.
  QPoint sidebarAnchor(windowGlobalRect.left(), windowGlobalRect.center().y());
  QScreen* screen = QGuiApplication::screenAt(sidebarAnchor);
  DesktopWindow* dw = screen ? static_cast<Application*>(qApp)->desktopWindowForScreen(screen) : nullptr;
  QColor tint(28, 28, 28);
  if(dw) {
    int colRight = sidebarColumnRight(ui.sidePane, this);
    QPoint originOffset = windowGlobalRect.topLeft() - screen->geometry().topLeft();
    QRect sampleRect(originOffset, QSize(colRight, windowGlobalRect.height()));
    tint = dw->averageColorForRect(sampleRect);
  }
  // Ignore tiny fluctuations (thumbnail repaints, subpixel drag jitter) so a
  // stationary window doesn't keep triggering repaints.
  auto channelDelta = [](int a, int b) { return qAbs(a - b); };
  if(channelDelta(tint.red(), sidebarTint_.red()) > 4 ||
     channelDelta(tint.green(), sidebarTint_.green()) > 4 ||
     channelDelta(tint.blue(), sidebarTint_.blue()) > 4) {
    sidebarTint_ = tint;
    update();
  }
}

// The constructor's updateSidebarBlurRegion() call runs before the window
// is shown, when ui.sidePane->isVisible() is still false -- so it disables
// blur, and nothing necessarily re-runs it afterwards (a resizeEvent is not
// guaranteed after show). Recompute once the window is actually visible.
void MainWindow::showEvent(QShowEvent* event) {
  QMainWindow::showEvent(event);
  updateSidebarBlurRegion();
  updateToolbarOverlayGeometry();
  updateStatusBarOverlayGeometry();
  updateWindowStrokeOverlayGeometry();
  if(QX11Info::isPlatformX11()) {
    updateSidebarWallpaperTint(frameGeometry());
  }
  // On Wayland the KWin script (see application.cpp's WindowGeometryTracker)
  // reports geometry on window discovery, so no direct call is needed here.
}

// Positions the floating toolbar overlay (see removeToolBar() in the ctor)
// across the full window width at y=0, above the content in z-order. The
// content underneath (ui.splitter, now reclaiming the space the toolbar used
// to reserve) is left untouched -- it's the toolbar that's repositioned to
// sit on top of it, not the other way around.
void MainWindow::updateToolbarOverlayGeometry() {
  if(!ui.toolBar)
    return;
  int h = ui.toolBar->sizeHint().height();
  ui.toolBar->setGeometry(0, 0, width(), h);
  ui.toolBar->raise();
  // ui.tabBar lives inside ui.frame's own layout now (see main-win.ui --
  // moved there so it spans only the content column, not the sidebar too),
  // which starts at y=0 like everything else in the splitter. Reserve
  // exactly the floating toolbar's height above it so tabs sit below the
  // title bar instead of underneath it -- but only while the tab bar is
  // actually shown (single-tab windows normally hide it): the reservation
  // and the tab bar share this one top margin (both are ui.frame's own
  // layout, see main-win.ui), so with the tab bar hidden it was previously
  // reserved anyway, landing as dead space that pushed the folder view's
  // icons/list down below the toolbar for no reason -- the actual visible
  // content, not just the tab bar, sits in that same margin. isHidden()
  // (not isVisible()) so this reads the tab bar's own explicit shown/hidden
  // state rather than ancestor visibility, which would read false for every
  // widget before the top-level window itself is first shown.
  //
  // Settings::contentTopPadding() is deliberately NOT added here -- an
  // earlier version folded it into this same static margin, but a static
  // margin is a dead zone: content can never scroll into it, so the padding
  // just became a permanent gap. It's applied instead as a scroll-through
  // inset on the folder view itself (see applyScrollInsets()), the same
  // treatment the sidebar already gets from ui.toolBar's height -- row 0
  // sits inset px down at rest, but scrolling slides real rows up through
  // that same strip, visible (blurred) under the toolbar instead of stuck
  // below it.
  if(ui.frame && ui.frame->layout()) {
    int reservedTop = (ui.tabBar && !ui.tabBar->isHidden()) ? h : 0;
    QMargins m = ui.frame->layout()->contentsMargins();
    if(m.top() != reservedTop) {
      m.setTop(reservedTop);
      ui.frame->layout()->setContentsMargins(m);
    }
  }
}

// Positions statusBarOverlay_ (see its declaration/long doc comment in
// mainwindow.h) across the full window width at the bottom, above the
// content in z-order -- same bottom-edge treatment
// updateToolbarOverlayGeometry() gives the top.
void MainWindow::updateStatusBarOverlayGeometry() {
  if(!statusBarOverlay_)
    return;
  // Not statusBarOverlay_->sizeHint() itself -- with breadcrumbSpacer_ (a
  // bare QWidget, no layout of its own) inside its QHBoxLayout, that came
  // back QSize(0, 0) even once breadcrumbBar_'s own sizeHint() had resolved
  // to something real, collapsing the whole bar to zero height and making
  // it (and the breadcrumb trail with it) disappear entirely. Deriving the
  // height from breadcrumbBar_ directly instead sidesteps that; the 24px
  // floor covers the moment before it's populated with any segments yet
  // (its own sizeHint() is 0 until updateBreadcrumb() adds the first one).
  int h = qMax(24, breadcrumbBar_ ? breadcrumbBar_->sizeHint().height() : 0);
  statusBarOverlay_->setGeometry(0, height() - h, width(), h);
  statusBarOverlay_->raise();
}

// Gives a view's vertical scrollbar a negative minimum (-inset): row 0
// sits inset px down at rest (value starts at the new minimum), and
// scrolling toward 0 slides real rows up through that same strip --
// genuinely rendered, so the top blur band grabs real (blurred) items
// instead of empty margin space. Only applied to the sidebar now (inset =
// ui.toolBar->height(), the space its own chrome -- traffic lights,
// back/forward, etc. -- needs): the content view (icon/list/etc.) used to
// get the same treatment via its own separate Settings::iconScrollInset(),
// but that whole mechanism -- and the setting -- was removed per
// feedback in favor of just letting content start at its natural
// position, with no reserved/artificial offset. Reasserted on every call
// (see the two call sites: the 80ms timer for idle upkeep, and
// resizeEvent() directly so a live drag-resize doesn't visibly lag/
// flicker between ticks) because QAbstractItemView resets the scrollbar's
// range -- including minimum -- back to 0 on its own relayouts; reading
// maximum() and calling setRange() in one shot (rather than setMinimum()
// alone) keeps min/max consistent instead of drifting.
// Animates toolbarHairlineOpacity_ toward target (0 or 1), starting from
// whatever it currently is -- so a fade-in interrupted by a fade-out (or
// vice versa) reverses smoothly instead of jumping.
void MainWindow::animateToolbarHairlineTo(qreal target) {
  if(qFuzzyCompare(toolbarHairlineOpacity_, target) &&
     toolbarHairlineAnim_->state() != QAbstractAnimation::Running)
    return;
  toolbarHairlineAnim_->stop();
  toolbarHairlineAnim_->setStartValue(toolbarHairlineOpacity_);
  toolbarHairlineAnim_->setEndValue(target);
  toolbarHairlineAnim_->start();
}

// Restarts the idle timer that eventually fades the hairline back out --
// called both when a scroll is detected and when the mouse leaves the
// titlebar, so either kind of activity keeps deferring the fade-out by the
// same amount.
void MainWindow::scheduleToolbarHairlineFadeOut() {
  toolbarHairlineIdleTimer_->start();
}

void MainWindow::applyScrollInsets() {
  // ScrollPerPixel is required first: QTreeView (the sidebar) defaults to
  // ScrollPerItem, where the scrollbar's unit is item *rows*, not pixels
  // -- a negative minimum there means "-55 rows", which sent it looking
  // up nonexistent negative model indices and rendered completely blank.
  // Settings::sidePaneTopOffset(): extra breathing room on top of the
  // toolbar-height reservation every sidebar row already sits below --
  // live-reloadable the same way, no separate settings-reload wiring
  // needed since this whole function re-reads Settings on every 80ms tick.
  int sidebarInset = ui.toolBar->height() + static_cast<Application*>(qApp)->settings().sidePaneTopOffset();
  auto applyPassThroughInset = [this](QAbstractItemView* v, int inset, bool keepScrollBarHidden = false) {
    if(!v)
      return;
    v->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    QScrollBar* vsb = v->verticalScrollBar();
    // Reasserted on every call (not just once per view) because
    // QAbstractItemView recomputes and overwrites this range -- including
    // resetting minimum back to 0 -- on its own relayouts (content
    // changes, resize, ...). `vsb->minimum() != -inset` is exactly the
    // signal that a relayout just happened and overwrote it, which is
    // also the one moment `vsb->maximum()` still holds Qt's own freshly
    // computed real range before this function's own override replaces it.
    if(vsb && vsb->minimum() != -inset) {
      int trueMax = vsb->maximum();
      bool wasAtRest = vsb->value() == vsb->minimum();
      // A folder with too little content to need real scrolling has a
      // real (Qt-computed) max of 0. Collapsing the range to a single
      // point (min == max == -inset) instead of leaving it [-inset, 0]
      // freezes the scrollbar at rest with nowhere to go -- no wheel
      // scroll, no draggable range -- while still parking row 0 at the
      // same inset offset below the band. A nonzero [-inset, 0] range,
      // by contrast, let the user scroll through `inset` px of genuinely
      // empty space for no reason, which is exactly what this avoids.
      int newMax = trueMax > 0 ? trueMax : -inset;
      vsb->setRange(-inset, newMax);
      if(wasAtRest)
        vsb->setValue(-inset);
      // Explicit on/off instead of ScrollBarAsNeeded: Qt's own
      // as-needed policy re-decides visibility from the range it *just*
      // saw before this function's override runs, which fought this
      // function's own range every tick and flickered the bar in/out.
      // Deciding it here, once, from the same real trueMax used above,
      // removes that tug-of-war.
      v->setVerticalScrollBarPolicy(!keepScrollBarHidden && trueMax > 0 ? Qt::ScrollBarAlwaysOn : Qt::ScrollBarAlwaysOff);
    }
    // Purely visual: the scrollbar's own geometry isn't affected by the
    // range change above (it still spans the view's full height, top
    // edge at y=0, underneath the blurred band), so it's repositioned by
    // hand to start right below the band -- reasserted on every call since
    // QAbstractScrollArea re-lays it out to full height on its own
    // relayouts, same as the range.
    if(vsb) {
      QRect geom = vsb->geometry();
      if(geom.top() != inset) {
        vsb->setGeometry(geom.left(), inset, geom.width(), v->height() - inset);
      }
    }
  };

  QAbstractItemView* sidebarView = qobject_cast<QAbstractItemView*>(ui.sidePane ? ui.sidePane->view() : nullptr);
  if(sidebarView != toolbarBlurLastSidebarView_)
    toolbarBlurLastSidebarView_ = sidebarView;
  // keepScrollBarHidden left at its default (false): the sidebar's own
  // scrollbar should show up the same way the content view's does, when
  // there's enough there to scroll.
  applyPassThroughInset(sidebarView, sidebarInset);

  // Settings::contentTopPadding(): same pass-through inset as the sidebar,
  // now applied to the current tab's folder view -- see the doc comment on
  // its use (removed) in updateToolbarOverlayGeometry(). Only the current
  // page, not every open tab: this runs on the 80ms idle timer, and
  // whichever page becomes current picks up the correct inset on its own
  // next tick, same as it would if it had never been backgrounded.
  QAbstractItemView* contentView = nullptr;
  if(TabPage* page = currentPage()) {
    if(View* fv = page->folderView())
      contentView = fv->childView();
  }
  applyPassThroughInset(contentView, static_cast<Application*>(qApp)->settings().contentTopPadding());

  // Toolbar hairline, scroll trigger: detected here rather than via a
  // per-view signal connection, since contentView already comes and goes
  // with whatever tab is current -- comparing this tick's scrollbar value
  // against the last tick's catches a scroll regardless of which view
  // produced it. First sighting of a view (or of one whose value hasn't
  // been read since a switch) has no valid "last" value to compare yet, so
  // it's recorded but not treated as a scroll.
  if(QScrollBar* contentScrollBar = contentView ? contentView->verticalScrollBar() : nullptr) {
    int currentValue = contentScrollBar->value();
    if(toolbarHairlineHasLastScrollValue_ && currentValue != toolbarHairlineLastScrollValue_) {
      animateToolbarHairlineTo(1.0);
      scheduleToolbarHairlineFadeOut();
    }
    toolbarHairlineLastScrollValue_ = currentValue;
    toolbarHairlineHasLastScrollValue_ = true;
  } else {
    toolbarHairlineHasLastScrollValue_ = false;
  }

  // The top blur band still needs to keep repainting as content scrolls
  // underneath it, though, hence the unconditional update() calls below.
  // Unconditional rather than only on a detected scroll-value change: the
  // band needs to track whatever's currently behind it continuously
  // (thumbnails finishing async, selection highlight changes, a folder
  // repainting for reasons that don't move the scrollbar, etc.), and an
  // edge-triggered repaint here previously produced a "blur flickers on
  // for one frame then cuts off" effect when the change detection missed
  // intermediate frames.
  ui.toolBar->update();
  if(topBlurOverlay_)
    topBlurOverlay_->update();
  // Reasserted continuously, not just on resize/show: this widget must
  // stay above every other sibling, and several of those (ui.toolBar,
  // statusBarOverlay_) get re-raised on their own update paths that don't
  // all funnel back through resizeEvent()/showEvent().
  if(windowStrokeOverlay_)
    windowStrokeOverlay_->raise();

  // Settings::backForwardGroupMarginLeft(): moves backForwardGroup_ itself
  // (overlapping backForwardLeftSpacer_/the sidebar when negative) instead
  // of resizing it -- true CSS-margin behavior, unlike the same setting's
  // effect on the pill's *internal* button padding (applyToolbarPillGroupSettings()).
  // QToolBarLayout has no notion of negative inter-item spacing, so this
  // repositions the widget directly via move() after every layout pass
  // instead, anchored on backForwardLeftSpacer_'s own right edge --
  // backForwardLeftSpacer_ is never itself moved this way, so its geometry
  // stays a stable, self-correcting reference every tick (rather than
  // drifting if this read back backForwardGroup_'s own already-offset
  // position instead).
  if(backForwardGroup_ && backForwardLeftSpacer_) {
    Settings& settings = static_cast<Application*>(qApp)->settings();
    int marginLeft = settings.backForwardGroupMarginLeft();
    int naturalX = backForwardLeftSpacer_->geometry().right() + 1;
    int desiredX = naturalX + marginLeft;
    if(backForwardGroup_->x() != desiredX)
      backForwardGroup_->move(desiredX, backForwardGroup_->y());

    // Settings::backForwardGroupMarginRight(): same move-not-resize
    // treatment, symmetric on the other side -- moves folderNameLabel_ (the
    // "Desktop -- Local" title) itself, anchored on backForwardGroup_'s own
    // just-updated right edge above, so a negative value pulls the title
    // toward (and can overlap) wherever the group currently ends up
    // regardless of its own marginLeft-driven position.
    if(folderNameLabel_) {
      int marginRight = settings.backForwardGroupMarginRight();
      int labelNaturalX = backForwardGroup_->x() + backForwardGroup_->width();
      int labelDesiredX = labelNaturalX + marginRight;
      if(folderNameLabel_->x() != labelDesiredX)
        folderNameLabel_->move(labelDesiredX, folderNameLabel_->y());
    }
  }
}

// Sizes/positions topBlurOverlay_ (see its declaration in mainwindow.h) to
// span the full window width at y=0, Settings::windowTopBlurHeight() tall.
// Called from resizeEvent() and wherever that setting can have changed
// live (updateFromSettings()).
void MainWindow::updateTopBlurOverlayGeometry() {
  if(!topBlurOverlay_)
    return;
  int h = static_cast<Application*>(qApp)->settings().windowTopBlurHeight();
  topBlurOverlay_->setGeometry(0, 0, width(), h);
  topBlurOverlay_->setVisible(h > 0);
}

// See the declaration in mainwindow.h. Covers the whole window and is
// re-raised on every call, since it must stay above every other sibling
// (ui.toolBar, statusBarOverlay_, topBlurOverlay_) no matter which of them
// last got raised.
void MainWindow::updateWindowStrokeOverlayGeometry() {
  if(!windowStrokeOverlay_)
    return;
  windowStrokeOverlay_->setGeometry(0, 0, width(), height());
  windowStrokeOverlay_->raise();
}

// "Liquid Glass" traffic-light backdrop -- pearOS Figma design system, node
// 483:9282 ("Liquid Glass - Small" / Dark / Active=True / Default).
// Reproduced natively via QPainter (gradients/shadow) rather than an
// exported bitmap, matching how the rest of the frameless chrome
// (rounded corners, sidebar tint, blur band) is already painted in code
// instead of shipped as image assets -- keeps it crisp at any size/DPI.
// Figma's source recipe (mix-blend-luminosity fill, inset shadows for the
// bevel) doesn't map 1:1 onto QPainter's blend modes, so this is a close
// visual approximation: frosted rounded-rect fill, a hairline highlight
// ring, a soft outer drop shadow, and a top/bottom inner bevel. Corner
// radius comes from the "glassRadius" property (see the header doc
// comment); square widgets (the traffic-light holders) end up as full
// circles, wider ones (the toolbar button group pills) as rounded rects.
void MainWindow::paintLiquidGlassPanel(QWidget* w) {
  QPainter p(w);
  p.setRenderHint(QPainter::Antialiasing);
  const qreal margin = 1.5; // room for the drop shadow so it isn't clipped
  QRectF rect(margin, margin, w->width() - 2 * margin, w->height() - 2 * margin);
  bool hasRadiusProp = false;
  qreal radius = w->property("glassRadius").toReal(&hasRadiusProp);
  if(!hasRadiusProp)
    radius = rect.height() / 2.0; // default: full stadium/circle shape
  radius = qMin(radius, qMin(rect.width(), rect.height()) / 2.0);

  // Soft outer shadow (Figma: 0px 8px 15px rgba(0,0,0,0.04) -- very faint).
  QRectF shadowRect = rect.translated(0, 1.5).adjusted(-3, -1, 3, 5);
  p.setPen(Qt::NoPen);
  p.setBrush(QColor(0, 0, 0, 22));
  p.drawRoundedRect(shadowRect, radius + 3, radius + 3);

  // Frosted fill (Figma: rgba(153,153,153,0.17) luminosity-blended -- flat
  // alpha fill reads close enough against the dark toolbar behind it).
  p.setBrush(QColor(153, 153, 153, 43));
  p.drawRoundedRect(rect, radius, radius);

  // Inner bevel: a soft light glow near the top and a faint dark rim at the
  // very top/bottom edges (Figma's inset shadows). Clipped to the shape.
  p.save();
  QPainterPath clip;
  clip.addRoundedRect(rect, radius, radius);
  p.setClipPath(clip);
  QLinearGradient bevel(rect.topLeft(), rect.bottomLeft());
  bevel.setColorAt(0.0, QColor(230, 230, 230, 60));
  bevel.setColorAt(0.18, QColor(230, 230, 230, 0));
  bevel.setColorAt(0.85, QColor(26, 26, 26, 0));
  bevel.setColorAt(1.0, QColor(26, 26, 26, 45));
  p.setBrush(bevel);
  p.drawRect(rect);
  p.restore();

  // Hairline highlight ring (Figma: 0.5px #a6a6a6 outline).
  QPen ring(QColor(166, 166, 166, 130));
  ring.setWidthF(0.75);
  p.setPen(ring);
  p.setBrush(Qt::NoBrush);
  p.drawRoundedRect(rect, radius, radius);
}

// Approximates macOS's "unified toolbar" vibrancy: grabs a live render of
// whatever the folder view currently has behind the toolbar's rect (already
// reflecting the current scroll position, since grab() renders the widget
// as it stands) and blurs it, so content appears to scroll past, blurred,
// underneath the toolbar instead of the toolbar just floating opaquely over
// a hard content edge.
// See the declaration in mainwindow.h for why this is a shared method
// rather than a lambda local to a single call site: it's used by
// paintEvent() to paint the window-wide top blur band there, with a
// painter bound to `this` rather than ui.toolBar, so its height isn't
// capped at the toolbar's own (see Settings::windowTopBlurHeight()).
// Adjusts saturation/brightness in place via per-pixel HSV scaling (1.0 =
// unchanged for both). Skipped entirely when both are neutral, since the
// per-pixel loop isn't free -- called on every repaint of the band, not
// just once, so a no-op fast path matters for the common case (defaults).
static void adjustSaturationBrightness(QImage& img, qreal saturation, qreal brightness) {
  if(qFuzzyCompare(saturation, 1.0) && qFuzzyCompare(brightness, 1.0))
    return;
  if(img.format() != QImage::Format_ARGB32)
    img = img.convertToFormat(QImage::Format_ARGB32);
  for(int y = 0; y < img.height(); ++y) {
    QRgb* line = reinterpret_cast<QRgb*>(img.scanLine(y));
    for(int x = 0; x < img.width(); ++x) {
      QColor c = QColor::fromRgba(line[x]);
      int h, s, v, a;
      c.getHsv(&h, &s, &v, &a);
      s = qBound(0, int(s * saturation), 255);
      v = qBound(0, int(v * brightness), 255);
      c.setHsv(h, s, v, a);
      line[x] = c.rgba();
    }
  }
}

// Chroma-key transparency (Settings::windowTopBlurKeyColor()/
// windowTopBlurKeyTolerance()): pixels within `tolerance` of `keyColor`
// fade toward alpha 0 the closer they are to an exact match, so a flat
// background color (typically the toolbar/window's own fill, which the
// grabbed snapshot is full of) becomes transparent instead of reading as
// a solid blurred patch -- real content (icons, colorful items) stays
// opaque since it isn't close to keyColor. tolerance <= 0 is a no-op.
static void applyColorKeyTransparency(QImage& img, const QColor& keyColor, int tolerance) {
  if(tolerance <= 0)
    return;
  if(img.format() != QImage::Format_ARGB32)
    img = img.convertToFormat(QImage::Format_ARGB32);
  const int kr = keyColor.red(), kg = keyColor.green(), kb = keyColor.blue();
  for(int y = 0; y < img.height(); ++y) {
    QRgb* line = reinterpret_cast<QRgb*>(img.scanLine(y));
    for(int x = 0; x < img.width(); ++x) {
      QRgb px = line[x];
      int dr = qRed(px) - kr, dg = qGreen(px) - kg, db = qBlue(px) - kb;
      qreal dist = std::sqrt(qreal(dr * dr + dg * dg + db * db));
      if(dist < tolerance) {
        qreal factor = dist / tolerance; // 0 at exact match, 1 at the edge of the range
        int newAlpha = qRound(qAlpha(px) * factor);
        line[x] = qRgba(qRed(px), qGreen(px), qBlue(px), newAlpha);
      }
    }
  }
}

void MainWindow::paintBlurBand(QPainter& p, QWidget* destWidget, const QRect& r, QWidget* source, bool roundLeft, bool roundRight,
                                const QColor& baseColor, int bandBlurRadius, qreal cornerRadius,
                                qreal saturation, qreal brightness, qreal tintOpacity, const QColor& tintColor,
                                const QColor& keyColor, int keyTolerance) const {
  if(r.isEmpty() || !source || !destWidget)
    return;
  // Grab padded on left/right/bottom and blur the padded image, then keep
  // only the centered r.size() crop. Blurring the unpadded snapshot
  // directly sampled transparent pixels past its own edges, which
  // darkened/greyed the band right at its boundary -- a visible seam
  // where the blur visibly "started" and "stopped" instead of it reading
  // as a plain, edgeless blur of the real content.
  int pad = bandBlurRadius;
  QPoint originInSource = source->mapFromGlobal(destWidget->mapToGlobal(r.topLeft()));
  // The region we'd *like* to grab, padded on left/right/bottom so blurring
  // doesn't sample transparent pixels past the band's own edges (see below).
  // `source` (ui.splitter) is inset from the actual window edges by the
  // frameless-resize margins (see setContentsMargins() in the constructor),
  // so this ideal rect routinely reaches past source->rect() at the
  // window's own left/right edges -- clipping it away with intersected()
  // left a real unblurred gap there (worse the larger bandBlurRadius/pad
  // was, reading as the blur "spreading" its margins rather than
  // intensifying). Instead, grab whatever of the ideal rect actually exists
  // and stretch its edge columns/row to fill the rest, same idea as the
  // top-edge stretch below.
  QRect ideal(originInSource.x() - pad, originInSource.y(), r.width() + 2 * pad, r.height() + pad);
  QRect clipped = ideal.intersected(source->rect());
  QPixmap snapshot = clipped.isEmpty() ? QPixmap() : source->grab(clipped);
  if(!snapshot.isNull() && (!(qFuzzyCompare(saturation, 1.0) && qFuzzyCompare(brightness, 1.0)) || keyTolerance > 0)) {
    QImage snapshotImg = snapshot.toImage();
    adjustSaturationBrightness(snapshotImg, saturation, brightness);
    // Before blurring: color-keying post-blur would smear the now-opaque
    // blurred pixels' colors into the to-be-keyed ones, giving a hazy ring
    // around anything that should have gone transparent instead of a clean
    // cutout.
    applyColorKeyTransparency(snapshotImg, keyColor, keyTolerance);
    snapshot = QPixmap::fromImage(snapshotImg);
  }
  QPixmap layer(r.size());
  layer.fill(Qt::transparent);
  {
    QPainter lp(&layer);
    lp.setRenderHint(QPainter::Antialiasing);
    if(!snapshot.isNull()) {
      // Canvas spans the full ideal rect (so the band's real content
      // always sits at local (pad, pad) below), plus one extra `pad` rows
      // on top to stretch-pad the real window top edge -- there's nothing
      // further up to grab there, and leaving it unpadded (sampling
      // transparent past the edge) darkened the blur right at the top.
      QPixmap extended(ideal.width(), ideal.height() + pad);
      // Transparent pre-fill: only the item glyphs themselves (icons,
      // text) should end up blurred here, with nothing else added behind
      // them -- no flat tint, no color patch. What's really behind the
      // band (the sidebar column's own opaque fill, already painted by
      // paintEvent()) shows through unchanged wherever there's no glyph.
      extended.fill(Qt::transparent);
      // No `pad +` here (unlike offsetY): extended's width already equals
      // ideal.width() with the pad baked directly into ideal's own left
      // edge/width, so the snapshot's placement within extended is exactly
      // how far clipping ate into ideal from the left -- 0 when nothing was
      // clipped. offsetY differs because extended reserves an *extra* pad
      // rows on top (beyond ideal.height()) for the top-edge stretch below.
      int offsetX = clipped.left() - ideal.left();
      int offsetY = pad + (clipped.top() - ideal.top());
      // Built separately from `extended` (rather than by copying back out
      // of it mid-paint, which is undefined behavior in Qt for a pixmap
      // that's the active target of its own QPainter): row 0 of `snapshot`,
      // edge-stretched the same way the full strip is below.
      QPixmap topRow(extended.width(), 1);
      topRow.fill(Qt::transparent);
      {
        QPainter tp(&topRow);
        tp.drawPixmap(offsetX, 0, snapshot.width(), 1, snapshot, 0, 0, snapshot.width(), 1);
        if(offsetX > 0)
          tp.drawPixmap(0, 0, offsetX, 1, snapshot, 0, 0, 1, 1);
        int rightGap = extended.width() - (offsetX + snapshot.width());
        if(rightGap > 0)
          tp.drawPixmap(offsetX + snapshot.width(), 0, rightGap, 1, snapshot, snapshot.width() - 1, 0, 1, 1);
        tp.end();
      }
      {
        QPainter ep(&extended);
        ep.drawPixmap(offsetX, offsetY, snapshot);
        // Stretch left/right edge columns into whatever got clipped away
        // by source->rect() above, so the band reaches the full window
        // width instead of leaving a blank strip at its far edges.
        if(offsetX > 0)
          ep.drawPixmap(0, offsetY, offsetX, snapshot.height(), snapshot, 0, 0, 1, snapshot.height());
        int rightGap = extended.width() - (offsetX + snapshot.width());
        if(rightGap > 0)
          ep.drawPixmap(offsetX + snapshot.width(), offsetY, rightGap, snapshot.height(), snapshot, snapshot.width() - 1, 0, 1,
                         snapshot.height());
        // Stretch the top row upward by `pad` to pad the real window top
        // edge -- there's nothing further up to grab there.
        ep.drawPixmap(0, 0, extended.width(), pad, topRow);
        ep.end();
      }
      QGraphicsScene scene;
      QGraphicsPixmapItem* item = new QGraphicsPixmapItem(extended);
      QGraphicsBlurEffect* blur = new QGraphicsBlurEffect;
      blur->setBlurRadius(bandBlurRadius);
      blur->setBlurHints(QGraphicsBlurEffect::QualityHint);
      item->setGraphicsEffect(blur);
      scene.addItem(item);
      QPixmap blurredPadded(extended.size());
      blurredPadded.fill(Qt::transparent);
      QPainter bp(&blurredPadded);
      scene.render(&bp, QRectF(0, 0, extended.width(), extended.height()), QRectF(0, 0, extended.width(), extended.height()));
      bp.end();
      lp.drawPixmap(0, 0, blurredPadded, pad, pad, r.width(), r.height());
    } else if(baseColor.isValid()) {
      lp.fillRect(layer.rect(), baseColor);
    }
  }

  // Blend the frosted-glass tint (Settings::windowTopBlurOpacity()) directly
  // into the already fully-opaque blurred layer, rather than via
  // p.setOpacity() on the final draw -- see the tintOpacity doc comment in
  // mainwindow.h for why. SourceOver here is a normal alpha blend of
  // baseColor over the existing (opaque) blurred pixels, so it tints
  // uniformly without exposing anything beneath.
  // kMaxTintAlpha caps how strong that veil can ever get: tintColor is a
  // fully-saturated accent (sidebarTint_), and elsewhere in the window this
  // same color is blended at ~4-10% (see mainWindowTintAmount in
  // paintEvent()) -- using the raw 0..1 tintOpacity directly here read as a
  // flat coat of paint over the blur at anything past ~40%, not a subtle
  // vibrancy veil. Scaling the whole 0..1 range down to this ceiling keeps
  // Settings::windowTopBlurOpacity() meaningful end-to-end (0 = no tint,
  // 255 = as strong as this effect should ever look) without a flat color
  // wash swamping the actual blurred content.
  constexpr qreal kMaxTintAlpha = 0.05;
  if(tintOpacity > 0.0 && tintColor.isValid()) {
    QColor tint = tintColor;
    tint.setAlphaF(qBound(0.0, tintOpacity, 1.0) * kMaxTintAlpha);
    QPainter tp(&layer);
    tp.fillRect(layer.rect(), tint);
    tp.end();
  }

  // Feather the band's own bottom edge: the sharp, unblurred view
  // continues directly below r (same content, no offset), so a hard
  // cutoff between "blurred" above and "sharp" below reads as a seam.
  // Fading this layer's alpha to 0 over its last few pixels lets the
  // real content underneath take over gradually instead.
  const int featherHeight = qMin(16, r.height());
  if(featherHeight > 0) {
    QPainter fp(&layer);
    fp.setCompositionMode(QPainter::CompositionMode_DestinationIn);
    QLinearGradient fade(0, r.height() - featherHeight, 0, r.height());
    fade.setColorAt(0, QColor(255, 255, 255, 255));
    fade.setColorAt(1, QColor(255, 255, 255, 0));
    fp.fillRect(QRect(0, r.height() - featherHeight, r.width(), featherHeight), fade);
    fp.end();
  }

  QPainterPath clip;
  if(cornerRadius > 0 && (roundLeft || roundRight)) {
    clip.moveTo(roundLeft ? cornerRadius : 0, 0);
    clip.lineTo(roundRight ? r.width() - cornerRadius : r.width(), 0);
    if(roundRight)
      clip.arcTo(r.width() - 2 * cornerRadius, 0, 2 * cornerRadius, 2 * cornerRadius, 90, -90);
    clip.lineTo(r.width(), r.height());
    clip.lineTo(0, r.height());
    if(roundLeft)
      clip.arcTo(0, 0, 2 * cornerRadius, 2 * cornerRadius, 180, -90);
    clip.closeSubpath();
  } else {
    clip.addRect(0, 0, r.width(), r.height());
  }
  p.save();
  p.translate(r.topLeft());
  p.setClipPath(clip);
  p.drawPixmap(0, 0, layer);
  p.restore();
}

// Edge resize for the frameless window: presses in the reserved 5px strips
// (see setContentsMargins() in the constructor) land on the MainWindow
// itself, everything else is covered by children. startSystemResize()
// delegates the actual resize to the compositor -- the only reliable way
// on Wayland, same reasoning as startSystemMove() in eventFilter().
void MainWindow::mousePressEvent(QMouseEvent* event) {
  if(event->button() == Qt::LeftButton && windowHandle() && !isMaximized()) {
    const int grip = 6;
    Qt::Edges edges;
    if(event->x() <= grip)
      edges |= Qt::LeftEdge;
    if(event->x() >= width() - grip)
      edges |= Qt::RightEdge;
    if(event->y() >= height() - grip)
      edges |= Qt::BottomEdge;
    if(event->y() <= grip)
      edges |= Qt::TopEdge;
    if(edges) {
      windowHandle()->startSystemResize(edges);
      return;
    }
  }
  QMainWindow::mousePressEvent(event);
}

// See the declaration in mainwindow.h.
QColor MainWindow::mainAreaTintColor() const {
  Settings& settings = static_cast<Application*>(qApp)->settings();
  qreal mainWindowTintAmount = settings.tintWindow() ? settings.mainWindowTint() / 100.0 : 0.0;
  QColor base = palette().color(QPalette::Window);
  return QColor::fromRgbF(
      base.redF()   + (sidebarTint_.redF()   - base.redF())   * mainWindowTintAmount,
      base.greenF() + (sidebarTint_.greenF() - base.greenF()) * mainWindowTintAmount,
      base.blueF()  + (sidebarTint_.blueF()  - base.blueF())  * mainWindowTintAmount);
}

// Custom window chrome: rounded outline, frosted (translucent) sidebar
// column, opaque content area. The native surface has an alpha channel
// (WA_TranslucentBackground), so whatever this doesn't paint stays
// genuinely transparent -- that's what makes the rounded corners real
// instead of drawn-on.
void MainWindow::paintEvent(QPaintEvent* event) {
  Q_UNUSED(event);
  QPainter p(this);
  p.setRenderHint(QPainter::Antialiasing);
  // Square corners when maximized, and also when KWin decorates the window
  // (DMG view mode drops FramelessWindowHint -- rounded corners on top of a
  // real decoration would clip into its titlebar).
  const bool frameless = windowFlags() & Qt::FramelessWindowHint;
  const qreal radius = (isMaximized() || !frameless) ? 0 : kWindowCornerRadius;
  QPainterPath winPath;
  winPath.addRoundedRect(rect(), radius, radius);
  int colRight = sidebarColumnRight(ui.sidePane, this);
  QPainterPath colPath;
  colPath.addRect(0, 0, colRight, height());
  // Frosted column: same tint the sidebar QSS used to carry, now painted
  // here so it spans the full window height like Finder's. Slightly darker
  // than the content area (like the reference). Fully opaque -- a partial
  // alpha here let KWin's blur-behind region (see updateSidebarBlurRegion())
  // show faint traces of the actual desktop wallpaper through it, which
  // reads as a stray diagonal smear rather than a clean frosted panel,
  // especially now that the toolbar band above it goes fully transparent
  // at rest and no longer masks it with an extra layer.
  // Blend in a hint of the wallpaper color behind the sidebar (see
  // updateSidebarWallpaperTint()) -- subtle, macOS-vibrancy-style, not a wash.
  auto blendTint = [](const QColor& base, const QColor& tint, qreal amount) {
    return QColor::fromRgbF(
        base.redF()   + (tint.redF()   - base.redF())   * amount,
        base.greenF() + (tint.greenF() - base.greenF()) * amount,
        base.blueF()  + (tint.blueF()  - base.blueF())  * amount);
  };
  Settings& settings = static_cast<Application*>(qApp)->settings();
  qreal sidebarTintAmount = settings.tintWindow() ? settings.sidebarTint() / 100.0 : 0.0;
  // Was a hardcoded dark grey, unlike mainAreaTintColor() right next to it --
  // the sidebar column never followed a light system theme, staying dark
  // even after switching to light mode. palette().color(QPalette::Window)
  // is the same live, theme-aware base mainAreaTintColor() already uses.
  QColor base = palette().color(QPalette::Window);
  QColor sidebarFill = blendTint(base, sidebarTint_, sidebarTintAmount);
  if(settings.transparency()) {
    // Lets KWin's blur-behind (see updateSidebarBlurRegion()) show through
    // the sidebar column -- transparencyPower() is how much (0 = opaque,
    // 100 = maximally see-through). Safe to use a real alpha here now that
    // there's no separate blurred band painted on top of this column at
    // scroll time (that combination used to read as a diagonal smear;
    // removed per user feedback -- the top blur band below is the only
    // blurred band left, and it's window-wide/scroll-independent).
    sidebarFill.setAlphaF(1.0 - (settings.transparencyPower() / 100.0));
  }
  p.fillPath(winPath.intersected(colPath), sidebarFill);
  // Rest of the window (toolbar/content area) gets the same wallpaper hint,
  // just much fainter -- it's not meant to read as its own vibrancy panel
  // like the sidebar, just to keep the whole chrome from feeling disconnected
  // from the sidebar's color when the window sits over strongly colored
  // wallpaper.
  p.fillPath(winPath.subtracted(colPath), mainAreaTintColor());

  // The top blur band used to be painted right here, but that runs before
  // ui.splitter (children always paint after their parent), so the actual
  // sidebar/content items -- painted afterward -- covered it completely;
  // it was invisible in practice except in the thin strip content doesn't
  // reach. It's now its own raised sibling widget (topBlurOverlay_, see the
  // constructor and its QEvent::Paint handling in eventFilter()), stacked
  // above ui.splitter but below ui.toolBar, so it actually reads as a
  // frosted veil over the items instead of sitting uselessly underneath
  // them.

  // Hairline separator below the toolbar, stopping at the sidebar column
  // instead of running across it. The breadcrumb bar's own matching
  // hairline is drawn in statusBarOverlay_'s own paint handling instead
  // (see eventFilter()) -- painted here, it would sit on that widget's own
  // top row and get covered by its fillRect() every repaint, since
  // children always paint after their parent.
  QColor hairline(127, 127, 127, 60);
  int lineLeft = qMax(colRight, 0);
  if(ui.toolBar && ui.toolBar->isVisible() && toolbarHairlineOpacity_ > 0.0) {
    int y = ui.toolBar->geometry().bottom();
    QColor fadedHairline = hairline;
    fadedHairline.setAlphaF(hairline.alphaF() * toolbarHairlineOpacity_);
    p.fillRect(QRect(lineLeft, y, width() - lineLeft, 1), fadedHairline);
  }

  // The window-outline hairline stroke used to be drawn right here, last,
  // on the theory that "last in paintEvent()" meant "on top of everything"
  // -- but children always paint after their *parent's own* paintEvent()
  // regardless of where in that function the drawing happens, so any
  // raised child reaching exactly to the window's edge (statusBarOverlay_'s
  // bottom, ui.toolBar's top corners) still painted over it there, leaving
  // visible gaps in the outline. It's now windowStrokeOverlay_, a real
  // raised sibling kept above every other child -- see its own paint
  // handling in eventFilter() and the doc comment in mainwindow.h.
}

void MainWindow::loadBookmarksMenu() {
  GList *allBookmarks = fm_bookmarks_get_all(bookmarks);
  QAction *before = ui.actionAddToBookmarks;

  for (GList *l = allBookmarks; l; l = l->next) {
    FmBookmarkItem *item = reinterpret_cast<FmBookmarkItem *>(l->data);
    BookmarkAction *action = new BookmarkAction(item, ui.menu_Bookmarks);
    connect(action, &QAction::triggered, this,
            &MainWindow::onBookmarkActionTriggered);
    ui.menu_Bookmarks->insertAction(before, action);
  }

  ui.menu_Bookmarks->insertSeparator(before);
  g_list_free_full(allBookmarks, (GDestroyNotify)fm_bookmark_item_unref);
}

void MainWindow::onBookmarksChanged(FmBookmarks *bookmarks, MainWindow *pThis) {
  // delete existing items
  if (!pThis->ui.menu_Bookmarks)
    return;

  QList<QAction *> actions = pThis->ui.menu_Bookmarks->actions();
  QList<QAction *>::const_iterator it = actions.begin();
  QList<QAction *>::const_iterator last_it = actions.end() - 2;

  while (it != last_it) {
    QAction *action = *it;
    ++it;
    pThis->ui.menu_Bookmarks->removeAction(action);
  }

  pThis->loadBookmarksMenu();
}

void MainWindow::onBookmarkActionTriggered() {
  BookmarkAction *action = static_cast<BookmarkAction *>(sender());
  FmPath *path = action->path();
  if (path) {
    Application *app = static_cast<Application *>(qApp);
    Settings &settings = app->settings();
    switch (settings.bookmarkOpenMethod()) {
    case OpenInCurrentTab: /* current tab */
    default:
      chdir(path);
      break;
    case OpenInNewTab: /* new tab */
      addTab(path);
      break;
    case OpenInNewWindow: /* new window */
      (new MainWindow(path))->show();
      break;
    }
  }
}

void MainWindow::on_actionCopy_triggered() {
  TabPage *page = currentPage();
  FmPathList *paths = page->selectedFilePaths();
  copyFilesToClipboard(paths);
  fm_path_list_unref(paths);
}

void MainWindow::on_actionCut_triggered() {
  TabPage *page = currentPage();
  FmPathList *paths = page->selectedFilePaths();
  cutFilesToClipboard(paths);
  fm_path_list_unref(paths);
}

void MainWindow::on_actionPaste_triggered() {
  pasteFilesFromClipboard(currentPage()->path(), this);
}

void MainWindow::on_actionDuplicate_triggered() {
  on_actionCopy_triggered();
  on_actionPaste_triggered();
}

void MainWindow::on_actionEmptyTrash_triggered() { Fm::Trash::emptyTrash(); }

void MainWindow::on_actionDelete_triggered() {
  Application *app = static_cast<Application *>(qApp);
  Settings &settings = app->settings();
  TabPage *page = currentPage();
  FmPathList *paths = page->selectedFilePaths();
  FileOperation::trashFiles(paths, settings.confirmTrash(), this);
  fm_path_list_unref(paths);
}

void MainWindow::on_actionDeleteWithoutTrash_triggered() {
  Application *app = static_cast<Application *>(qApp);
  Settings &settings = app->settings();
  TabPage *page = currentPage();
  FmPathList *paths = page->selectedFilePaths();
  FileOperation::deleteFiles(paths, settings.confirmDelete(), this);
  fm_path_list_unref(paths);
}

void MainWindow::on_actionRename_triggered() {
  TabPage *page = currentPage();
  if (page && page->folderView() && page->folderView()->childView()) {
      QModelIndexList selected = page->folderView()->childView()->selectionModel()->selectedIndexes();
      if (!selected.isEmpty()) {
          page->folderView()->childView()->edit(selected.first());
          return;
      }
  }

  FmFileInfoList *files = page->selectedFiles();

  for (GList *l = fm_file_info_list_peek_head_link(files); l; l = l->next) {
    FmFileInfo *file = FM_FILE_INFO(l->data);
    Fm::renameFile(file, NULL);
  }
  fm_file_info_list_unref(files);
}

void MainWindow::on_actionQuickLook_triggered() {
  // Explicit user request (Space press / menu item): toggle. Pressing Space
  // again on the same already-shown file must always close it -- see the
  // identical DesktopWindow::onQuickLookActivated()/showOrRefreshQuickLook()
  // split in desktopwindow.cpp, mirrored here.
  showOrRefreshQuickLook(true);
}

void MainWindow::showOrRefreshQuickLook(bool allowToggleClose) {
  TabPage* page = currentPage();
  if (!page) return;
  FmFileInfoList* files = page->selectedFiles();
  if (files && fm_file_info_list_get_length(files) > 0) {
      FmFileInfo* info = FM_FILE_INFO(fm_file_info_list_peek_head_link(files)->data);

      QRect sourceRect = geometry();
      if (page->folderView()) {
          QAbstractItemView* itemView = qobject_cast<QAbstractItemView*>(page->folderView()->childView());
          if (itemView) {
              QModelIndexList selected = itemView->selectionModel()->selectedIndexes();
              if (!selected.isEmpty()) {
                  QRect rect = itemView->visualRect(selected.first());
                  sourceRect = QRect(itemView->viewport()->mapToGlobal(rect.topLeft()), rect.size());
              }
          }
      }

      QuickLookDialog* existing = QuickLookDialog::activeInstance();
      QString path = QString::fromUtf8(fm_path_to_str(fm_file_info_get_path(info)));
      if (existing && allowToggleClose && existing->currentPath() == path) {
          existing->closeWithAnimation();
      } else if (existing) {
          existing->refreshWith(info, sourceRect);
      } else {
          QuickLookDialog* dialog = new QuickLookDialog(info, sourceRect, this);
          dialog->show();
      }
  }
  if (files) fm_file_info_list_unref(files);
}

void MainWindow::on_actionSelectAll_triggered() { currentPage()->selectAll(); }

void MainWindow::on_actionInvertSelection_triggered() {
  currentPage()->invertSelection();
}

void MainWindow::on_actionPreferences_triggered() {
  Application *app = reinterpret_cast<Application *>(qApp);
  app->preferences(QString());
}

void MainWindow::onBackForwardContextMenu(QPoint pos) {
  // show a popup menu for browsing history here.
  QToolButton *btn = static_cast<QToolButton *>(sender());
  TabPage *page = currentPage();
  Fm::BrowseHistory &history = page->browseHistory();
  int current = history.currentIndex();
  QMenu menu;
  for (int i = 0; i < history.size(); ++i) {
    const BrowseHistoryItem &item = history.at(i);
    Fm::Path path = item.path();
    QAction *action = menu.addAction(path.displayName());
    if (i == current) {
      // make the current path bold and checked
      action->setCheckable(true);
      action->setChecked(true);
      QFont font = menu.font();
      font.setBold(true);
      action->setFont(font);
    }
  }
  QAction *selectedAction = menu.exec(btn->mapToGlobal(pos));
  if (selectedAction) {
    int index = menu.actions().indexOf(selectedAction);
    filterBar->clear();
    page->jumpToHistory(index);
    updateUIForCurrentPage();
  }
}

void MainWindow::onRaiseWindow(const QString &path) {
  // all tab pages
  int n = ui.stackedWidget->count();
  for (int i = 0; i < n; ++i) {
    TabPage *page = static_cast<TabPage *>(ui.stackedWidget->widget(i));
    if (page) {
      QString ourPath = page->pathName();
      if (path == ourPath) {
        raise();
        activateWindow();
        // Bring Filer to front
        Display *dpy;
        dpy = XOpenDisplay(NULL);
        XRaiseWindow(dpy, effectiveWinId());
        XCloseDisplay(dpy);
        bool maximized = isMaximized();
        if (isMinimized()) {
          showNormal();
          if (maximized) // window was maximized before being minimized -
                         // showNormal restores it
            showMaximized();
        }
        break;
      }
    }
  }
}

void MainWindow::onCloseWindow(const QString &path) {
  // all tab pages
  int n = ui.stackedWidget->count();
  for (int i = 0; i < n; ++i) {
    TabPage *page = static_cast<TabPage *>(ui.stackedWidget->widget(i));
    if (page) {
      QString ourPath = page->pathName();
      if (path == ourPath) {
        page->close();
      }
    }
  }
  n = ui.stackedWidget->count();
  qDebug() << "Widgets left:" << n;
  if (n = 1) {
    // qDebug() << "No more tabs left. Now close the whole window";
    close();
  }
}

void MainWindow::onRaiseWindowAndSelectItems(const QString &path,
                                             const QStringList &items) {
  onRaiseWindow(path);
  TabPage *page = currentPage();
  if (page) {
    page->folderView()->selectFiles(items, false);
  }
}

void MainWindow::updateFromSettings(Settings &settings) {
  // apply settings

  updateToolbarStyleSheet();
  updateScrollBarStyleSheet();

  // side pane
  ui.sidePane->setIconSize(
      QSize(settings.sidePaneIconSize(), settings.sidePaneIconSize()));
  ui.sidePane->applySettings(settings);

  // Sidebar width (Settings::splitterPos()) -- only ever applied once, at
  // construction, before this; onSplitterMoved() keeps it saved whenever
  // the user drags the splitter by hand, but never the other way around,
  // so a config-file edit had no live effect. Re-set here the same way the
  // ctor does. QSplitter::setSizes() (unlike an actual drag) doesn't emit
  // splitterMoved(), so updateSidebarSpacerWidth()/updateSidebarBlurRegion()
  // -- which resync sidebarSpacer_/breadcrumbSpacer_ and the wallpaper-tint
  // blur region to the real resulting width -- are called explicitly here,
  // same as onSplitterMoved() does for a real drag.
  if(ui.splitter->sizes().value(0) != settings.splitterPos()) {
    QList<int> sizes = ui.splitter->sizes();
    int total = sizes.value(0) + sizes.value(1);
    sizes[0] = settings.splitterPos();
    sizes[1] = qMax(0, total - settings.splitterPos());
    ui.splitter->setSizes(sizes);
    updateSidebarBlurRegion();
    updateSidebarSpacerWidth();
  }

  // top blur band -- height/power/opacity/saturation/brightness/key color
  // are all live-reloadable (see Application::onTintSettingsFileChanged()).
  updateTopBlurOverlayGeometry();
  if(topBlurOverlay_)
    topBlurOverlay_->update();

  // All five toolbar groups' margin/height/width, plus the two global
  // icon-size settings.
  applyToolbarGroupSettings(settings);

  // Settings::breadcrumbIconSize(): updateBreadcrumb() rebuilds the trail
  // from scratch every time it runs, so a size-only change (no actual
  // navigation) needs an explicit re-run with the current path to pick it
  // up live.
  if(TabPage* page = currentPage())
    updateBreadcrumb(page->pathName());

  // Search bar expand/collapse threshold (Settings::searchBarExpandWidth()).
  updateSearchBarMode();

  // View-mode segmented-control collapse threshold (Settings::viewModeCollapseWidth()).
  updateViewModeCollapseMode();

  // tabs
  ui.tabBar->setTabsClosable(settings.showTabClose());

  // all tab pages
  int n = ui.stackedWidget->count();

  for (int i = 0; i < n; ++i) {
    TabPage *page = static_cast<TabPage *>(ui.stackedWidget->widget(i));
    page->updateFromSettings(settings);
  }

  // spatial mode
  ui.actionNewWin->setVisible(!settings.spatialMode()); // probono
  ui.actionGoUpAndCloseCurrentWindow->setVisible(
      settings.spatialMode()); // probono
  // Tab bar visibility: never shown in spatial mode; otherwise only when
  // there's more than one tab, unless AlwaysShowTabs overrides that. This
  // used to be two separate assignments -- an earlier unconditional
  // `setVisible(!settings.spatialMode())` right after the tab-count check
  // above always won (same function, runs later), forcing the tab bar
  // visible with a single tab open every time updateFromSettings() ran
  // (startup, and now also live settings reload).
  ui.tabBar->setVisible(!settings.spatialMode() &&
                         (settings.alwaysShowTabs() || ui.tabBar->count() > 1));
  ui.sidePane->setVisible(!settings.spatialMode());
  ui.toolBar->setVisible(!settings.spatialMode());
  // Left margin only here -- top is the toolbar-height reservation, live
  // only while the tab bar is visible (see updateToolbarOverlayGeometry()),
  // which this call reapplies now that the tab bar's own visibility above
  // is settled.
  QMargins m = ui.frame->layout()->contentsMargins();
  m.setLeft(settings.spatialMode() ? 0 : 1);
  m.setRight(0);
  m.setBottom(0);
  ui.frame->layout()->setContentsMargins(m);
  updateToolbarOverlayGeometry();
}

static const char *su_cmd_subst(char opt, gpointer user_data) {
  return (const char *)user_data;
}

static FmAppCommandParseOption su_cmd_opts[] = {{'s', su_cmd_subst}, {0, NULL}};

void MainWindow::on_actionOpenAsRoot_triggered() {
  TabPage *page = currentPage();

  if (page) {
    Application *app = static_cast<Application *>(qApp);
    Settings &settings = app->settings();

    if (!settings.suCommand().isEmpty()) {
      // run the su command
      // FIXME: it's better to get the filename of the current process rather
      // than hard-code filer-qt here.
      QByteArray suCommand = settings.suCommand().toLocal8Bit();
      char *cmd = NULL;
      QByteArray programCommand = app->applicationFilePath().toLocal8Bit();
      programCommand += " %U";

      if (fm_app_command_parse(suCommand.constData(), su_cmd_opts, &cmd,
                               gpointer(programCommand.constData())) == 0) {
        /* no %s found so just append to it */
        g_free(cmd);
        cmd = g_strconcat(suCommand.constData(), programCommand.constData(),
                          NULL);
      }

      GAppInfo *appInfo = g_app_info_create_from_commandline(
          cmd, NULL, GAppInfoCreateFlags(0), NULL);
      g_free(cmd);

      if (appInfo) {
        FmPath *cwd = page->path();
        GError *err = NULL;
        char *uri = fm_path_to_uri(cwd);
        GList *uris = g_list_prepend(NULL, uri);

        if (!g_app_info_launch_uris(appInfo, uris, NULL, &err)) {
          QMessageBox::critical(this, tr("Error"),
                                QString::fromUtf8(err->message));
          g_error_free(err);
        }

        g_list_free(uris);
        g_free(uri);
        g_object_unref(appInfo);
      }
    } else {
      // show an error message and ask the user to set the command
      QMessageBox::critical(this, tr("Error"),
                            tr("Switch user command is not set."));
      app->preferences("advanced");
    }
  }
}

void MainWindow::on_actionFindFiles_triggered() {
  Application *app = static_cast<Application *>(qApp);
  FmPathList *selectedPaths = currentPage()->selectedFilePaths();
  QStringList paths;
  if (selectedPaths) {
    for (GList *l = fm_path_list_peek_head_link(selectedPaths); l;
         l = l->next) {
      // FIXME: is it ok to use display name here?
      // This might be broken on filesystems with non-UTF-8 filenames.
      Fm::Path path(FM_PATH(l->data));
      paths.append(path.displayName(false));
    }
    fm_path_list_unref(selectedPaths);
  } else {
    paths.append(currentPage()->pathName());
  }
  app->findFiles(paths);
}

void MainWindow::on_actionOpenTerminal_triggered() {
  TabPage *page = currentPage();
  if (page) {
    Application *app = static_cast<Application *>(qApp);
    app->openFolderInTerminal(page->path());
  }
}

void MainWindow::onShortcutNextTab() {
  int current = ui.tabBar->currentIndex();
  if (current < ui.tabBar->count() - 1)
    ui.tabBar->setCurrentIndex(current + 1);
  else
    ui.tabBar->setCurrentIndex(0);
}

void MainWindow::onShortcutPrevTab() {
  int current = ui.tabBar->currentIndex();
  if (current > 0)
    ui.tabBar->setCurrentIndex(current - 1);
  else
    ui.tabBar->setCurrentIndex(ui.tabBar->count() - 1);
}

// Switch to nth tab when Alt+n or Ctrl+n is pressed
void MainWindow::onShortcutJumpToTab() {
  QShortcut *shortcut = reinterpret_cast<QShortcut *>(sender());
  QKeySequence seq = shortcut->key();
  int keyValue = seq[0];
  // See the source code of QKeySequence and refer to the method:
  // QString QKeySequencePrivate::encodeString(int key,
  // QKeySequence::SequenceFormat format). Then we know how to test if a key
  // sequence contains a modifier. It's a shame that Qt has no API for this
  // task.

  if ((keyValue & Qt::ALT) == Qt::ALT) // test if we have Alt key pressed
    keyValue -= Qt::ALT;
  else if ((keyValue & Qt::CTRL) ==
           Qt::CTRL) // test if we have Ctrl key pressed
    keyValue -= Qt::CTRL;

  // now keyValue should contains '0' - '9' only
  int index;
  if (keyValue == '0')
    index = 9;
  else
    index = keyValue - '1';
  if (index < ui.tabBar->count())
    ui.tabBar->setCurrentIndex(index);
}

void MainWindow::updateSidebarSpacerWidth() {
  int targetWidth = sidebarColumnRight(ui.sidePane, this);
  if (sidebarSpacer_) {
    if (targetWidth > 0) {
      sidebarSpacer_->setFixedWidth(targetWidth);
    } else {
      sidebarSpacer_->setFixedWidth(86);
    }
  }
  // Keeps the breadcrumb trail starting exactly where the content area
  // begins (same x as the back/forward buttons above it), not at the
  // status bar's left edge under the sidebar.
  if (breadcrumbSpacer_) {
    breadcrumbSpacer_->setFixedWidth(targetWidth > 0 ? targetWidth : 86);
  }
}

} // namespace Filer
