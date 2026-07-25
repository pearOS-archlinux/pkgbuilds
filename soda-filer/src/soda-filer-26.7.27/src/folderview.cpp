#include <QWindow>
#include <QGuiApplication>
/*
    <one line to give the library's name and an idea of what it does.>
    Copyright (C) 2012  Hong Jen Yee (PCMan) <pcman.tw@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/


#include "folderview.h"
#include "foldermodel.h"
#include "mainwindow.h"
#include <QHeaderView>
#include <QVBoxLayout>
#include <QContextMenuEvent>
#include "proxyfoldermodel.h"
#include "folderitemdelegate.h"
#include "tags.h"
#include "folderview_p.h"
#include "dndactionmenu.h"
#include "fileoperation.h"
#include "filemenu.h"
#include "foldermenu.h"
#include "filelauncher.h"
#include <QTimer>
#include <QDate>
#include <QDebug>
#include <QMimeData>
#include <QHoverEvent>
#include <QApplication>
#include <QScrollBar>
#include <QVariantAnimation>
#include <QEasingCurve>
#include <algorithm>
#include <cstring>
#include <QMetaType>
#include <QList>
#include <QStringList>
#include <QFileInfo>
#include <QDir>
#include <QProcess>
#include <QSettings>
#include "folderview_p.h"
#include "bundle.h"
#include "desktopwindow.h"
#include "desktopmainwindow.h"
#include "application.h"
#include "windowregistry.h"

Q_DECLARE_OPAQUE_POINTER(FmFileInfo*)

namespace Fm {

FolderViewListView::FolderViewListView(QWidget* parent):
  QListView(parent),
  activationAllowed_(true) {
  connect(this, &QListView::activated, this, &FolderViewListView::activation);
}

FolderViewListView::~FolderViewListView() {
}

void FolderViewListView::startDrag(Qt::DropActions supportedActions) {
  if(movement() != Static)
    QListView::startDrag(supportedActions);
  else
    QAbstractItemView::startDrag(supportedActions);
}

void FolderViewListView::mousePressEvent(QMouseEvent* event) {
  QListView::mousePressEvent(event);
  static_cast<FolderView*>(parent())->childMousePressEvent(event);
}

QModelIndex FolderViewListView::indexAt(const QPoint& point) const {
  QModelIndex index = QListView::indexAt(point);
  // NOTE: QListView has a severe design flaw here. It does hit-testing based on the
  // total bound rect of the item. The width of an item is determined by max(icon_width, text_width).
  // So if the text label is much wider than the icon, when you click outside the icon but
  // the point is still within the outer bound rect, the item is still selected.
  // This results in very poor usability. Let's do precise hit-testing here.
  // An item is hit only when the point is in the icon or text label.
  // If the point is in the bound rectangle but outside the icon or text, it should not be selected.
  if(viewMode() == QListView::IconMode && index.isValid()) {
    // FIXME: this hack only improves the usability partially. We still need more precise sizeHint handling.
    // FolderItemDelegate* delegate = static_cast<FolderItemDelegate*>(itemDelegateForColumn(FolderModel::ColumnFileName));
    // Q_ASSERT(delegate != NULL);
    // We use the grid size - (2, 2) as the size of the bounding rectangle of the whole item.
    // The width of the text label hence is gridSize.width - 2, and the width and height of the icon is from iconSize().
    QRect visRect = visualRect(index); // visibal area on the screen
    QSize itemSize = gridSize();
    itemSize.setWidth(itemSize.width() - 2);
    itemSize.setHeight(itemSize.height() - 2);
    QSize _iconSize = iconSize();
    int textHeight = itemSize.height() - _iconSize.height();
    if(point.y() < visRect.bottom() - textHeight) {
      // the point is in the icon area, not over the text label
      int iconXMargin = (itemSize.width() - _iconSize.width()) / 2;
      if(point.x() < (visRect.left() + iconXMargin) || point.x() > (visRect.right() - iconXMargin))
	return QModelIndex();
    }
    // qDebug() << "visualRect: " << visRect << "point:" << point;
  }
  return index;
}


// NOTE:
// QListView has a problem which I consider a bug or a design flaw.
// When you set movement property to Static, theoratically the icons
// should not be movable. However, if you turned on icon mode,
// the icons becomes freely movable despite the value of movement is Static.
// To overcome this bug, we override all drag handling methods, and
// call QAbstractItemView directly, bypassing QListView.
// In this way, we can workaround the buggy behavior.
// The drag handlers of QListView basically does the same things
// as its parent QAbstractItemView, but it also stores the currently
// dragged item and paint them in the view as needed.
// TODO: I really should file a bug report to Qt developers.

void FolderViewListView::dragEnterEvent(QDragEnterEvent* event) {
  qDebug() << "FolderViewListView::dragEnterEvent(QDragEnterEvent* event)";
  if(movement() != Static)
    QListView::dragEnterEvent(event);
  else
    QAbstractItemView::dragEnterEvent(event);
  qDebug("dragEnterEvent");
  //static_cast<FolderView*>(parent())->childDragEnterEvent(event);
}

void FolderViewListView::dragLeaveEvent(QDragLeaveEvent* e) {
  qDebug() << "FolderViewListView::dragLeaveEvent(QDragLeaveEvent* e)";
  if(movement() != Static)
    QListView::dragLeaveEvent(e);
  else
    QAbstractItemView::dragLeaveEvent(e);
  static_cast<FolderView*>(parent())->childDragLeaveEvent(e);
}

void FolderViewListView::dragMoveEvent(QDragMoveEvent* e) {
  // dragMove is called while you are moving over the target widget,
  // it doesn't replace dropEvent which is for when you drop on your target
  qDebug() << "FolderViewListView::dragMoveEvent(QDragMoveEvent* e)";

  if(this->dropIndicatorPosition() == QAbstractItemView::OnItem){
      qDebug() << "Drag and drop hovering over an item";
  }

  if(movement() != Static)
    QListView::dragMoveEvent(e);
  else
    QAbstractItemView::dragMoveEvent(e);
  static_cast<FolderView*>(parent())->childDragMoveEvent(e);
}

void FolderViewListView::dropEvent(QDropEvent* e) {
  qDebug() << "FolderViewListView::dropEvent(QDropEvent* e)";
  static_cast<FolderView*>(parent())->childDropEvent(e);

  if(movement() != Static)
    QListView::dropEvent(e);
  else
    QAbstractItemView::dropEvent(e);
}

void FolderViewListView::keyPressEvent(QKeyEvent* event) {
  if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
    if (state() != EditingState) {
        QModelIndexList selected = selectionModel()->selectedIndexes();
        if (!selected.isEmpty()) {
            edit(selected.first());
            return; // consume the event
        }
    }
  } else if (event->key() == Qt::Key_Space) {
      if (state() != EditingState) {
          Q_EMIT static_cast<FolderView*>(parent())->quickLookRequested();
          return; // consume the event
      }
  }
  QListView::keyPressEvent(event);
}

void FolderViewListView::mouseReleaseEvent(QMouseEvent* event) {
  qDebug() << "FolderViewListView::mouseReleaseEvent(QMouseEvent* event)";
  bool activationWasAllowed = activationAllowed_;
//  if ((!style()->styleHint(QStyle::SH_ItemView_ActivateItemOnSingleClick, NULL, this)) || (event->button() != Qt::LeftButton)) {
//    activationAllowed_ = false;
//  }

  QListView::mouseReleaseEvent(event);

  activationAllowed_ = activationWasAllowed;
}

void FolderViewListView::mouseDoubleClickEvent(QMouseEvent* event) {
  qDebug() << "FolderViewListView::mouseDoubleClickEvent(QMouseEvent* event)";
  bool activationWasAllowed = activationAllowed_;
//  if ((style()->styleHint(QStyle::SH_ItemView_ActivateItemOnSingleClick, NULL, this)) || (event->button() != Qt::LeftButton)) {
//    activationAllowed_ = false;
//  }

  QListView::mouseDoubleClickEvent(event);

  activationAllowed_ = activationWasAllowed;
}

void FolderViewListView::activation(const QModelIndex &index) {
  if (activationAllowed_) {
    Q_EMIT activatedFiltered(index);
  }
}

//-----------------------------------------------------------------------------

FolderViewTreeView::FolderViewTreeView(QWidget* parent):
  QTreeView(parent),
  layoutTimer_(NULL),
  doingLayout_(false),
  activationAllowed_(true) {

  header()->setStretchLastSection(true);
  setIndentation(0);

  connect(this, &QTreeView::activated, this, &FolderViewTreeView::activation);

  this->setAlternatingRowColors(true); // probono
}

FolderViewTreeView::~FolderViewTreeView() {
  if(layoutTimer_)
    delete layoutTimer_;
}

void FolderViewTreeView::setModel(QAbstractItemModel* model) {
  QTreeView::setModel(model);
  layoutColumns();
  if(ProxyFolderModel* proxyModel = qobject_cast<ProxyFolderModel*>(model)) {
    connect(proxyModel, &ProxyFolderModel::sortFilterChanged, this, &FolderViewTreeView::onSortFilterChanged,
            Qt::UniqueConnection);
    onSortFilterChanged();
  }
}

void FolderViewTreeView::mousePressEvent(QMouseEvent* event) {
  QTreeView::mousePressEvent(event);
  static_cast<FolderView*>(parent())->childMousePressEvent(event);
}

void FolderViewTreeView::dragEnterEvent(QDragEnterEvent* event) {
  QTreeView::dragEnterEvent(event);
  static_cast<FolderView*>(parent())->childDragEnterEvent(event);
}

void FolderViewTreeView::dragLeaveEvent(QDragLeaveEvent* e) {
  QTreeView::dragLeaveEvent(e);
  static_cast<FolderView*>(parent())->childDragLeaveEvent(e);
}

void FolderViewTreeView::dragMoveEvent(QDragMoveEvent* e) {
  QTreeView::dragMoveEvent(e);
  static_cast<FolderView*>(parent())->childDragMoveEvent(e);
}

void FolderViewTreeView::dropEvent(QDropEvent* e) {
  static_cast<FolderView*>(parent())->childDropEvent(e);
  QTreeView::dropEvent(e);
}

// the default list mode of QListView handles column widths
// very badly (worse than gtk+) and it's not very flexible.
// so, let's handle column widths outselves.
void FolderViewTreeView::layoutColumns() {
  // qDebug("layoutColumns");
  if(!model())
    return;
  doingLayout_ = true;
  QHeaderView* headerView = header();
  // the width that's available for showing the columns.
  int availWidth = viewport()->contentsRect().width();
  int desiredWidth = 0;

  // get the width that every column want
  int numCols = headerView->count();
  if(numCols > 0) {
    int* widths = new int[numCols]; // array to store the widths every column needs
    int column;
    for(column = 0; column < numCols; ++column) {
      int columnId = headerView->logicalIndex(column);
      // get the size that the column needs
      widths[column] = sizeHintForColumn(columnId);
      // compute the total width needed
      desiredWidth += widths[column];
    }

    int filenameColumn = headerView->visualIndex(FolderModel::ColumnFileName);
    // if the total witdh we want exceeds the available space
    if(desiredWidth > availWidth) {
      // Compute the width available for the filename column
      int filenameAvailWidth = availWidth - desiredWidth + widths[filenameColumn];

      // Compute the minimum acceptable width for the filename column
      int filenameMinWidth = qMin(200, sizeHintForColumn(filenameColumn));

      if (filenameAvailWidth > filenameMinWidth) {
        // Shrink the filename column to the available width
        widths[filenameColumn] = filenameAvailWidth;
      }
      else {
        // Set the filename column to its minimum width
        widths[filenameColumn] = filenameMinWidth;
      }
    }
    else {
      // Fill the extra available space with the filename column
      widths[filenameColumn] += availWidth - desiredWidth;
    }

    // really do the resizing for every column
    for(int column = 0; column < numCols; ++column) {
      headerView->resizeSection(column, widths[column]);
    }
    delete []widths;
  }
  doingLayout_ = false;

  if(layoutTimer_) {
    delete layoutTimer_;
    layoutTimer_ = NULL;
  }
}

void FolderViewTreeView::resizeEvent(QResizeEvent* event) {
  QAbstractItemView::resizeEvent(event);
  // prevent endless recursion.
  // When manually resizing columns, at the point where a horizontal scroll
  // bar has to be inserted or removed, the vertical size changes, a resize
  // event  occurs and the column headers are flickering badly if the column
  // layout is modified at this point. Therefore only layout the columns if
  // the horizontal size changes.
  if(!doingLayout_ && event->size().width() != event->oldSize().width())
    layoutColumns(); // layoutColumns() also triggers resizeEvent
}

void FolderViewTreeView::rowsInserted(const QModelIndex& parent, int start, int end) {
  QTreeView::rowsInserted(parent, start, end);
  queueLayoutColumns();
}

void FolderViewTreeView::rowsAboutToBeRemoved(const QModelIndex& parent, int start, int end) {
  QTreeView::rowsAboutToBeRemoved(parent, start, end);
  queueLayoutColumns();
}

void FolderViewTreeView::dataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight) {
  QTreeView::dataChanged(topLeft, bottomRight);
  // FIXME: this will be very inefficient
  // queueLayoutColumns();
}

void FolderViewTreeView::reset() {
  // Sometimes when the content of the model is radically changed, Qt does reset()
  // on the model rather than doing large amount of insertion and deletion.
  // This is for performance reason so in this case rowsInserted() and rowsAboutToBeRemoved()
  // might not be called. Hence we also have to re-layout the columns when the model is reset.
  // This fixes bug #190
  // https://github.com/lxde/filer-qt/issues/190
  QTreeView::reset();
  queueLayoutColumns();
}

void FolderViewTreeView::queueLayoutColumns() {
  // qDebug("queueLayoutColumns");
  if(!layoutTimer_) {
    layoutTimer_ = new QTimer();
    layoutTimer_->setSingleShot(true);
    layoutTimer_->setInterval(0);
    connect(layoutTimer_, &QTimer::timeout, this, &FolderViewTreeView::layoutColumns);
  }
  layoutTimer_->start();
}

void FolderViewTreeView::mouseReleaseEvent(QMouseEvent* event) {
  bool activationWasAllowed = activationAllowed_;
//  if ((!style()->styleHint(QStyle::SH_ItemView_ActivateItemOnSingleClick, NULL, this)) || (event->button() != Qt::LeftButton)) {
//    activationAllowed_ = false;
//  }

  QTreeView::mouseReleaseEvent(event);

  activationAllowed_ = activationWasAllowed;
}

void FolderViewTreeView::mouseDoubleClickEvent(QMouseEvent* event) {
  bool activationWasAllowed = activationAllowed_;
//  if ((style()->styleHint(QStyle::SH_ItemView_ActivateItemOnSingleClick, NULL, this)) || (event->button() != Qt::LeftButton)) {
//    activationAllowed_ = false;
//  }

  QTreeView::mouseDoubleClickEvent(event);

  activationAllowed_ = activationWasAllowed;
}

void FolderViewTreeView::keyPressEvent(QKeyEvent* event) {
  if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
    if (state() != EditingState) {
        QModelIndexList selected = selectionModel()->selectedIndexes();
        if (!selected.isEmpty()) {
            edit(selected.first());
            return; // consume the event
        }
    }
  } else if (event->key() == Qt::Key_Space) {
      if (state() != EditingState) {
          Q_EMIT static_cast<FolderView*>(parent())->quickLookRequested();
          return; // consume the event
      }
  }
  QTreeView::keyPressEvent(event);
}

void FolderViewTreeView::activation(const QModelIndex &index) {
  if (activationAllowed_) {
    Q_EMIT activatedFiltered(index);
  }
}

void FolderViewTreeView::onSortFilterChanged() {
  if(QSortFilterProxyModel* proxyModel = qobject_cast<QSortFilterProxyModel*>(model())) {
    header()->setSortIndicatorShown(true);
    header()->setSortIndicator(proxyModel->sortColumn(), proxyModel->sortOrder());
    if (!isSortingEnabled()) {
      setSortingEnabled(true);
    }
  }
}


//-----------------------------------------------------------------------------

FolderView::FolderView(ViewMode _mode, QWidget* parent):
  QWidget(parent),
  view(NULL),
  mode((ViewMode)0),
  autoSelectionDelay_(600),
  gridSpacing_(0),
  autoSelectionTimer_(NULL),
  selChangedTimer_(NULL),
  springLoadedFolderPath(""),
  springLoadedFolderTimer_(NULL),
  fileLauncher_(NULL),
  model_(NULL) {

  springLoadedFolderTimer_ = new QTimer();

  // Fixed folder-view appearance, independent of the desktop settings:
  // 64px icons, 12pt regular (weight 400) labels.
  QFont viewFont = font();
  viewFont.setPointSize(12);
  viewFont.setWeight(QFont::Normal);
  setFont(viewFont);

  iconSize_[IconMode - FirstViewMode] = QSize(64, 64);
  iconSize_[CompactMode - FirstViewMode] = QSize(24, 24);
  iconSize_[ThumbnailMode - FirstViewMode] = QSize(128, 128);
  iconSize_[DetailedListMode - FirstViewMode] = QSize(24, 24);

  QVBoxLayout* layout = new QVBoxLayout();
  layout->setMargin(0);
  setLayout(layout);

  setViewMode(_mode);
  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

  connect(this, &FolderView::clicked, this, &FolderView::onFileClicked);
}

FolderView::~FolderView() {
}

void FolderView::onItemActivated(QModelIndex index) {
  if(index.isValid() && index.model()) {
    QVariant data = index.model()->data(index, FolderModel::FileInfoRole);
    FmFileInfo* info = (FmFileInfo*)data.value<void*>();
    if(info) {
      if (!(QApplication::keyboardModifiers() & (Qt::ShiftModifier | Qt::ControlModifier | Qt::AltModifier | Qt::MetaModifier))) {
        Q_EMIT clicked(ActivatedClick, info);
      }
    }
  }
}

void FolderView::onSelChangedTimeout() {
  selChangedTimer_->deleteLater();
  selChangedTimer_ = NULL;

  QItemSelectionModel* selModel = selectionModel();
  int nSel = 0;
  if(viewMode() == DetailedListMode)
    nSel = selModel->selectedRows().count();
  else {
    nSel = selModel->selectedIndexes().count();
  }
  // qDebug()<<"selected:" << nSel;
  Q_EMIT selChanged(nSel); // FIXME: this is inefficient

  // probono: Scroll to the first selected item
  if(nSel)
      view->scrollTo(selModel->selectedIndexes().first(), QAbstractItemView::EnsureVisible);

}

void FolderView::onSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected) {
  // It's possible that the selected items change too often and this slot gets called for thousands of times.
  // For example, when you select thousands of files and delete them, we will get one selectionChanged() event
  // for every deleted file. So, we use a timer to delay the handling to avoid too frequent updates of the UI.
  if(!selChangedTimer_) {
    selChangedTimer_ = new QTimer(this);
    selChangedTimer_->setSingleShot(true);
    connect(selChangedTimer_, &QTimer::timeout, this, &FolderView::onSelChangedTimeout);
    selChangedTimer_->start(200);
  }
}


void FolderView::setViewMode(ViewMode _mode) {
  if(_mode == mode) // if it's the same more, ignore
    return;
  // FIXME: retain old selection

  // since only detailed list mode uses QTreeView, and others
  // all use QListView, it's wise to preserve QListView when possible.
  bool recreateView = false;
  if(view && (mode == DetailedListMode || _mode == DetailedListMode)) {
    delete view; // FIXME: no virtual dtor?
    view = NULL;
    recreateView = true;
  }
  mode = _mode;
  QSize iconSize = iconSize_[mode - FirstViewMode];

  if(mode == DetailedListMode) {
    FolderViewTreeView* treeView = new FolderViewTreeView(this);
    connect(treeView, &FolderViewTreeView::activatedFiltered, this, &FolderView::onItemActivated);
    connect(treeView, &FolderViewTreeView::quickLookRequested, this, &FolderView::quickLookRequested);

    treeView->setFrameStyle(QFrame::NoFrame); // probono: No border

    view = treeView;
    // FIXME: Make folders expandable; https://github.com/helloSystem/Filer/issues/115
    // What is missing? Do we need another model than a ProxyFolderModel?
    treeView->setItemsExpandable(true); // probono: was: false; but this seems to have no effect. QUESTION: Why?
    treeView->setRootIsDecorated(true); // probono: was: false; but this seems to have no effect. QUESTION: Why?
    treeView->setAnimated(true); // probono
    treeView->setAutoExpandDelay(2000); // probono: ms until items in a tree are opened during a drag and drop operation
    treeView->setAllColumnsShowFocus(true); // probono: was: false
    // Qt's default edit triggers include DoubleClicked, and
    // QAbstractItemView starts an item's rename edit BEFORE emitting
    // activated() on a double click -- when edit() succeeds (which it
    // always does here, since name cells carry Qt::ItemIsEditable),
    // activated() never fires at all, so double-clicking an item renamed it
    // instead of opening it. Same fix as DesktopWindow's listView_ (see its
    // constructor), just never applied to the base FolderView's own views
    // used by regular folder windows / the DMG viewer.
    treeView->setEditTriggers(QAbstractItemView::EditKeyPressed);

    // set our own custom delegate
    FolderItemDelegate* delegate = new FolderItemDelegate(treeView);
    treeView->setItemDelegateForColumn(FolderModel::ColumnFileName, delegate);
  }
  else {
    FolderViewListView* listView;    
    if(view)
      listView = static_cast<FolderViewListView*>(view);
    else {
      listView = new FolderViewListView(this);
      connect(listView, &FolderViewListView::activatedFiltered, this, &FolderView::onItemActivated);
      connect(listView, &FolderViewListView::quickLookRequested, this, &FolderView::quickLookRequested);
      view = listView;
    }

    listView->setFrameStyle(QFrame::NoFrame); // probono: No border
    // Qt's QAbstractItemView opaquely fills its viewport with QPalette::Base
    // by default, which otherwise hides MainWindow::paintEvent()'s
    // wallpaper-tinted background fill behind empty space in the view (gaps
    // between icons, area below the last row, ...).
    listView->viewport()->setAutoFillBackground(false);
    // See the matching comment above FolderViewTreeView::setEditTriggers()
    // in this function -- same double-click-renames-instead-of-opens fix.
    listView->setEditTriggers(QAbstractItemView::EditKeyPressed);

    // set our own custom delegate
    FolderItemDelegate* delegate = new FolderItemDelegate(listView);
    listView->setItemDelegateForColumn(FolderModel::ColumnFileName, delegate);
    // FIXME: should we expose the delegate?
    listView->setMovement(QListView::Static);
    listView->setResizeMode(QListView::Adjust);
    listView->setWrapping(true);
    listView->setStyleSheet("padding-right: 0px;");
    switch(mode) {
      case IconMode: {
        listView->setViewMode(QListView::IconMode);
        listView->setDropIndicatorShown(false); // probono
        // probono: Make objects (icons) freely movable. TODO: Could also snap to grid using QListView::Snap
        listView->setMovement(QListView::Free); // probono: https://doc.qt.io/qt-5/qlistview.html#Movement-enum
        listView->setResizeMode(QListView::Fixed);
        // No padding-top here (was 10px): QSS box-model padding on a
        // scrolling view's viewport doesn't interact correctly with Qt's
        // own scroll-region math -- it shifted the *visible* content down
        // at rest, but that offset didn't scroll consistently with the
        // rest of the content, reading as a residual/inconsistent gap
        // that came and went across relayouts (settings reload, resize).
        listView->setStyleSheet("padding-top: 0px;");
        listView->setWordWrap(true);
        listView->setFlow(QListView::LeftToRight); // probono: Why is there RightToLeft? We'd need it for the Desktop
        break;
      }
      case CompactMode: {
        listView->setViewMode(QListView::ListMode);
        listView->setDropIndicatorShown(true); // probono
        listView->setStyleSheet("padding-top: 0px;");
        listView->setWordWrap(false);
        listView->setFlow(QListView::QListView::TopToBottom);
        break;
      }
      case ThumbnailMode: {
        listView->setViewMode(QListView::IconMode);
        listView->setDropIndicatorShown(false); // probono
        // probono: Make objects (icons) freely movable. TODO: Could also snap to grid using QListView::Snap
        listView->setMovement(QListView::Free); // probono: https://doc.qt.io/qt-5/qlistview.html#Movement-enum
        listView->setResizeMode(QListView::Fixed);
        // See the matching comment in the IconMode case above.
        listView->setStyleSheet("padding-top: 0px;");
        listView->setWordWrap(true);
        listView->setFlow(QListView::LeftToRight);
        break;
      }
      default:;
    }
    updateGridSize();
  }
  if(view) {
    // we have to install the event filter on the viewport instead of the view itself.
    view->viewport()->installEventFilter(this);
    // we want the QEvent::HoverMove event for single click + auto-selection support
    view->viewport()->setAttribute(Qt::WA_Hover, true);
    view->setContextMenuPolicy(Qt::NoContextMenu); // defer the context menu handling to parent widgets
    view->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    view->setIconSize(iconSize);

    view->setSelectionMode(QAbstractItemView::ExtendedSelection);
    layout()->addWidget(view);
    view->setFont(font());

    // enable dnd
    view->setDragEnabled(true);
    view->setAcceptDrops(true);
    view->setDragDropMode(QAbstractItemView::DragDrop);
    view->setDropIndicatorShown(true);

    if(model_) {
      // FIXME: preserve selections
      model_->setThumbnailSize(iconSize.width());
      view->setModel(model_);
      if (!inherits("DesktopWindow")) {
        view->doItemsLayout();
      }
      if(recreateView)
        connect(view->selectionModel(), &QItemSelectionModel::selectionChanged, this, &FolderView::onSelectionChanged);
    }
  }
}

// set proper grid size for the QListView based on current view mode, icon size, and font size.
void FolderView::updateGridSize() {
  if(mode == DetailedListMode || !view)
    return;
  FolderViewListView* listView = static_cast<FolderViewListView*>(view);
  QSize icon = iconSize(mode); // size of the icon
  QFontMetrics fm = fontMetrics(); // size of current font
  QSize grid; // the final grid size
  int gridSpacingLeft = 60, gridSpacingRight = 60, gridSpacingTop = 5, gridSpacingBottom = 5;
  int textSize = 0, textMarginTop = 0, textMarginBottom = 0, textWeight = 0;
  QString textAnchor = QStringLiteral("center");
  Filer::Application* app = static_cast<Filer::Application*>(qApp);
  if(app) {
    Filer::Settings& settings = app->settings();
    gridSpacingLeft = settings.gridSpacingLeft();
    gridSpacingRight = settings.gridSpacingRight();
    gridSpacingTop = settings.gridSpacingTop();
    gridSpacingBottom = settings.gridSpacingBottom();
    textSize = settings.iconViewTextSize();
    textAnchor = settings.iconViewTextAnchor();
    textMarginTop = settings.iconViewTextMarginTop();
    textMarginBottom = settings.iconViewTextMarginBottom();
    textWeight = settings.iconViewTextWeight();
  }
  switch(mode) {
    case IconMode:
    case ThumbnailMode: {
      static const int kIconTextGap = 5;
      static const int kMaxTextHeight = 32;
      grid.setWidth(icon.width() + gridSpacingLeft + gridSpacingRight);
      // textMarginTop/Bottom added here too, not just passed to the
      // delegate: FolderItemDelegate::drawText() shrinks its own available
      // text area by the same margins, so without also growing the cell to
      // compensate, any nonzero margin just ate into the fixed
      // kMaxTextHeight budget -- past a few px it silently dropped every
      // visible line (the delegate's own overflow guard, not a crash).
      grid.setHeight(icon.height() + kIconTextGap + kMaxTextHeight + gridSpacingTop + gridSpacingBottom + textMarginTop + textMarginBottom);
      break;
    }
    default:
      ; // do not use grid size
  }
  listView->setGridSize(grid);
  // qobject_cast, not static_cast: DesktopWindow (also a FolderView, see
  // its updateFromSettings() -> setIconSize() -> here) installs its own
  // DesktopItemDelegate on this column instead, an unrelated sibling class
  // -- a static_cast reinterpreted that object's actual memory as a
  // FolderItemDelegate (silent undefined behavior; the smaller int-only
  // setters below apparently never landed on a field with a destructor,
  // so this went unnoticed) rather than the clean nullptr qobject_cast
  // gives when the type doesn't actually match. Confirmed as the cause of
  // a real crash (free(): invalid size) once setTextStyle() below started
  // writing a real QString through it.
  FolderItemDelegate* delegate = qobject_cast<FolderItemDelegate*>(listView->itemDelegateForColumn(FolderModel::ColumnFileName));
  if(delegate) {
    delegate->setGridSize(grid);
    delegate->setGridPadding(gridSpacingLeft, gridSpacingRight, gridSpacingTop, gridSpacingBottom);
    delegate->setTextStyle(textSize, textAnchor, textMarginTop, textMarginBottom, textWeight);
  }
}

void FolderView::setIconSize(ViewMode mode, QSize size) {
  Q_ASSERT(mode >= FirstViewMode && mode <= LastViewMode);
  iconSize_[mode - FirstViewMode] = size;
  if(viewMode() == mode) {
    view->setIconSize(size);
    if(model_)
      model_->setThumbnailSize(size.width());
    updateGridSize();
  }
}

QSize FolderView::iconSize(ViewMode mode) const {
  Q_ASSERT(mode >= FirstViewMode && mode <= LastViewMode);
  return iconSize_[mode - FirstViewMode];
}

FolderView::ViewMode FolderView::viewMode() const {
  return mode;
}

void FolderView::setAutoSelectionDelay(int delay) {
  autoSelectionDelay_ = delay;
}

QAbstractItemView* FolderView::childView() const {
  return view;
}

ProxyFolderModel* FolderView::model() const {
  return model_;
}

void FolderView::setModel(ProxyFolderModel* model) {
  if(view) {
    view->setModel(model);
    QSize iconSize = iconSize_[mode - FirstViewMode];
    model->setThumbnailSize(iconSize.width());
    if(view->selectionModel())
      connect(view->selectionModel(), &QItemSelectionModel::selectionChanged, this, &FolderView::onSelectionChanged);

    if (!inherits("DesktopWindow")) {
      connect(model, &QAbstractItemModel::modelReset, this, [this]() {
        if (view) {
          view->doItemsLayout();
        }
      });
      connect(model, &QAbstractItemModel::layoutChanged, this, [this]() {
        if (view) {
          view->doItemsLayout();
        }
      });
      connect(model, &QAbstractItemModel::rowsInserted, this, [this]() {
        if (view) {
          view->doItemsLayout();
        }
      });
      connect(model, &QAbstractItemModel::rowsRemoved, this, [this]() {
        if (view) {
          view->doItemsLayout();
        }
      });
    }
  }
  if(model_)
    delete model_;
  model_ = model;
}

bool FolderView::event(QEvent* event) {
  switch(event->type()) {
    case QEvent::StyleChange:
      break;
    case QEvent::FontChange:
      updateGridSize();
      break;
  };
  return QWidget::event(event);
}

void FolderView::contextMenuEvent(QContextMenuEvent* event) {
  QWidget::contextMenuEvent(event);
  
  if(QGuiApplication::platformName().contains("wayland")) {
      if(window()) {
          window()->activateWindow();
          window()->raise();
      }
  }
  
  QPoint pos = event->pos();
  QPoint view_pos = view->mapFromParent(pos);
  QPoint viewport_pos = view->viewport()->mapFromParent(view_pos);
  emitClickedAt(ContextMenuClick, viewport_pos);
}

void FolderView::childMousePressEvent(QMouseEvent* event) {
  // called from mousePressEvent() of child view
  Qt::MouseButton button = event->button();
  if(button == Qt::MiddleButton) {
    emitClickedAt(MiddleClick, event->pos());
  } else if (button == Qt::BackButton) {
    Q_EMIT clickedBack();
  } else if (button == Qt::ForwardButton) {
    Q_EMIT clickedForward();
  }
}

void FolderView::emitClickedAt(ClickType type, const QPoint& pos) {
  // indexAt() needs a point in "viewport" coordinates.
  QModelIndex index = view->indexAt(pos);
  if(index.isValid()) {
    QVariant data = index.data(FolderModel::FileInfoRole);
    FmFileInfo* info = reinterpret_cast<FmFileInfo*>(data.value<void*>());
    Q_EMIT clicked(type, info);
  }
  else {
    // FIXME: should we show popup menu for the selected files instead
    // if there are selected files?
    if(type == ContextMenuClick) {
      // clear current selection if clicked outside selected files
      view->clearSelection();
      Q_EMIT clicked(type, NULL);
    }
  }
}

QModelIndexList FolderView::selectedRows(int column) const {
  QItemSelectionModel* selModel = selectionModel();
  if(selModel) {
    return selModel->selectedRows(column);
  }
  return QModelIndexList();
}

// This returns all selected "cells", which means all cells of the same row are returned.
QModelIndexList FolderView::selectedIndexes() const {
  QItemSelectionModel* selModel = selectionModel();
  if(selModel) {
    return selModel->selectedIndexes();
  }
  return QModelIndexList();
}

QItemSelectionModel* FolderView::selectionModel() const {
  return view ? view->selectionModel() : NULL;
}

FmPathList* FolderView::selectedFilePaths() const {
  if(model_) {
    QModelIndexList selIndexes = mode == DetailedListMode ? selectedRows() : selectedIndexes();
    if(!selIndexes.isEmpty()) {
      FmPathList* paths = fm_path_list_new();
      QModelIndexList::const_iterator it;
      for(it = selIndexes.begin(); it != selIndexes.end(); ++it) {
        FmFileInfo* file = model_->fileInfoFromIndex(*it);
        fm_path_list_push_tail(paths, fm_file_info_get_path(file));
      }
      return paths;
    }
  }
  return NULL;
}

void FolderView::selectFiles(QStringList files, bool add) {
  if(!model_ || files.count() == 0)
    return;

  // maybe the model is not loaded yet. if so, save our desired selections
  // and this will be called again when loading is complete ^_^
  int count = model_->rowCount();
  if(count == 0) {
    qDebug() << "FolderView::selectFiles() Deferring until loading complete";
    ((FolderModel *)model_->sourceModel())->wantToSelect(files,add,this);
    return;
  }

  bool singleFile(files.count() == 1);
  QModelIndex index, firstIndex;

  QItemSelectionModel::SelectionFlags flags = QItemSelectionModel::Select;
  if(mode == DetailedListMode)
    flags |= QItemSelectionModel::Rows;

  for(int row = 0; row < count; ++row) {
    if(files.count() == 0)
      break;
    index = model_->index(row, 0);
    auto info = model_->fileInfoFromIndex(index);
    FmPath *infopath = fm_file_info_get_path(info);
    for(auto iter = files.begin(); iter != files.end(); ++iter) {
      FmFileInfo *fi = fm_file_info_new_from_native_file(nullptr, iter->toUtf8(), nullptr);
      FmPath *fipath = fm_file_info_get_path(fi);
      //qDebug() << "COMPARING " << fm_path_display_name(infopath,false)
      //	 << fm_path_display_name(fipath,false);
      if(fm_path_equal(infopath, fipath)) {
        if(!firstIndex.isValid()) {
          firstIndex = index;
          if(!add)
            selectionModel()->clear();
        }
        selectionModel()->select(index, flags);
        files.erase(iter);
        break;
      }
    }
  }

  if(firstIndex.isValid()) {
    view->scrollTo(firstIndex, QAbstractItemView::EnsureVisible);
    if(singleFile) {
      selectionModel()->setCurrentIndex(firstIndex, QItemSelectionModel::Current);
    }
  }
}

FmFileInfoList* FolderView::selectedFiles() const {
  if(model_) {
    QModelIndexList selIndexes = mode == DetailedListMode ? selectedRows() : selectedIndexes();
    if(!selIndexes.isEmpty()) {
      FmFileInfoList* files = fm_file_info_list_new();
      QModelIndexList::const_iterator it;
      for(it = selIndexes.constBegin(); it != selIndexes.constEnd(); ++it) {
        FmFileInfo* file = model_->fileInfoFromIndex(*it);
        fm_file_info_list_push_tail(files, file);
      }
      return files;
    }
  }
  return NULL;
}

void FolderView::selectAll() {
  if(mode == DetailedListMode)
    view->selectAll();
  else {
    // NOTE: By default QListView::selectAll() selects all columns in the model.
    // However, QListView only show the first column. Normal selection by mouse
    // can only select the first column of every row. I consider this discripancy yet
    // another design flaw of Qt. To make them consistent, we do it ourselves by only
    // selecting the first column of every row and do not select all columns as Qt does.
    // This will trigger one selectionChanged event per row, which is very inefficient,
    // but we have no other choices to workaround the Qt bug.
    // I'll report a Qt bug for this later.
    if(model_) {
      int rowCount = model_->rowCount();
      for(int row = 0; row < rowCount; ++row) {
        QModelIndex index = model_->index(row, 0);
        selectionModel()->select(index, QItemSelectionModel::Select);
      }
    }
  }
}

void FolderView::invertSelection() {
  if(model_) {
    QItemSelectionModel* selModel = view->selectionModel();
    int rows = model_->rowCount();
    QItemSelectionModel::SelectionFlags flags = QItemSelectionModel::Toggle;
    if(mode == DetailedListMode)
      flags |= QItemSelectionModel::Rows;
    for(int row = 0; row < rows; ++row) {
      QModelIndex index = model_->index(row, 0);
      selModel->select(index, flags);
    }
  }
}

void FolderView::childDragEnterEvent(QDragEnterEvent* event) {
  qDebug("FolderView::childDragEnterEvent(QDragEnterEvent* event) = drag enter");
  if(event->mimeData()->hasFormat("text/uri-list")) {
    event->accept();
  }
  else
    event->ignore();
}

void FolderView::childDragLeaveEvent(QDragLeaveEvent* e) {
  qDebug("FolderView::childDragLeaveEvent(QDragLeaveEvent* e) = drag leave");
  e->accept();
}

void FolderView::childDragMoveEvent(QDragMoveEvent* e) {
  // dragMove is called while you are moving over the target widget,
  // it doesn't replace dropEvent which is for when you drop on your target
  qDebug("FolderView::childDragMoveEvent(QDragMoveEvent* e) = drag move");

  qDebug() << "Alive? 1";
  // Here we got: QCoreApplication::postEvent: Unexpected null receiver
  // We need to check whether springLoadedFolderTimer_ is still alive
    // before we use it.
    if(springLoadedFolderTimer_ && springLoadedFolderTimer_->isActive()) {
        springLoadedFolderTimer_->stop();
        springLoadedFolderTimer_->deleteLater();
    }
  qDebug() << "Alive? 2";
  springLoadedFolderTimer_ = NULL;

  springLoadedFolderPath = "";
  qDebug() << "Alive? 4";

  // Find out what has been dragged onto what
  QStringList sourcePaths = {};
  qDebug() << "Alive? 5";
  QString destinationPath = "";
  const QMimeData *mimeData = e->mimeData();

  qDebug() << "MIME dropped:" << mimeData->formats();

  qDebug() << "Alive? 6";

  // probono: Find out the source path (where objects are coming from)
  QModelIndexList sourceIndexes = selectedIndexes(); // the dragged items (source)
  if(mimeData->hasFormat("text/uri-list")) {
      for ( const QUrl  url : mimeData->urls()) {
          qDebug() << "url from MIME data:" << url;
          if(url.scheme() == "file" && url.toLocalFile() != "") {
              sourcePaths.append(QString(url.toLocalFile()));
          } else {
              // A URL was dropped that is not a local file (e.g., from a web browser)
              // Let's assume we want to link those (which needs to be implemented elsewhere)
              e->setDropAction(Qt::LinkAction);
              return;
          }
      }
  } else if(mimeData->hasFormat("application/x-qabstractitemmodeldatalist")) {
      for (const QModelIndex index : sourceIndexes) {
          FmFileInfo* info = model_->fileInfoFromIndex(index);
          sourcePaths.append(QString(fm_path_to_str(fm_file_info_get_path(info))));
      }
  }
  qDebug() << "sourcePaths" << sourcePaths;



  // probono: Find out the destination path (where objects have been dropped)
  QModelIndex dropIndex = view->indexAt(e->pos()); // the item dropped on (destination)
  if(dropIndex.isValid()){
      // Dropped on an object (e.g., a folder or a document)
      QModelIndex index = model_->index(dropIndex.row(), 0);
      FmFileInfo* info = model_->fileInfoFromIndex(index);
      // animate...
      destinationPath = QString(fm_path_to_str(fm_file_info_get_path(info)));
  } else {
       return;
  }

  qDebug() << "destinationPath" << destinationPath;

  FmFileInfo *fileInfo = fm_file_info_new_from_native_file(nullptr, destinationPath.toUtf8(), nullptr);
  bool isAppDirOrBundle = Fm::checkWhetherAppDirOrBundle(fileInfo);
  fm_file_info_unref(fileInfo);

  if(QFileInfo(destinationPath).isDir() && ! isAppDirOrBundle && QFileInfo(sourcePaths.first()).dir() != destinationPath && sourcePaths.contains(destinationPath) == false) {
      if(QGuiApplication::platformName().contains("wayland")) {
          return;
      }
      qDebug() << "proboo: Open spring-loaded folder:" << destinationPath;
      if(!springLoadedFolderTimer_) {
          springLoadedFolderTimer_ = new QTimer(this);
          springLoadedFolderTimer_->setSingleShot(true);

          springLoadedFolderPath = destinationPath;

          connect(springLoadedFolderTimer_, &QTimer::timeout, this, &FolderView::onSpringLoadedFolderTimeout);
          springLoadedFolderTimer_->start(500);
      }
  }
}

void FolderView::onSpringLoadedFolderTimeout() {
    qDebug() << __func__;

    Filer::Application* app = static_cast<Filer::Application*>(qApp);

    if((app->springLoadedFolderPreviouslyOpened != "") && (app->springLoadedFolderPreviouslyOpened != springLoadedFolderPath)){
        qDebug() << "TODO: Close:" << app->springLoadedFolderPreviouslyOpened;
        // FIXME: It would be nice to close the window after some time; could not achieve this with QTimer. How to do this?
        // WindowRegistry::instance().checkPathAndClose(app->springLoadedFolderPreviouslyOpened);
    }

    bool isAlreadyOpen = WindowRegistry::instance().checkPathAndRaise(springLoadedFolderPath);
    if(!isAlreadyOpen){
        app->springLoadedFolderPreviouslyOpened = springLoadedFolderPath;
        app->launchFiles(NULL, {springLoadedFolderPath}, true);
    }
}

void FolderView::childDropEvent(QDropEvent* e) {
  qDebug("FolderView::childDropEvent(QDropEvent* e) = drop onto item");

  // Find out what has been dragged onto what
  QStringList sourcePaths = {};
  QString destinationPath = "";
  const QMimeData *mimeData = e->mimeData();

  qDebug() << "MIME dropped:" << mimeData->formats();

  // probono: Find out the source path (where objects are coming from)
  QModelIndexList sourceIndexes = selectedIndexes(); // the dragged items (source)
  if(mimeData->hasFormat("text/uri-list")) {
      for ( const QUrl  url : mimeData->urls()) {
          qDebug() << "url from MIME data:" << url;
          if(url.scheme() == "file" && url.toLocalFile() != "") {
              sourcePaths.append(QString(url.toLocalFile()));
          } else {
              // A URL was dropped that is not a local file (e.g., from a web browser)
              // Let's assume we want to link those (which needs to be implemented elsewhere)
              e->setDropAction(Qt::LinkAction);
              return;
          }
      }
  } else if(mimeData->hasFormat("application/x-qabstractitemmodeldatalist")) {
      for (const QModelIndex index : sourceIndexes) {
          FmFileInfo* info = model_->fileInfoFromIndex(index);
          sourcePaths.append(QString(fm_path_to_str(fm_file_info_get_path(info))));
      }
  }
  qDebug() << "sourcePaths" << sourcePaths;

  // probono: Find out the destination path (where objects have been dropped)
  QModelIndex dropIndex = view->indexAt(e->pos()); // the item dropped on (destination)
  if(dropIndex.isValid()){
      // Dropped on an object (e.g., a folder or a document)
      QModelIndex index = model_->index(dropIndex.row(), 0);
      FmFileInfo* info = model_->fileInfoFromIndex(index);
      if (info == nullptr)
          return;
      destinationPath = QString(fm_path_to_str(fm_file_info_get_path(info)));
  }

  // Check if we are dropping in the same directory (internal move/reposition)
  QString destDir = dropIndex.isValid() ? destinationPath : QString::fromUtf8(fm_path_to_str(path()));
  bool isSameDir = false;
  if (!sourcePaths.isEmpty()) {
      QString sourceDir = QFileInfo(sourcePaths[0]).absolutePath();
      if (QDir(sourceDir).canonicalPath() == QDir(destDir).canonicalPath()) {
          isSameDir = true;
      }
  }

  if (isSameDir) {
      qDebug() << "DND: same directory, repositioning icons on grid";
      e->setDropAction(Qt::MoveAction);
      e->accept();
      return;
  }

  // If we dropped on the background of a different directory:
  if (!dropIndex.isValid()) {
      qDebug("drop outside of an item");
      if(e->keyboardModifiers() == Qt::NoModifier) {
          e->setDropAction(Qt::CopyAction);
      }
      e->accept();
      return;
  }

  if (destinationPath.isEmpty())
      return;
  qDebug() << "destinationPath" << destinationPath;

  // probono: Check whether the destination is an application,
  // if so, launch it and open the dropped document
  if(e->keyboardModifiers() != Qt::NoModifier) {
      qDebug() << "Modifier key pressed. TODO: Implement opening all MIME types only in this case";
  }
  FmFileInfo *fileInfo = fm_file_info_new_from_native_file(nullptr, destinationPath.toUtf8(), nullptr);
  bool isAppDirOrBundle = Fm::checkWhetherAppDirOrBundle(fileInfo);
  fm_file_info_unref(fileInfo);
  if(isAppDirOrBundle) {
      e->setDropAction(Qt::IgnoreAction);
      qDebug() << "Opening using the 'launch' command";
      QProcess::startDetached("launch", QStringList({destinationPath}) + sourcePaths);
      return;
  }

  // probono: If item dropped on itself, do not ask which action to take
  if(sourcePaths.length() == 1 && sourcePaths[0] == destinationPath){
      e->setDropAction(Qt::MoveAction);
      return;
  }

  // probono: If dropped on Trash, do not ask which action to take
  if(QFileInfo(destinationPath).fileName() == "trash-can.desktop"){
      e->setDropAction(Qt::MoveAction);
      return;
  }

  // If dragging from a DMG, explicitly copy to destination to avoid Qt internal move issues
  if(!sourcePaths.isEmpty() && sourcePaths[0].startsWith("/tmp/pearos_dmg_mounts/")) {
      FmPathList* srcFiles = fm_path_list_new();
      for(const QString& p : sourcePaths) {
          fm_path_list_push_tail(srcFiles, fm_path_new_for_str(p.toUtf8().constData()));
      }
      FmPath* dest = fm_path_new_for_str(destinationPath.toUtf8().constData());
      FileOperation::copyFiles(srcFiles, dest);
      fm_path_list_unref(srcFiles);
      fm_path_unref(dest);
      
      e->setDropAction(Qt::IgnoreAction);
      e->accept();
      return;
  }

  if(e->keyboardModifiers() == Qt::NoModifier) {
      e->setDropAction(Qt::CopyAction);
  }
}

bool FolderView::eventFilter(QObject* watched, QEvent* event) {
  // NOTE: Instead of simply filtering the drag and drop events of the child view in
  // the event filter, we overrided each event handler virtual methods in
  // both QListView and QTreeView and added some childXXXEvent() callbacks.
  // We did this because of a design flaw of Qt.
  // All QAbstractScrollArea derived widgets, including QAbstractItemView
  // contains an internal child widget, which is called a viewport.
  // The events actually comes from the child viewport, not the parent view itself.
  // Qt redirects the events of viewport to the viewportEvent() method of
  // QAbstractScrollArea and let the parent widget handle the events.
  // Qt implemented this using a event filter installed on the child viewport widget.
  // That means, when we try to install an event filter on the viewport,
  // there is already a filter installed by Qt which will be called before ours.
  // So we can never intercept the event handling of QAbstractItemView by using a filter.
  // That's why we override respective virtual methods for different events.
  if(view && watched == view->viewport()) {
    switch(event->type()) {
    case QEvent::HoverMove:
      // activate items on single click
      // probono: this is an anti-feature
//      if(style()->styleHint(QStyle::SH_ItemView_ActivateItemOnSingleClick)) {
//        QHoverEvent* hoverEvent = static_cast<QHoverEvent*>(event);
//        QModelIndex index = view->indexAt(hoverEvent->pos()); // find out the hovered item
//        if(index.isValid()) { // change the cursor to a hand when hovering on an item
//          setCursor(Qt::PointingHandCursor);
//          if(!selectionModel()->hasSelection())
//            selectionModel()->setCurrentIndex(index, QItemSelectionModel::Current);
//        }
//        else
//          setCursor(Qt::ArrowCursor);
//        // turn on auto-selection for hovered item when single click mode is used.
//        if(autoSelectionDelay_ > 0 && model_) {
//          if(!autoSelectionTimer_) {
//            autoSelectionTimer_ = new QTimer(this);
//            connect(autoSelectionTimer_, &QTimer::timeout, this, &FolderView::onAutoSelectionTimeout);
//            lastAutoSelectionIndex_ = QModelIndex();
//          }
//          autoSelectionTimer_->start(autoSelectionDelay_);
//        }
//        break;
//      }
    case QEvent::HoverLeave:
//      if(style()->styleHint(QStyle::SH_ItemView_ActivateItemOnSingleClick))
//        setCursor(Qt::ArrowCursor);
      break;
    case QEvent::Wheel:
      // This is to fix #85: Scrolling doesn't work in compact view
      // Actually, I think it's the bug of Qt, not ours.
      // When in compact mode, only the horizontal scroll bar is used and the vertical one is hidden.
      // So, when a user scroll his mouse wheel, it's reasonable to scroll the horizontal scollbar.
      // Qt does not implement such a simple feature, unfortunately.
      // We do it by forwarding the scroll event in the viewport to the horizontal scrollbar.
      // FIXME: if someday Qt supports this, we have to disable the workaround.
      if(mode == CompactMode) {
        QScrollBar* scroll = view->horizontalScrollBar();
        if(scroll) {
          QApplication::sendEvent(scroll, event);
          return true;
        }
      }
      break;
    }
  }
  return QObject::eventFilter(watched, event);
}

// this slot handles auto-selection of items.
void FolderView::onAutoSelectionTimeout() {
  if(QApplication::mouseButtons() != Qt::NoButton)
    return;

  Qt::KeyboardModifiers mods = QApplication::keyboardModifiers();
  QPoint pos = view->viewport()->mapFromGlobal(QCursor::pos()); // convert to viewport coordinates
  QModelIndex index = view->indexAt(pos); // find out the hovered item
  QItemSelectionModel::SelectionFlags flags = (mode == DetailedListMode ? QItemSelectionModel::Rows : QItemSelectionModel::NoUpdate);
  QItemSelectionModel* selModel = view->selectionModel();

  if(mods & Qt::ControlModifier) { // Ctrl key is pressed
    if(selModel->isSelected(index) && index != lastAutoSelectionIndex_) {
      // unselect a previously selected item
      selModel->select(index, flags|QItemSelectionModel::Deselect);
      lastAutoSelectionIndex_ = QModelIndex();
    }
    else {
      // select an unselected item
      selModel->select(index, flags|QItemSelectionModel::Select);
      lastAutoSelectionIndex_ = index;
    }
    selModel->setCurrentIndex(index, QItemSelectionModel::NoUpdate); // move the cursor
  }
  else if(mods & Qt::ShiftModifier) { // Shift key is pressed
    // select all items between current index and the hovered index.
    QModelIndex current = selModel->currentIndex();
    if(selModel->hasSelection() && current.isValid()) {
      selModel->clear(); // clear old selection
      selModel->setCurrentIndex(current, QItemSelectionModel::NoUpdate);
      int begin = current.row();
      int end = index.row();
      if(begin > end)
        qSwap(begin, end);
      for(int row = begin; row <= end; ++row) {
        QModelIndex sel = model_->index(row, 0);
        selModel->select(sel, flags|QItemSelectionModel::Select);
      }
    }
    else { // no items are selected, select the hovered item.
      if(index.isValid()) {
        selModel->select(index, flags|QItemSelectionModel::SelectCurrent);
        selModel->setCurrentIndex(index, QItemSelectionModel::NoUpdate);
      }
    }
    lastAutoSelectionIndex_ = index;
  }
  else if(mods == Qt::NoModifier) { // no modifier keys are pressed.
    if(index.isValid()) {
      // select the hovered item
      view->clearSelection();
      selModel->select(index, flags|QItemSelectionModel::SelectCurrent);
      selModel->setCurrentIndex(index, QItemSelectionModel::NoUpdate);
    }
    lastAutoSelectionIndex_ = index;
  }

  autoSelectionTimer_->deleteLater();
  autoSelectionTimer_ = NULL;
}

void FolderView::onFileClicked(int type, FmFileInfo* fileInfo) {
  if(type == ActivatedClick) {
    if(fileLauncher_) {
      GList* files = g_list_append(NULL, fileInfo);
      fileLauncher_->launchFiles(NULL, files);
      g_list_free(files);
    }
  }
  else if(type == ContextMenuClick) {
    FmPath* folderPath = NULL;
    FmFileInfoList* files = selectedFiles();
    if (files) {
      FmFileInfo* first = fm_file_info_list_peek_head(files);
      if (fm_file_info_list_get_length(files) == 1 && fm_file_info_is_dir(first))
        folderPath = fm_file_info_get_path(first);
    }
    if (!folderPath)
      folderPath = path();
    QMenu* menu = NULL;
    if(fileInfo) {
      // show context menu
      if (FmFileInfoList* files = selectedFiles()) {
        Fm::FileMenu* fileMenu = new Fm::FileMenu(files, fileInfo, folderPath, this);
        fileMenu->setFileLauncher(fileLauncher_);
        connect(fileMenu, &Fm::FileMenu::quickLookRequested, this, &FolderView::quickLookRequested);
        connect(fileMenu, &Fm::FileMenu::selectFileRequested, this, &FolderView::selectFileAfterCreation);
        prepareFileMenu(fileMenu);
        fm_file_info_list_unref(files);
        menu = fileMenu;
      }
    }
    else {
      Fm::FolderMenu* folderMenu = new Fm::FolderMenu(this, this);
      prepareFolderMenu(folderMenu);
      menu = folderMenu;
    }
    if (menu) {
      menu->exec(QCursor::pos());
      delete menu;
    }
  }
}

bool FolderView::selectItemByName(const QString& name) {
  if(!model_ || !view)
    return false;
  int count = model_->rowCount();
  for(int row = 0; row < count; ++row) {
    QModelIndex index = model_->index(row, 0);
    FmFileInfo* file = model_->fileInfoFromIndex(index);
    if(file && name == QString::fromUtf8(fm_file_info_get_name(file))) {
      view->selectionModel()->select(index, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
      view->selectionModel()->setCurrentIndex(index, QItemSelectionModel::NoUpdate);
      return true;
    }
  }
  return false;
}

void FolderView::selectFileAfterCreation(QString path) {
  QString name = QFileInfo(path).fileName();
  if(selectItemByName(name))
    return;
  // the folder monitor hasn't picked the new file up yet -- poll briefly
  QTimer* timer = new QTimer(this);
  timer->setInterval(100);
  int* tries = new int(0);
  connect(timer, &QTimer::timeout, this, [this, timer, name, tries]() {
    if(selectItemByName(name) || ++(*tries) >= 30) {
      timer->stop();
      timer->deleteLater();
      delete tries;
    }
  });
  timer->start();
}

void FolderView::prepareFileMenu(FileMenu* menu) {
}

void FolderView::prepareFolderMenu(FolderMenu* menu) {
}

namespace {
// Per-column comparator for Clean Up By, mirroring desktopItemLessThan()
// in desktopwindow.cpp (keep the two in sync) -- a LOCAL sort of the file
// list; the model itself is never sorted, for the same reasons documented
// on DesktopWindow::computeGridPositionsSorted().
bool folderItemLessThan(FmFileInfo* a, FmFileInfo* b, int column) {
  if(column == FolderModel::ColumnFileMTime)
    return fm_file_info_get_mtime(a) < fm_file_info_get_mtime(b);
  if(column == FolderModel::ColumnFileCTime)
    return fm_file_info_get_ctime(a) < fm_file_info_get_ctime(b);
  if(column == FolderModel::ColumnFileSize) {
    bool aDir = fm_file_info_is_dir(a), bDir = fm_file_info_is_dir(b);
    if(aDir != bDir)
      return aDir;
    return fm_file_info_get_size(a) < fm_file_info_get_size(b);
  }
  if(column == FolderModel::ColumnFileType) {
    bool aDir = fm_file_info_is_dir(a), bDir = fm_file_info_is_dir(b);
    if(aDir != bDir)
      return aDir;
    QString ta = QString::fromUtf8(fm_mime_type_get_desc(fm_file_info_get_mime_type(a)));
    QString tb = QString::fromUtf8(fm_mime_type_get_desc(fm_file_info_get_mime_type(b)));
    return ta.localeAwareCompare(tb) < 0;
  }
  if(column == FolderModel::ColumnFileTags) {
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
  return strcmp(fm_file_info_get_collate_key(a), fm_file_info_get_collate_key(b)) < 0;
}
}

void FolderView::cleanUpIcons() {
  if(mode != IconMode && mode != ThumbnailMode)
    return;
  QVector<QModelIndex> order;
  for(int row = 0; row < model_->rowCount(); ++row)
    order.push_back(model_->index(row, 0));
  animateIconsTo(order);
}

void FolderView::cleanUpIconsBy(int column) {
  if(mode != IconMode && mode != ThumbnailMode)
    return;
  QVector<QPair<QModelIndex, FmFileInfo*> > items;
  for(int row = 0; row < model_->rowCount(); ++row) {
    QModelIndex index = model_->index(row, 0);
    items.push_back(qMakePair(index, model_->fileInfoFromIndex(index)));
  }
  std::stable_sort(items.begin(), items.end(), [column](const QPair<QModelIndex, FmFileInfo*>& a, const QPair<QModelIndex, FmFileInfo*>& b) {
      return folderItemLessThan(a.second, b.second, column);
  });
  QVector<QModelIndex> order;
  for(const auto& item : items)
    order.push_back(item.first);
  animateIconsTo(order);
}

// Slides icons into row-major grid order (left-to-right, top-to-bottom --
// the folder-window flow, unlike the desktop's top-right columns) on a
// single shared animation timeline, same 300ms InOutQuad as
// DesktopWindow::animateRelayout().
void FolderView::animateIconsTo(const QVector<QModelIndex>& order) {
  FolderViewListView* listView = static_cast<FolderViewListView*>(view);
  QSize grid = listView->gridSize();
  if(!grid.isValid() || order.isEmpty())
    return;
  int spacing = listView->spacing();
  int stepX = grid.width() + spacing;
  int stepY = grid.height() + spacing;
  int cols = qMax(1, (listView->viewport()->width() + spacing) / stepX);
  QPoint scrollOffset(listView->horizontalScrollBar()->value(), listView->verticalScrollBar()->value());

  struct Move { QPersistentModelIndex index; QPoint start; QPoint end; QPoint contentsEnd; };
  QVector<Move> moves;
  QVector<QPair<QPersistentModelIndex, QPoint> > finalPositions;
  for(int i = 0; i < order.size(); ++i) {
    const QModelIndex& index = order.at(i);
    // Viewport coordinates for the animation, contents coordinates for the
    // final setPositionForIndex() (they differ by the scroll offset).
    QPoint end((i % cols) * stepX - scrollOffset.x(), (i / cols) * stepY - scrollOffset.y());
    QPoint contentsEnd = end + scrollOffset;
    QPoint start = listView->visualRect(index).topLeft();
    finalPositions.push_back(qMakePair(QPersistentModelIndex(index), contentsEnd));
    if(start != end)
      moves.push_back({QPersistentModelIndex(index), start, end, contentsEnd});
  }

  // qobject_cast, not static_cast: DesktopWindow (also a FolderView) installs
  // its own DesktopItemDelegate on this same column instead, an unrelated
  // sibling class -- a static_cast there reinterprets that object's memory
  // as a FolderItemDelegate, silent undefined behavior rather than the
  // clean nullptr qobject_cast gives when the type doesn't actually match.
  FolderItemDelegate* delegate = qobject_cast<FolderItemDelegate*>(listView->itemDelegateForColumn(FolderModel::ColumnFileName));

  auto applyFinal = [listView, finalPositions, delegate]() {
    if(delegate)
      delegate->clearAnimationOffsets();
    for(const auto& fp : finalPositions) {
      if(fp.first.isValid())
        listView->setPositionForIndex(fp.second, fp.first);
    }
    listView->viewport()->update();
  };

  if(moves.isEmpty() || !delegate) {
    applyFinal();
    return;
  }

  QVariantAnimation* anim = new QVariantAnimation(listView);
  anim->setDuration(300);
  anim->setEasingCurve(QEasingCurve::InOutQuad);
  anim->setStartValue(0.0);
  anim->setEndValue(1.0);
  connect(anim, &QVariantAnimation::valueChanged, listView, [listView, delegate, moves](const QVariant& v) {
      qreal t = v.toReal();
      QHash<QPersistentModelIndex, QPoint> offsets;
      for(const auto& m : moves) {
        if(!m.index.isValid())
          continue;
        QPoint p = m.start + QPointF((m.end - m.start) * t).toPoint();
        offsets[m.index] = p - m.start;
      }
      delegate->setAnimationOffsets(offsets);
      listView->viewport()->update();
  });
  connect(anim, &QVariantAnimation::finished, listView, applyFinal);
  anim->start(QAbstractAnimation::DeleteWhenStopped);
}

} // namespace Fm
