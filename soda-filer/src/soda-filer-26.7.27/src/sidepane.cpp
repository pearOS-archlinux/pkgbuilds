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


#include "sidepane.h"
#include <QComboBox>
#include <QVBoxLayout>
#include <QHeaderView>
#include "placesview.h"
#include "dirtreeview.h"
#include "dirtreemodel.h"
#include "path.h"
#include "filemenu.h"
#include <QScrollBar>
#include <QApplication>
#include <QProxyStyle>
#include <QTimer>
#include <QAbstractItemView>
#include "settings.h"

namespace Fm {

// Adds a configurable gap between a sidebar item's icon and its text label
// -- there's no QSS property for this (QStyleSheetStyle computes the
// decoration/text split itself once any ::item rule is active, so it's not
// something a stylesheet can reach), so it's done via the one mechanism
// Qt exposes for it: overriding the style's subElementRect().
class ItemStyle : public QProxyStyle {
public:
  explicit ItemStyle(int spacing = 0): QProxyStyle(), spacing_(spacing) {}
  void setSpacing(int spacing) { spacing_ = spacing; }
  QRect subElementRect(SubElement element, const QStyleOption* option, const QWidget* widget) const override {
    QRect rect = QProxyStyle::subElementRect(element, option, widget);
    if(element == QStyle::SE_ItemViewItemText && spacing_ != 0)
      rect.setLeft(rect.left() + spacing_);
    return rect;
  }
private:
  int spacing_;
};

// Sidebar scrollbar visibility (see Filer::SidePaneScrollBarMode). Only the
// vertical scrollbar is ever shown -- the sidebar's width is fixed, so a
// horizontal one never makes sense.
static const char* const kScrollHideTimerName = "sidePaneScrollHideTimer";

static void applyScrollBarMode(QAbstractItemView* view, int mode) {
  view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

  // Tear down any previous "while scrolling" wiring first: this function
  // gets called repeatedly on the same view (every live settings reload,
  // not just once at creation), so without this each call would stack
  // another QTimer + valueChanged connection on top of the last one.
  QTimer* existingTimer = view->findChild<QTimer*>(QLatin1String(kScrollHideTimerName));
  if(existingTimer) {
    QObject::disconnect(view->verticalScrollBar(), &QAbstractSlider::valueChanged, view, nullptr);
    delete existingTimer;
  }

  switch(mode) {
    case Filer::ScrollBarAlways:
      view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
      break;
    case Filer::ScrollBarWhileScrolling: {
      view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
      // Flash the scrollbar on scroll, then hide it again after a short
      // idle period -- the timer/connections are parented to the view so
      // they're torn down automatically when SidePane::setMode() replaces it.
      QTimer* hideTimer = new QTimer(view);
      hideTimer->setObjectName(QLatin1String(kScrollHideTimerName));
      hideTimer->setSingleShot(true);
      hideTimer->setInterval(800);
      QObject::connect(hideTimer, &QTimer::timeout, view, [view]() {
        view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
      });
      QObject::connect(view->verticalScrollBar(), &QAbstractSlider::valueChanged, view, [view, hideTimer](int) {
        view->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        hideTimer->start();
      });
      break;
    }
    case Filer::ScrollBarAuto:
    default:
      // Native "as needed" -- on styles/platforms with transient/overlay
      // scrollbars (e.g. mouse shows a persistent bar, trackpad doesn't),
      // this is where that distinction is actually made, below Qt's level.
      view->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
      break;
  }
}

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
                                 const QColor& selectorColor, int selectorOpacity) {
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
    // Horizontal margin is 0 here -- the selection pill's actual left/right
    // bounds are computed by PlacesView's ItemDelegate instead (so the
    // right edge specifically can be independent, see
    // Settings::sidePaneSelectorRightInset()); this rule still supplies
    // the pill's vertical margin (item spacing) and its rounded shape.
    "QTreeView::item, QListView::item { border-radius: 8px; padding: 0px %2px; margin: %1px 0px; color: %4; font-size: 11px; }"
    // macOS selects sidebar rows with a neutral grey pill (the accent color
    // only tints the row's icon), not a filled accent-blue bar.
    "QTreeView::item:selected:enabled, QListView::item:selected:enabled { background-color: %3; color: %4; }"
    // No hover highlight in the sidebar -- only the selection pill above
    // should ever show a background.
    "QTreeView::item:hover:!selected:enabled, QListView::item:hover:!selected:enabled { background-color: transparent; }"
    "QTreeView::item:disabled, QListView::item:disabled { color: %5; background-color: transparent; }"
    "QTreeView::branch { background: transparent; }"
    // No custom QScrollBar rules here anymore -- the sidebar's scrollbar
    // now matches the folder view's (src/folderview.cpp has none either),
    // i.e. the native/system scrollbar rather than a custom thin
    // translucent handle.
  ).arg(itemSpacing).arg(horizontalPadding).arg(selectorRgba).arg(textRgba).arg(disabledTextRgba);
}

SidePane::SidePane(QWidget* parent):
  QWidget(parent),
  showHidden_(false),
  mode_(ModeNone),
  view_(NULL),
  combo_(NULL),
  currentPath_(NULL),
  iconSize_(24, 24) {

  verticalLayout = new QVBoxLayout(this);
  verticalLayout->setContentsMargins(0, 0, 0, 0);



  combo_ = new QComboBox(this);
  combo_->setFrame(false);
  combo_->addItem(tr("Places"));
  combo_->addItem(tr("Directory Tree"));
  connect(combo_, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &SidePane::onComboCurrentIndexChanged);
  // verticalLayout->addWidget(combo_); // probono: Hide for now. TODO: Move this setting to the menu bar
  // Not in the layout, but a child widget with no layout still paints at
  // (0,0) of its parent once shown -- it floated over the sidebar's first
  // rows as a phantom dropdown. Hide it explicitly (it still works as the
  // places/dirtree mode switch programmatically via setMode()).
  combo_->hide();

  itemStyle_ = new ItemStyle(iconTextSpacing_);
}

SidePane::~SidePane() {
  if(currentPath_)
    fm_path_unref(currentPath_);
  delete itemStyle_;
  // qDebug("delete SidePane");
}

void SidePane::onPlacesViewChdirRequested(int type, FmPath* path) {
  Q_EMIT chdirRequested(type, path);
}

void SidePane::onDirTreeViewChdirRequested(int type, FmPath* path) {
  Q_EMIT chdirRequested(type, path);
}

void SidePane::onComboCurrentIndexChanged(int current) {
  if(current != mode_) {
    setMode(Mode(current));
  }
}

void SidePane::setIconSize(QSize size) {
  iconSize_ = size;
  switch(mode_) {
    case ModePlaces:
      static_cast<PlacesView*>(view_)->setIconSize(size);
      break;
    case ModeDirTree:
      static_cast<QTreeView*>(view_)->setIconSize(size);
      break;
    default:;
  }
}

void SidePane::setCurrentPath(FmPath* path) {
  Q_ASSERT(path != NULL);
  if(currentPath_)
    fm_path_unref(currentPath_);
  currentPath_ = fm_path_ref(path);
  QString styleSheet = buildSidePaneStyleSheet(itemSpacing_, horizontalPadding_, fontWeight_, selectorColor_, selectorOpacity_);
  QFont itemFont = sidePaneItemFont(fontWeight_, itemFontSize_);
  switch(mode_) {
    case ModePlaces: {
      PlacesView* pv = static_cast<PlacesView*>(view_);
      pv->setCurrentPath(path);
      pv->setFrameStyle(QFrame::NoFrame); // probono: No border
      pv->setStyleSheet(styleSheet); // FIXME: Get colors from theme, do not hardcode
      pv->setFont(itemFont);
      pv->viewport()->setAutoFillBackground(false);
      pv->setFocusPolicy(Qt::NoFocus);
      pv->horizontalScrollBar()->setStyle(qApp->style());
      pv->verticalScrollBar()->setStyle(qApp->style());
      pv->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
      pv->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
      break;
    }
    case ModeDirTree: {
      DirTreeView* dtv = static_cast<DirTreeView*>(view_);
      dtv->setCurrentPath(path);
      dtv->setFrameStyle(QFrame::NoFrame); // probono: No border
      dtv->setStyleSheet(styleSheet); // FIXME: Get colors from theme, do not hardcode
      dtv->setFont(itemFont);
      dtv->viewport()->setAutoFillBackground(false);
      dtv->setFocusPolicy(Qt::NoFocus);
      dtv->horizontalScrollBar()->setStyle(qApp->style());
      dtv->verticalScrollBar()->setStyle(qApp->style());
      dtv->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
      dtv->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
      break;
    }
    default:;
  }
}

void SidePane::applySettings(Filer::Settings& settings) {
  itemSpacing_ = settings.sidePaneItemSpacing();
  horizontalPadding_ = settings.sidePaneItemHorizontalPadding();
  fontWeight_ = settings.sidePaneFontWeight();
  itemFontSize_ = settings.sidePaneItemFontSize();
  categoryFontSize_ = settings.sidePaneCategoryFontSize();
  categoryLeftPadding_ = settings.sidePaneCategoryLeftPadding();
  iconTextSpacing_ = settings.sidePaneIconTextSpacing();
  categorySpacingTop_ = settings.sidePaneCategorySpacing();
  categorySpacingBottom_ = settings.sidePaneCategorySpacingBottom();
  selectorRightInset_ = settings.sidePaneSelectorRightInset();
  selectorColor_ = settings.sidePaneSelectorColor();
  selectorOpacity_ = settings.sidePaneSelectorOpacity();
  scrollBarMode_ = settings.sidePaneScrollBarMode();

  QString styleSheet = buildSidePaneStyleSheet(itemSpacing_, horizontalPadding_, fontWeight_, selectorColor_, selectorOpacity_);
  QFont itemFont = sidePaneItemFont(fontWeight_, itemFontSize_);
  itemStyle_->setSpacing(iconTextSpacing_);
  switch(mode_) {
    case ModePlaces: {
      PlacesView* pv = static_cast<PlacesView*>(view_);
      pv->setStyleSheet(styleSheet);
      pv->setFont(itemFont);
      pv->setIconTextSpacing(iconTextSpacing_);
      pv->setCategorySpacing(categorySpacingTop_, categorySpacingBottom_);
      pv->setCategoryFontSize(categoryFontSize_);
      pv->setCategoryLeftPadding(categoryLeftPadding_);
      pv->setSelectorRightInset(selectorRightInset_);
      applyScrollBarMode(pv, scrollBarMode_);
      // Called on every live settings/theme reload (see
      // Application::onTintSettingsFileChanged()/onSystemThemeConfigChanged()),
      // so an accent-color change re-tints these without needing the window
      // recreated -- see PlacesModel::updateIcons()'s doc comment.
      pv->updateAccentIcons();
      pv->style()->polish(pv);
      pv->update();
      break;
    }
    case ModeDirTree: {
      DirTreeView* dtv = static_cast<DirTreeView*>(view_);
      dtv->setStyleSheet(styleSheet);
      dtv->setFont(itemFont);
      applyScrollBarMode(dtv, scrollBarMode_);
      dtv->style()->polish(dtv);
      dtv->update();
      break;
    }
    default:;
  }
}

void SidePane::updateAccentIcons() {
  if(mode_ == ModePlaces) {
    static_cast<PlacesView*>(view_)->updateAccentIcons();
  }
}

SidePane::Mode SidePane::modeByName(const char* str) {
  if(str == NULL)
    return ModeNone;
  if(strcmp(str, "places") == 0)
    return ModePlaces;
  if(strcmp(str, "dirtree") == 0)
    return ModeDirTree;
  return ModeNone;
}

const char* SidePane::modeName(SidePane::Mode mode) {
  switch(mode) {
  case ModePlaces:
    return "places";
  case ModeDirTree:
    return "dirtree";
  default:
    return NULL;
  }
}

#if 0 // FIXME: are these APIs from libfm-qt needed?

QString SidePane::modeLabel(SidePane::Mode mode) {
  switch(mode) {
  case ModePlaces:
    return tr("Places");
  case ModeDirTree:
    return tr("Directory Tree");
  }
  return QString();
}

QString SidePane::modeTooltip(SidePane::Mode mode) {
  switch(mode) {
  case ModePlaces:
    return tr("Shows list of common places, devices, and bookmarks in sidebar");
  case ModeDirTree:
    return tr("Shows tree of directories in sidebar");
  }
  return QString();
}
#endif

bool SidePane::setHomeDir(const char* home_dir) {
  if(view_ == NULL)
    return false;
  // TODO: SidePane::setHomeDir

  switch(mode_) {
  case ModePlaces:
    // static_cast<PlacesView*>(view_);
    return true;
  case ModeDirTree:
    // static_cast<PlacesView*>(view_);
    return true;
  default:;
  }
  return true;
}

void SidePane::initDirTree() {
  // TODO
  DirTreeModel* model = new DirTreeModel(view_);
  FmFileInfoJob* job = fm_file_info_job_new(NULL, FM_FILE_INFO_JOB_NONE);
  model->setShowHidden(showHidden_);

  GList* l;
  /* query FmFileInfo for home dir and root dir, and then,
    * add them to dir tree model */
  fm_file_info_job_add(job, fm_path_get_home());
  fm_file_info_job_add(job, fm_path_get_root());
  /* FIXME: maybe it's cleaner to use run_async here? */
  fm_job_run_sync_with_mainloop(FM_JOB(job));
  for(l = fm_file_info_list_peek_head_link(job->file_infos); l; l = l->next) {
      FmFileInfo* fi = FM_FILE_INFO(l->data);
      model->addRoot(fi);
  }
  g_object_unref(job);

  static_cast<DirTreeView*>(view_)->setModel(model);
}

void SidePane::setMode(Mode mode) {
  if(mode == mode_)
    return;

  if(view_) {
    delete view_;
    view_ = NULL;
    //if(sp->update_popup)
    //  g_signal_handlers_disconnect_by_func(sp->view, on_item_popup, sp);
  }
  mode_ = mode;

  combo_->setCurrentIndex(mode);
  switch(mode) {
  case ModePlaces: {
    PlacesView* placesView = new Fm::PlacesView(this);
    view_ = placesView;
    placesView->setIconSize(iconSize_);
    placesView->setCategorySpacing(categorySpacingTop_, categorySpacingBottom_);
    placesView->setCategoryFontSize(categoryFontSize_);
    placesView->setCategoryLeftPadding(categoryLeftPadding_);
    placesView->setSelectorRightInset(selectorRightInset_);
    placesView->setIconTextSpacing(iconTextSpacing_);
    placesView->setCurrentPath(currentPath_);
    connect(placesView, &PlacesView::chdirRequested, this, &SidePane::onPlacesViewChdirRequested);
    break;
  }
  case ModeDirTree: {
    DirTreeView* dirTreeView = new Fm::DirTreeView(this);
    view_ = dirTreeView;
    initDirTree();
    dirTreeView->setIconSize(iconSize_);
    dirTreeView->setCurrentPath(currentPath_);
    connect(dirTreeView, &DirTreeView::chdirRequested, this, &SidePane::onDirTreeViewChdirRequested);
    connect(dirTreeView, &DirTreeView::openFolderInNewWindowRequested,
            this, &SidePane::openFolderInNewWindowRequested);
    connect(dirTreeView, &DirTreeView::openFolderInNewTabRequested,
            this, &SidePane::openFolderInNewTabRequested);
    connect(dirTreeView, &DirTreeView::openFolderInTerminalRequested,
            this, &SidePane::openFolderInTerminalRequested);
    connect(dirTreeView, &DirTreeView::createNewFolderRequested,
            this, &SidePane::createNewFolderRequested);
    connect(dirTreeView, &DirTreeView::prepareFileMenu,
            this, &SidePane::prepareFileMenu);
    break;
  }
  default:;
  }
  if(view_) {
    // if(sp->update_popup)
    //  g_signal_connect(sp->view, "item-popup", G_CALLBACK(on_item_popup), sp);
    verticalLayout->addWidget(view_);
    QAbstractItemView* itemView = qobject_cast<QAbstractItemView*>(view_);
    if(itemView) {
      itemView->setStyle(itemStyle_);
      applyScrollBarMode(itemView, scrollBarMode_);
    }
  }
  Q_EMIT modeChanged(mode);
}

void SidePane::setShowHidden(bool show_hidden) {
  if(view_ == NULL || show_hidden == showHidden_)
    return;
  showHidden_ = show_hidden;
  if(mode_ == ModeDirTree) {
    DirTreeView* dirTreeView = static_cast<DirTreeView*>(view_);
    DirTreeModel* model = static_cast<DirTreeModel*>( dirTreeView->model());
    if(model)
      model->setShowHidden(showHidden_);
  }
}

} // namespace Fm
