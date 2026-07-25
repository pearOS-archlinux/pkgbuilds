/*
 * Filer's own file-open/save dialog, used by the xdg-desktop-portal
 * FileChooser backend (see filechooserportal.h) so that e.g. browser
 * "upload from computer" forms open Filer instead of a foreign dialog.
 *
 * Presented as a fullscreen translucent overlay with a centered panel:
 * the panel cannot be moved, is always exactly centered, has no window
 * buttons, and can only be resized horizontally -- dragging either side
 * edge grows both sides symmetrically so it stays centered (this design
 * also sidesteps Wayland's inability to position toplevels).
 *
 * Left side: the file manager's places sidebar (toggleable). Header: three
 * icon-only dropdown buttons -- view options (show/hide sidebar, view mode:
 * icons/list/columns), group-by, and a quick-jump places menu -- followed
 * by a search box. Column view is the default, matching Finder's Open panel.
 */

#ifndef FILER_OPENDIALOG_H
#define FILER_OPENDIALOG_H

#include <QDialog>
#include <QStringList>
#include <libfm/fm.h>

class QLabel;
class QPushButton;
class QToolButton;
class QLineEdit;
class QModelIndex;
class QStackedWidget;
class QMenu;

namespace Fm {
class CachedFolderModel;
class ProxyFolderModel;
class ProxyFolderModelFilter;
class FolderView;
class PlacesView;
class ColumnView;
}

namespace Filer {

class OpenDialog : public QDialog {
    Q_OBJECT
public:
    enum Mode {
        OpenMode,
        SaveMode
    };

    OpenDialog(FmPath* startPath, Mode mode, bool multiple, bool directoryMode,
               const QString& title, const QString& acceptLabel, QWidget* parent = 0);
    ~OpenDialog();

    // file:// URIs of the accepted selection
    QStringList selectedUris() const {
        return uris_;
    }

    void setCurrentName(const QString& name); // SaveMode: prefill file name

protected:
    void showEvent(QShowEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void paintEvent(QPaintEvent* event) override;
    bool eventFilter(QObject* watched, QEvent* event) override;

private Q_SLOTS:
    void onUpClicked();
    void onOpenClicked();
    void onViewClicked(int type, FmFileInfo* info);
    void onPlaceChdirRequested(int type, FmPath* path);
    void onColumnDirSelected(void* path);
    void onColumnFileSelected(void* fileInfo);
    void onColumnFileActivated(void* fileInfo);
    void onSearchTextChanged(const QString& text);
    void updateButtons();

private:
    enum ViewKind {
        ViewIcons,
        ViewList,
        ViewColumns
    };
    enum GroupBy {
        GroupNone,
        GroupName,
        GroupKind,
        GroupApplication,
        GroupDateLastOpened,
        GroupDateAdded,
        GroupDateModified,
        GroupDateCreated,
        GroupSize,
        GroupTags
    };

    void chdir(FmPath* path);
    void loadFolderModel(FmPath* path);
    void setViewKind(ViewKind kind);
    void setGroupBy(GroupBy g);
    void activateInfo(FmFileInfo* info);
    void rememberRecentPlace(FmPath* path);
    void rebuildPlacesMenu();
    QStringList selectionUris() const;
    void relayoutPanel();
    int edgeAt(const QPoint& panelPos) const; // -1 left, 1 right, 0 none
    int sidebarColumnRight() const; // right edge of the frosted sidebar strip, in this widget's coords
    void updateSidebarBlurRegion(); // keeps KWin's blur-behind restricted to that strip

    QWidget* panel_;
    Fm::PlacesView* placesView_;
    QStackedWidget* stack_;
    Fm::FolderView* folderView_;   // icons + list modes
    Fm::ColumnView* columnView_;
    QLabel* pathLabel_;
    QToolButton* upButton_;
    QToolButton* groupByButton_;
    QToolButton* showSidebarButton_;
    QToolButton* placesButton_;
    QMenu* placesMenu_;
    QLineEdit* searchEdit_;
    QAction* showSidebarAction_;
    QAction* iconsAction_;
    QAction* listAction_;
    QAction* columnsAction_;
    QPushButton* openButton_;
    QPushButton* cancelButton_;
    QLineEdit* nameEdit_; // SaveMode only

    Fm::CachedFolderModel* folderModel_;
    Fm::ProxyFolderModel* proxyModel_;
    Fm::ProxyFolderModelFilter* searchFilter_;
    FmFolder* folder_;
    FmPath* currentPath_;
    QStringList recentPlaces_; // local paths, most-recent-first

    Mode mode_;
    bool multiple_;
    bool directoryMode_;
    ViewKind viewKind_;
    GroupBy groupBy_;
    QStringList uris_;

    // lateral (symmetric) resize state
    bool resizing_;
    int resizeEdge_;
    int pressX_;
    int pressWidth_;
    int panelWidth_;
};

} // namespace Filer

#endif // FILER_OPENDIALOG_H
