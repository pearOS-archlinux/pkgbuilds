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


#ifndef PCMANFM_APPLICATION_H
#define PCMANFM_APPLICATION_H

#include <QApplication>
#include "settings.h"
#include "libfmqt.h"
#include "editbookmarksdialog.h"
#include <QVector>
#include <QPointer>
#include <QTranslator>
#include <gio/gio.h>


class QScreen;
class QProcess;

class QFileSystemWatcher;

namespace Filer {

class DesktopWindow;
class PreferencesDialog;
class DesktopPreferencesDialog;

class Application : public QApplication {
  Q_OBJECT
  Q_PROPERTY(bool desktopManagerEnabled READ desktopManagerEnabled)

public:
  Application(int& argc, char** argv);
  virtual ~Application();

  void init();
  int exec();

  Settings& settings() {
    return settings_;
  }

  Fm::LibFmQt& libFm() {
    return libFm_;
  }

  GVolumeMonitor* volumeMonitor;

  // public interface exported via dbus
  void launchFiles(QString cwd, QStringList paths, bool inNewWindow);
  void setWallpaper(QString path, QString modeString);
  void preferences(QString page);
  void desktopPrefrences();
  void editBookmarks();
  void desktopManager(bool enabled);
  void findFiles(QStringList paths = QStringList());
  void ShowFolders(QStringList uriList, QString startupId);
  void ShowItems(QStringList uriList, QString startupId);
  void ShowItemProperties(QStringList uriList, QString startupId);

  bool desktopManagerEnabled() {
    return enableDesktopManager_;
  }

  void updateFromSettings();
  void updateDesktopsFromSettings();

  void openFolderInTerminal(FmPath* path);
  void openFolders(FmFileInfoList* files);

  QString profileName() {
    return profileName_;
  }

  const QVector<DesktopWindow*>& desktopWindows() const {
    return desktopWindows_;
  }

  // Looks up the DesktopWindow for the given monitor (its index in
  // QGuiApplication::screens() matches desktopWindows_'s index, the same
  // invariant createDesktopWindow(screenNum) relies on). Returns nullptr if
  // screen is null or has no corresponding DesktopWindow (yet).
  DesktopWindow* desktopWindowForScreen(QScreen* screen) const;

  QString springLoadedFolderPreviouslyOpened;

Q_SIGNALS:
  void openFolderAndSelectItems(QString folder, QStringList items);

protected Q_SLOTS:
  void onAboutToQuit();
  void onSigtermNotified();

  void onLastWindowClosed();
  void onSaveStateRequest(QSessionManager & manager);
  void onScreenResized(int num);
  void onScreenCountChanged(int newCount);
  void initVolumeManager();

  void onVirtualGeometryChanged(const QRect& rect);
  void onScreenDestroyed(QObject* screenObj);
  void onScreenAdded(QScreen* newScreen);
  void reloadDesktopsAsNeeded();

  void onFindFileAccepted();

protected:
  virtual bool event(QEvent* event) override;
  virtual bool eventFilter(QObject* watched, QEvent* event);
  bool parseCommandLineArgs();
  DesktopWindow* createDesktopWindow(int screenNum);
  bool autoMountVolume(GVolume* volume, bool interactive = true);

  static void onVolumeAdded(GVolumeMonitor* monitor, GVolume* volume, Application* pThis);

private Q_SLOTS:
  void onUserDirsChanged();
  void onSystemThemeConfigChanged();
  // Lets editing settings.conf's TintWindow/SidebarTint/MainWindowTint
  // directly, while Filer is running, take effect immediately instead of
  // needing a restart -- see Settings::reloadTintSettings().
  void onTintSettingsFileChanged();

private:
  void initWatch();
  void refreshSystemIconTheme();
  void repaintAllWindows();
  void installSigtermHandler();
  void startDock();
  void stopDock();
  void startTopBar();
  void stopTopBar();
  // Wayland only: loads/runs the filer-window-tracker.js KWin script, which
  // reports this window's real screen geometry back to WindowGeometryTracker
  // via D-Bus (a Wayland client can't query its own global position itself).
  void loadWindowTrackerKwinScript();

  bool isPrimaryInstance;
  Fm::LibFmQt libFm_;
  Settings settings_;
  QString profileName_;
  bool daemonMode_;
  bool enableDesktopManager_;
  QVector<DesktopWindow*> desktopWindows_;
  // filer-dock is a separate Qt6/KF6 process (Filer itself is Qt5 -- Qt5
  // and Qt6 cannot be linked into the same executable) that hosts the
  // pearos-dock Plasma applet standalone, outside plasmashell. Spawned
  // alongside the desktop, killed when desktop management is disabled.
  QProcess* dockProcess_ = nullptr;
  // filer-topbar is its sibling for the top edge: it hosts the pearOS
  // top-bar plasmoids (Pear Menu, clock, ...) in a layer-shell window with
  // a real exclusive zone, replacing the QMenuBar Filer used to draw at
  // the top of the desktop window (which, living in a desktop-layer
  // surface, could never reserve screen space -- maximized windows just
  // covered it). While it runs, each DesktopWindow collapses that embedded
  // menubar and reserves the strip via setReservedTopArea().
  QProcess* topBarProcess_ = nullptr;
  // Crash/hang respawn bookkeeping -- see the QProcess::finished handlers
  // wired up in startDock()/startTopBar(). Both children also self-exit
  // (see their own main.cpp) if their own startup watchdog decides they've
  // hung rather than actually crashed, so "finished unexpectedly" covers
  // both cases equally from here. Timestamps (epoch ms) of recent
  // unintentional exits, pruned to the last few minutes -- too many in
  // that window means something is wrong on every single launch (bad
  // build, missing dependency, ...), so respawning forever would just
  // spin a silent crash loop instead of actually recovering anything.
  QList<qint64> dockRestartTimestamps_;
  QList<qint64> topBarRestartTimestamps_;
  bool dockIntentionalStop_ = false;
  bool topBarIntentionalStop_ = false;
  QPointer<PreferencesDialog> preferencesDialog_;
  QPointer<DesktopPreferencesDialog> desktopPreferencesDialog_;
  QPointer<Fm::EditBookmarksDialog> editBookmarksialog_;
  QTranslator translator;
  QTranslator qtTranslator;

  QFileSystemWatcher *userDirsWatcher_;
  QString userDirsFile_;
  // Watches kdeglobals and the GTK settings.ini for live icon-theme/accent-
  // color changes. Filer only reads those once at startup as a manual
  // fallback (see refreshSystemIconTheme()) when Qt's own platform theme
  // integration doesn't report an icon theme -- which is also exactly the
  // case where Qt itself never re-notifies us of a live theme change, so
  // without this watcher both symptoms need a full restart to pick up.
  QFileSystemWatcher *themeConfigWatcher_;
  QFileSystemWatcher *tintSettingsWatcher_;
  QString userDesktopFolder_;
  bool lxqtRunning_;

  int argc_;
  char** argv_;
};

}

#endif // PCMANFM_APPLICATION_H
