#ifndef FM_COLUMNVIEW_H
#define FM_COLUMNVIEW_H

#include <QScrollArea>
#include <QList>
#include <libfm/fm.h>

class QHBoxLayout;
class QVBoxLayout;
class QListView;
class QModelIndex;
class QLabel;

namespace Fm {

class CachedFolderModel;
class ProxyFolderModel;

// Finder-style Miller columns: a horizontal chain of single-directory list
// columns. Selecting a directory in any column truncates the chain after it
// and appends a new column with that directory's contents; each column has
// its own CachedFolderModel/ProxyFolderModel pair (one model can only show
// one directory). Selecting a plain file (not a directory) instead shows a
// Quick-Look-style info panel to the right of the last column: large icon
// preview, name, kind, and Created/Modified/Tags.
class ColumnView : public QScrollArea {
  Q_OBJECT
public:
  explicit ColumnView(QWidget* parent = 0);
  virtual ~ColumnView();

  // Clears every column and starts a fresh chain rooted at path.
  void setRootPath(FmPath* path);

  // The most recently selected item anywhere in the chain (file or
  // directory), or NULL if nothing is selected. Owned by ColumnView --
  // do not unref.
  FmFileInfo* currentFileInfo() const {
    return currentInfo_;
  }

Q_SIGNALS:
  // A directory was selected somewhere in the chain (its contents are now
  // the rightmost column). NOT emitted for the initial root. Passed as
  // void* (an FmPath*) -- see fileActivated below.
  void dirSelected(void* path);
  // A non-directory item was activated (double-click/Enter). Passed as
  // void* because FmFileInfo is an opaque C struct -- Qt's metatype
  // machinery (dragged in by signal signatures) requires complete types.
  void fileActivated(void* fileInfo);
  // The current selection changed to a plain file (info panel now showing
  // it) or was cleared (fileInfo == NULL, e.g. a directory was selected
  // instead, opening a new column).
  void fileSelected(void* fileInfo);

private:
  void addColumn(FmPath* path);
  void removeColumnsAfter(int index);
  int columnIndexOf(const QObject* view) const;
  void showInfoPanel(FmFileInfo* info, ProxyFolderModel* proxy, const QModelIndex& proxyIndex);
  void hideInfoPanel();
  void removeTrailingStretch();
  void setCurrentInfo(FmFileInfo* info);

  struct Column {
    QListView* view;
    CachedFolderModel* model;
    ProxyFolderModel* proxy;
  };

  QWidget* container_;
  QHBoxLayout* hbox_;
  QList<Column> columns_;

  FmFileInfo* currentInfo_;

  bool infoPanelShown_;
  QWidget* infoPanel_;
  QLabel* infoIcon_;
  QLabel* infoName_;
  QLabel* infoKind_;
  QLabel* infoCreated_;
  QLabel* infoModified_;
  QLabel* infoTags_;
};

}

#endif // FM_COLUMNVIEW_H
