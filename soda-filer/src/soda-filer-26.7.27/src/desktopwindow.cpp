#include <QDialog>
#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QDateTime>
#include <QGuiApplication>
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

#include "desktopwindow.h"
#include "dmgmountregistry.h"
#include <QFileInfo>
#include <QWidget>
#include <QDesktopWidget>
#include <QPainter>
#include <QImage>
#include <QImageReader>
#include <QFile>
#include <QMainWindow>
#include <QMenuBar>
#include <QPixmap>
#include <QPalette>
#include <QBrush>
#include <QLayout>
#include <QDebug>
#include <QTimer>
#include <QSettings>
#include <QStringBuilder>
#include "quicklookdialog.h"
#include <QDir>
#include <QShortcut>
#include <QDropEvent>
#include <QMimeData>
#include <QVariantAnimation>
#include <QEasingCurve>
#include <QCursor>
#include <QSignalBlocker>
#include <QPointer>
#include <QVector>
#include <QPair>
#include <algorithm>
#include <cstring>

#include "./application.h"
#include "mainwindow.h"
#include "desktopitemdelegate.h"
#include "foldermenu.h"
#include "filemenu.h"
#include "foldermodel.h"
#include "folderview_p.h"
#include "fileoperation.h"
#include "filepropsdialog.h"
#include "utilities.h"
#include "path.h"
#include "xdgdir.h"
#include "desktopmainwindow.h"
#include "path.h"
#include "utilities.h"
#include "windowregistry.h"
#include "ui_about.h"
#include "tabpage.h"
#include "trash.h"
#include "viewoptionspopup.h"
#include "tags.h"

#include <QX11Info>
#include <QScreen>
#include <QStandardPaths>
#include <QProcess>
#include <QApplication>
#include <QStorageInfo>
#include <xcb/xcb.h>
#include <X11/Xlib.h>
#include <KWindowSystem>
#include <netwm.h>

using namespace Filer;

DesktopWindow::DesktopWindow(int screenNum):
    View(Fm::FolderView::IconMode),
    screenNum_(screenNum),
    folder_(NULL),
    model_(NULL),
    proxyModel_(NULL),
    listView_(NULL),
    fileLauncher_(NULL),
    wallpaperMode_(WallpaperNone),
    relayoutTimer_(NULL),
    desktopArrangeColumn_(ArrangeNone),
    desktopIconSize_(64),
    desktopGridSpacing_(120),
    desktopTextSize_(12),
    desktopLabelRight_(false),
    desktopShowItemInfo_(false),
    desktopShowIconPreview_(false),
    desktopMainWindow_(NULL){

    QDesktopWidget* desktopWidget = QApplication::desktop();
    setObjectName("DesktopWindow");
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_X11NetWmWindowTypeDesktop);
    setAttribute(Qt::WA_DeleteOnClose);
    // updateWallpaper() paints the wallpaper via stylesheet background-color/
    // background-image/border-image on this widget (#DesktopWindow). QWidget
    // (unlike QFrame) never renders its own stylesheet background unless this
    // attribute is set -- without it the wallpaper silently fails to show.
    setAttribute(Qt::WA_StyledBackground);

    // Give Filer an icon, e.g., in the Windows menu
    // DesktopWindow->setIcon(QIcon::fromTheme("desktop")); // This does not work because DesktopWindow is not a QWindow subclass
    qApp->setWindowIcon(QIcon::fromTheme("desktop"));

    // probono: Show wallpaper immediately (before constructing the icons on the Desktop)
    Settings& settings = static_cast<Application* >(qApp)->settings();
    setWallpaperFile(settings.wallpaper());
    setWallpaperMode(settings.wallpaperMode());
    setBackground(settings.desktopBgColor());
    updateWallpaper();
    update();
    this->show();
    KWindowSystem::setType(winId(), NET::Desktop);
    KWindowSystem::setState(winId(), NET::SkipTaskbar | NET::SkipPager | NET::SkipSwitcher);

    // set our custom file launcher
    View::setFileLauncher(&fileLauncher_);

    listView_ = static_cast<Fm::FolderViewListView*>(childView());
    listView_->setMovement(QListView::Free);
    // In Qt5, QAbstractItemView doesn't consult QStyle::pixelMetric() for its
    // drag-start threshold -- it reads QApplication::startDragDistance()
    // directly (a single global, ~10px by default). Left at the default, a
    // deliberate tiny nudge of a desktop icon (e.g. by a millimeter) never
    // even registers as a drag, so "free" arrangement wasn't actually free.
    // This is applied application-wide (see application.cpp's constructor)
    // since Qt5 offers no per-widget override for it.
    listView_->setResizeMode(QListView::Adjust);
    listView_->setFlow(QListView::TopToBottom);
    // Qt's default edit triggers include DoubleClicked, and QAbstractItemView
    // starts an item's rename edit BEFORE emitting activated() on a double
    // click -- when edit() succeeds (which it always does here, since name
    // cells carry Qt::ItemIsEditable), activated() never fires at all, so
    // double-clicking a desktop icon renamed it instead of opening it.
    // No SelectedClicked either: clicking an already-selected item must not
    // start a rename. F2/Enter still work via EditKeyPressed (see
    // FolderViewListView::keyPressEvent()).
    listView_->setEditTriggers(QAbstractItemView::EditKeyPressed);

    // give listView_ an object name so we can refer to it in stylesheets -
    // this is actually the widget that has the wallpaper background
    listView_->setObjectName("DesktopListView");

    // NOTE: When XRnadR is in use, the all screens are actually combined to form a
    // large virtual desktop and only one DesktopWindow needs to be created and screenNum is -1.
    // In some older multihead setups, such as xinerama, every physical screen
    // is treated as a separate desktop so many instances of DesktopWindow may be created.
    // In this case we only want to show desktop icons on the primary screen.
    if(desktopWidget->isVirtualDesktop() || screenNum_ == desktopWidget->primaryScreen()) {
        loadItemPositions();

        // Deliberately NOT loading the real desktop folder here (no
        // fm_folder_from_path()/model_->setFolder() yet) -- this model_ only
        // exists to give proxyModel_ a source to satisfy setModel() below.
        // The only caller of this constructor (Application::startDesktopManager())
        // calls updateFromSettings() immediately afterward, which always
        // calls setDesktopFolder() -- so loading the folder here too used to
        // mean loading it twice in a row for the same path. That mattered
        // for more than duplicate work: FolderModel::onFilesAdded() pumps
        // the event loop mid-callback (QCoreApplication::processEvents(),
        // itself a workaround for a separate, unrelated icon-refresh
        // timing issue -- see its own doc comment), which could let the
        // second setDesktopFolder() call's `delete model_` run *while the
        // first model's own onFilesAdded() was still executing on its
        // stack* -- a use-after-free the moment that call resumed and
        // touched `this`. Reliably didn't reproduce in a Debug build in
        // months of local testing, but crashed almost immediately in a
        // Release build (NDEBUG optimizes/reorders just enough to turn
        // the same race into a guaranteed segfault instead of a lucky
        // near-miss) -- exactly the environment a real packaged install
        // actually runs in.
        model_ = new Fm::FolderModel();

        proxyModel_ = new Fm::ProxyFolderModel();
        proxyModel_->setSourceModel(model_);
        proxyModel_->setShowThumbnails(settings.showThumbnails());
        proxyModel_->sort(Fm::FolderModel::ColumnFileMTime);
        proxyModel_->setDesktopMode();

        // Connected *before* setModel() below on purpose: Qt invokes slots in
        // connection order, and setModel() is what wires up QListView's own
        // internal layoutChanged/rowsInserted handling, which recomputes (and
        // can paint) its default top-left-origin grid. With that connected
        // first, our onLayoutChanged()/onRowsInserted() -- which call
        // setUpdatesEnabled(false) specifically to hide that default grid --
        // ran too late to stop the first frame of it from ever reaching the
        // screen, which read as every icon flickering to the left edge for a
        // frame (most noticeable during a live theme switch, which re-sorts/
        // reloads the model several times in quick succession) before
        // relayoutItems() snapped them back to their pinned spots.
        connect(proxyModel_, &Fm::ProxyFolderModel::rowsInserted, this, &DesktopWindow::onRowsInserted);
        connect(proxyModel_, &Fm::ProxyFolderModel::rowsAboutToBeRemoved, this, &DesktopWindow::onRowsAboutToBeRemoved);
        connect(proxyModel_, &Fm::ProxyFolderModel::layoutChanged, this, &DesktopWindow::onLayoutChanged);
        // A full model reset (e.g. triggered by an icon-theme reload during
        // a live theme switch) invalidates every persistent index/position
        // QListView was tracking, same underlying problem as layoutChanged
        // -- reuse the same suppress-then-relayout handler.
        connect(proxyModel_, &Fm::ProxyFolderModel::modelReset, this, &DesktopWindow::onLayoutChanged);
        // Belt-and-suspenders on top of the connect-order trick above: these
        // "about to" signals are guaranteed by Qt to fire before *anything*
        // reacts, so they close the race unconditionally instead of relying
        // on connection order (an implementation detail) -- see
        // onLayoutAboutToChange()'s doc comment.
        connect(proxyModel_, &Fm::ProxyFolderModel::layoutAboutToBeChanged, this, &DesktopWindow::onLayoutAboutToChange);
        connect(proxyModel_, &Fm::ProxyFolderModel::modelAboutToBeReset, this, &DesktopWindow::onLayoutAboutToChange);
        setModel(proxyModel_);

        connect(listView_, &QListView::indexesMoved, this, &DesktopWindow::onIndexesMoved);
        // NOTE: model_ is connected to fileRenamed in setDesktopFolder(),
        // not here -- updateFromSettings() calls setDesktopFolder() during
        // startup, which deletes this model_ and replaces it with a new
        // instance, so a connection made here would end up attached to an
        // already-deleted object while the live model_ has no listener.
    }

    // set our own delegate
    delegate_ = new DesktopItemDelegate(listView_);
    listView_->setItemDelegateForColumn(Fm::FolderModel::ColumnFileName, delegate_);
    if(proxyModel_) {
        // Thumbnails load asynchronously; ProxyFolderModel::onThumbnailLoaded()
        // emits dataChanged() once one's ready, which does trigger a repaint,
        // but the delegate's iconPixmapCache_ is keyed by (path, iconMode,
        // size) -- none of which change when a thumbnail replaces the
        // generic icon underneath the same key, so paint() kept reusing the
        // pre-thumbnail pixmap it had already cached. Only selecting the
        // item (which switches iconMode to Selected, a fresh cache key) ever
        // showed the real thumbnail, making it look like previews only
        // appeared on selection. Clearing the cache on dataChanged fixes it.
        connect(proxyModel_, &QAbstractItemModel::dataChanged, delegate_, &DesktopItemDelegate::clearIconCache);
    }

    // remove frame
    listView_->setFrameShape(QFrame::NoFrame);
    // inhibit scrollbars FIXME: this should be optional in the future
    listView_->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    listView_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    connect(this, &DesktopWindow::openDirRequested, this, &DesktopWindow::onOpenDirRequested);

    listView_->installEventFilter(this);
    listView_->viewport()->installEventFilter(this);

    // setup shortcuts
    QShortcut* shortcut;
    shortcut = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_Down), this); // pronono: open
    connect(shortcut, &QShortcut::activated, this, &DesktopWindow::onOpenActivated); // probono
    shortcut = new QShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_Down), this); // pronono: open
    connect(shortcut, &QShortcut::activated, this, &DesktopWindow::onOpenActivated); // probono

    shortcut = new QShortcut(QKeySequence(Qt::CTRL + Qt::ALT + Qt::Key_Down), this); // pronono: open
    connect(shortcut, &QShortcut::activated, this, &DesktopWindow::onOpenWithActivated); // probono

    /*
     * probono: Commenting these out
     * for those that are alraedy defined in the Menu solves QAction::event: Ambiguous shortcut overload

    shortcut = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_X), this); // cut
    connect(shortcut, &QShortcut::activated, this, &DesktopWindow::onCutActivated);

    shortcut = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_C), this); // copy
    connect(shortcut, &QShortcut::activated, this, &DesktopWindow::onCopyActivated);

    shortcut = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_V), this); // paste
    connect(shortcut, &QShortcut::activated, this, &DesktopWindow::onPasteActivated);

    shortcut = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_A), this); // select all
    connect(shortcut, &QShortcut::activated, listView_, &QListView::selectAll);

    shortcut = new QShortcut(QKeySequence(Qt::Key_Delete), this); // delete
    connect(shortcut, &QShortcut::activated, this, &DesktopWindow::onDeleteActivated);

    shortcut = new QShortcut(QKeySequence(Qt::Key_F2), this); // rename
    connect(shortcut, &QShortcut::activated, this, &DesktopWindow::onRenameActivated);

    shortcut = new QShortcut(QKeySequence(Qt::Key_Return), this); // rename
    connect(shortcut, &QShortcut::activated, this, &DesktopWindow::onRenameActivated);

    shortcut = new QShortcut(QKeySequence(Qt::Key_Enter), this); // rename
    connect(shortcut, &QShortcut::activated, this, &DesktopWindow::onRenameActivated);

    // Deliberately no separate QShortcut for Qt::Key_Space here: Space is
    // already handled by FolderViewListView::keyPressEvent() (folderview.cpp),
    // which emits quickLookRequested() -> onQuickLookActivated() below. A
    // second, redundant WindowShortcut on the same key fired for the same
    // keypress (since QShortcut's default WindowShortcut context matches as
    // soon as any child of this window has focus, not just this widget
    // itself), causing Quick Look to open-then-immediately-close (or
    // close-then-reopen) on a single Space press -- exactly what made
    // "Space to exit" feel unreliable.

    shortcut = new QShortcut(QKeySequence(Qt::ALT + Qt::Key_Return), this); // properties
    connect(shortcut, &QShortcut::activated, this, &DesktopWindow::onFilePropertiesActivated);

    */

    // Plain Delete deliberately does nothing; permanent deletion is
    // Cmd+Option+Backspace (trash stays on actionDelete's Cmd+Backspace).
    // Cmd = Meta, Option = Alt.
    shortcut = new QShortcut(QKeySequence(Qt::META + Qt::ALT + Qt::Key_Backspace), this); // force delete
    connect(shortcut, &QShortcut::activated, this, &DesktopWindow::onDeleteWithoutTrashActivated);

    shortcut = new QShortcut(QKeySequence(Qt::META + Qt::Key_J), this); // Show View Options
    connect(shortcut, &QShortcut::activated, this, &DesktopWindow::onShowViewOptions);

    desktopMainWindow_ = new DesktopMainWindow(this);
    this->addActions(desktopMainWindow_->findChildren<QAction*>());

    updateMenu();

    connect(desktopMainWindow_, &DesktopMainWindow::open, this, &DesktopWindow::onOpenActivated);
    connect(desktopMainWindow_, &DesktopMainWindow::openWith, this, &DesktopWindow::onOpenWithActivated);
    connect(desktopMainWindow_, &DesktopMainWindow::fileProperties, this, &DesktopWindow::onFilePropertiesActivated);
    connect(desktopMainWindow_, &DesktopMainWindow::preferences, this, &DesktopWindow::onFilerPreferences);
    connect(desktopMainWindow_, &DesktopMainWindow::openFolder, this, &DesktopWindow::onOpenFolder);
    connect(static_cast<Application*>(qApp), &Application::openFolderAndSelectItems, this, &DesktopWindow::onOpenFolderAndSelectItems);
    connect(desktopMainWindow_, &DesktopMainWindow::openTrash, this, &DesktopWindow::onOpenTrash);
    connect(desktopMainWindow_, &DesktopMainWindow::openDesktop, this, &DesktopWindow::onOpenDesktop);
    connect(desktopMainWindow_, &DesktopMainWindow::openDocuments, this, &DesktopWindow::onOpenDocuments);
    connect(desktopMainWindow_, &DesktopMainWindow::openDownloads, this, &DesktopWindow::onOpenDownloads);
    connect(desktopMainWindow_, &DesktopMainWindow::openHome, this, &DesktopWindow::onOpenHome);
    connect(desktopMainWindow_, &DesktopMainWindow::goUp, this, &DesktopWindow::onGoUp);
    connect(desktopMainWindow_, &DesktopMainWindow::cut, this, &DesktopWindow::onCutActivated);
    connect(desktopMainWindow_, &DesktopMainWindow::copy, this, &DesktopWindow::onCopyActivated);
    connect(desktopMainWindow_, &DesktopMainWindow::paste, this, &DesktopWindow::onPasteActivated);

    connect(this, &FolderView::quickLookRequested, this, &DesktopWindow::onQuickLookActivated);
    // Auto-update Quick Look when selection changes while it's open. Uses a
    // dedicated slot (never toggle-closes) so this is never confused with an
    // explicit Space press on the currently-shown file.
    connect(this, &FolderView::selChanged, this, [this](int nSel) {
        if (QuickLookDialog::activeInstance() && nSel > 0)
            onQuickLookRefreshOnSelectionChange();
    });

    connect(desktopMainWindow_, &DesktopMainWindow::duplicate, this, &DesktopWindow::onDuplicateActivated);
    connect(desktopMainWindow_, &DesktopMainWindow::emptyTrash, this, &DesktopWindow::onEmptyTrashActivated);
    connect(desktopMainWindow_, &DesktopMainWindow::del, this, &DesktopWindow::onDeleteActivated);
    connect(desktopMainWindow_, &DesktopMainWindow::rename, this, &DesktopWindow::onRenameActivated);
    connect(desktopMainWindow_, &DesktopMainWindow::selectAll, listView_, &QListView::selectAll);
    connect(desktopMainWindow_, &DesktopMainWindow::invert, this, &FolderView::invertSelection);
    connect(desktopMainWindow_, &DesktopMainWindow::newFolder, this, &DesktopWindow::onNewFolder);
    connect(desktopMainWindow_, &DesktopMainWindow::newBlankFile, this, &DesktopWindow::onNewBlankFile);
    connect(desktopMainWindow_, &DesktopMainWindow::openTerminal, this, &DesktopWindow::onOpenTerminal);
    connect(desktopMainWindow_, &DesktopMainWindow::search, this, &DesktopWindow::onFindFiles);
    connect(desktopMainWindow_, &DesktopMainWindow::about, this, &DesktopWindow::onAbout);
    connect(desktopMainWindow_, &DesktopMainWindow::editBookmarks, this, &DesktopWindow::onEditBookmarks);
    connect(desktopMainWindow_, &DesktopMainWindow::showHidden, this, &DesktopWindow::onShowHidden);
    connect(proxyModel_, &Fm::ProxyFolderModel::sortFilterChanged, this, &DesktopWindow::updateMenu);
    connect(desktopMainWindow_, &DesktopMainWindow::sortColumn, this, &DesktopWindow::onSortColumn);
    connect(desktopMainWindow_, &DesktopMainWindow::sortOrder, this, &DesktopWindow::onSortOrder);
    connect(desktopMainWindow_, &DesktopMainWindow::folderFirst, this, &DesktopWindow::onFolderFirst);
    connect(desktopMainWindow_, &DesktopMainWindow::caseSensitive, this, &DesktopWindow::onCaseSensitive);
    connect(desktopMainWindow_, &DesktopMainWindow::reload, this, &DesktopWindow::onReload);
}

// Called (by Application::createDesktopWindow) when filer-topbar is
// running: the bar is a separate layer-shell window covering the top `px`
// of the screen, so the QMenuBar this window embeds at its own top edge
// would sit invisibly underneath it. Collapse the menubar to zero height
// rather than hide() it -- a hidden QMenuBar's actions drop out of
// QShortcutMap's reachable-widget set, which would kill every menu
// shortcut (Ctrl+C/V/A, ...) on the desktop, while a zero-height one stays
// "visible" to shortcut resolution. The layout margin keeps the icon area
// starting below the bar, where the menubar's own height used to push it.
void DesktopWindow::setReservedTopArea(int px) {
    layout()->setContentsMargins(0, px, 0, 0);
}

DesktopWindow::~DesktopWindow() {
    listView_->removeEventFilter(this);

    if(relayoutTimer_)
        delete relayoutTimer_;

    if(proxyModel_)
        delete proxyModel_;

    if(model_)
        delete model_;

    if(folder_)
        g_object_unref(folder_);
}

void DesktopWindow::setBackground(const QColor& color) {
    bgColor_ = color;
}

void DesktopWindow::setForeground(const QColor& color) {
    fgColor_ = color;
    delegate_->setTextColor(color);
}

void DesktopWindow::setShadow(const QColor& color) {
    shadowColor_ = color;
    delegate_->setShadowColor(color);
}

void DesktopWindow::onOpenDirRequested(FmPath* path, int target) {

    // just raise the window if it's already open
    if (WindowRegistry::instance().checkPathAndRaise(fm_path_to_str(path))) {
      return;
    }

    Application* app = static_cast<Application*>(qApp);
    MainWindow* newWin = new MainWindow(path);
    // apply window size from app->settings
    if ( ! app->settings().spatialMode() ) {
      newWin->resize(app->settings().windowWidth(), app->settings().windowHeight());
      if(app->settings().windowMaximized()) {
              newWin->setWindowState(newWin->windowState() | Qt::WindowMaximized);
      }
    }
    newWin->show();

    // Raise the window if it's open by now
    WindowRegistry::instance().checkPathAndRaise(fm_path_to_str(path));
}

void DesktopWindow::resizeEvent(QResizeEvent* event) {
    QWidget::resizeEvent(event);

    // resize wall paper if needed
    if(isVisible() && wallpaperMode_ != WallpaperNone && wallpaperMode_ != WallpaperTransparent && wallpaperMode_ != WallpaperTile) {
        updateWallpaper();
        update();
    }
    queueRelayout(100); // Qt use a 100 msec delay for relayout internally so we use it, too.
}

void DesktopWindow::setDesktopFolder() {
    FmPath *path = fm_path_new_for_path(XdgDir::readDesktopDir().toStdString().c_str());
    FmFolder* folder = fm_folder_from_path(path);
    if (model_)
      delete model_;
    model_ = new Fm::FolderModel();
    model_->setFolder(folder, true);
    folder_ = reinterpret_cast<FmFolder*>(g_object_ref(model_->folder()));
    proxyModel_->setSourceModel(model_);
    // Renaming a file is reported by the filesystem watcher as an async
    // remove+insert of the underlying FmFileInfo (not an in-place update),
    // which would otherwise make onRowsAboutToBeRemoved() drop the file's
    // pinned position and onRowsInserted() auto-place it back in the next
    // free grid slot -- i.e. the icon visibly jumps elsewhere right after a
    // rename. fileRenamed() fires synchronously at the moment the rename is
    // issued, before that cycle happens, so we can migrate the pinned
    // position from the old name to the new one first. Connected here
    // (rather than once in the constructor) because this function replaces
    // model_ with a brand new instance every time it runs.
    connect(model_, &Fm::FolderModel::fileRenamed, this, &DesktopWindow::onFileRenamed);
}

void DesktopWindow::setWallpaperFile(QString filename) {
    wallpaperFile_ = filename;
}

void DesktopWindow::setWallpaperMode(WallpaperMode mode) {
    wallpaperMode_ = mode;
}

QImage DesktopWindow::loadWallpaperFile(QSize requiredSize) {
    qDebug() << "loadWallpaperFile";
    // NOTE: for ease of programming, we only use the cache for the primary screen.
    bool useCache = (screenNum_ == -1 || screenNum_ == 0);
    QFile info;
    QString cacheFileName;
    if(useCache) {
        // see if we have a scaled version cached on disk
        cacheFileName = QString::fromLocal8Bit(qgetenv("XDG_CACHE_HOME"));
        if(cacheFileName.isEmpty())
            cacheFileName = QDir::homePath() % QLatin1String("/.cache");
        Application* app = static_cast<Application*>(qApp);
        cacheFileName += QLatin1String("%1/filer-qt/") % app->profileName();
        QDir().mkpath(cacheFileName); // ensure that the cache dir exists
        cacheFileName += QLatin1String("/wallpaper.cache");

        // read info file
        QString origin;
        info.setFileName(cacheFileName % ".info");
        if(info.open(QIODevice::ReadOnly)) {
            // FIXME: we need to compare mtime to see if the cache is out of date
            origin = QString::fromLocal8Bit(info.readLine());
            info.close();
            if(!origin.isEmpty()) {
                // try to see if we can get the size of the cached image.
                QImageReader reader(cacheFileName);
                reader.setAutoDetectImageFormat(true);
                QSize cachedSize = reader.size();
                qDebug() << "size of cached file" << cachedSize << ", requiredSize:" << requiredSize;
                if(cachedSize.isValid()) {
                    if(cachedSize == requiredSize) { // see if the cached wallpaper has the size we want
                        QImage image = reader.read(); // return the loaded image
                        qDebug() << "origin" << origin;
                        if(origin == wallpaperFile_)
                            return image;
                    }
                }
            }
        }
        qDebug() << "no cached wallpaper. generate a new one!";
    }

    // we don't have a cached scaled image, load the original file
    QImage image(wallpaperFile_);
    qDebug() << "size of original image" << image.size();
    if(image.isNull() || image.size() == requiredSize) // if the original size is what we want
        return image;

    // scale the original image
    QImage scaled = image.scaled(requiredSize.width(), requiredSize.height(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    // FIXME: should we save the scaled image if its size is larger than the original image?

    if(useCache) {
        // write the path of the original image to the .info file
        if(info.open(QIODevice::WriteOnly)) {
            info.write(wallpaperFile_.toLocal8Bit());
            info.close();

            // write the scaled cache image to disk
            const char* format; // we keep jpg format for *.jpg files, and use png format for others.
            if(wallpaperFile_.endsWith(QLatin1String(".jpg"), Qt::CaseInsensitive) || wallpaperFile_.endsWith(QLatin1String(".jpeg"), Qt::CaseInsensitive))
                format = "JPG";
            else
                format = "PNG";
            scaled.save(cacheFileName, format);
        }
        qDebug() << "wallpaper cached saved to " << cacheFileName;
        // FIXME: we might delay the write of the cached image?
    }
    return scaled;
}

// Regenerate wallpaperPixmap_ according to current settings; paintEvent()
// draws it (and/or bgColor_) directly. Qt's stylesheet "border-image"
// property -- previously used here for Stretch/Fit -- doesn't render on
// this style/platform combination (no visible error, just a solid black
// window), so the image is painted with QPainter instead of CSS.
void DesktopWindow::updateWallpaper() {
    setStyleSheet(QStringLiteral("#DesktopListView { background: transparent; background-color: transparent; }"));
    wallpaperPixmap_ = QPixmap();

    if(wallpaperFile_.isEmpty() || size().isEmpty()
       || wallpaperMode_ == WallpaperNone || wallpaperMode_ == WallpaperTransparent) {
        return;
    }

    switch(wallpaperMode_) {
        case Filer::DesktopWindow::WallpaperStretch:
            // ignores aspect ratio, fills the whole desktop -- loadWallpaperFile()
            // already scales (and caches the scaled result) to exactly this size.
            wallpaperPixmap_ = QPixmap::fromImage(loadWallpaperFile(size()));
            break;
        case Filer::DesktopWindow::WallpaperFit: {
            QImage image(wallpaperFile_);
            if(!image.isNull()) {
                wallpaperPixmap_ = QPixmap::fromImage(
                    image.scaled(size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
            }
            break;
        }
        case Filer::DesktopWindow::WallpaperCenter:
        case Filer::DesktopWindow::WallpaperTile:
            wallpaperPixmap_ = QPixmap(wallpaperFile_);
            break;
        default:
            break;
    }
}

QColor DesktopWindow::averageColorForRect(const QRect& localRect) const {
    const QColor fallback(60, 60, 60);
    QRect clipped = localRect.intersected(wallpaperPixmap_.rect());
    if(wallpaperPixmap_.isNull() || clipped.isEmpty()) {
        return fallback;
    }
    // Downsample the sampled region to a single pixel -- a cheap average-color
    // extraction (bilinear filtering during the scale does the averaging).
    QImage onePixel = wallpaperPixmap_.copy(clipped).toImage()
        .scaled(1, 1, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    return onePixel.pixelColor(0, 0);
}

void DesktopWindow::paintEvent(QPaintEvent* event) {
    QPainter painter(this);
    if(wallpaperMode_ == WallpaperTransparent) {
        // leave the window's own (compositor-blended) background showing through
    }
    else if(wallpaperPixmap_.isNull()) {
        painter.fillRect(rect(), bgColor_);
    }
    else {
        switch(wallpaperMode_) {
            case WallpaperTile:
                painter.fillRect(rect(), bgColor_);
                painter.drawTiledPixmap(rect(), wallpaperPixmap_);
                break;
            case WallpaperCenter:
                painter.fillRect(rect(), bgColor_);
                painter.drawPixmap((width() - wallpaperPixmap_.width()) / 2,
                                    (height() - wallpaperPixmap_.height()) / 2,
                                    wallpaperPixmap_);
                break;
            case WallpaperFit:
                painter.fillRect(rect(), bgColor_);
                painter.drawPixmap((width() - wallpaperPixmap_.width()) / 2,
                                    (height() - wallpaperPixmap_.height()) / 2,
                                    wallpaperPixmap_);
                break;
            default: // Stretch
                painter.drawPixmap(0, 0, wallpaperPixmap_);
                break;
        }
    }
    QWidget::paintEvent(event);
}

void DesktopWindow::updateFromSettings(Settings& settings) {
    setDesktopFolder();
    setWallpaperFile(settings.wallpaper());
    setWallpaperMode(settings.wallpaperMode());
    setFont(settings.desktopFont());
    // Desktop icons use fixed macOS-style metrics (64px icons, 100x110 grid cells)
    // regardless of the user's general icon-size setting, which only applies to
    // regular folder windows via FolderView::updateGridSize()'s generic formula.
    static const int kDesktopIconSize = 64;
    static const QSize kDesktopGridSize(100, 110);
    setIconSize(Fm::FolderView::IconMode, QSize(kDesktopIconSize, kDesktopIconSize));
    listView_->setGridSize(QSize()); // disable grid snapping!
    queueRelayout();
    setForeground(settings.desktopFgColor());
    setBackground(settings.desktopBgColor());
    setShadow(settings.desktopShadowColor());
    updateWallpaper();
    update();
}

void DesktopWindow::onFileClicked(int type, FmFileInfo* fileInfo) {
    if(type == Fm::FolderView::ContextMenuClick) {
        // The desktop window is a NET::Desktop-type window (see the
        // KWindowSystem::setType() call in the constructor), which most
        // window managers never give input focus to on their own. If
        // another application still has focus when a context menu is
        // requested, the QMenu popup opened by the base implementation
        // below can fail to grab focus properly and gets shown as a plain
        // decorated top-level window instead of a borderless popup.
        // Explicitly activating the desktop window first makes the
        // following menu behave like a normal popup regardless of what had
        // focus beforehand.
        KWindowSystem::forceActiveWindow(winId());
    }
    View::onFileClicked(type, fileInfo);
}

void DesktopWindow::prepareFileMenu(Fm::FileMenu* menu) {
    // qDebug("DesktopWindow::prepareFileMenu");
    // The desktop window is a NET::Desktop-type window, which most window
    // managers keep permanently below normal windows and never give focus
    // to on their own. A QMenu popup spawned from it can end up window-
    // managed/stacked like the (unfocused) desktop itself instead of as a
    // borderless overlay on top of everything, which looks like the menu
    // opened "inside" whatever window currently has focus. Bypassing the
    // directly on top, regardless of focus/stacking state.
    if(!QGuiApplication::platformName().contains("wayland")) {
        menu->setWindowFlags(menu->windowFlags() | Qt::X11BypassWindowManagerHint);
    }

    Filer::View::prepareFileMenu(menu);

    // Finder-style single-item menus are identical on the desktop and in the
    // file manager -- no desktop-only additions.
    if(menu->finderStyle())
        return;

    QAction* action = new QAction(tr("Stic&k to Current Position"), menu);
    action->setCheckable(true);
    menu->insertSeparator(menu->separator2());
    menu->insertAction(menu->separator2(), action);

    FmFileInfoList* files = menu->files();
    // select exactly one item
    if(fm_file_info_list_get_length(files) == 1) {
        FmFileInfo* file = menu->firstFile();
        if(customItemPos_.find(fm_file_info_get_name(file)) != customItemPos_.end()) {
            // the file item has a custom position
            action->setChecked(true);
        }
    }
    connect(action, &QAction::toggled, this, &DesktopWindow::onStickToCurrentPos);
}

void DesktopWindow::prepareFolderMenu(Fm::FolderMenu* menu) {
    // See the identical comment in prepareFileMenu(): bypass the window
    // manager so this popup always renders as a borderless overlay on top,
    // regardless of whether the desktop window currently has focus.
    if(!QGuiApplication::platformName().contains("wayland")) {
        menu->setWindowFlags(menu->windowFlags() | Qt::X11BypassWindowManagerHint);
    }

    // Discard FolderMenu's default-constructed action set (Create New/Paste/
    // Select All/Sorting/Show Hidden/Get Info) -- the desktop uses a distinct,
    // macOS-style menu with an exact required order. Note: after clear(), none
    // of FolderMenu's accessor methods (pasteAction(), sortAction(), etc.) may
    // be called again, since the QActions they returned are now deleted.
    menu->clear();

    QAction* newFolderAction = menu->addAction(tr("New Folder"));
    connect(newFolderAction, &QAction::triggered, this, &DesktopWindow::onNewFolder);

    QAction* getInfoAction = menu->addAction(tr("Get Info"));
    connect(getInfoAction, &QAction::triggered, this, [this]() {
        if(FmFileInfo* info = folderInfo())
            Fm::FilePropsDialog::showForFile(info);
    });

    QAction* changeBgAction = menu->addAction(tr("Change Desktop Background..."));
    connect(changeBgAction, &QAction::triggered, this, &DesktopWindow::onDesktopPreferences);

    menu->addSeparator();

    QAction* useStacksAction = menu->addAction(tr("Use Stacks"));
    useStacksAction->setCheckable(true);
    useStacksAction->setChecked(static_cast<Application*>(qApp)->settings().useStacks());
    connect(useStacksAction, &QAction::toggled, this, &DesktopWindow::onUseStacksToggled);

    QMenu* sortByMenu = menu->addMenu(tr("Sort By"));
    buildSortCriteriaMenu(sortByMenu, false);

    // Like Finder: while the desktop is locked to a sort criterion
    // (desktopArrangeColumn_ >= 0), icons are locked to the sorted grid, so
    // the one-shot "Clean Up"/"Clean Up By" actions -- which only make sense
    // for manually-placed icons -- are hidden. "Grid" doesn't reorder or lock
    // anything (icons stay draggable, just snapped), so it keeps them.
    if(desktopArrangeColumn_ < 0) {
        QAction* cleanUpAction = menu->addAction(tr("Clean Up"));
        connect(cleanUpAction, &QAction::triggered, this, &DesktopWindow::onCleanUp);

        QMenu* cleanUpByMenu = menu->addMenu(tr("Clean Up By"));
        buildSortCriteriaMenu(cleanUpByMenu, true);
    }

    menu->addSeparator();

    QAction* viewOptionsAction = menu->addAction(tr("Show View Options"));
    connect(viewOptionsAction, &QAction::triggered, this, &DesktopWindow::onShowViewOptions);

    // Deliberately no custom stylesheet/WA_TranslucentBackground here: on
    // setups without an active compositor (common on plain X11/some WMs),
    // a translucent QMenu renders as an opaque gray box mispositioned
    // relative to the popup's real geometry instead of a blended
    // background, and is illegible. FileMenu/FolderMenu (right-click on a
    // file) don't customize QMenu's look either -- stick to that same
    // native rendering here for reliability across environments.
}

void DesktopWindow::buildSortCriteriaMenu(QMenu* parent, bool isCleanUpBy) {
    struct Entry { QString label; int column; };
    const Entry entries[] = {
        { tr("Name"), Fm::FolderModel::ColumnFileName },
        { tr("Kind"), Fm::FolderModel::ColumnFileType },
        { tr("Date Modified"), Fm::FolderModel::ColumnFileMTime },
        { tr("Date Created"), Fm::FolderModel::ColumnFileCTime },
        { tr("Size"), Fm::FolderModel::ColumnFileSize },
        { tr("Tags"), Fm::FolderModel::ColumnFileTags },
    };
    if(isCleanUpBy) {
        for(const Entry& e : entries) {
            QAction* action = parent->addAction(e.label);
            int column = e.column;
            connect(action, &QAction::triggered, this, [this, column]() { onCleanUpBy(column); });
        }
        return;
    }

    // "Sort By" keeps the desktop continuously arranged (checkable, exclusive,
    // with a "None" option to go back to free placement) -- unlike
    // "Clean Up By" above, which is a one-shot nudge into the grid.
    QActionGroup* group = new QActionGroup(parent);
    group->setExclusive(true);

    QAction* noneAction = parent->addAction(tr("None"));
    noneAction->setCheckable(true);
    noneAction->setChecked(desktopArrangeColumn_ == ArrangeNone);
    group->addAction(noneAction);
    connect(noneAction, &QAction::triggered, this, [this]() { onArrangeBy(ArrangeNone); });

    // "Grid" locks icons to the grid without imposing any sort order --
    // equivalent to Windows Explorer's "Align icons to grid".
    QAction* gridAction = parent->addAction(tr("Grid"));
    gridAction->setCheckable(true);
    gridAction->setChecked(desktopArrangeColumn_ == ArrangeGrid);
    group->addAction(gridAction);
    connect(gridAction, &QAction::triggered, this, [this]() { onArrangeBy(ArrangeGrid); });

    parent->addSeparator();

    for(const Entry& e : entries) {
        QAction* action = parent->addAction(e.label);
        int column = e.column;
        action->setCheckable(true);
        action->setChecked(desktopArrangeColumn_ == column);
        group->addAction(action);
        connect(action, &QAction::triggered, this, [this, column]() { onArrangeBy(column); });
    }
}

void DesktopWindow::onDesktopPreferences() {
    static_cast<Application* >(qApp)->desktopPrefrences();
}

void DesktopWindow::onFilerPreferences()
{
    static_cast<Application* >(qApp)->preferences(QString());
}

void DesktopWindow::onGoUp()
{
    FmPath* path = fm_path_new_for_path(XdgDir::readDesktopDir().toStdString().c_str());
    FmPath* parent = fm_path_get_parent(path);
    if (parent)
      onOpenFolder(fm_path_to_str(parent));
}

void DesktopWindow::onNewFolder()
{
    FmPath* path = fm_path_new_for_path(XdgDir::readDesktopDir().toStdString().c_str());
    createFileOrFolder(Fm::CreateNewFolder, path);
}

void DesktopWindow::onNewBlankFile()
{
    FmPath* path = fm_path_new_for_path(XdgDir::readDesktopDir().toStdString().c_str());
    createFileOrFolder(Fm::CreateNewTextFile, path);
}

void DesktopWindow::onOpenTerminal()
{
    FmPath* path = fm_path_new_for_path(XdgDir::readDesktopDir().toStdString().c_str());
    Application* app = static_cast<Application*>(qApp);
    app->openFolderInTerminal(path);
}

void DesktopWindow::onFindFiles()
{
    Application* app = static_cast<Application*>(qApp);
    FmPathList* selectedPaths = selectedFilePaths();
    QStringList paths;
    if(selectedPaths) {
      for(GList* l = fm_path_list_peek_head_link(selectedPaths); l; l = l->next) {
        // FIXME: is it ok to use display name here?
        // This might be broken on filesystems with non-UTF-8 filenames.
        Fm::Path path(FM_PATH(l->data));
        paths.append(path.displayName(false));
      }
      fm_path_list_unref(selectedPaths);
    }
    else {
      paths.append(XdgDir::readDesktopDir().toStdString().c_str());
    }
    app->findFiles(paths);
}

void DesktopWindow::onAbout()
{
    // the about dialog
    class AboutDialog : public QDialog {
    public:
      explicit AboutDialog(QWidget* parent = 0, Qt::WindowFlags f = 0) {
        ui.setupUi(this);
        ui.version->setText(tr("Version: %1").arg(FILER_VERSION));
      }
    private:
      Ui::AboutDialog ui;
    };
    AboutDialog dialog(this);
    dialog.exec();
}

void DesktopWindow::onEditBookmarks()
{
    Application* app = static_cast<Application*>(qApp);
    app->editBookmarks();
}

void DesktopWindow::onShowHidden(bool hidden)
{
    proxyModel_->setShowHidden(hidden);
}

void DesktopWindow::onSortColumn(int column)
{
    proxyModel_->sort(column, proxyModel_->sortOrder());
}

// Sets/clears the persistent "Sort By" arrangement (see desktopArrangeColumn_).
void DesktopWindow::onArrangeBy(int column)
{
    desktopArrangeColumn_ = column;
    applyGridSnapState();
    if(column == ArrangeNone) {
        queueRelayout();
        return;
    }
    QHash<QByteArray, QPoint> start = capturePositions();
    if(column == ArrangeGrid) {
        // "Grid" only snaps each icon to its nearest cell in place -- unlike
        // a real sort column, it doesn't reorder anything, so relayoutItems()
        // keeps treating icons as individually pinned afterwards (same as
        // ArrangeNone), just constrained to land on a cell when dragged.
        animateRelayout(start, computeSnappedPositions());
        return;
    }
    // Forget manual placements -- like Finder, turning on "Sort By" replaces
    // whatever custom layout existed with the automatic, continuously-kept
    // arrangement (relayoutItems() takes over from here for future
    // inserts/removes since customItemPos_ is now empty).
    animateRelayout(start, computeArrangedPositions());
}

// Three movement regimes, keyed off desktopArrangeColumn_:
//  - ArrangeNone: fully free -- no forced grid pitch, so even a one-pixel
//    nudge is honored (onIconSizeChanged()/onGridSpacingChanged() both call
//    this after resizing the grid, since QListView::setIconSize() etc. can
//    otherwise leave a stale gridSize in place that constrains dragging).
//  - ArrangeGrid: still draggable, but QListView::Snap pulls the dropped
//    icon to the nearest grid cell -- this is "Align to Grid", not a sort,
//    so there's no order to protect by locking the icons.
//  - a real column: locked (Static) like Finder's "Sort By", since dragging
//    would fight the continuously-kept sort order.
void DesktopWindow::applyGridSnapState()
{
    if(desktopArrangeColumn_ == ArrangeNone) {
        listView_->setMovement(QListView::Free);
        listView_->setGridSize(QSize());
    }
    else if(desktopArrangeColumn_ == ArrangeGrid) {
        listView_->setMovement(QListView::Snap);
        listView_->setGridSize(gridCellSize());
    }
    else {
        listView_->setMovement(QListView::Static);
        listView_->setGridSize(gridCellSize());
    }
}

// Target positions for a real sort-column arrangement (desktopArrangeColumn_
// >= 0) -- sorts by that column, then fills the grid in that order. Must not
// be called for ArrangeNone/ArrangeGrid (see computeSnappedPositions() for
// the latter).
QHash<QByteArray, QPoint> DesktopWindow::computeArrangedPositions() const
{
    return computeGridPositionsSorted(desktopArrangeColumn_, proxyModel_->sortOrder());
}

// Rounds each icon's current pixel position to its nearest grid cell,
// preserving relative placement -- unlike computeGridPositions(), which
// discards it and refills top-right-down in row order. Used by "Sort By >
// Grid" (Windows-Explorer-style "Align icons to grid"): icons keep whatever
// arrangement the user already gave them, just snapped onto the raster.
QHash<QByteArray, QPoint> DesktopWindow::computeSnappedPositions() const
{
    QSize grid = gridCellSize();
    int stepX = grid.width() + listView_->spacing();
    int stepY = grid.height() + listView_->spacing();
    QDesktopWidget* desktop = qApp->desktop();
    QHash<QByteArray, QPoint> result;
    QHash<QByteArray, QPoint> current = capturePositions();
    for(auto it = current.constBegin(); it != current.constEnd(); ++it) {
        QPoint pos = it.value();
        int screen = desktop->isVirtualDesktop() ? desktop->screenNumber(pos) : screenNum_;
        QRect workArea = desktop->availableGeometry(screen);
        workArea.adjust(12, 12, -12, -12); // matches the margin used by relayoutItems()/computeGridPositionsForOrder()
        int cellX = qRound(double(pos.x() - workArea.left()) / stepX);
        int cellY = qRound(double(pos.y() - workArea.top()) / stepY);
        result[it.key()] = QPoint(workArea.left() + cellX * stepX, workArea.top() + cellY * stepY);
    }
    return result;
}

void DesktopWindow::onSortOrder(Qt::SortOrder order)
{
    proxyModel_->sort(proxyModel_->sortColumn(), order);
}

void DesktopWindow::onFolderFirst(bool first)
{
    proxyModel_->setFolderFirst(first);
}

void DesktopWindow::onCaseSensitive(Qt::CaseSensitivity sensitivity)
{
    proxyModel_->setSortCaseSensitivity(sensitivity);
}

void DesktopWindow::onReload()
{
  if(folder_) {
    fm_folder_reload(folder_);
  }
}

void DesktopWindow::updateMenu()
{
    desktopMainWindow_->setShowHidden(proxyModel_->showHidden());
    desktopMainWindow_->setSortColumn(proxyModel_->sortColumn());
    desktopMainWindow_->setSortOrder(proxyModel_->sortOrder());
    desktopMainWindow_->setFolderFirst(proxyModel_->folderFirst());
    desktopMainWindow_->setCaseSensitive(proxyModel_->sortCaseSensitivity());
}

void DesktopWindow::onRowsInserted(const QModelIndex& parent, int start, int end) {
    // Newly inserted rows have no explicitly-set position yet, so QListView
    // (even in Free movement mode) briefly lays them out with its own
    // default top-left-origin grid until relayoutItems() runs (queued
    // below) and repositions them. Suppress repaints until then so that
    // default layout is never actually painted -- re-enabled at the end of
    // relayoutItems().
    listView_->setUpdatesEnabled(false);
    queueRelayout();
}

void DesktopWindow::onRowsAboutToBeRemoved(const QModelIndex& parent, int start, int end) {
    if(!customItemPos_.isEmpty()) {
        // also delete stored custom item positions for the items currently being removed.
        bool changed = false;
        for(int row = start; row <= end ;++row) {
            QModelIndex index = parent.child(row, 0);
            FmFileInfo* file = proxyModel_->fileInfoFromIndex(index);
            if(file) { // remove custom position for the item
                if(customItemPos_.remove(fm_file_info_get_name(file)))
                    changed = true;
            }
        }
        if(changed)
            saveItemPositions();
    }
    queueRelayout();
}

void DesktopWindow::onFileRenamed(const QByteArray& oldName, const QByteArray& newName) {
    QHash<QByteArray, QPoint>::iterator it = customItemPos_.find(oldName);
    if(it != customItemPos_.end()) {
        QPoint pos = it.value();
        customItemPos_.erase(it);
        customItemPos_[newName] = pos;
        saveItemPositions();
    }
    // NOTE: no repaint-suppression trick here (an earlier attempt tried
    // disabling listView_ updates across the filesystem watcher's async
    // remove+insert cycle to avoid a brief visual blink of the renamed
    // icon). Two different versions of that caused the WHOLE desktop to
    // stop repainting for multiple seconds after any rename -- either
    // because the "insert" half's name match could silently miss (leaving
    // it stuck until a multi-second safety timeout), or for reasons that
    // didn't reproduce cleanly under instrumentation but were clearly worse
    // than the blink it was meant to fix. A brief blink is a far smaller
    // problem than the desktop appearing frozen, so this only fixes the
    // actually-reported bug: the icon jumping to a new position.
}

void DesktopWindow::onLayoutChanged() {
    // A sort (or other layout change) renumbers which file occupies which
    // row, which resets QListView's per-row position bookkeeping and makes
    // it briefly re-lay-out with its own default grid before
    // relayoutItems() (queued below) corrects it. Suppress repaints until
    // then -- re-enabled at the end of relayoutItems(). (Also connected to
    // modelReset -- see the constructor -- since a full reset invalidates
    // position bookkeeping the exact same way.)
    listView_->setUpdatesEnabled(false);
    queueRelayout();
}

void DesktopWindow::onLayoutAboutToChange() {
    // Belt-and-suspenders alongside onLayoutChanged()'s setUpdatesEnabled(false):
    // that one only wins the race against QListView's own internal
    // layoutChanged/modelReset handling if it happens to be connected first
    // (see the constructor's comment), which is an implementation detail of
    // connection order rather than something to depend on. layoutAboutToBeChanged/
    // modelAboutToBeReset are guaranteed to fire *before* anything -- including
    // Qt's own internal view slots -- reacts, so disabling updates here closes
    // that race unconditionally instead of hoping connection order holds.
    // relayoutItems() (queued from onLayoutChanged() right after) still does
    // the actual re-enable once positions are corrected.
    listView_->setUpdatesEnabled(false);
}

void DesktopWindow::onIndexesMoved(const QModelIndexList& indexes) {
    // remember the custom position for the items
    Q_FOREACH(const QModelIndex& index, indexes) {
        // Under some circumstances, Qt might emit indexMoved for
        // every single cells in the same row. (when QAbstractItemView::SelectItems is set)
        // So indexes list may contain several indixes for the same row.
        // Since we only care about rows, not individual cells,
        // let's handle column 0 of every row here.
        if(index.column() == 0) {
            FmFileInfo* file = proxyModel_->fileInfoFromIndex(index);
            QRect itemRect = listView_->rectForIndex(index);
            QByteArray name = fm_file_info_get_name(file);
            customItemPos_[name] = itemRect.topLeft();
            // qDebug() << "indexMoved:" << name << index << itemRect;
        }
    }
    saveItemPositions();
    queueRelayout();
}

// Grid cell pitch (distance between adjacent icon slots), derived from the
// current icon size/spacing/label-position settings. Shared by relayoutItems()
// (instant path) and computeGridPositionsForOrder() (Clean Up path) so that
// changing the Grid Spacing setting affects both consistently.
//
// Spec (label-below-icon mode): 64px icon, 5px icon-to-text gap, up to 2
// lines of text capped at 32px, then Grid Spacing (default 54px) as the
// empty gap to the next cell -- giving a 118x155 cell at the defaults
// (118 = 64 + 54, 155 = 64 + 5 + 32 + 54). Must stay in sync with
// DesktopItemDelegate::sizeHint(), which computes the same pitch for the
// view's own layout hinting.
QSize DesktopWindow::gridCellSize() const {
    static const int kIconTextGap = 5;
    static const int kMaxTextHeight = 32; // budget for up to kMaxTextLines lines
    // gridSpacing only spreads cells horizontally; rows keep a small fixed
    // padding so the setting never changes the top-bottom distance.
    static const int kVerticalPadding = 10;
    int cellW = desktopLabelRight_
        ? desktopIconSize_ + 120 + desktopGridSpacing_
        : desktopIconSize_ + desktopGridSpacing_;
    int cellH = desktopLabelRight_
        ? qMax(desktopIconSize_, 40) + kVerticalPadding
        : desktopIconSize_ + kIconTextGap + kMaxTextHeight + kVerticalPadding;
    return QSize(cellW, cellH);
}

// A DMG's desktop icon (see dmgmountregistry.h/filelauncher.cpp -- a
// symlink into the extracted folder, since there's no real block-device
// mount to show) gets the same "always in a fixed corner, ignored by
// Clean Up By" treatment as trash-can.desktop below. -1 means name isn't
// one of the tracked DMG desktop links; otherwise, its stacking order
// among currently open DMG icons (0 = topmost/most-recent).
static int dmgVolumeDesktopStackIndex(const QByteArray& name) {
    int idx = 0;
    for(const Filer::DmgMount& mount : Filer::DmgMountRegistry::instance().mounts()) {
        if(!mount.desktopLinkPath.isEmpty() && QFileInfo(mount.desktopLinkPath).fileName().toUtf8() == name)
            return idx;
        ++idx;
    }
    return -1;
}

// QListView does item layout in a very inflexible way, so let's do our custom layout again.
// FIXME: this is very inefficient, but due to the design flaw of QListView, this is currently the only workaround.
void DesktopWindow::relayoutItems() {
    // qDebug("relayoutItems()");
    if(relayoutTimer_) {
        // this slot might be called from the timer, so we cannot delete it directly here.
        relayoutTimer_->deleteLater();
        relayoutTimer_ = NULL;
    }

    // While locked to a real sort column, every relayout -- including ones
    // triggered by files being added/removed -- must keep icons in sorted
    // order rather than falling back to the pinned-position logic below.
    // ArrangeGrid does NOT take this path: it doesn't reorder anything, so
    // the normal customItemPos_-pinning logic below (same as ArrangeNone)
    // is exactly what should place/keep its icons.
    if(desktopArrangeColumn_ >= 0) {
        applyGridPositions(computeArrangedPositions());
        listView_->setUpdatesEnabled(true);
        return;
    }

    QDesktopWidget* desktop = qApp->desktop();
    int screen = 0;
    int row = 0;
    int rowCount = proxyModel_->rowCount();
    bool positionsChanged = false;
    for(;;) {
        if(desktop->isVirtualDesktop()) {
            if(screen >= desktop->numScreens())
                break;
        }else {
            screen = screenNum_;
        }
        QRect workArea = desktop->availableGeometry(screen);
        workArea.adjust(12, 12, -12, -12); // add a 12 pixel margin to the work area
        QSize grid = gridCellSize();
        QPoint pos = workArea.topRight(); // probono: Desktop icons on the right-hand side
        pos.setX(pos.x() - grid.width() ); // probono: Desktop icons on the right-hand side
        for(; row < rowCount; ++row) {
            QModelIndex index = proxyModel_->index(row, 0);
            FmFileInfo* file = proxyModel_->fileInfoFromIndex(index);
            QByteArray name = fm_file_info_get_name(file);
            QHash<QByteArray, QPoint>::iterator it = customItemPos_.find(name);
            if(it != customItemPos_.end()) { // the item has a custom position
                QPoint customPos = *it;
                listView_->setPositionForIndex(customPos, index);
                // qDebug() << "set custom pos:" << name << row << index << customPos;
                continue;
            }
            // check if the current pos is alredy occupied by a custom item
            bool used = false;
            for(it = customItemPos_.begin(); it != customItemPos_.end(); ++it) {
                QPoint customPos = *it;
                if(QRect(customPos, grid).contains(pos)) {
                    used = true;
                    break;
                }
            }
            // probono:  Draw trash in bottom-right position
            int dmgStackIdx = dmgVolumeDesktopStackIndex(name);
            if(name == "trash-can.desktop") {
                QPoint trashPos = workArea.topRight();
                trashPos.setY(workArea.bottomRight().y() - grid.height() - listView_->spacing()); // probono
                trashPos.setX(trashPos.x() - grid.width());
                listView_->setPositionForIndex(trashPos, index);
            } else if(dmgStackIdx >= 0) {
                // Mounted DMG volumes always start at the top-right corner
                // (item #1), like a real Finder desktop, stacking downward
                // for additional ones -- until the user drags one
                // manually, at which point the customItemPos_ check above
                // already takes over.
                QPoint dmgPos = workArea.topRight();
                dmgPos.setX(dmgPos.x() - grid.width());
                dmgPos.setY(dmgPos.y() + dmgStackIdx * (grid.height() + listView_->spacing()));
                listView_->setPositionForIndex(dmgPos, index);
            } else {
                listView_->setPositionForIndex(pos, index);
                if(used) { // go to next pos
                    --row;
                }
                else {
                    // Permanently pin this icon at its newly assigned slot
                    // (like macOS/Windows: once a file is placed, it stays
                    // there -- removing another file elsewhere must not
                    // shift this one). Without this, every icon lacking a
                    // customItemPos_ entry gets densely re-packed on every
                    // relayout, so deleting one file collapses the gap it
                    // left behind instead of leaving it empty.
                    customItemPos_[name] = pos;
                    positionsChanged = true;
                }
                // qDebug() << "set pos" << name << row << index << pos;
                // move to next cell in the column
                pos.setY(pos.y() + grid.height() + listView_->spacing());
                if(pos.y() + grid.height() * 1.5  > workArea.bottom()) { // probono: The 1.5 factor was added so that we have the last line exclusively for the trash. TODO: Find better solution?
                    // if the next position may exceed the bottom of work area, go to the top of next column
                    pos.setX(pos.x() - grid.width() - listView_->spacing()); // probono: Desktop icons on the right-hand side
                    pos.setY(workArea.top());

                    // check if the new column exceeds the right margin of work area
                    if(pos.x() + grid.width() > workArea.right()) {
                        if(desktop->isVirtualDesktop()) {
                            // in virtual desktop mode, go to next screen
                            ++screen;
                            break;
                        }
                    }
                }
            }
        }
        if(row >= rowCount)
            break;
    }
    if(positionsChanged)
        saveItemPositions();
    // Positions are consistent again (matches onRowsInserted()/onLayoutChanged()
    // disabling updates before queuing this call); safe to paint now.
    listView_->setUpdatesEnabled(true);
}

// Fills the desktop grid (top-right corner, columns growing leftward) for
// icons in exactly the given order, keyed by file name. Shared by
// computeGridPositions() (native proxy-model row order) and
// computeGridPositionsSorted() (a locally-sorted order computed without
// ever touching the model, see below).
QHash<QByteArray, QPoint> DesktopWindow::computeGridPositionsForOrder(const QVector<QByteArray>& orderedNames) const {
    QHash<QByteArray, QPoint> result;
    QDesktopWidget* desktop = qApp->desktop();
    int screen = 0;
    int idx = 0;
    int count = orderedNames.size();
    for(;;) {
        if(desktop->isVirtualDesktop()) {
            if(screen >= desktop->numScreens())
                break;
        }
        else {
            screen = screenNum_;
        }
        QRect workArea = desktop->availableGeometry(screen);
        workArea.adjust(12, 12, -12, -12);
        QSize grid = gridCellSize();
        QPoint pos = workArea.topRight();
        pos.setX(pos.x() - grid.width());
        for(; idx < count; ++idx) {
            const QByteArray& name = orderedNames.at(idx);
            int dmgStackIdx = dmgVolumeDesktopStackIndex(name);
            if(name == "trash-can.desktop") {
                QPoint trashPos = workArea.topRight();
                trashPos.setY(workArea.bottomRight().y() - grid.height() - listView_->spacing());
                trashPos.setX(trashPos.x() - grid.width());
                result[name] = trashPos;
            }
            else if(dmgStackIdx >= 0) {
                // Clean Up / Clean Up By must not reflow a mounted DMG
                // volume into the arranged order -- it always stays at
                // the top-right corner, same as relayoutItems().
                QPoint dmgPos = workArea.topRight();
                dmgPos.setX(dmgPos.x() - grid.width());
                dmgPos.setY(dmgPos.y() + dmgStackIdx * (grid.height() + listView_->spacing()));
                result[name] = dmgPos;
            }
            else {
                result[name] = pos;
                pos.setY(pos.y() + grid.height() + listView_->spacing());
                if(pos.y() + grid.height() * 1.5 > workArea.bottom()) {
                    pos.setX(pos.x() - grid.width() - listView_->spacing());
                    pos.setY(workArea.top());
                    if(pos.x() + grid.width() > workArea.right()) {
                        if(desktop->isVirtualDesktop()) {
                            ++screen;
                            break;
                        }
                    }
                }
            }
        }
        if(idx >= count)
            break;
    }
    return result;
}

// Pure grid-position computation for "Clean Up": unlike relayoutItems() (the
// instant path used for drag/insert/remove/resize churn, which still honors
// customItemPos_ pins), this always ignores pinned positions -- that's the
// whole point of Clean Up: forget where icons were dragged to.
QHash<QByteArray, QPoint> DesktopWindow::computeGridPositions() const {
    QVector<QByteArray> names;
    for(int row = 0; row < proxyModel_->rowCount(); ++row) {
        QModelIndex index = proxyModel_->index(row, 0);
        FmFileInfo* file = proxyModel_->fileInfoFromIndex(index);
        names.push_back(fm_file_info_get_name(file));
    }
    return computeGridPositionsForOrder(names);
}

namespace {
// Comparator mirroring ProxyFolderModel::lessThan()'s per-column logic, but
// operating on a plain, local FmFileInfo* list rather than the live model.
// Used by computeGridPositionsSorted() so "Clean Up By" never has to call
// proxyModel_->sort() (which emits layoutChanged and causes QListView to
// briefly re-lay-out with its own default grid -- see animateRelayout()).
bool desktopItemLessThan(FmFileInfo* a, FmFileInfo* b, int column) {
    if(column == Fm::FolderModel::ColumnFileMTime)
        return fm_file_info_get_mtime(a) < fm_file_info_get_mtime(b);
    if(column == Fm::FolderModel::ColumnFileCTime)
        return fm_file_info_get_ctime(a) < fm_file_info_get_ctime(b);
    if(column == Fm::FolderModel::ColumnFileSize) {
        bool aDir = fm_file_info_is_dir(a), bDir = fm_file_info_is_dir(b);
        if(aDir != bDir)
            return aDir; // directories first, like the folderFirst_ convention elsewhere
        return fm_file_info_get_size(a) < fm_file_info_get_size(b);
    }
    if(column == Fm::FolderModel::ColumnFileType) {
        bool aDir = fm_file_info_is_dir(a), bDir = fm_file_info_is_dir(b);
        if(aDir != bDir)
            return aDir;
        QString ta = QString::fromUtf8(fm_mime_type_get_desc(fm_file_info_get_mime_type(a)));
        QString tb = QString::fromUtf8(fm_mime_type_get_desc(fm_file_info_get_mime_type(b)));
        return ta.localeAwareCompare(tb) < 0;
    }
    if(column == Fm::FolderModel::ColumnFileTags) {
        QString pa = QString::fromUtf8(fm_path_to_str(fm_file_info_get_path(a)));
        QString pb = QString::fromUtf8(fm_path_to_str(fm_file_info_get_path(b)));
        bool oka, okb;
        QString ta = Fm::getFileTag(pa, oka);
        QString tb = Fm::getFileTag(pb, okb);
        if(oka != okb)
            return oka; // tagged items before untagged
        if(oka && okb) {
            const QVector<Fm::TagInfo>& palette = Fm::tagPalette();
            int ia = -1, ib = -1;
            for(int i = 0; i < palette.size(); ++i) {
                if(palette.at(i).name == ta) ia = i;
                if(palette.at(i).name == tb) ib = i;
            }
            if(ia != ib)
                return ia < ib;
        }
        // same tag (or both untagged): fall through to name comparison below
    }
    // Name (default, and fallback for equal Tags/Kind/Size)
    return strcmp(fm_file_info_get_collate_key(a), fm_file_info_get_collate_key(b)) < 0;
}
}

// Computes Clean Up By's target positions from a LOCALLY-sorted copy of the
// file list -- proxyModel_ itself is never sorted/touched here. This is
// deliberate: calling proxyModel_->sort() emits layoutChanged, which causes
// QListView to reset its per-row position bookkeeping and briefly re-lay
// itself out with Qt's own default grid before our animation's first tick
// corrects it, turning the intended slide into a left-then-right jump. By
// only ever computing target *positions* -- never reordering the model
// itself -- animateRelayout() is the sole thing moving icons on screen.
QHash<QByteArray, QPoint> DesktopWindow::computeGridPositionsSorted(int column, Qt::SortOrder order) const {
    QVector<QPair<QByteArray, FmFileInfo*> > items;
    for(int row = 0; row < proxyModel_->rowCount(); ++row) {
        QModelIndex index = proxyModel_->index(row, 0);
        FmFileInfo* file = proxyModel_->fileInfoFromIndex(index);
        items.push_back(qMakePair(QByteArray(fm_file_info_get_name(file)), file));
    }
    std::stable_sort(items.begin(), items.end(), [column, order](const QPair<QByteArray, FmFileInfo*>& a, const QPair<QByteArray, FmFileInfo*>& b) {
        if(order == Qt::AscendingOrder)
            return desktopItemLessThan(a.second, b.second, column);
        return desktopItemLessThan(b.second, a.second, column);
    });
    QVector<QByteArray> names;
    for(const auto& item : items)
        names.push_back(item.first);
    return computeGridPositionsForOrder(names);
}

void DesktopWindow::applyGridPositions(const QHash<QByteArray, QPoint>& positions) {
    for(int row = 0; row < proxyModel_->rowCount(); ++row) {
        QModelIndex index = proxyModel_->index(row, 0);
        FmFileInfo* file = proxyModel_->fileInfoFromIndex(index);
        QByteArray name = fm_file_info_get_name(file);
        QHash<QByteArray, QPoint>::const_iterator it = positions.constFind(name);
        if(it != positions.constEnd())
            listView_->setPositionForIndex(*it, index);
    }
}

// Snapshot of each file's CURRENT on-screen pixel position, keyed by file
// name rather than row/index. Needed because a model sort() reorders which
// file occupies which row without moving anything on screen, so capturing
// positions by index (before vs. after sort) would silently mix up which
// pixel position belongs to which file.
QHash<QByteArray, QPoint> DesktopWindow::capturePositions() const {
    QHash<QByteArray, QPoint> result;
    for(int row = 0; row < proxyModel_->rowCount(); ++row) {
        QModelIndex index = proxyModel_->index(row, 0);
        FmFileInfo* file = proxyModel_->fileInfoFromIndex(index);
        QByteArray name = fm_file_info_get_name(file);
        result[name] = listView_->visualRect(index).topLeft();
    }
    return result;
}

// Animates all desktop icons simultaneously from startPositions to
// targetPositions (both name-keyed) on a single shared QVariantAnimation
// timeline (300ms, InOutQuad). Since every icon has a unique start/end cell
// and all move together at the same t, there is no need for explicit
// collision-avoidance -- this matches how Finder's own Clean Up animation
// behaves.
void DesktopWindow::animateRelayout(const QHash<QByteArray, QPoint>& startPositions, const QHash<QByteArray, QPoint>& targetPositions) {
    struct Move { QPersistentModelIndex index; QPoint start; QPoint end; };
    QVector<Move> moves;
    for(int row = 0; row < proxyModel_->rowCount(); ++row) {
        QModelIndex index = proxyModel_->index(row, 0);
        FmFileInfo* file = proxyModel_->fileInfoFromIndex(index);
        QByteArray name = fm_file_info_get_name(file);
        QHash<QByteArray, QPoint>::const_iterator targetIt = targetPositions.constFind(name);
        if(targetIt == targetPositions.constEnd())
            continue;
        QPoint end = *targetIt;
        QPoint start = startPositions.value(name, end);
        if(start != end)
            moves.push_back({QPersistentModelIndex(index), start, end});
    }

    customItemPos_.clear();
    saveItemPositions();

    // Cancel any relayout that got queued by a sort()/model change just
    // before this call (e.g. from Clean Up By's proxyModel_->sort()) -- an
    // instant, non-animated relayoutItems() firing on the next event-loop
    // tick would otherwise snap icons to their final spot and make the
    // animation below invisible.
    if(relayoutTimer_) {
        relayoutTimer_->stop();
        relayoutTimer_->deleteLater();
        relayoutTimer_ = NULL;
    }

    if(moves.isEmpty()) {
        applyGridPositions(targetPositions);
        for(auto it = targetPositions.constBegin(); it != targetPositions.constEnd(); ++it)
            customItemPos_[it.key()] = it.value();
        saveItemPositions();
        return;
    }

    // QListView emits indexesMoved() not just for user drags but also for
    // setPositionForIndex() calls made below on every animation tick. Left
    // connected, each tick would feed straight back into onIndexesMoved()
    // -> queueRelayout() -> relayoutItems(), which snaps every icon to its
    // final position immediately -- turning the slide into a jump after a
    // single frame. Block listView_'s signals for the whole animation and
    // unblock only once it's actually finished.
    listView_->blockSignals(true);

    QVariantAnimation* anim = new QVariantAnimation(this);
    anim->setDuration(300);
    anim->setEasingCurve(QEasingCurve::InOutQuad);
    anim->setStartValue(0.0);
    anim->setEndValue(1.0);
    auto lastRects = QSharedPointer<QHash<QPersistentModelIndex, QRect>>::create();
    connect(anim, &QVariantAnimation::valueChanged, this, [this, moves, lastRects](const QVariant& v) {
        qreal t = v.toReal();
        QHash<QPersistentModelIndex, QPoint> offsets;
        QRegion updateRegion;
        for(const auto& m : moves) {
            if(!m.index.isValid())
                continue;
            QPoint p = m.start + QPointF((m.end - m.start) * t).toPoint();
            QPoint offset = p - m.start;
            offsets[m.index] = offset;
            
            QRect itemRect = listView_->visualRect(m.index);
            QRect newRect = itemRect.translated(offset).adjusted(-60, -60, 60, 60); // inflate to cover shadow/text overflows
            
            if (lastRects->contains(m.index)) {
                updateRegion += lastRects->value(m.index);
            } else {
                updateRegion += itemRect.adjusted(-60, -60, 60, 60);
            }
            updateRegion += newRect;
            (*lastRects)[m.index] = newRect;
        }
        if (delegate_) delegate_->setAnimationOffsets(offsets);
        listView_->viewport()->update(updateRegion);
    });
    connect(anim, &QVariantAnimation::finished, this, [this, targetPositions]() {
        if (delegate_) delegate_->clearAnimationOffsets();
        applyGridPositions(targetPositions); // exact final snap, avoids float rounding drift
        listView_->blockSignals(false);
        // Pin every icon at its freshly tidied slot so the grid behaves like
        // macOS/Windows afterwards: removing one file later leaves its slot
        // empty instead of collapsing the whole layout back together.
        for(auto it = targetPositions.constBegin(); it != targetPositions.constEnd(); ++it)
            customItemPos_[it.key()] = it.value();
        saveItemPositions();
    });
    anim->start(QAbstractAnimation::DeleteWhenStopped);
}

void DesktopWindow::onCleanUp() {
    QHash<QByteArray, QPoint> start = capturePositions();
    animateRelayout(start, computeGridPositions());
}

void DesktopWindow::onCleanUpBy(int column) {
    QHash<QByteArray, QPoint> start = capturePositions();
    // Deliberately never call proxyModel_->sort() for Clean Up By, even
    // after the animation finishes: computeGridPositionsSorted() sorts a
    // local copy of the file list itself, matching how the reference
    // implementation (pearos-desktop's cleanUpDesktopBy()) always
    // recomputes its file list fresh rather than keeping a persistently
    // sorted model. Calling sort() here -- even signal-blocked -- reorders
    // the model's underlying row list while the animation's QModelIndexes
    // are still in flight, which is exactly the kind of interference this
    // function exists to avoid.
    QHash<QByteArray, QPoint> target = computeGridPositionsSorted(column, proxyModel_->sortOrder());
    animateRelayout(start, target);
}

void DesktopWindow::onUseStacksToggled(bool checked) {
    static_cast<Application*>(qApp)->settings().setUseStacks(checked);
    // NOTE: grouping/rendering logic for Stacks is not yet implemented; this
    // only persists the toggle state for a future feature.
}

void DesktopWindow::onShowViewOptions() {
    ViewOptionsPopup* popup = new ViewOptionsPopup(this);
    connect(popup, &ViewOptionsPopup::stackByChanged, this, &DesktopWindow::onStackByChanged);
    connect(popup, &ViewOptionsPopup::sortByChanged, this, &DesktopWindow::onSortByChanged);
    connect(popup, &ViewOptionsPopup::iconSizeChanged, this, &DesktopWindow::onIconSizeChanged);
    connect(popup, &ViewOptionsPopup::gridSpacingChanged, this, &DesktopWindow::onGridSpacingChanged);
    connect(popup, &ViewOptionsPopup::textSizeChanged, this, &DesktopWindow::onTextSizeChanged);
    connect(popup, &ViewOptionsPopup::labelPositionChanged, this, &DesktopWindow::onLabelPositionChanged);
    connect(popup, &ViewOptionsPopup::showItemInfoChanged, this, &DesktopWindow::onShowItemInfoChanged);
    connect(popup, &ViewOptionsPopup::showIconPreviewChanged, this, &DesktopWindow::onShowIconPreviewChanged);
    
    popup->setIconSizeValue(desktopIconSize_);
    popup->setGridSpacingValue(desktopGridSpacing_);
    popup->setTextSizeValue(desktopTextSize_);
    popup->setLabelPositionValue(desktopLabelRight_);
    popup->setShowItemInfoValue(desktopShowItemInfo_);
    popup->setShowIconPreviewValue(desktopShowIconPreview_);
    
    popup->move(QCursor::pos());
    popup->show();
}

void DesktopWindow::loadItemPositions() {
    // load custom item positions
    Settings& settings = static_cast<Application*>(qApp)->settings();
    QString configFile = QString("%1/desktop-items-%2.conf").arg(settings.profileDir(settings.profileName())).arg(screenNum_);
    QSettings file(configFile, QSettings::IniFormat);
    
    file.beginGroup("ViewOptions");
    desktopIconSize_ = file.value("iconSize", 64).toInt();
    desktopGridSpacing_ = file.value("gridSpacing", 120).toInt();
    desktopTextSize_ = file.value("textSize", 12).toInt();
    desktopLabelRight_ = file.value("labelRight", false).toBool();
    desktopShowItemInfo_ = file.value("showItemInfo", false).toBool();
    desktopShowIconPreview_ = file.value("showIconPreview", false).toBool();
    file.endGroup();
    
    Q_FOREACH(const QString& name, file.childGroups()) {
        if (name == "ViewOptions") continue;
        file.beginGroup(name);
        QVariant var = file.value("pos");
        if(var.isValid())
            customItemPos_[name.toUtf8()] = var.toPoint();
        file.endGroup();
    }
}

void DesktopWindow::saveItemPositions() {
    Settings& settings = static_cast<Application*>(qApp)->settings();
    // store custom item positions
    QString configFile = QString("%1/desktop-items-%2.conf").arg(settings.profileDir(settings.profileName())).arg(screenNum_);
    // FIXME: using QSettings here is inefficient and it's not friendly to UTF-8.
    QSettings file(configFile, QSettings::IniFormat);
    file.clear(); // remove all existing entries

    file.beginGroup("ViewOptions");
    file.setValue("iconSize", desktopIconSize_);
    file.setValue("gridSpacing", desktopGridSpacing_);
    file.setValue("textSize", desktopTextSize_);
    file.setValue("labelRight", desktopLabelRight_);
    file.setValue("showItemInfo", desktopShowItemInfo_);
    file.setValue("showIconPreview", desktopShowIconPreview_);
    file.endGroup();

    // FIXME: we have to remove dead entries not associated to any files?
    QHash<QByteArray, QPoint>::iterator it;
    for(it = customItemPos_.begin(); it != customItemPos_.end(); ++it) {
        const QByteArray& name = it.key();
        QPoint pos = it.value();
        file.beginGroup(QString::fromUtf8(name, name.length()));
        file.setValue("pos", pos);
        file.endGroup();
    }
}

void DesktopWindow::pinNewItemNear(const QByteArray& refName, const QByteArray& newName, const QPoint& offset) {
    QPoint refPos;
    QHash<QByteArray, QPoint>::const_iterator it = customItemPos_.constFind(refName);
    if(it != customItemPos_.constEnd())
        refPos = it.value();
    else {
        // the reference item has no pinned position; use its current visual one
        bool found = false;
        int rowCount = proxyModel_->rowCount();
        for(int row = 0; row < rowCount; ++row) {
            QModelIndex index = proxyModel_->index(row, 0);
            FmFileInfo* file = proxyModel_->fileInfoFromIndex(index);
            if(file && refName == fm_file_info_get_name(file)) {
                refPos = listView_->visualRect(index).topLeft();
                found = true;
                break;
            }
        }
        if(!found)
            return; // not an item of this desktop
    }
    customItemPos_[newName] = refPos + offset;
    saveItemPositions();
    queueRelayout();
}

void DesktopWindow::onStickToCurrentPos(bool toggled) {
    QAction* action = static_cast<QAction*>(sender());
    Fm::FileMenu* menu = static_cast<Fm::FileMenu*>(action->parent());

    QModelIndexList indexes = listView_->selectionModel()->selectedIndexes();
    if(!indexes.isEmpty()) {
        FmFileInfo* file = menu->firstFile();
        QByteArray name = fm_file_info_get_name(file);
        QModelIndex index = indexes.first();
        if(toggled) { // remember to current custom position
            QRect itemRect = listView_->rectForIndex(index);
            customItemPos_[name] = itemRect.topLeft();
            saveItemPositions();
        }
        else { // cancel custom position and perform relayout
            QHash<QByteArray, QPoint>::iterator it = customItemPos_.find(name);
            if(it != customItemPos_.end()) {
                customItemPos_.erase(it);
                saveItemPositions();
                relayoutItems();
            }
        }
    }
}

void DesktopWindow::queueRelayout(int delay) {
    // qDebug() << "queueRelayout";
    if(!relayoutTimer_) {
        relayoutTimer_ = new QTimer();
        relayoutTimer_->setSingleShot(true);
        connect(relayoutTimer_, &QTimer::timeout, this, &DesktopWindow::relayoutItems);
        relayoutTimer_->start(delay);
    }
}

// slots for file operations

void DesktopWindow::onCutActivated() {
    if(FmPathList* paths = selectedFilePaths()) {
        Fm::cutFilesToClipboard(paths);
        fm_path_list_unref(paths);
    }
}

void DesktopWindow::onCopyActivated() {
    if(FmPathList* paths = selectedFilePaths()) {
        Fm::copyFilesToClipboard(paths);
        fm_path_list_unref(paths);
    }
}

void DesktopWindow::onPasteActivated() {
    Fm::pasteFilesFromClipboard(path());
}

void DesktopWindow::onDuplicateActivated() {
    DesktopWindow::onCopyActivated();
    DesktopWindow::onPasteActivated();
}

void DesktopWindow::onEmptyTrashActivated() {
    Fm::Trash::emptyTrash();
}

void DesktopWindow::onDeleteActivated() {
    if(FmPathList* paths = selectedFilePaths()) {

        // probono: Check if mountpoints are contained
        bool sourcePathsContainMountpoints = false;
        for(GList* l = fm_path_list_peek_head_link(paths); l; l = l->next) {
            FmPath* path = FM_PATH(l->data);
            QString sourcePathStr =  QString(fm_path_to_str(path));

            for (const QStorageInfo storageInfo : QStorageInfo::mountedVolumes()) {
                if(storageInfo.rootPath() == sourcePathStr) {
                    qDebug() << sourcePathStr << "is a mountpoint";
                    sourcePathsContainMountpoints = true;
                    break;
                }
            }
        }

        qDebug() << "sourcePathsContainMountpoints:" << sourcePathsContainMountpoints;
        if(sourcePathsContainMountpoints == false) {
            Filer::Application* app = static_cast<Filer::Application*>(qApp);
            Fm::FileOperation::trashFiles(paths, app->settings().confirmTrash());
        } else {
            // Similar code is in foldermodel.cpp
            // Do the unmounting natively in Qt without the need for an external program
            // The dark side does this with something like
            // GVolume* volume = volumeItem->volume();
            // op->unmount(volumeItem->volume());
            for(GList* l = fm_path_list_peek_head_link(paths); l; l = l->next) {
                FmPath* path = FM_PATH(l->data);
                QString sourcePathStr =  QString(fm_path_to_str(path));
                QProcess p;
                p.setProgram("eject-and-clean");
                p.setArguments({sourcePathStr});
                qDebug() << p.program() << p.arguments();
                p.start();
                p.waitForFinished();
                qDebug() <<  "p.exitCode():" << p.exitCode();
                if(p.exitCode() != 0) {
                    QMessageBox::warning(nullptr, " ", QString("Cannot eject %1, 'eject-and-clean' command line tool missing or returned an error.").arg(sourcePathStr));
                }
            }
        }
        fm_path_list_unref(paths);
    }
}

void DesktopWindow::onDeleteWithoutTrashActivated() {
    if(FmPathList* paths = selectedFilePaths()) {
        Settings& settings = static_cast<Application*>(qApp)->settings();
        Fm::FileOperation::deleteFiles(paths, settings.confirmDelete(), this);
        fm_path_list_unref(paths);
    }
}

void DesktopWindow::onRenameActivated() {
    if (listView_) {
        QModelIndexList selected = listView_->selectionModel()->selectedIndexes();
        if (!selected.isEmpty()) {
            listView_->edit(selected.first());
            return;
        }
    }
    // Fallback just in case
    if(FmFileInfoList* files = selectedFiles()) {
        for(GList* l = fm_file_info_list_peek_head_link(files); l; l = l->next) {
            FmFileInfo* info = FM_FILE_INFO(l->data);
            Fm::renameFile(info, NULL);
            fm_file_info_list_unref(files);
        }
    }
}

void DesktopWindow::onQuickLookActivated() {
    // Explicit user request (Space press / menu item): toggle. Pressing
    // Space again on the same already-shown file must always close it --
    // this is what "Space to exit" means, regardless of which widget
    // technically has keyboard focus at the moment.
    showOrRefreshQuickLook(true);
}

void DesktopWindow::onQuickLookRefreshOnSelectionChange() {
    // Selection changed while Quick Look happens to be open: just update the
    // preview to the new file, never close -- this is not a user request to
    // dismiss Quick Look.
    showOrRefreshQuickLook(false);
}

void DesktopWindow::showOrRefreshQuickLook(bool allowToggleClose) {
    if(FmFileInfoList* files = selectedFiles()) {
        if (fm_file_info_list_get_length(files) > 0) {
            FmFileInfo* info = FM_FILE_INFO(fm_file_info_list_peek_head_link(files)->data);

            QRect sourceRect = geometry();
            QAbstractItemView* itemView = qobject_cast<QAbstractItemView*>(childView());
            if (itemView) {
                QModelIndexList selected = itemView->selectionModel()->selectedIndexes();
                if (!selected.isEmpty()) {
                    QRect rect = itemView->visualRect(selected.first());
                    sourceRect = QRect(itemView->viewport()->mapToGlobal(rect.topLeft()), rect.size());
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
        fm_file_info_list_unref(files);
    }
}

// probono
void DesktopWindow::onOpenActivated()
{
    if(FmFileInfoList* files = selectedFiles()) {
        if(View::fileLauncher()) {
            View::fileLauncher()->launchFiles(NULL, files);
        }
        else { // use the default launcher
            Fm::FileLauncher launcher;
            launcher.launchFiles(NULL, files);
        }
    }
}

// probono
void DesktopWindow::onOpenWithActivated()
{
    qDebug() << "DesktopWindow::onOpenWithActivated()";
    if(FmFileInfoList* files = selectedFiles()) {
        FmPathList* paths = fm_path_list_new_from_file_info_list(files);
        for(GList* l = fm_path_list_peek_head_link(paths); l; l = l->next) {
            FmPath* path = FM_PATH(l->data);
            QString sourcePathStr =  QString(fm_path_to_str(path));
            QProcess p;
            p.setProgram("open");
            p.setArguments({"--chooser", sourcePathStr});
            qDebug() << p.program() << p.arguments();
            p.startDetached();
        }
        fm_path_list_unref(paths);
    }
}

void DesktopWindow::onShowContentsActivated()
{
    if(FmFileInfoList* files = selectedFiles()) {
        if(View::fileLauncher()) {
            View::fileLauncher()->launchFiles(NULL, files, true);
        }
        else { // use the default launcher
            Fm::FileLauncher launcher;
            launcher.launchFiles(NULL, files, true);
        }
    }
}

void DesktopWindow::onFilePropertiesActivated() {
    if(FmFileInfoList* files = selectedFiles()) {
        Fm::FilePropsDialog::showForFiles(files);
        fm_file_info_list_unref(files);
    }
}

static void forwardMouseEventToRoot(QMouseEvent* event) {
    xcb_ungrab_pointer(QX11Info::connection(), event->timestamp());
    // forward the event to the root window
    xcb_button_press_event_t xcb_event;
    uint32_t mask = 0;
    xcb_event.state = 0;
    switch(event->type()) {
    case QEvent::MouseButtonPress:
        xcb_event.response_type = XCB_BUTTON_PRESS;
        mask = XCB_EVENT_MASK_BUTTON_PRESS;
        break;
    case QEvent::MouseButtonRelease:
        xcb_event.response_type = XCB_BUTTON_RELEASE;
        mask = XCB_EVENT_MASK_BUTTON_RELEASE;
        break;
    default:
        return;
    }

    // convert Qt button to XCB button
    switch(event->button()) {
    case Qt::LeftButton:
        xcb_event.detail = 1;
        xcb_event.state |= XCB_BUTTON_MASK_1;
        break;
    case Qt::MiddleButton:
        xcb_event.detail = 2;
        xcb_event.state |= XCB_BUTTON_MASK_2;
        break;
    case Qt::RightButton:
        xcb_event.detail = 3;
        xcb_event.state |= XCB_BUTTON_MASK_3;
        break;
    default:
        xcb_event.detail = 0;
    }

    // convert Qt modifiers to XCB states
    if(event->modifiers() & Qt::ShiftModifier)
        xcb_event.state |= XCB_MOD_MASK_SHIFT;
    if(event->modifiers() & Qt::ControlModifier)
        xcb_event.state |= XCB_MOD_MASK_SHIFT;
    if(event->modifiers() & Qt::AltModifier)
        xcb_event.state |= XCB_MOD_MASK_1;

    xcb_event.sequence = 0;
    xcb_event.time = event->timestamp();

    WId root = QX11Info::appRootWindow(QX11Info::appScreen());
    xcb_event.event = root;
    xcb_event.root = root;
    xcb_event.child = 0;

    xcb_event.root_x = event->globalX();
    xcb_event.root_y = event->globalY();
    xcb_event.event_x = event->x();
    xcb_event.event_y = event->y();
    xcb_event.same_screen = 1;

    xcb_send_event(QX11Info::connection(), 0, root, mask, (char*)&xcb_event);
    xcb_flush(QX11Info::connection());
}

bool DesktopWindow::event(QEvent* event)
{
    switch(event->type()) {
    case QEvent::WinIdChange: {
        qDebug() << "winid change:" << effectiveWinId();
        if(effectiveWinId() == 0)
            break;
        // set freedesktop.org EWMH hints properly
        if(QX11Info::isPlatformX11() && QX11Info::connection()) {
            xcb_connection_t* con = QX11Info::connection();
            const char* atom_name = "_NET_WM_WINDOW_TYPE_DESKTOP";
            xcb_atom_t atom = xcb_intern_atom_reply(con, xcb_intern_atom(con, 0, strlen(atom_name), atom_name), NULL)->atom;
            const char* prop_atom_name = "_NET_WM_WINDOW_TYPE";
            xcb_atom_t prop_atom = xcb_intern_atom_reply(con, xcb_intern_atom(con, 0, strlen(prop_atom_name), prop_atom_name), NULL)->atom;
            xcb_atom_t XA_ATOM = 4;
            xcb_change_property(con, XCB_PROP_MODE_REPLACE, effectiveWinId(), prop_atom, XA_ATOM, 32, 1, &atom);
        }
        break;
    }
#undef FontChange // FontChange is defined in the headers of XLib and clashes with Qt, let's undefine it.
    case QEvent::FontChange:
        queueRelayout();
        break;

    default:
        break;
    }

    return QWidget::event(event);
}

#undef FontChange // this seems to be defined in Xlib headers as a macro, undef it!

void DesktopWindow::onStackByChanged(int index) {
    // index maps to: None, Kind, Date Last Opened, Date Added, Date Modified, Date Created, Tags
    // Stacking is currently a placeholder feature in Filer
    queueRelayout();
}

void DesktopWindow::onSortByChanged(int index) {
    // None, Snap To Grid, Kind, Date Last Opened, Date Added, Date Modified, Date Created, Tags
    if(index == 1) { // Snap To Grid
        listView_->setGridSize(gridCellSize());
    } else {
        listView_->setGridSize(QSize()); // Free drag
    }
    saveItemPositions();
    queueRelayout();
}

void DesktopWindow::onIconSizeChanged(int size) {
    desktopIconSize_ = size;
    setIconSize(Fm::FolderView::IconMode, QSize(size, size));
    // setIconSize() (base FolderView) recomputes and re-applies its own
    // gridSize; reassert ours so free arrangement stays truly unconstrained
    // (and locked arrangement keeps using our own grid pitch).
    applyGridSnapState();
    // The grid cell pitch depends on the icon size, so already-pinned icons
    // must be re-flowed into the new grid -- just re-saving their old pixel
    // positions (as queueRelayout()/relayoutItems() would do, since it always
    // honors customItemPos_ pins) would leave them exactly where they were.
    customItemPos_.clear();
    applyGridPositions(desktopArrangeColumn_ >= 0 ? computeArrangedPositions() : computeGridPositions());
    saveItemPositions();
}

void DesktopWindow::onGridSpacingChanged(int spacing) {
    desktopGridSpacing_ = spacing;
    delegate_->setGridSpacing(spacing);
    applyGridSnapState();
    // Same reasoning as onIconSizeChanged(): the grid cell pitch depends on
    // spacing, so pinned icons must be re-flowed, not left at their old spots.
    customItemPos_.clear();
    applyGridPositions(desktopArrangeColumn_ >= 0 ? computeArrangedPositions() : computeGridPositions());
    saveItemPositions();
}

void DesktopWindow::onTextSizeChanged(int size) {
    desktopTextSize_ = size;
    delegate_->setTextSize(size);
    saveItemPositions();
    queueRelayout();
}

void DesktopWindow::onLabelPositionChanged(bool right) {
    desktopLabelRight_ = right;
    delegate_->setLabelPosition(right);
    // Cell pitch formula differs between label-right and label-below modes.
    customItemPos_.clear();
    applyGridPositions(computeGridPositions());
    saveItemPositions();
}

void DesktopWindow::onShowItemInfoChanged(bool show) {
    desktopShowItemInfo_ = show;
    delegate_->setShowItemInfo(show);
    saveItemPositions();
    queueRelayout();
}

void DesktopWindow::onShowIconPreviewChanged(bool show) {
    desktopShowIconPreview_ = show;
    delegate_->setShowIconPreview(show);
    if(proxyModel_) proxyModel_->setShowThumbnails(show);
    saveItemPositions();
    queueRelayout();
}

bool DesktopWindow::eventFilter(QObject * watched, QEvent * event) {
    if(!listView_) return false;
    if(watched == listView_) {
        switch(event->type()) {
        case QEvent::FontChange:
            if(model_)
                queueRelayout();
            break;
        default:
            break;
        }
    } else if(watched == listView_->viewport()) {
        if(event->type() == QEvent::Wheel) {
            // Scrollbars are disabled on the desktop (it's not a scrollable
            // view), but QAbstractItemView still scrolls its viewport on
            // wheel events even with the scrollbar hidden. Swallow it here.
            return true;
        }
        static QPoint leftClickPos;
        if(event->type() == QEvent::MouseButtonPress) {
            QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
            if(mouseEvent->button() == Qt::LeftButton) {
                leftClickPos = mouseEvent->pos();
            }
        }
        else if(event->type() == QEvent::MouseButtonRelease) {
            QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
            if(mouseEvent->button() == Qt::LeftButton) {
                if((mouseEvent->pos() - leftClickPos).manhattanLength() < QApplication::startDragDistance()) {
                    QModelIndex index = listView_->indexAt(mouseEvent->pos());
                    if(!index.isValid()) {
                        QProcess::startDetached("qdbus", QStringList() << "org.kde.kglobalaccel" << "/component/kwin" << "invokeShortcut" << "Show Desktop");
                    }
                }
            }
        }
    }
    return false;
}

void DesktopWindow::childDropEvent(QDropEvent* e) {
    qDebug() << "DesktopWindow::childDropEvent(QDropEvent* e)";
    bool moveItem = false;
    if(e->source() == listView_ && e->keyboardModifiers() == Qt::NoModifier) {
        // drag source is our list view, and no other modifier keys are pressed
        // => we're dragging desktop items
        const QMimeData *mimeData = e->mimeData();
        if(mimeData->hasFormat("application/x-qabstractitemmodeldatalist")) {
            QModelIndex dropIndex = listView_->indexAt(e->pos());
            if(dropIndex.isValid()) { // drop on an item
                QModelIndexList selected = selectedIndexes(); // the dragged items
                if(selected.contains(dropIndex)) { // drop on self, ignore
                    moveItem = true;
                }
            }
            else { // drop on a blank area
                moveItem = true;
            }
        }
    }
    if(moveItem)
        e->accept();
    else
        Fm::FolderView::childDropEvent(e);
}

void DesktopWindow::closeEvent(QCloseEvent *event) {
    // prevent the desktop window from being closed.
  event->ignore();
}

void DesktopWindow::onOpenFolder(QString folder)
{
    qDebug() << "DesktopWindow::onOpenFolder: " << folder;
    FmPath* path = fm_path_new_for_str(folder.toLocal8Bit().data());
    onOpenDirRequested(path, 0);
    fm_path_unref(path);
}

void DesktopWindow::onOpenFolderAndSelectItems(QString folder, QStringList items)
{
    FmPath* path = fm_path_new_for_str(folder.toLocal8Bit().data());

    // just raise the window if it's already open
    if (WindowRegistry::instance().checkPathAndSelectItems(fm_path_to_str(path), items)) {
      return;
    }

    Application* app = static_cast<Application*>(qApp);
    MainWindow* newWin = new MainWindow(path);
    // apply window size from app->settings
    if ( ! app->settings().spatialMode() ) {
      newWin->resize(app->settings().windowWidth(), app->settings().windowHeight());
      if(app->settings().windowMaximized()) {
              newWin->setWindowState(newWin->windowState() | Qt::WindowMaximized);
      }
    }
    newWin->show();
    TabPage* page = newWin->currentPage();
    if(page) {
      page->folderView()->selectFiles(items, false);
    }

    // Raise the window if it's open by now
    WindowRegistry::instance().checkPathAndRaise(fm_path_to_str(path));

    fm_path_unref(path);
}

void DesktopWindow::onOpenTrash()
{
    // onOpenDirRequested(fm_path_get_trash(), 0); // Do not use trash://
    FmPath* path;
    path = fm_path_new_for_str(QString(QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + "/Trash/files").toUtf8());
    onOpenDirRequested(path, 0);    // onOpenDirRequested(fm_path_get_trash(), 0); // Do not use trash://
    fm_path_unref(path);
}

void DesktopWindow::onOpenDesktop()
{
    onOpenDirRequested(fm_path_get_desktop(), 0);
}

void DesktopWindow::onOpenDocuments()
{
    FmPath* path;
    path = fm_path_new_for_str(QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation).toLocal8Bit().data());
    onOpenDirRequested(path, 0);
    fm_path_unref(path);
}

void DesktopWindow::onOpenDownloads()
{
    FmPath* path;
    path = fm_path_new_for_str(QStandardPaths::writableLocation(QStandardPaths::DownloadLocation).toLocal8Bit().data());
    onOpenDirRequested(path, 0);
    fm_path_unref(path);
}

void DesktopWindow::onOpenHome()
{
    onOpenDirRequested(fm_path_get_home(), 0);
}

void DesktopWindow::setScreenNum(int num) {
    if(screenNum_ != num) {
        screenNum_ = num;
        queueRelayout();
    }
}
