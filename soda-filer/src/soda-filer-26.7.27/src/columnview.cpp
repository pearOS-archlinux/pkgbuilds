#include "columnview.h"
#include "cachedfoldermodel.h"
#include "proxyfoldermodel.h"
#include "foldermodel.h"
#include "icontheme.h"
#include "tags.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QListView>
#include <QItemSelectionModel>
#include <QScrollBar>
#include <QTimer>
#include <QLabel>
#include <QFrame>
#include <QDateTime>
#include <QStyledItemDelegate>

namespace Fm {

static const int kColumnWidth = 220;
static const int kInfoPanelWidth = 240;

namespace {
// The row list and the Quick-Look-style info panel share the same
// thumbnail-enabled proxy model (the info panel needs real image/video
// previews), but the rows themselves should only ever show the generic
// mime-type icon -- real previews belong solely to the dedicated info
// panel. This delegate overrides the row icon back to the mime-type icon,
// discarding whatever thumbnail QImage the shared proxy's DecorationRole
// returns.
class ColumnRowIconDelegate : public QStyledItemDelegate {
public:
  explicit ColumnRowIconDelegate(QObject* parent = nullptr): QStyledItemDelegate(parent) {}

protected:
  void initStyleOption(QStyleOptionViewItem* option, const QModelIndex& index) const override {
    QStyledItemDelegate::initStyleOption(option, index);
    if(FmFileInfo* info = static_cast<FmFileInfo*>(index.data(FolderModel::FileInfoRole).value<void*>()))
      option->icon = IconTheme::icon(fm_file_info_get_icon(info));
    // QStyledItemDelegate::initStyleOption() sizes decorationSize off the
    // *raw* Qt::DecorationRole value -- still the proxy's 96px thumbnail
    // QImage, even though the icon above just got replaced -- so rows
    // stayed thumbnail-tall despite showing a plain mime icon. Clamp back
    // to the row list's actual icon size (see addColumn()'s
    // col.view->setIconSize()).
    if(option->decorationSize.height() > 24 || option->decorationSize.width() > 24)
      option->decorationSize = QSize(24, 24);
  }

  QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override {
    QSize size = QStyledItemDelegate::sizeHint(option, index);
    int compactHeight = qMax(24, option.fontMetrics.height()) + 8;
    if(size.height() > compactHeight)
      size.setHeight(compactHeight);
    return size;
  }
};
} // namespace

ColumnView::ColumnView(QWidget* parent):
  QScrollArea(parent),
  currentInfo_(NULL),
  infoPanelShown_(false) {
  setWidgetResizable(true);
  setFrameStyle(QFrame::NoFrame);
  setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
  setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

  container_ = new QWidget(this);
  hbox_ = new QHBoxLayout(container_);
  hbox_->setContentsMargins(0, 0, 0, 0);
  hbox_->setSpacing(0);

  // Quick-Look-style info panel for a selected plain file: large icon,
  // name, kind, then "Information" (Created/Modified/Tags). Built once,
  // hidden until a file (not a directory) is selected.
  infoPanel_ = new QWidget(container_);
  // Unlike the fixed-width file columns, the info panel is the last thing
  // in the chain and should soak up all remaining horizontal space (see
  // removeTrailingStretch()/showInfoPanel()) rather than sit at a fixed
  // narrow width.
  infoPanel_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  infoPanel_->setMinimumWidth(kInfoPanelWidth);
  QVBoxLayout* infoBox = new QVBoxLayout(infoPanel_);
  infoBox->setContentsMargins(16, 16, 16, 16);
  infoBox->setSpacing(4);

  infoIcon_ = new QLabel(infoPanel_);
  infoIcon_->setAlignment(Qt::AlignCenter);
  infoIcon_->setFixedHeight(200);
  infoBox->addWidget(infoIcon_);

  infoName_ = new QLabel(infoPanel_);
  infoName_->setAlignment(Qt::AlignCenter);
  infoName_->setWordWrap(true);
  QFont boldFont = infoName_->font();
  boldFont.setBold(true);
  infoName_->setFont(boldFont);
  infoBox->addWidget(infoName_);

  infoKind_ = new QLabel(infoPanel_);
  infoKind_->setAlignment(Qt::AlignCenter);
  infoKind_->setWordWrap(true);
  infoBox->addSpacing(4);
  infoBox->addWidget(infoKind_);

  infoBox->addSpacing(12);
  QFrame* sep = new QFrame(infoPanel_);
  sep->setFrameShape(QFrame::HLine);
  infoBox->addWidget(sep);
  infoBox->addSpacing(4);

  QLabel* infoHeading = new QLabel(tr("Information"), infoPanel_);
  QFont headingFont = infoHeading->font();
  headingFont.setBold(true);
  infoHeading->setFont(headingFont);
  infoBox->addWidget(infoHeading);

  infoCreated_ = new QLabel(infoPanel_);
  infoCreated_->setWordWrap(true);
  infoBox->addWidget(infoCreated_);
  infoModified_ = new QLabel(infoPanel_);
  infoModified_->setWordWrap(true);
  infoBox->addWidget(infoModified_);
  infoTags_ = new QLabel(infoPanel_);
  infoTags_->setWordWrap(true);
  infoBox->addWidget(infoTags_);

  infoBox->addStretch(1);
  infoPanel_->setVisible(false);
  // infoPanel_ is NOT added to hbox_ here -- it's inserted/removed exactly
  // like a column (see showInfoPanel()/hideInfoPanel()), always occupying
  // the "next column" slot right after the selection that revealed it.

  hbox_->addStretch(1); // keeps columns (and infoPanel_, when shown) left-packed
  setWidget(container_);
}

ColumnView::~ColumnView() {
  removeColumnsAfter(-1);
  setCurrentInfo(NULL);
}

void ColumnView::setRootPath(FmPath* path) {
  removeColumnsAfter(-1);
  hideInfoPanel();
  setCurrentInfo(NULL);
  if(path)
    addColumn(path);
}

void ColumnView::setCurrentInfo(FmFileInfo* info) {
  if(currentInfo_)
    fm_file_info_unref(currentInfo_);
  currentInfo_ = info ? fm_file_info_ref(info) : NULL;
}

void ColumnView::showInfoPanel(FmFileInfo* info, ProxyFolderModel* proxy, const QModelIndex& proxyIndex) {
  QString path = QString::fromUtf8(fm_path_to_str(fm_file_info_get_path(info)));

  // Real image/video preview when available: the column's own proxy model
  // already has thumbnails turned on (see addColumn()), backed by the same
  // GIO/gdk-pixbuf thumbnailers the Icons/List/Gallery views use -- covers
  // videos too, not just images, as long as a thumbnailer is installed.
  // Falls back to the plain mime-type icon otherwise, and gets upgraded to
  // the real thumbnail asynchronously via the dataChanged connection in
  // addColumn() once it finishes loading.
  QIcon fallbackIcon = IconTheme::icon(fm_file_info_get_icon(info));
  infoIcon_->setPixmap(fallbackIcon.pixmap(96, 96));
  if(proxy && proxyIndex.isValid()) {
    QVariant dec = proxy->data(proxyIndex, Qt::DecorationRole);
    if(dec.canConvert<QImage>()) {
      QImage img = dec.value<QImage>();
      if(!img.isNull())
        infoIcon_->setPixmap(QPixmap::fromImage(img).scaled(
            QSize(280, 200), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
  }
  infoName_->setText(QString::fromUtf8(fm_file_info_get_disp_name(info)));

  FmMimeType* mimeType = fm_file_info_get_mime_type(info);
  infoKind_->setText(mimeType ? QString::fromUtf8(fm_mime_type_get_desc(mimeType)) : QString());

  QDateTime ctime = QDateTime::fromSecsSinceEpoch(fm_file_info_get_ctime(info));
  QDateTime mtime = QDateTime::fromSecsSinceEpoch(fm_file_info_get_mtime(info));
  infoCreated_->setText(tr("Created: %1").arg(ctime.toString("d MMM yyyy 'at' hh:mm")));
  infoModified_->setText(tr("Modified: %1").arg(mtime.toString("d MMM yyyy 'at' hh:mm")));

  bool ok = false;
  QString tag = getFileTag(path, ok);
  infoTags_->setText(tr("Tags: %1").arg((ok && !tag.isEmpty()) ? tag : tr("None")));

  if(!infoPanelShown_) {
    // Occupies the "next column" slot, right after the last real column --
    // but unlike a fixed-width column, nothing (no trailing stretch) comes
    // after it, so its Expanding size policy lets it soak up all remaining
    // width in the scroll area, per the user's request.
    removeTrailingStretch();
    hbox_->addWidget(infoPanel_);
    infoPanelShown_ = true;
  }
  infoPanel_->setVisible(true);
}

void ColumnView::hideInfoPanel() {
  if(infoPanelShown_) {
    hbox_->removeWidget(infoPanel_);
    infoPanelShown_ = false;
    hbox_->addStretch(1); // restore left-packing of the fixed-width columns
  }
  infoPanel_->setVisible(false);
}

void ColumnView::removeTrailingStretch() {
  if(hbox_->count() > 0) {
    QLayoutItem* last = hbox_->itemAt(hbox_->count() - 1);
    if(last && last->spacerItem()) {
      hbox_->removeItem(last);
      delete last;
    }
  }
}

int ColumnView::columnIndexOf(const QObject* view) const {
  for(int i = 0; i < columns_.size(); ++i) {
    if(columns_.at(i).view == view)
      return i;
  }
  return -1;
}

void ColumnView::addColumn(FmPath* path) {
  Column col;
  col.model = CachedFolderModel::modelFromPath(path);
  col.proxy = new ProxyFolderModel();
  col.proxy->setSourceModel(col.model);
  col.proxy->sort(FolderModel::ColumnFileName, Qt::AscendingOrder);
  // Real thumbnails (images AND videos, via the same GIO/gdk-pixbuf
  // thumbnailers the Icons/List/Gallery views use) for both this column's
  // own rows and the big info-panel preview when one of them is selected --
  // see the dataChanged connection below, which upgrades the panel once an
  // async thumbnail finishes loading.
  col.proxy->setShowThumbnails(true);
  col.proxy->setThumbnailSize(96);

  col.view = new QListView(container_);
  col.view->setViewMode(QListView::ListMode);
  col.view->setUniformItemSizes(true);
  col.view->setIconSize(QSize(24, 24));
  col.view->setFixedWidth(kColumnWidth);
  col.view->setFrameStyle(QFrame::NoFrame);
  col.view->setEditTriggers(QAbstractItemView::NoEditTriggers);
  col.view->setSelectionMode(QAbstractItemView::SingleSelection);
  col.view->setModel(col.proxy);
  col.view->setItemDelegate(new ColumnRowIconDelegate(col.view));
  // A visible right divider between columns, like Finder's.
  col.view->setStyleSheet(QStringLiteral("QListView { border: none; border-right: 1px solid rgba(127, 127, 127, 60); }"));

  hbox_->insertWidget(hbox_->count() - 1, col.view); // before the trailing stretch
  columns_.append(col);

  // Upgrades the info panel's preview once an async thumbnail (image or
  // video) finishes loading for the row it's currently showing.
  connect(col.proxy, &QAbstractItemModel::dataChanged, this,
          [this, proxy = col.proxy](const QModelIndex& topLeft, const QModelIndex&) {
      if(!infoPanelShown_ || !currentInfo_)
        return;
      FmFileInfo* rowInfo = proxy->fileInfoFromIndex(topLeft);
      if(!rowInfo || !fm_path_equal(fm_file_info_get_path(rowInfo), fm_file_info_get_path(currentInfo_)))
        return;
      QVariant dec = proxy->data(topLeft, Qt::DecorationRole);
      if(dec.canConvert<QImage>()) {
        QImage img = dec.value<QImage>();
        if(!img.isNull())
          infoIcon_->setPixmap(QPixmap::fromImage(img).scaled(
              QSize(280, 200), Qt::KeepAspectRatio, Qt::SmoothTransformation));
      }
  });

  // Selecting a directory: truncate the chain after this column, open the
  // directory in a fresh column to the right. Selecting a plain file
  // instead shows the Quick-Look-style info panel in that same "next
  // column" slot. Either way, auto-scroll to reveal it -- no matter how
  // many columns are already open (matches Finder).
  connect(col.view->selectionModel(), &QItemSelectionModel::currentChanged, this,
          [this, view = col.view, proxy = col.proxy](const QModelIndex& current, const QModelIndex&) {
      int idx = columnIndexOf(view);
      if(idx < 0 || !current.isValid())
        return;
      FmFileInfo* info = static_cast<FmFileInfo*>(current.data(FolderModel::FileInfoRole).value<void*>());
      if(!info)
        return;
      removeColumnsAfter(idx);
      hideInfoPanel();
      setCurrentInfo(info);
      if(fm_file_info_is_dir(info)) {
        FmPath* dirPath = fm_file_info_get_path(info);
        addColumn(dirPath);
        Q_EMIT dirSelected(dirPath);
      }
      else {
        showInfoPanel(info, proxy, current);
        Q_EMIT fileSelected(info);
      }
      // Scroll so the newly revealed column/info panel is visible once the
      // layout has actually placed it (hence queued, not immediate).
      QTimer::singleShot(0, this, [this]() {
          horizontalScrollBar()->setValue(horizontalScrollBar()->maximum());
      });
  });

  connect(col.view, &QListView::activated, this, [this](const QModelIndex& index) {
      FmFileInfo* info = static_cast<FmFileInfo*>(index.data(FolderModel::FileInfoRole).value<void*>());
      if(info && !fm_file_info_is_dir(info))
        Q_EMIT fileActivated(info);
  });
}

// Removes every column strictly after `index` (so -1 clears all).
void ColumnView::removeColumnsAfter(int index) {
  while(columns_.size() > index + 1) {
    Column col = columns_.takeLast();
    delete col.view;   // also disconnects its signals
    delete col.proxy;
    col.model->unref(); // CachedFolderModel is refcounted, not delete-able
  }
}

}
