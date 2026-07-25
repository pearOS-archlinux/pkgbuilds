#include "opendialog.h"
#include "cachedfoldermodel.h"
#include "proxyfoldermodel.h"
#include "foldermodel.h"
#include "folderview.h"
#include "placesview.h"
#include "sidepane.h"
#include "columnview.h"
#include "application.h"

#include <KWindowEffects>

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QToolButton>
#include <QLineEdit>
#include <QStackedWidget>
#include <QMenu>
#include <QActionGroup>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QApplication>
#include <QStyle>
#include <QScreen>
#include <QUrl>
#include <QFileInfo>
#include <QDir>
#include <QStandardPaths>
#include <QStorageInfo>
#include <QPainter>
#include <QPainterPath>

namespace Filer {

static const int kPanelMinWidth = 640;
static const int kEdgeGrip = 10; // px hit area on the panel's side edges
static const int kSidebarWidth = 170;
static const int kMaxRecentPlaces = 8;
static constexpr qreal kPanelCornerRadius = 12; // matches MainWindow's kWindowCornerRadius family

// QIcon::fromTheme's two-argument overload only offers one fallback name;
// the pearOS icon theme doesn't necessarily ship every name used below
// (e.g. "view-sort-ascending"), which previously left some toolbar buttons
// rendering with no icon at all -- and, being icon-only with no text,
// effectively invisible. QStyle::standardIcon() always returns something,
// so it's the guaranteed-visible last resort.
static QIcon themedIcon(const QString& name, const QString& altName, QStyle::StandardPixmap fallback) {
    QIcon icon = QIcon::fromTheme(name);
    if(icon.isNull())
        icon = QIcon::fromTheme(altName);
    if(icon.isNull())
        icon = qApp->style()->standardIcon(fallback);
    return icon;
}

// Filters the icons/list proxy model by the search box's text (substring,
// case-insensitive, against the display name). Column view has no exposed
// per-column proxy to hook into, so the search box only affects Icons/List.
namespace {
class SearchFilter : public Fm::ProxyFolderModelFilter {
public:
    bool filterAcceptsRow(const Fm::ProxyFolderModel*, FmFileInfo* info) const override {
        if(text_.isEmpty())
            return true;
        return QString::fromUtf8(fm_file_info_get_disp_name(info)).contains(text_, Qt::CaseInsensitive);
    }
    void setText(const QString& text) {
        text_ = text;
    }
private:
    QString text_;
};
}

OpenDialog::OpenDialog(FmPath* startPath, Mode mode, bool multiple, bool directoryMode,
                       const QString& title, const QString& acceptLabel, QWidget* parent):
    QDialog(parent),
    nameEdit_(NULL),
    folderModel_(NULL),
    searchFilter_(new SearchFilter()),
    folder_(NULL),
    currentPath_(NULL),
    mode_(mode),
    multiple_(multiple),
    directoryMode_(directoryMode),
    viewKind_(ViewColumns),
    groupBy_(GroupNone),
    resizing_(false),
    resizeEdge_(0),
    panelWidth_(980) {

    // Fullscreen translucent overlay; the visible "dialog" is panel_,
    // kept centered by relayoutPanel().
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    // deliberately NOT modal: a portal dialog must never freeze the rest of
    // Filer (desktop included) while it waits for the user
    setMouseTracking(true);
    winId(); // realize the native window now so windowHandle() is valid

    // panel_ paints nothing itself -- OpenDialog::paintEvent() fills it
    // (frosted sidebar column + opaque content, rounded corners), exactly
    // like MainWindow's own chrome, so this looks like the file manager
    // window itself, just without the traffic-light buttons.
    panel_ = new QWidget(this);
    panel_->setObjectName("openDialogPanel");
    panel_->setAttribute(Qt::WA_TranslucentBackground);
    panel_->setMouseTracking(true);
    panel_->installEventFilter(this);

    QHBoxLayout* panelBox = new QHBoxLayout(panel_);
    panelBox->setContentsMargins(0, 0, 0, 0);
    panelBox->setSpacing(0);

    // ---- left: the file manager's places sidebar, styled identically
    // (transparent background over the frosted column paintEvent() draws)
    placesView_ = new Fm::PlacesView(panel_);
    placesView_->setFixedWidth(kSidebarWidth);
    placesView_->setFrameStyle(QFrame::NoFrame);
    {
      Settings& settings = static_cast<Application*>(qApp)->settings();
      placesView_->setStyleSheet(Fm::buildSidePaneStyleSheet(
          settings.sidePaneItemSpacing(), settings.sidePaneItemHorizontalPadding(), settings.sidePaneFontWeight(),
          settings.sidePaneSelectorColor(), settings.sidePaneSelectorOpacity()));
      placesView_->setFont(Fm::sidePaneItemFont(settings.sidePaneFontWeight(), settings.sidePaneItemFontSize()));
      placesView_->setCategorySpacing(settings.sidePaneCategorySpacing(), settings.sidePaneCategorySpacingBottom());
      placesView_->setCategoryFontSize(settings.sidePaneCategoryFontSize());
      placesView_->setCategoryLeftPadding(settings.sidePaneCategoryLeftPadding());
      placesView_->setSelectorRightInset(settings.sidePaneSelectorRightInset());
      placesView_->setIconTextSpacing(settings.sidePaneIconTextSpacing());
    }
    placesView_->viewport()->setAutoFillBackground(false);
    connect(placesView_, SIGNAL(chdirRequested(int,FmPath*)),
            this, SLOT(onPlaceChdirRequested(int,FmPath*)));
    panelBox->addWidget(placesView_);

    // ---- right: header / views / buttons
    QWidget* right = new QWidget(panel_);
    right->setMouseTracking(true);
    QVBoxLayout* vbox = new QVBoxLayout(right);
    vbox->setContentsMargins(16, 12, 16, 12);
    vbox->setSpacing(8);
    panelBox->addWidget(right, 1);

    // header row 1: Up + folder name, title on the right
    QHBoxLayout* header = new QHBoxLayout();
    upButton_ = new QToolButton(panel_);
    upButton_->setIcon(QIcon::fromTheme("go-up"));
    upButton_->setAutoRaise(true);
    connect(upButton_, &QToolButton::clicked, this, &OpenDialog::onUpClicked);
    header->addWidget(upButton_);
    pathLabel_ = new QLabel(panel_);
    QFont boldFont = pathLabel_->font();
    boldFont.setBold(true);
    pathLabel_->setFont(boldFont);
    header->addWidget(pathLabel_);
    header->addStretch(1);
    QLabel* titleLabel = new QLabel(title.isEmpty() ? tr("Open") : title, panel_);
    header->addWidget(titleLabel);
    vbox->addLayout(header);

    // header row 2: view-options / group-by / places dropdowns (icon-only,
    // Finder Open-panel style) followed by the search box
    QHBoxLayout* toolRow = new QHBoxLayout();

    // Group 1: Icons / List / Columns, a real segmented control (matches
    // MainWindow's view-mode grouping convention) instead of hiding the
    // three modes inside a dropdown menu. One tooltip on the whole group,
    // not per button.
    QActionGroup* viewGroup = new QActionGroup(this);
    iconsAction_ = new QAction(themedIcon("view-list-icons", "view-grid", QStyle::SP_FileDialogContentsView), tr("as Icons"), viewGroup);
    listAction_ = new QAction(themedIcon("view-list-details", "view-list-text", QStyle::SP_FileDialogListView), tr("as List"), viewGroup);
    columnsAction_ = new QAction(themedIcon("view-file-columns", "view-split-left-right", QStyle::SP_FileDialogDetailedView), tr("as Columns"), viewGroup);
    Q_FOREACH(QAction* a, viewGroup->actions())
        a->setCheckable(true);
    columnsAction_->setChecked(true);
    connect(iconsAction_, &QAction::triggered, this, [this]() { setViewKind(ViewIcons); });
    connect(listAction_, &QAction::triggered, this, [this]() { setViewKind(ViewList); });
    connect(columnsAction_, &QAction::triggered, this, [this]() { setViewKind(ViewColumns); });
    QWidget* viewModeGroup = new QWidget(panel_);
    QHBoxLayout* viewModeLayout = new QHBoxLayout(viewModeGroup);
    viewModeLayout->setContentsMargins(2, 2, 2, 2);
    viewModeLayout->setSpacing(0);
    Q_FOREACH(QAction* a, viewGroup->actions()) {
        QToolButton* b = new QToolButton(viewModeGroup);
        b->setDefaultAction(a);
        b->setAutoRaise(true);
        b->setToolTip(QString());
        viewModeLayout->addWidget(b);
    }
    viewModeGroup->setStyleSheet(QStringLiteral(
        "QWidget { background-color: rgba(127, 127, 127, 20); border-radius: 10px; }"
        "QToolButton:checked { background-color: rgba(255, 255, 255, 45); border-radius: 8px; }"));
    viewModeGroup->setToolTip(tr("Show items as icons, in a list, or in columns"));
    toolRow->addWidget(viewModeGroup);

    // Single button between the two groups: Group By.
    groupByButton_ = new QToolButton(panel_);
    groupByButton_->setIcon(themedIcon("view-sort-ascending", "view-list-tree", QStyle::SP_FileDialogDetailedView));
    groupByButton_->setToolTip(tr("Group By"));
    groupByButton_->setPopupMode(QToolButton::InstantPopup);
    groupByButton_->setAutoRaise(true);
    QMenu* groupMenu = new QMenu(groupByButton_);
    QActionGroup* groupActions = new QActionGroup(groupMenu);
    struct { const char* label; GroupBy g; } groupItems[] = {
        {"None", GroupNone}, {"Name", GroupName}, {"Kind", GroupKind},
        {"Application", GroupApplication}, {"Date Last Opened", GroupDateLastOpened},
        {"Date Added", GroupDateAdded}, {"Date Modified", GroupDateModified},
        {"Date Created", GroupDateCreated}, {"Size", GroupSize}, {"Tags", GroupTags},
    };
    for(const auto& item : groupItems) {
        if(item.g == GroupName) // separator right after "None"
            groupMenu->addSeparator();
        QAction* a = new QAction(tr(item.label), groupActions);
        a->setCheckable(true);
        if(item.g == GroupNone)
            a->setChecked(true);
        GroupBy g = item.g;
        connect(a, &QAction::triggered, this, [this, g]() { setGroupBy(g); });
        groupMenu->addAction(a);
    }
    groupByButton_->setMenu(groupMenu);
    toolRow->addWidget(groupByButton_);

    // Group 2: Show Sidebar toggle + Quick Jump places menu.
    showSidebarButton_ = new QToolButton(panel_);
    showSidebarButton_->setIcon(themedIcon("sidebar-show", "view-sidebar", QStyle::SP_DirIcon));
    showSidebarButton_->setCheckable(true);
    showSidebarButton_->setChecked(true);
    showSidebarButton_->setAutoRaise(true);
    showSidebarAction_ = new QAction(tr("Show Sidebar"), this);
    showSidebarAction_->setCheckable(true);
    showSidebarAction_->setChecked(true);
    showSidebarButton_->setDefaultAction(showSidebarAction_);
    connect(showSidebarAction_, &QAction::toggled, placesView_, &QWidget::setVisible);
    connect(showSidebarAction_, &QAction::toggled, this, [this](bool) { relayoutPanel(); });

    placesButton_ = new QToolButton(panel_);
    placesButton_->setIcon(QIcon::fromTheme("folder", QIcon::fromTheme("user-bookmarks")));
    placesButton_->setToolTip(tr("Quick Jump"));
    placesButton_->setPopupMode(QToolButton::InstantPopup);
    placesButton_->setAutoRaise(true);
    placesMenu_ = new QMenu(placesButton_);
    connect(placesMenu_, &QMenu::aboutToShow, this, &OpenDialog::rebuildPlacesMenu);
    placesButton_->setMenu(placesMenu_);

    QWidget* sidebarPlacesGroup = new QWidget(panel_);
    QHBoxLayout* sidebarPlacesLayout = new QHBoxLayout(sidebarPlacesGroup);
    sidebarPlacesLayout->setContentsMargins(2, 2, 2, 2);
    sidebarPlacesLayout->setSpacing(0);
    sidebarPlacesLayout->addWidget(showSidebarButton_);
    sidebarPlacesLayout->addWidget(placesButton_);
    sidebarPlacesGroup->setStyleSheet(QStringLiteral(
        "QWidget { background-color: rgba(127, 127, 127, 20); border-radius: 10px; }"
        "QToolButton:checked { background-color: rgba(255, 255, 255, 45); border-radius: 8px; }"));
    toolRow->addWidget(sidebarPlacesGroup);

    searchEdit_ = new QLineEdit(panel_);
    searchEdit_->setPlaceholderText(tr("Search"));
    searchEdit_->setClearButtonEnabled(true);
    connect(searchEdit_, &QLineEdit::textChanged, this, &OpenDialog::onSearchTextChanged);
    toolRow->addWidget(searchEdit_, 1);

    vbox->addLayout(toolRow);

    // ---- model (shared by Icons/List; Column view manages its own)
    Application* app = static_cast<Application*>(qApp);
    proxyModel_ = new Fm::ProxyFolderModel();
    proxyModel_->setShowHidden(false);
    proxyModel_->setShowThumbnails(true);
    proxyModel_->sort(Fm::FolderModel::ColumnFileName, Qt::AscendingOrder);
    proxyModel_->setFolderFirst(app->settings().sortFolderFirst());
    proxyModel_->addFilter(searchFilter_);

    // ---- stacked views
    stack_ = new QStackedWidget(panel_);
    vbox->addWidget(stack_, 1);

    // icons + list share one Fm::FolderView (its internal mode switches)
    folderView_ = new Fm::FolderView(Fm::FolderView::IconMode, panel_);
    folderView_->setModel(proxyModel_);
    connect(folderView_, SIGNAL(clicked(int,FmFileInfo*)),
            this, SLOT(onViewClicked(int,FmFileInfo*)));
    connect(folderView_, &Fm::FolderView::selChanged, this, &OpenDialog::updateButtons);
    stack_->addWidget(folderView_);

    // Finder-style Miller columns, default view -- also shows a Quick-Look
    // style info panel (icon/name/kind/Created/Modified/Tags) when a plain
    // file is selected (Fm::ColumnView itself, shared with the file manager).
    columnView_ = new Fm::ColumnView(panel_);
    connect(columnView_, SIGNAL(dirSelected(void*)), this, SLOT(onColumnDirSelected(void*)));
    connect(columnView_, SIGNAL(fileSelected(void*)), this, SLOT(onColumnFileSelected(void*)));
    connect(columnView_, SIGNAL(fileActivated(void*)), this, SLOT(onColumnFileActivated(void*)));
    stack_->addWidget(columnView_);

    // SaveMode: file name entry
    if(mode_ == SaveMode) {
        QHBoxLayout* nameRow = new QHBoxLayout();
        nameRow->addWidget(new QLabel(tr("Save As:"), panel_));
        nameEdit_ = new QLineEdit(panel_);
        connect(nameEdit_, &QLineEdit::textChanged, this, &OpenDialog::updateButtons);
        nameRow->addWidget(nameEdit_, 1);
        vbox->addLayout(nameRow);
    }

    // bottom-right: Cancel and Open
    QHBoxLayout* buttons = new QHBoxLayout();
    buttons->addStretch(1);
    cancelButton_ = new QPushButton(tr("Cancel"), panel_);
    connect(cancelButton_, &QPushButton::clicked, this, &QDialog::reject);
    buttons->addWidget(cancelButton_);
    QString openText = !acceptLabel.isEmpty() ? acceptLabel
                       : (mode_ == SaveMode ? tr("Save") : tr("Open"));
    openButton_ = new QPushButton(openText, panel_);
    openButton_->setDefault(true);
    connect(openButton_, &QPushButton::clicked, this, &OpenDialog::onOpenClicked);
    buttons->addWidget(openButton_);
    vbox->addLayout(buttons);

    setViewKind(ViewColumns); // Column view is the default

    FmPath* initial = startPath ? startPath : fm_path_get_home();
    chdir(initial);
}

OpenDialog::~OpenDialog() {
    if(folder_)
        g_object_unref(folder_);
    if(folderModel_)
        folderModel_->unref();
    if(proxyModel_) {
        proxyModel_->removeFilter(searchFilter_);
        delete proxyModel_;
    }
    delete searchFilter_;
    if(currentPath_)
        fm_path_unref(currentPath_);
}

void OpenDialog::setCurrentName(const QString& name) {
    if(nameEdit_)
        nameEdit_->setText(name);
}

void OpenDialog::setViewKind(ViewKind kind) {
    viewKind_ = kind;
    iconsAction_->setChecked(kind == ViewIcons);
    listAction_->setChecked(kind == ViewList);
    columnsAction_->setChecked(kind == ViewColumns);
    if(kind == ViewColumns) {
        stack_->setCurrentWidget(columnView_);
        if(currentPath_)
            columnView_->setRootPath(currentPath_);
    }
    else {
        if(currentPath_)
            loadFolderModel(currentPath_); // Column view may have navigated since the last sync
        folderView_->setViewMode(kind == ViewIcons ? Fm::FolderView::IconMode
                                                   : Fm::FolderView::DetailedListMode);
        if(QAbstractItemView* v = folderView_->childView())
            v->setSelectionMode(multiple_ ? QAbstractItemView::ExtendedSelection
                                          : QAbstractItemView::SingleSelection);
        stack_->setCurrentWidget(folderView_);
    }
    updateButtons();
}

// "Group by" has no true grouping (section headers) backing it here --
// approximated the same way Finder's own list falls back for criteria with
// no stored metadata: sort by the closest matching real column. Applies to
// Icons/List only (Column view manages its own per-column models, sorted
// by name, and has no exposed hook for this).
void OpenDialog::setGroupBy(GroupBy g) {
    groupBy_ = g;
    Fm::FolderModel::ColumnId col = Fm::FolderModel::ColumnFileName;
    switch(g) {
    case GroupKind: col = Fm::FolderModel::ColumnFileType; break;
    case GroupDateModified: col = Fm::FolderModel::ColumnFileMTime; break;
    case GroupDateCreated: col = Fm::FolderModel::ColumnFileCTime; break;
    case GroupSize: col = Fm::FolderModel::ColumnFileSize; break;
    case GroupTags: col = Fm::FolderModel::ColumnFileTags; break;
    // Application / Date Last Opened / Date Added: not tracked anywhere in
    // this codebase -- fall back to Name, same as GroupNone/GroupName.
    default: col = Fm::FolderModel::ColumnFileName; break;
    }
    proxyModel_->sort(col, Qt::AscendingOrder);
}

// (Re)points folderModel_/proxyModel_ at path, skipping the work if they
// already are. Kept separate from chdir() so onColumnDirSelected() can keep
// the Icons/List model in sync (for when the user switches views later)
// without disturbing ColumnView's own already-correct column chain.
void OpenDialog::loadFolderModel(FmPath* path) {
    if(folder_ && fm_path_equal(fm_folder_get_path(folder_), path))
        return;
    if(folder_) {
        g_object_unref(folder_);
        folder_ = NULL;
    }
    Fm::CachedFolderModel* oldModel = folderModel_;
    folder_ = fm_folder_from_path(path);
    folderModel_ = Fm::CachedFolderModel::modelFromFolder(folder_);
    proxyModel_->setSourceModel(folderModel_);
    if(oldModel)
        oldModel->unref();
}

void OpenDialog::chdir(FmPath* path) {
    if(currentPath_)
        fm_path_unref(currentPath_);
    currentPath_ = fm_path_ref(path);

    loadFolderModel(path);

    char* dispName = fm_path_display_basename(path);
    pathLabel_->setText(QString::fromUtf8(dispName));
    g_free(dispName);
    upButton_->setEnabled(fm_path_get_parent(path) != NULL);
    placesView_->setCurrentPath(path);
    rememberRecentPlace(path);

    if(viewKind_ == ViewColumns)
        columnView_->setRootPath(path);

    updateButtons();
}

void OpenDialog::rememberRecentPlace(FmPath* path) {
    char* pathStr = fm_path_to_str(path);
    QString qpath = QString::fromUtf8(pathStr);
    g_free(pathStr);
    recentPlaces_.removeAll(qpath);
    recentPlaces_.prepend(qpath);
    while(recentPlaces_.size() > kMaxRecentPlaces)
        recentPlaces_.removeLast();
}

void OpenDialog::rebuildPlacesMenu() {
    placesMenu_->clear();

    auto addPlace = [this](const QString& label, const QString& localPath, const QString& iconName) {
        if(localPath.isEmpty() || !QFileInfo(localPath).isDir())
            return;
        QAction* a = placesMenu_->addAction(QIcon::fromTheme(iconName), label);
        connect(a, &QAction::triggered, this, [this, localPath]() {
            FmPath* p = fm_path_new_for_str(localPath.toUtf8().constData());
            chdir(p);
            fm_path_unref(p);
        });
    };

    addPlace(tr("Downloads"), QStandardPaths::writableLocation(QStandardPaths::DownloadLocation), "folder-download");
    addPlace(tr("Home"), QDir::homePath(), "user-home");
    addPlace(tr("Users"), QStringLiteral("/home"), "system-users");

    QStorageInfo root("/");
    QString diskLabel = root.displayName();
    addPlace(diskLabel.isEmpty() ? tr("Macintosh HD") : diskLabel, QStringLiteral("/"), "drive-harddisk");

    placesMenu_->addSeparator();
    QMenu* recentMenu = placesMenu_->addMenu(QIcon::fromTheme("document-open-recent"), tr("Recent Places"));
    if(recentPlaces_.isEmpty()) {
        recentMenu->addAction(tr("No Recent Places"))->setEnabled(false);
    }
    else {
        Q_FOREACH(const QString& p, recentPlaces_) {
            QAction* a = recentMenu->addAction(QFileInfo(p).fileName().isEmpty() ? p : QFileInfo(p).fileName());
            a->setToolTip(p);
            connect(a, &QAction::triggered, this, [this, p]() {
                FmPath* fp = fm_path_new_for_str(p.toUtf8().constData());
                chdir(fp);
                fm_path_unref(fp);
            });
        }
    }
}

void OpenDialog::onUpClicked() {
    FmPath* parent = fm_path_get_parent(currentPath_);
    if(parent)
        chdir(parent);
}

void OpenDialog::onPlaceChdirRequested(int type, FmPath* path) {
    Q_UNUSED(type);
    if(path)
        chdir(path);
}

void OpenDialog::onColumnDirSelected(void* path) {
    // ColumnView already opened the new column itself; just keep our own
    // path/Up-button/sidebar-highlight/recent-places state in sync (without
    // asking it to reset its columns again -- hence not calling chdir()).
    FmPath* fpath = static_cast<FmPath*>(path);
    if(currentPath_)
        fm_path_unref(currentPath_);
    currentPath_ = fm_path_ref(fpath);
    loadFolderModel(fpath);
    char* dispName = fm_path_display_basename(fpath);
    pathLabel_->setText(QString::fromUtf8(dispName));
    g_free(dispName);
    upButton_->setEnabled(fm_path_get_parent(fpath) != NULL);
    placesView_->setCurrentPath(fpath);
    rememberRecentPlace(fpath);
    updateButtons();
}

void OpenDialog::onColumnFileSelected(void*) {
    updateButtons();
}

void OpenDialog::onColumnFileActivated(void* fileInfo) {
    FmFileInfo* info = static_cast<FmFileInfo*>(fileInfo);
    if(info)
        activateInfo(info);
}

void OpenDialog::onViewClicked(int type, FmFileInfo* info) {
    if(type == Fm::FolderView::ActivatedClick && info)
        activateInfo(info);
}

void OpenDialog::onSearchTextChanged(const QString& text) {
    static_cast<SearchFilter*>(searchFilter_)->setText(text);
    proxyModel_->updateFilters();
}

void OpenDialog::activateInfo(FmFileInfo* info) {
    if(fm_file_info_is_dir(info))
        chdir(fm_file_info_get_path(info));
    else if(mode_ == OpenMode && !directoryMode_)
        onOpenClicked();
}

QStringList OpenDialog::selectionUris() const {
    QStringList uris;
    if(viewKind_ == ViewColumns) {
        FmFileInfo* info = columnView_->currentFileInfo();
        if(info && directoryMode_ == (bool)fm_file_info_is_dir(info)) {
            char* pathStr = fm_path_to_str(fm_file_info_get_path(info));
            uris << QUrl::fromLocalFile(QString::fromUtf8(pathStr)).toString();
            g_free(pathStr);
        }
    }
    else if(FmFileInfoList* files = folderView_->selectedFiles()) {
        for(GList* l = fm_file_info_list_peek_head_link(files); l; l = l->next) {
            FmFileInfo* info = FM_FILE_INFO(l->data);
            if(directoryMode_ != (bool)fm_file_info_is_dir(info))
                continue;
            char* pathStr = fm_path_to_str(fm_file_info_get_path(info));
            uris << QUrl::fromLocalFile(QString::fromUtf8(pathStr)).toString();
            g_free(pathStr);
        }
        fm_file_info_list_unref(files);
    }
    return uris;
}

void OpenDialog::onOpenClicked() {
    if(mode_ == SaveMode) {
        QString name = nameEdit_ ? nameEdit_->text().trimmed() : QString();
        if(name.isEmpty())
            return;
        char* dirPath = fm_path_to_str(currentPath_);
        uris_ = QStringList()
            << QUrl::fromLocalFile(QString::fromUtf8(dirPath) + "/" + name).toString();
        g_free(dirPath);
        accept();
        return;
    }

    // a lone selected directory descends instead (unless picking directories)
    if(!directoryMode_) {
        FmFileInfo* loneDir = NULL;
        if(viewKind_ == ViewColumns) {
            FmFileInfo* info = columnView_->currentFileInfo();
            if(info && fm_file_info_is_dir(info))
                loneDir = info;
        }
        else if(FmFileInfoList* files = folderView_->selectedFiles()) {
            if(fm_file_info_list_get_length(files) == 1) {
                FmFileInfo* info = fm_file_info_list_peek_head(files);
                if(info && fm_file_info_is_dir(info))
                    loneDir = info;
            }
            fm_file_info_list_unref(files);
        }
        if(loneDir) {
            chdir(fm_file_info_get_path(loneDir));
            return;
        }
    }

    uris_ = selectionUris();
    if(uris_.isEmpty() && directoryMode_) {
        // no selection: pick the folder being viewed
        char* pathStr = fm_path_to_str(currentPath_);
        uris_ << QUrl::fromLocalFile(QString::fromUtf8(pathStr)).toString();
        g_free(pathStr);
    }
    if(!uris_.isEmpty())
        accept();
}

void OpenDialog::updateButtons() {
    if(mode_ == SaveMode) {
        openButton_->setEnabled(nameEdit_ && !nameEdit_->text().trimmed().isEmpty());
        return;
    }
    if(directoryMode_) {
        openButton_->setEnabled(true);
        return;
    }
    bool hasSelection;
    if(viewKind_ == ViewColumns)
        hasSelection = columnView_->currentFileInfo() != NULL;
    else {
        FmFileInfoList* files = folderView_->selectedFiles();
        hasSelection = files && fm_file_info_list_get_length(files) > 0;
        if(files)
            fm_file_info_list_unref(files);
    }
    openButton_->setEnabled(hasSelection);
}

void OpenDialog::relayoutPanel() {
    int panelHeight = height() / 2;
    panelWidth_ = qBound(kPanelMinWidth, panelWidth_, width() - 80);
    panel_->setGeometry((width() - panelWidth_) / 2, (height() - panelHeight) / 2,
                        panelWidth_, panelHeight);
    updateSidebarBlurRegion();
    update();
}

// Right edge (in this widget's coordinates) of the full-height frosted
// sidebar strip -- the same "column, not just the widget's own rect" trick
// MainWindow uses for its sidebar, so the frost spans the whole panel height.
int OpenDialog::sidebarColumnRight() const {
    if(!placesView_->isVisible())
        return panel_->x();
    return placesView_->mapTo(const_cast<OpenDialog*>(this), QPoint(placesView_->width(), 0)).x();
}

// Restricts KWin's blur-behind to just the sidebar column, exactly like
// MainWindow::updateSidebarBlurRegion() -- a no-op with no visual effect if
// the user doesn't have a blur-behind effect (e.g. better-blur) active.
void OpenDialog::updateSidebarBlurRegion() {
    if(!windowHandle())
        return;
    int colRight = sidebarColumnRight();
    if(colRight <= panel_->x()) {
        KWindowEffects::enableBlurBehind(windowHandle(), false);
        return;
    }
    QPainterPath panelPath;
    panelPath.addRoundedRect(panel_->geometry(), kPanelCornerRadius, kPanelCornerRadius);
    QRegion region = QRegion(panelPath.toFillPolygon().toPolygon())
        & QRegion(panel_->x(), panel_->y(), colRight - panel_->x(), panel_->height());
    KWindowEffects::enableBlurBehind(windowHandle(), true, region);
}

void OpenDialog::showEvent(QShowEvent* event) {
    QDialog::showEvent(event);
    QScreen* screen = QGuiApplication::primaryScreen();
    if(screen)
        setGeometry(screen->geometry());
    showFullScreen();
    relayoutPanel();
}

void OpenDialog::keyPressEvent(QKeyEvent* event) {
    if(event->key() == Qt::Key_Escape) {
        reject();
        return;
    }
    QDialog::keyPressEvent(event);
}

int OpenDialog::edgeAt(const QPoint& panelPos) const {
    if(panelPos.y() < 0 || panelPos.y() > panel_->height())
        return 0;
    if(panelPos.x() >= -kEdgeGrip && panelPos.x() <= kEdgeGrip)
        return -1;
    if(panelPos.x() >= panel_->width() - kEdgeGrip && panelPos.x() <= panel_->width() + kEdgeGrip)
        return 1;
    return 0;
}

void OpenDialog::mousePressEvent(QMouseEvent* event) {
    QPoint panelPos = panel_->mapFromParent(event->pos());
    int edge = edgeAt(panelPos);
    if(edge != 0) {
        resizing_ = true;
        resizeEdge_ = edge;
        pressX_ = event->globalX();
        pressWidth_ = panelWidth_;
        return;
    }
    // click outside the panel cancels
    if(!panel_->geometry().contains(event->pos()))
        reject();
}

void OpenDialog::mouseMoveEvent(QMouseEvent* event) {
    if(resizing_) {
        int dx = event->globalX() - pressX_;
        // dragging an edge outward grows BOTH sides so the panel stays
        // centered (dragging the left edge left also extends the right edge)
        panelWidth_ = pressWidth_ + 2 * dx * resizeEdge_;
        relayoutPanel();
        return;
    }
    QPoint panelPos = panel_->mapFromParent(event->pos());
    setCursor(edgeAt(panelPos) != 0 ? Qt::SizeHorCursor : Qt::ArrowCursor);
}

void OpenDialog::mouseReleaseEvent(QMouseEvent* event) {
    Q_UNUSED(event);
    resizing_ = false;
}

// Custom chrome for panel_, painted here (not on panel_ itself, which stays
// fully transparent) since panel_'s children need to sit visually on top of
// it: rounded outline, frosted (translucent, KWin-blurred) sidebar column,
// opaque content area -- pixel-identical to MainWindow's own chrome (see the
// comment on MainWindow::paintEvent()), just without the traffic lights and
// with a dimming scrim over the rest of the screen, sheet-style.
void OpenDialog::paintEvent(QPaintEvent*) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    p.fillRect(rect(), QColor(0, 0, 0, 90));

    QPainterPath panelPath;
    panelPath.addRoundedRect(panel_->geometry(), kPanelCornerRadius, kPanelCornerRadius);

    int colRight = sidebarColumnRight();
    QPainterPath colPath;
    colPath.addRect(panel_->x(), panel_->y(), colRight - panel_->x(), panel_->height());

    p.fillPath(panelPath.intersected(colPath), QColor(28, 28, 28, 205));
    p.fillPath(panelPath.subtracted(colPath), palette().color(QPalette::Window));
    p.setPen(QPen(QColor(0, 0, 0, 80), 1));
    p.drawPath(panelPath);
}

bool OpenDialog::eventFilter(QObject* watched, QEvent* event) {
    if(watched == panel_ && (event->type() == QEvent::MouseMove
                             || event->type() == QEvent::MouseButtonPress
                             || event->type() == QEvent::MouseButtonRelease)) {
        // forward edge-resize handling for events landing on the panel itself
        QMouseEvent* me = static_cast<QMouseEvent*>(event);
        QPoint pos = panel_->mapToParent(me->pos());
        QMouseEvent mapped(me->type(), pos, me->globalPos(), me->button(), me->buttons(), me->modifiers());
        if(event->type() == QEvent::MouseButtonPress) {
            if(edgeAt(me->pos()) != 0) {
                mousePressEvent(&mapped);
                return true;
            }
        }
        else if(event->type() == QEvent::MouseMove) {
            if(resizing_) {
                mouseMoveEvent(&mapped);
                return true;
            }
            setCursor(edgeAt(me->pos()) != 0 ? Qt::SizeHorCursor : Qt::ArrowCursor);
        }
        else if(resizing_) {
            mouseReleaseEvent(&mapped);
            return true;
        }
    }
    return QDialog::eventFilter(watched, event);
}

} // namespace Filer
