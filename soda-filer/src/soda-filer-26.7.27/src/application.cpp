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


#include "application.h"
#include "mainwindow.h"
#include "desktopwindow.h"
#include "filechooserportal.h"
#include <QDBusConnection>

#include <QDBusInterface>
#include <QDBusMessage>
#include <QDBusReply>
#include <QDir>
#include <QDesktopWidget>
#include <QVector>
#include <QLocale>
#include <QLibraryInfo>
#include <QPixmapCache>
#include <QFile>
#include <QMessageBox>
#include <QCommandLineParser>
#include <QSocketNotifier>
#include <QTimer>
#include <QSettings>
#include <QDateTime>

#include <gio/gio.h>
#include <sys/socket.h>

#include "applicationadaptor.h"
#include "preferencesdialog.h"
#include "desktoppreferencesdialog.h"
#include "mountoperation.h"
#include "autorundialog.h"
#include "launcher.h"
#include "filesearchdialog.h"

#include <QScreen>
#include <QWindow>
#include <QProcess>
#include <QProcessEnvironment>
#include <QStandardPaths>
#include <QStyleFactory>
#include <QMenu>
#include <QPainterPath>
#include <KWindowEffects>

#include <X11/Xlib.h>

#include "xdgdir.h"
#include <QFileSystemWatcher>

#include "dbusinterface.h"
#include "windowgeometrytracker.h"

using namespace Filer;
static const char* serviceName = "org.freedesktop.FileManager1";
static const char* ifaceName = "org.freedesktop.FileManager1";

// Context-menu look (macOS 27 Figma reference: rounded popup, muted
// separators, a rounded pill highlight on the selected/submenu item) --
// applied app-wide via qApp's own stylesheet, rather than at each menu's
// own construction site (filemenu.cpp, foldermenu.cpp, folderview.cpp,
// placesview.cpp, ... all build their own QMenus). Qt auto-enables
// Qt::WA_TranslucentBackground and a rounded window mask for any QMenu
// whose effective stylesheet sets border-radius, so this alone is enough
// for the shape/rounding -- the KWin blur-behind (real frosted glass, not
// just a flat alpha blend) is separately wired up in eventFilter() below,
// since that's a window-manager effect QSS can't reach.
//
// Two variants, not one: the Figma reference itself is light, but this
// app is normally run under a dark system theme (Kvantum's pearOS-dark)
// -- a light menu popping up over a dark app read as visibly wrong/
// mismatched, so applyMenuStyleSheet() below picks whichever variant
// matches the current QPalette's actual lightness, live, instead of
// hardcoding one. Both are kept (not just the dark one) since a light
// pearOS theme is a real, supported option too.
static constexpr int kMenuCornerRadius = 10;
static const QString kMenuStyleSheetLight = QStringLiteral(
    "QMenu {"
    "  background-color: rgba(246, 246, 246, 200);"
    "  border: 1px solid rgba(0, 0, 0, 40);"
    "  border-radius: %1px;"
    "  padding: 6px;"
    "}"
    "QMenu::item {"
    "  padding: 6px 24px 6px 12px;"
    "  border-radius: 6px;"
    "  color: #1d1d1f;"
    "  background: transparent;"
    "  text-align: left;"
    "}"
    "QMenu::item:selected {"
    "  background-color: rgba(0, 0, 0, 25);"
    "}"
    "QMenu::item:disabled {"
    "  color: rgba(29, 29, 31, 110);"
    "}"
    "QMenu::separator {"
    "  height: 1px;"
    "  background: rgba(0, 0, 0, 35);"
    "  margin: 6px 10px;"
    "}"
    "QMenu::right-arrow {"
    "  width: 10px;"
    "  height: 10px;"
    "}"
).arg(kMenuCornerRadius);
static const QString kMenuStyleSheetDark = QStringLiteral(
    "QMenu {"
    "  background-color: rgba(48, 48, 48, 210);"
    "  border: 1px solid rgba(255, 255, 255, 30);"
    "  border-radius: %1px;"
    "  padding: 6px;"
    "}"
    "QMenu::item {"
    "  padding: 6px 24px 6px 12px;"
    "  border-radius: 6px;"
    "  color: #f0f0f0;"
    "  background: transparent;"
    "  text-align: left;"
    "}"
    "QMenu::item:selected {"
    "  background-color: rgba(255, 255, 255, 30);"
    "}"
    "QMenu::item:disabled {"
    "  color: rgba(240, 240, 240, 110);"
    "}"
    "QMenu::separator {"
    "  height: 1px;"
    "  background: rgba(255, 255, 255, 35);"
    "  margin: 6px 10px;"
    "}"
    "QMenu::right-arrow {"
    "  width: 10px;"
    "  height: 10px;"
    "}"
).arg(kMenuCornerRadius);

// QPalette::Window's own lightness is what Kvantum (or whatever style is
// active) actually painted the app in, so this tracks the real active
// theme rather than guessing from a setting -- true for both a system
// dark/light switch and a light pearOS variant, not just this one theme.
static bool isDarkPalette() {
    return qApp->palette().color(QPalette::Window).lightness() < 128;
}

static void applyMenuStyleSheet() {
    qApp->setStyleSheet(isDarkPalette() ? kMenuStyleSheetDark : kMenuStyleSheetLight);
}

Application::Application(int& argc, char** argv):
    QApplication(argc, argv),
    libFm_(),
    settings_(),
    profileName_("default"),
    daemonMode_(false),
    desktopWindows_(),
    enableDesktopManager_(false),
    preferencesDialog_(),
    volumeMonitor(NULL),
    userDirsWatcher_(NULL),
    themeConfigWatcher_(NULL),
    tintSettingsWatcher_(NULL),
    lxqtRunning_(false),
    springLoadedFolderPreviouslyOpened(""),
    editBookmarksialog_() {

    argc_ = argc;
    argv_ = argv;

    // Qt5's default drag-start threshold (~10px) is tuned for regular list/
    // tree views; on the desktop it means a deliberate, small nudge of an
    // icon (e.g. by a millimeter) never registers as a drag at all. This is
    // application-wide because Qt5 offers no per-widget override (QAbstractItemView
    // reads QApplication::startDragDistance() directly, not a QStyle metric).
    QApplication::setStartDragDistance(3);

    // Context-menu look + KWin blur-behind (see kMenuStyleSheet's doc
    // comment and eventFilter() below) -- installed globally here rather
    // than per-menu, since menus are built all over the codebase.
    applyMenuStyleSheet();
    installEventFilter(this);

    // Taskbar/dock identity: on Wayland the compositor resolves a window's
    // taskbar icon and label from its xdg app_id -> matching .desktop file,
    // and Qt derives the app_id from desktopFileName(). Without this it
    // falls back to the binary name ("Filer"), which matches no desktop
    // file -> generic Wayland cog icon. pinder.desktop (Name=Pinder,
    // Icon=system-file-manager, so the icon theme decides the artwork)
    // is installed to ~/.local/share/applications.
    QGuiApplication::setDesktopFileName(QStringLiteral("pinder"));

    // QApplication::setStyle and QApplication::setPalette removed to let Filer inherit system themes (Kvantum, Breeze, etc.)

    QDBusConnection dbus = QDBusConnection::sessionBus();
    if(dbus.registerService(serviceName)) {
        // we successfully registered the service
        isPrimaryInstance = true;

        settings_.load(profileName_);
/*
        // As soon as possible, show a fullscreen window with the background while we are still doing stuff
        QWidget stillWorkingWindow;
        auto wallpaperMode_ = settings().wallpaperMode();
        switch (wallpaperMode_) {
        case Filer::DesktopWindow::WallpaperNone:
            stillWorkingWindow.setStyleSheet("background-color: " + settings().desktopBgColor().name());
            break;
        case Filer::DesktopWindow::WallpaperTransparent:
            stillWorkingWindow.setStyleSheet("background-color: transparent");
            break;
        case Filer::DesktopWindow::WallpaperStretch:
            if (! settings().wallpaper().isEmpty())
                stillWorkingWindow.setStyleSheet("border-image: url(" +  settings().wallpaper() + ") stretch stretch");
            break;
        case Filer::DesktopWindow::WallpaperFit: // FIXME - how do we fit with correct aspect in qt stylesheets?
            if (! settings().wallpaper().isEmpty())
                stillWorkingWindow.setStyleSheet("border-image: url(" +  settings().wallpaper() + ") stretch");
            break;
        case Filer::DesktopWindow::WallpaperCenter:
            if (! settings().wallpaper().isEmpty())
                stillWorkingWindow.setStyleSheet("background-image: url(" +  settings().wallpaper()
                                                 + "); background-repeat: no-repeat; background-position: center; background-color: "
                                                 + settings().desktopBgColor().name());
            break;
        case Filer::DesktopWindow::WallpaperTile:
            if (! settings().wallpaper().isEmpty())
                stillWorkingWindow.setStyleSheet("background-image: url(" + settings().wallpaper() + ")");
            break;
        }

        stillWorkingWindow.setAutoFillBackground(true);
        stillWorkingWindow.showFullScreen();
        stillWorkingWindow.setCursor(QCursor(Qt::WaitCursor));
*/
        desktop()->installEventFilter(this);

        new ApplicationAdaptor(this);
        dbus.registerObject(QStringLiteral("/org/freedesktop/FileManager1"), this);

        // xdg-desktop-portal FileChooser backend: serves Open/Save dialogs
        // to portal-using apps (browser upload forms etc.) with Filer's own
        // dialog. Needs the "filer" portal preferred in portals.conf.
        FileChooserPortal::registerPortal(this);

        connect(this, &Application::aboutToQuit, this, &Application::onAboutToQuit);
        // aboutToQuit() is not signalled on SIGTERM, install signal handler
        installSigtermHandler();

        // decrease the cache size to reduce memory usage
        QPixmapCache::setCacheLimit(2048);

        refreshSystemIconTheme();

        // kdeglobals/gtk settings.ini are only read once above -- watch them
        // so an icon-theme or accent-color change made while Filer is
        // running (e.g. via System Settings) doesn't need a full restart.
        // Theme-applying tools save these atomically (write a temp file,
        // rename over the original), which replaces the inode -- on this
        // kernel/Qt combination that silently drops an inotify watch on the
        // *file* without ever emitting fileChanged(), so a second theme
        // change after the first one would go unnoticed forever. Watching
        // the parent directories' directoryChanged() instead survives any
        // number of atomic replaces, since the directory itself never gets
        // replaced.
        themeConfigWatcher_ = new QFileSystemWatcher(this);
        const QString kdeGlobalsPath = QDir::homePath() + "/.config/kdeglobals";
        const QString gtkSettingsPath = QDir::homePath() + "/.config/gtk-3.0/settings.ini";
        if(QFile::exists(kdeGlobalsPath)) {
            themeConfigWatcher_->addPath(kdeGlobalsPath);
        }
        if(QFile::exists(gtkSettingsPath)) {
            themeConfigWatcher_->addPath(gtkSettingsPath);
        }
        themeConfigWatcher_->addPath(QDir::homePath() + "/.config");
        QDir().mkpath(QDir::homePath() + "/.config/gtk-3.0");
        themeConfigWatcher_->addPath(QDir::homePath() + "/.config/gtk-3.0");
        connect(themeConfigWatcher_, &QFileSystemWatcher::fileChanged, this, &Application::onSystemThemeConfigChanged);
        connect(themeConfigWatcher_, &QFileSystemWatcher::directoryChanged, this, &Application::onSystemThemeConfigChanged);

        // Live-reload for the sidebar/window wallpaper vibrancy settings
        // (TintWindow/SidebarTint/MainWindowTint/Transparency/
        // TransparencyPower) -- same atomic-replace concern as kdeglobals
        // above (editors/tools often write via temp-file-then-rename), so
        // this watches the settings.conf file's own parent directory too.
        tintSettingsWatcher_ = new QFileSystemWatcher(this);
        const QString settingsConfPath = settings_.profileDir(settings_.profileName(), true) + "/settings.conf";
        if(QFile::exists(settingsConfPath)) {
            tintSettingsWatcher_->addPath(settingsConfPath);
        }
        tintSettingsWatcher_->addPath(settings_.profileDir(settings_.profileName(), true));
        connect(tintSettingsWatcher_, &QFileSystemWatcher::fileChanged, this, &Application::onTintSettingsFileChanged);
        connect(tintSettingsWatcher_, &QFileSystemWatcher::directoryChanged, this, &Application::onTintSettingsFileChanged);

        // probono: On systems that are supposed to have a global menu bar,
        // pre-load the kded appmenu module so it is ready by the time the
        // filer-topbar's org.kde.plasma.appmenu plasmoid connects.
        //
        // NOTE: we must NOT block here waiting for the D-Bus name
        // com.canonical.AppMenu.Registrar -- that name only appears once
        // filer-topbar's appmenu plasmoid actually connects to the kded
        // module, and filer-topbar is started later in desktopManager().
        // Blocking here would deadlock startup.
        QString globalMenuEnv  = QString::fromLocal8Bit(qgetenv("UBUNTU_MENUPROXY"));
        if ( ! globalMenuEnv.isEmpty() ) {
            qDebug("UBUNTU_MENUPROXY is set, pre-loading kded appmenu module...");
            QDBusMessage msg = QDBusMessage::createMethodCall(
                QStringLiteral("org.kde.kded6"),
                QStringLiteral("/kded"),
                QStringLiteral("org.kde.kded6"),
                QStringLiteral("loadModule"));
            msg << QStringLiteral("appmenu");
            QDBusMessage reply = QDBusConnection::sessionBus().call(msg, QDBus::Block, 3000);
            if (reply.type() == QDBusMessage::ErrorMessage) {
                // kded6 not available – try kded5 as fallback
                QDBusMessage msg5 = QDBusMessage::createMethodCall(
                    QStringLiteral("org.kde.kded5"),
                    QStringLiteral("/kded"),
                    QStringLiteral("org.kde.kded5"),
                    QStringLiteral("loadModule"));
                msg5 << QStringLiteral("appmenu");
                QDBusConnection::sessionBus().call(msg5, QDBus::Block, 3000);
            }
        }

        // Check if LXQt Session is running. LXQt has it's own Desktop Folder
        // editor. We just hide our editor when LXQt is running.
        QDBusInterface* lxqtSessionIface = new QDBusInterface(
                    QStringLiteral("org.lxqt.session"),
                    QStringLiteral("/LXQtSession"));
        if (lxqtSessionIface) {
            if (lxqtSessionIface->isValid()) {
                lxqtRunning_ = true;
                userDesktopFolder_ = XdgDir::readDesktopDir();
                initWatch();
            }
            delete lxqtSessionIface;
            lxqtSessionIface = 0;
        }

        DBusInterface();

        // Sidebar wallpaper tint (see MainWindow::updateSidebarWallpaperTint()):
        // registers the /WindowTracker D-Bus object under our existing
        // service, then -- on Wayland only, where geometry() can't be
        // trusted -- loads the KWin script that feeds it real window
        // geometry from the compositor side. On X11, moveEvent()/resizeEvent()
        // already drive the tint directly, so the script isn't needed there.
        new WindowGeometryTracker(this);
        if(QGuiApplication::platformName().contains(QLatin1String("wayland"))) {
            loadWindowTrackerKwinScript();
        }

//        stillWorkingWindow.close();

    }
    else {
        // an service of the same name is already registered.
        // we're not the first instance
        isPrimaryInstance = false;
    }

    // probono: Do some sanity checks to see whether key components of helloDesktop are available;
    // this is mainly for users running Filer outside of helloSystem. These used to be blocking
    // QMessageBox::exec() calls -- disruptive on every single startup (and during development,
    // every direct-binary-launch-for-testing) for conditions that don't actually stop the app
    // from working. Log them instead so they're still discoverable (e.g. via journalctl) without
    // interrupting the user.
    if(! QProcessEnvironment::systemEnvironment().contains("LAUNCHED_BUNDLE"))
        qWarning("%s was not launched by the launch command from an application bundle.",
                  qUtf8Printable(qApp->applicationDisplayName()));

    if(QStandardPaths::findExecutable("launch") == "")
        qWarning("The 'launch' command is missing.");

    if(QStandardPaths::findExecutable("open") == "")
        qWarning("The 'open' command is missing.");

    // This command ejects/unmounts a mountpoint and removes the mountpoint directory
    // TODO: Remove the need for this simple external helper by internalizing its code;
    // It shouldn't be all to hard: unmount the mountpoint, check whether the directory is empty, and delete it
    if(QStandardPaths::findExecutable("eject-and-clean") == "")
        qWarning("The 'eject-and-clean' command is missing.");

    // Remove the environment variables so that they can't leak into
    // processes started by Filer
    QProcessEnvironment::systemEnvironment().remove("LAUNCHED_BUNDLE");
    QProcessEnvironment::systemEnvironment().remove("LAUNCHED_COMMAND");

}

Application::~Application() {
    stopDock();
    stopTopBar();
    desktop()->removeEventFilter(this);

    if(volumeMonitor) {
        g_signal_handlers_disconnect_by_func(volumeMonitor, gpointer(onVolumeAdded), this);
        g_object_unref(volumeMonitor);
    }

    // if(enableDesktopManager_)
    //   removeNativeEventFilter(this);
}

void Application::initWatch()
{
    // probono: Obscure functionality from XDG; remove? https://wiki.archlinux.org/title/XDG_user_directories
    QFile file_ (QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + QStringLiteral("/user-dirs.dirs"));
    if(! file_.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << Q_FUNC_INFO << "Could not read: " << userDirsFile_;
        userDirsFile_ = QString();
    } else {
        userDirsFile_ = file_.fileName();
    }

    userDirsWatcher_ = new QFileSystemWatcher(this);
    userDirsWatcher_->addPath(userDirsFile_);
    connect(userDirsWatcher_, &QFileSystemWatcher::fileChanged, this, &Application::onUserDirsChanged);
}

// Resolves the fallback icon theme name from GTK/kdeglobals, exactly as
// done once at startup in the constructor. Split out so it can be re-run
// live when those config files change (see onSystemThemeConfigChanged()).
void Application::refreshSystemIconTheme() {
    if(!settings_.useFallbackIconTheme()) {
        return;
    }
    QString systemIconTheme;
    QSettings gtkSettings(QDir::homePath() + "/.config/gtk-3.0/settings.ini", QSettings::IniFormat);
    systemIconTheme = gtkSettings.value("Settings/gtk-icon-theme-name").toString();
    if(systemIconTheme.isEmpty()) {
        QSettings kdeGlobals(QDir::homePath() + "/.config/kdeglobals", QSettings::IniFormat);
        systemIconTheme = kdeGlobals.value("Icons/Theme").toString();
    }
    QIcon::setThemeName(systemIconTheme.isEmpty() ? settings_.fallbackIconThemeName()
                                                  : systemIconTheme);
    Fm::IconTheme::checkChanged();
}

// Forces every open window to repaint so custom-painted views (folder view,
// desktop icons, sidepane) pick up a live accent-color/palette change --
// QWidget doesn't automatically repaint itself just because QPalette::Highlight
// changed underneath it.
void Application::repaintAllWindows() {
    const auto widgets = QApplication::allWidgets();
    for(QWidget* w : widgets) {
        if(w->isVisible()) {
            w->update();
        }
    }
}

void Application::onSystemThemeConfigChanged() {
    // Editors/desktop settings often save config files atomically (write a
    // temp file, then rename over the original), which drops the old inode
    // from the watch -- re-add the path so future changes keep being seen.
    const QString kdeGlobalsPath = QDir::homePath() + "/.config/kdeglobals";
    const QString gtkSettingsPath = QDir::homePath() + "/.config/gtk-3.0/settings.ini";
    if(QFile::exists(kdeGlobalsPath) && !themeConfigWatcher_->files().contains(kdeGlobalsPath)) {
        themeConfigWatcher_->addPath(kdeGlobalsPath);
    }
    if(QFile::exists(gtkSettingsPath) && !themeConfigWatcher_->files().contains(gtkSettingsPath)) {
        themeConfigWatcher_->addPath(gtkSettingsPath);
    }

    refreshSystemIconTheme();
    repaintAllWindows();
    // On setups where the platform theme plugin doesn't also fire a Qt
    // ApplicationPaletteChange/ThemeChange event for this same kdeglobals
    // edit, Application::event() below never runs -- so the sidebar's cached
    // QSS (palette-derived text color, see buildSidePaneStyleSheet()) would
    // otherwise only pick up the new theme once the window is recreated.
    updateFromSettings();
}

void Application::onTintSettingsFileChanged() {
    const QString settingsConfPath = settings_.profileDir(settings_.profileName(), true) + "/settings.conf";
    if(QFile::exists(settingsConfPath) && !tintSettingsWatcher_->files().contains(settingsConfPath)) {
        tintSettingsWatcher_->addPath(settingsConfPath);
    }
    settings_.reloadTintSettings();
    repaintAllWindows();

    // Same live-reload treatment for the sidebar/folder-view layout keys
    // (icon size, spacing, padding, fonts, selector color/opacity/reach,
    // scrollbar mode, grid spacing): re-read them from disk, then push them
    // to every open window via the normal updateFromSettings() path
    // (repaintAllWindows() above only repaints, it doesn't resize the
    // sidebar's icon size or rebuild its stylesheet/grid). This used to
    // diff every single field before deciding to call updateFromSettings()
    // -- with a dozen-plus fields now, that comparison had become more
    // code than the update itself, so just call it unconditionally; it's
    // cheap (re-applies stylesheets/fonts/grid sizes, doesn't relayout
    // from scratch).
    settings_.reloadSidePaneLayoutSettings();
    updateFromSettings();
}

bool Application::parseCommandLineArgs() {
    bool keepRunning = false;
    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption profileOption(QStringList() << "p" << "profile", tr("Name of configuration profile"), tr("PROFILE"));
    parser.addOption(profileOption);

    QCommandLineOption daemonOption(QStringList() << "d" << "daemon-mode", tr("Run Filer as a daemon"));
    parser.addOption(daemonOption);

    QCommandLineOption quitOption(QStringList() << "q" << "quit", tr("Quit Filer"));
    parser.addOption(quitOption);

    QCommandLineOption desktopOption("desktop", tr("Launch desktop manager (deprecated)"));
    parser.addOption(desktopOption);

    QCommandLineOption desktopOffOption("desktop-off", tr("Turn off desktop manager if it's running"));
    parser.addOption(desktopOffOption);

    QCommandLineOption desktopPrefOption("desktop-pref", tr("Open desktop preference dialog on the page with the specified name"), tr("NAME"));
    parser.addOption(desktopPrefOption);

    QCommandLineOption newWindowOption(QStringList() << "n" << "new-window", tr("Open new window"));
    parser.addOption(newWindowOption);

    QCommandLineOption findFilesOption(QStringList() << "f" << "find-files", tr("Open Find Files utility"));
    parser.addOption(findFilesOption);

    QCommandLineOption setWallpaperOption(QStringList() << "w" << "set-wallpaper", tr("Set desktop wallpaper from image FILE"), tr("FILE"));
    parser.addOption(setWallpaperOption);

    // don't translate list of modes in description, please
    QCommandLineOption wallpaperModeOption("wallpaper-mode", tr("Set mode of desktop wallpaper. MODE=(none|transparent|stretch|fit|center|tile)"), tr("MODE"));
    parser.addOption(wallpaperModeOption);

    QCommandLineOption showPrefOption("show-pref", tr("Open Preferences dialog on the page with the specified name"), tr("NAME"));
    parser.addOption(showPrefOption);

    parser.addPositionalArgument("files", tr("Files or directories to open"), tr("[FILE1, FILE2,...]"));

    parser.process(arguments());

    if(isPrimaryInstance) {
        qDebug("isPrimaryInstance");

        if(parser.isSet(daemonOption))
            daemonMode_ = true;
        if(parser.isSet(profileOption))
            profileName_ = parser.value(profileOption);

        // load settings
        settings_.load(profileName_);

        // desktop icon management

        // probono: We always want to show the desktop if we are the primary instance
        QStringList paths = parser.positionalArguments();
        bool implicitDesktopOption = false;

        // if(parser.isSet(desktopOption)) {
        if(parser.isSet(desktopOffOption) == false and paths.isEmpty()) {
            implicitDesktopOption = true;
            desktopManager(true);
            keepRunning = true;
        }
        else if(parser.isSet(desktopOffOption))
            desktopManager(false);

        if(parser.isSet(desktopPrefOption)) { // desktop preference dialog
            desktopPrefrences();
            keepRunning = true;
        }
        else if(parser.isSet(findFilesOption)) { // file searching utility
            findFiles(parser.positionalArguments());
            keepRunning = true;
        }
        else if(parser.isSet(showPrefOption)) { // preferences dialog
            preferences(parser.value(showPrefOption));
            keepRunning = true;
        }
        else if(parser.isSet(setWallpaperOption) || parser.isSet(wallpaperModeOption)) // set wall paper
            setWallpaper(parser.value(setWallpaperOption), parser.value(wallpaperModeOption));
        else {
            if(!parser.isSet(desktopOption) && !parser.isSet(desktopOffOption) && !implicitDesktopOption) {
                if(paths.isEmpty()) {
                    // if no path is specified and we're using daemon mode,
                    // don't open current working directory
                    if(!daemonMode_)
                        paths.push_back(QDir::currentPath());
                }
                if(!paths.isEmpty())
                    launchFiles(QDir::currentPath(), paths, parser.isSet(newWindowOption));
                keepRunning = true;
            }
        }
    }
    else {
        QDBusConnection dbus = QDBusConnection::sessionBus();
        QDBusInterface iface(serviceName, QStringLiteral("/org/freedesktop/FileManager1"), ifaceName, dbus, this);
        if(parser.isSet(quitOption)) {
            iface.call("quit");
            return false;
        }

        if(parser.isSet(desktopOption))
            iface.call("desktopManager", true);
        else if(parser.isSet(desktopOffOption))
            iface.call("desktopManager", false);

        if(parser.isSet(desktopPrefOption)) { // desktop preference dialog
            iface.call("desktopPrefrences", parser.value(desktopPrefOption));
        }
        else if(parser.isSet(findFilesOption)) { // file searching utility
            iface.call("findFiles", parser.positionalArguments());
        }
        else if(parser.isSet(showPrefOption)) { // preferences dialog
            iface.call("preferences", parser.value(showPrefOption));
        }
        else if(parser.isSet(setWallpaperOption) || parser.isSet(wallpaperModeOption)) { // set wall paper
            iface.call("setWallpaper", parser.value(setWallpaperOption), parser.value(wallpaperModeOption));
        }
        else {
            if(!parser.isSet(desktopOption) && !parser.isSet(desktopOffOption)) {
                QStringList paths = parser.positionalArguments();
                if(paths.isEmpty()) {
                    paths.push_back(QDir::currentPath());
                }
                iface.call("launchFiles", QDir::currentPath(), paths, parser.isSet(newWindowOption));
            }
        }
    }
    return keepRunning;
}

void Application::init() {

}

int Application::exec() {

    if(!parseCommandLineArgs())
        return 0;

    if(daemonMode_) { // keep running even when there is no window opened.
        setQuitOnLastWindowClosed(false);
    }

  if (isPrimaryInstance) {
    // Every 10 seconds call a lambda function that deletes empty mount points
    qDebug() << "Periodically checking for empty mount points";
    // Check if /media exists and exit with an error if it doesn't
    if (!QDir("/media").exists()) {
      qCritical() << "The /media directory does not exist. Exiting.";
      qApp->exit(1);
    }
    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, []() {
      // qDebug() << "Checking for empty mount points";
      // Get all directories in /media that are empty
      QDir mediaDir("/media");
      QStringList emptyDirs;
      for (const QFileInfo &fileInfo :
           mediaDir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot)) {
        QDir dir(fileInfo.absoluteFilePath());
        if (dir.entryInfoList(QDir::AllEntries | QDir::NoDotAndDotDot)
                .isEmpty()) {
          emptyDirs << fileInfo.absoluteFilePath();
        }
      }
      // Delete the empty mount points
      for (const QString &dir : emptyDirs) {
        // If the directory is still empty after 3 seconds, delete it
        QTimer::singleShot(3000, [dir]() {
          QDir dirObj(dir);
          if (dirObj.entryInfoList(QDir::AllEntries | QDir::NoDotAndDotDot)
                  .isEmpty()) {
            qDebug() << "Deleting empty mount point" << dir;
            QDir().rmdir(dir);
          }
        });
      }
    });
    timer->start(5000);
  }
    
    volumeMonitor = g_volume_monitor_get();
    // delay the volume manager a little because in newer versions of glib/gio there's a problem.
    // when the first volume monitor object is created, it discovers volumes asynchonously.
    // g_volume_monitor_get() immediately returns while the monitor is still discovering devices.
    // So initially g_volume_monitor_get_volumes() returns nothing, but shortly after that
    // we get volume-added signals for all of the volumes. This is not what we want.
    // So, we wait for 3 seconds here to let it finish device discovery.
    QTimer::singleShot(3000, this, SLOT(initVolumeManager()));

    return QCoreApplication::exec();
}


void Application::onUserDirsChanged()
{
    qDebug() << Q_FUNC_INFO;
    bool file_deleted = !userDirsWatcher_->files().contains(userDirsFile_);
    if(file_deleted) {
        // if our config file is already deleted, reinstall a new watcher
        userDirsWatcher_->addPath(userDirsFile_);
    }

    const QString d = XdgDir::readDesktopDir();
    if (d != userDesktopFolder_) {
        userDesktopFolder_ = d;
        const QDir dir(d);
        if (dir.exists()) {
            const int N = desktopWindows_.size();
            for(int i = 0; i < N; ++i) {
                desktopWindows_.at(i)->setDesktopFolder();
            }
        } else {
            qWarning("Application::onUserDirsChanged: %s doesn't exist",
                     qUtf8Printable(userDesktopFolder_));
        }
    }
}

void Application::onAboutToQuit() {
    qDebug("aboutToQuit");
    settings_.save();
}

bool Application::eventFilter(QObject* watched, QEvent* event) {
    // Real KWin blur-behind (the "liquid glass" effect) for every context
    // menu app-wide, matching applyMenuStyleSheet()'s translucent fill/
    // rounded corners -- a flat QSS alpha color alone would just
    // dim whatever's behind the menu, not actually blur it. QMenu doesn't
    // have its final size until Qt positions/shows it, so this runs on
    // Show rather than at construction. Installed as a global filter
    // (installEventFilter(this) in the ctor) since menus are built all
    // over the codebase, not in one central place.
    if(event->type() == QEvent::Show) {
        if(QMenu* menu = qobject_cast<QMenu*>(watched)) {
            if(menu->windowHandle()) {
                QPainterPath path;
                path.addRoundedRect(menu->rect(), kMenuCornerRadius, kMenuCornerRadius);
                QRegion region(path.toFillPolygon().toPolygon());
                KWindowEffects::enableBlurBehind(menu->windowHandle(), true, region);
            }
        }
    }
    return QObject::eventFilter(watched, event);
}

bool Application::event(QEvent* event) {
    // On setups where Qt's platform theme plugin does report live palette/
    // style changes (unlike the kdeglobals-watching path above, which covers
    // setups where it doesn't), these events fire but nothing forces a
    // repaint of custom-painted views -- so pick them up too.
    switch(event->type()) {
    case QEvent::ApplicationPaletteChange:
    case QEvent::ThemeChange:
    case QEvent::StyleChange:
        // Also re-picks the context-menu QSS variant (see
        // applyMenuStyleSheet()'s doc comment) -- the palette read in the
        // ctor may predate Kvantum actually applying the session's real
        // theme, and this is the same signal that catches a genuine
        // light/dark switch at runtime.
        applyMenuStyleSheet();
        repaintAllWindows();
        // repaintAllWindows() above only repaints -- it doesn't rebuild the
        // sidebar's cached QSS (buildSidePaneStyleSheet() bakes the current
        // palette's text color into a literal string once, it doesn't
        // re-follow the palette on its own). A system-level theme/accent
        // change (kdeglobals, picked up here rather than through
        // onTintSettingsFileChanged()'s settings.conf watch) needs the same
        // updateFromSettings() push so the sidebar text color doesn't stay
        // stuck until the window is closed and reopened.
        updateFromSettings();
        break;
    default:
        break;
    }
    return QApplication::event(event);
}

void Application::onLastWindowClosed() {

}

void Application::onSaveStateRequest(QSessionManager& manager) {

}

// filer-dock hosts the pearos-dock Plasma applet (org.kde.taskmanager +
// org.kde.plasma.plasmoid) outside plasmashell, in its own Qt6/KF6 process
// -- it cannot be linked into Filer itself, which is Qt5. See
// filer-dock/README.md for how it's built and why it needs
// QML_IMPORT_PATH set explicitly rather than relying on a system-wide one.
void Application::startDock() {
    if(dockProcess_)
        return;

    QString exePath = QCoreApplication::applicationDirPath() + "/filer-dock";
    if(!QFile::exists(exePath)) {
        // Fall back to the location it lands in during development, when
        // Filer is run straight from its own build directory rather than
        // installed alongside filer-dock.
        exePath = QCoreApplication::applicationDirPath() + "/../../filer-dock/build/bin/filer-dock";
    }
    if(!QFile::exists(exePath)) {
        qWarning() << "filer-dock executable not found, not starting the dock. Looked at" << exePath;
        return;
    }

    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    QString qmlImportPath = QDir::homePath() + "/.local/lib/qml";
    QString existing = env.value("QML_IMPORT_PATH");
    env.insert("QML_IMPORT_PATH", existing.isEmpty() ? qmlImportPath : (qmlImportPath + ":" + existing));

    // org.kde.panel (the panel containment filer-dock hosts PearDock inside)
    // isn't provided by any system package here -- plasma-desktop isn't
    // installed, and filer-dock deliberately doesn't depend on plasmashell --
    // so it's built and installed user-locally too (see
    // ../panel-containment-src/), the same way PearDock is. Its compiled
    // plugin needs QT_PLUGIN_PATH, not QML_IMPORT_PATH, to be found.
    QString qtPluginPath = QDir::homePath() + "/.local/lib/plugins";
    QString existingPluginPath = env.value("QT_PLUGIN_PATH");
    env.insert("QT_PLUGIN_PATH", existingPluginPath.isEmpty() ? qtPluginPath : (qtPluginPath + ":" + existingPluginPath));

    dockIntentionalStop_ = false;
    dockProcess_ = new QProcess(this);
    dockProcess_->setProcessChannelMode(QProcess::ForwardedChannels);
    dockProcess_->setProcessEnvironment(env);
    dockProcess_->setProgram(exePath);
    connect(dockProcess_, &QProcess::errorOccurred, this, [](QProcess::ProcessError error) {
        qWarning() << "filer-dock failed to start:" << error;
    });
    // Covers both an actual crash and a self-detected hang: filer-dock's
    // own startup watchdog (see filer-dock/main.cpp) exits itself if it
    // never finishes loading, so from here both look the same -- the
    // process ended without stopDock() having asked it to. Respawn, but
    // with a backoff: more than 5 unintentional exits inside 60s means
    // something is wrong on every single launch (bad build, missing
    // dependency, ...), and respawning forever at that point would just
    // spin a silent, invisible crash loop instead of actually recovering.
    connect(dockProcess_, qOverload<int, QProcess::ExitStatus>(&QProcess::finished), this,
            [this](int exitCode, QProcess::ExitStatus exitStatus) {
        if(dockIntentionalStop_)
            return;
        qWarning() << "filer-dock exited unexpectedly (code" << exitCode << "status" << exitStatus << ") -- respawning";
        dockProcess_->deleteLater();
        dockProcess_ = nullptr;
        const qint64 now = QDateTime::currentMSecsSinceEpoch();
        dockRestartTimestamps_.append(now);
        while(!dockRestartTimestamps_.isEmpty() && now - dockRestartTimestamps_.first() > 60000)
            dockRestartTimestamps_.removeFirst();
        if(dockRestartTimestamps_.size() > 5) {
            qCritical() << "filer-dock has crashed" << dockRestartTimestamps_.size() << "times in the last minute -- giving up, not respawning again";
            return;
        }
        QTimer::singleShot(1000, this, &Application::startDock);
    });
    dockProcess_->start();
}

void Application::stopDock() {
    if(!dockProcess_)
        return;
    dockIntentionalStop_ = true;
    dockProcess_->terminate();
    if(!dockProcess_->waitForFinished(2000))
        dockProcess_->kill();
    dockProcess_->deleteLater();
    dockProcess_ = nullptr;
}

// filer-topbar hosts the pearOS top-bar plasmoids (xyz.pearos.pearmenu,
// PearControlCentre, PearClock -- see filer-topbar/main.cpp) in its own
// Qt6/KF6 layer-shell window with a real exclusive zone, so maximized
// windows start below the bar. Same hosting model and the same environment
// requirements as filer-dock above. Must be started before the desktop
// windows are created: createDesktopWindow() checks topBarProcess_ to
// decide whether to reserve the bar's strip.
void Application::startTopBar() {
    if(topBarProcess_)
        return;

    QString exePath = QCoreApplication::applicationDirPath() + "/filer-topbar";
    if(!QFile::exists(exePath)) {
        // Fall back to the location it lands in during development, when
        // Filer is run straight from its own build directory rather than
        // installed alongside filer-topbar.
        exePath = QCoreApplication::applicationDirPath() + "/../../filer-topbar/build/bin/filer-topbar";
    }
    if(!QFile::exists(exePath)) {
        qWarning() << "filer-topbar executable not found, not starting the top bar. Looked at" << exePath;
        return;
    }

    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    QString qmlImportPath = QDir::homePath() + "/.local/lib/qml";
    QString existing = env.value("QML_IMPORT_PATH");
    env.insert("QML_IMPORT_PATH", existing.isEmpty() ? qmlImportPath : (qmlImportPath + ":" + existing));

    // org.kde.panel's compiled plugin lives in the user-local plugin prefix,
    // same as for filer-dock -- see the comment in startDock().
    QString qtPluginPath = QDir::homePath() + "/.local/lib/plugins:/usr/lib/qt6/plugins-appmenu";
    QString existingPluginPath = env.value("QT_PLUGIN_PATH");
    env.insert("QT_PLUGIN_PATH", existingPluginPath.isEmpty() ? qtPluginPath : (qtPluginPath + ":" + existingPluginPath));

    topBarIntentionalStop_ = false;
    topBarProcess_ = new QProcess(this);
    topBarProcess_->setProcessChannelMode(QProcess::ForwardedChannels);
    topBarProcess_->setProcessEnvironment(env);
    topBarProcess_->setProgram(exePath);
    connect(topBarProcess_, &QProcess::errorOccurred, this, [](QProcess::ProcessError error) {
        qWarning() << "filer-topbar failed to start:" << error;
    });
    // Same crash-or-hang respawn as startDock() -- see its comment.
    // filer-topbar's own startup watchdog (filer-topbar/main.cpp) exits it
    // if it never finishes loading its plasmoids, so this catches both.
    connect(topBarProcess_, qOverload<int, QProcess::ExitStatus>(&QProcess::finished), this,
            [this](int exitCode, QProcess::ExitStatus exitStatus) {
        if(topBarIntentionalStop_)
            return;
        qWarning() << "filer-topbar exited unexpectedly (code" << exitCode << "status" << exitStatus << ") -- respawning";
        topBarProcess_->deleteLater();
        topBarProcess_ = nullptr;
        const qint64 now = QDateTime::currentMSecsSinceEpoch();
        topBarRestartTimestamps_.append(now);
        while(!topBarRestartTimestamps_.isEmpty() && now - topBarRestartTimestamps_.first() > 60000)
            topBarRestartTimestamps_.removeFirst();
        if(topBarRestartTimestamps_.size() > 5) {
            qCritical() << "filer-topbar has crashed" << topBarRestartTimestamps_.size() << "times in the last minute -- giving up, not respawning again";
            return;
        }
        QTimer::singleShot(1000, this, &Application::startTopBar);
    });
    topBarProcess_->start();
}

void Application::stopTopBar() {
    if(!topBarProcess_)
        return;
    topBarIntentionalStop_ = true;
    topBarProcess_->terminate();
    if(!topBarProcess_->waitForFinished(2000))
        topBarProcess_->kill();
    topBarProcess_->deleteLater();
    topBarProcess_ = nullptr;
}

void Application::loadWindowTrackerKwinScript() {
    // CMake installs this at ${CMAKE_INSTALL_PREFIX}/Resources/kwin-scripts/
    // (see src/CMakeLists.txt) -- Resources/ sits directly inside Filer.app
    // next to the Filer binary itself, not one level above it.
    QString scriptPath = QCoreApplication::applicationDirPath() + "/Resources/kwin-scripts/filer-window-tracker.js";
    if(!QFile::exists(scriptPath)) {
        // Fall back to the location it lands in during development, when
        // Filer is run straight from its own build directory rather than
        // installed (same fallback pattern as sound.cpp's WAV lookup).
        scriptPath = QCoreApplication::applicationDirPath() + "/../../src/kwin-scripts/filer-window-tracker.js";
    }
    if(!QFile::exists(scriptPath)) {
        qWarning() << "filer-window-tracker.js not found, sidebar wallpaper tint won't track window position. Looked at" << scriptPath;
        return;
    }

    QDBusInterface scripting(QStringLiteral("org.kde.KWin"), QStringLiteral("/Scripting"),
                              QStringLiteral("org.kde.kwin.Scripting"));
    if(!scripting.isValid()) {
        qWarning() << "org.kde.kwin.Scripting D-Bus interface not available, sidebar wallpaper tint won't track window position.";
        return;
    }
    // KWin keeps scripts loaded across separate Filer process launches (it's
    // the compositor's state, not ours) -- loadScript() on an already-loaded
    // name returns -1 rather than re-loading, which would otherwise build an
    // invalid "/Scripting/Script-1" object path below and abort the process
    // (QDBusInterface asserts on invalid paths). Unload any stale instance
    // first so this is idempotent across restarts.
    //
    // "-v2": after enough unload/reload cycles in one KWin session (heavy
    // Filer restarts during development, or just a long-lived login), this
    // exact plugin name was observed to end up permanently wedged -- still
    // reporting isScriptLoaded() true and accepting a fresh loadScript()+
    // run() with no error, but its JS body silently never executing again
    // (confirmed by adding print() calls directly to the script: they
    // fired under a brand-new name immediately, never under this one).
    // Since KWin's own scripting engine -- not this file -- owns whatever
    // internal state got stuck, and there's no unwedge API to call, the
    // name itself is the only lever available: bump this suffix if it
    // ever happens again rather than debugging the compositor.
    QDBusReply<bool> alreadyLoaded = scripting.call(QStringLiteral("isScriptLoaded"), QStringLiteral("filer-window-tracker-v2"));
    if(alreadyLoaded.isValid() && alreadyLoaded.value()) {
        scripting.call(QStringLiteral("unloadScript"), QStringLiteral("filer-window-tracker-v2"));
    }
    QDBusReply<int> scriptId = scripting.call(QStringLiteral("loadScript"), scriptPath, QStringLiteral("filer-window-tracker-v2"));
    if(!scriptId.isValid() || scriptId.value() < 0) {
        qWarning() << "Failed to load filer-window-tracker.js:" << scriptId.error().message();
        return;
    }
    QDBusInterface(QStringLiteral("org.kde.KWin"), QStringLiteral("/Scripting/Script%1").arg(scriptId.value()),
                    QStringLiteral("org.kde.kwin.Script")).call(QStringLiteral("run"));
}

void Application::desktopManager(bool enabled) {
    // TODO: turn on or turn off desktpo management (desktop icons & wallpaper)
    qDebug("desktopManager: %d", enabled);
    QDesktopWidget* desktopWidget = desktop();
    if(enabled) {
        if(!enableDesktopManager_) {
            // installNativeEventFilter(this);
            Q_FOREACH(QScreen* screen, screens()) {
                connect(screen, &QScreen::virtualGeometryChanged, this, &Application::onVirtualGeometryChanged);
                connect(screen, &QObject::destroyed, this, &Application::onScreenDestroyed);
            }
            connect(this, &QApplication::screenAdded, this, &Application::onScreenAdded);
            connect(desktopWidget, &QDesktopWidget::resized, this, &Application::onScreenResized);
            connect(desktopWidget, &QDesktopWidget::screenCountChanged, this, &Application::onScreenCountChanged);

            // Before the desktop windows: createDesktopWindow() reserves the
            // top bar's strip only if topBarProcess_ is already set.
            startTopBar();

            // NOTE: there are two modes
            // When virtual desktop is used (all screens are combined to form a large virtual desktop),
            // we only create one DesktopWindow. Otherwise, we create one for each screen.
            if(desktopWidget->isVirtualDesktop()) {
                DesktopWindow* window = createDesktopWindow(-1);
                desktopWindows_.push_back(window);
            }
            else {
                int n = desktopWidget->numScreens();
                desktopWindows_.reserve(n);
                for(int i = 0; i < n; ++i) {
                    DesktopWindow* window = createDesktopWindow(i);
                    desktopWindows_.push_back(window);
                }
            }
            startDock();
        }
    }
    else {
        if(enableDesktopManager_) {
            stopDock();
            stopTopBar();
            disconnect(desktopWidget, &QDesktopWidget::resized, this, &Application::onScreenResized);
            disconnect(desktopWidget, &QDesktopWidget::screenCountChanged, this, &Application::onScreenCountChanged);
            int n = desktopWindows_.size();
            for(int i = 0; i < n; ++i) {
                DesktopWindow* window = desktopWindows_.at(i);
                delete window;
            }
            desktopWindows_.clear();
            Q_FOREACH(QScreen* screen, screens()) {
                disconnect(screen, &QScreen::virtualGeometryChanged, this, &Application::onVirtualGeometryChanged);
                disconnect(screen, &QObject::destroyed, this, &Application::onScreenDestroyed);
            }
            disconnect(this, &QApplication::screenAdded, this, &Application::onScreenAdded);
            // removeNativeEventFilter(this);
        }
    }
    enableDesktopManager_ = enabled;
}

void Application::desktopPrefrences() {
    // show desktop preference window
    if(!desktopPreferencesDialog_) {
        desktopPreferencesDialog_ = new DesktopPreferencesDialog();
    }
    desktopPreferencesDialog_.data()->show();
    desktopPreferencesDialog_.data()->raise();
    desktopPreferencesDialog_.data()->activateWindow();
}

void Application::onFindFileAccepted() {
    Fm::FileSearchDialog* dlg = static_cast<Fm::FileSearchDialog*>(sender());
    Fm::Path uri = dlg->searchUri();
    // FIXME: we should be able to open it in an existing window
    FmPathList* paths = fm_path_list_new();
    fm_path_list_push_tail(paths, uri.data());
    Launcher(NULL).launchPaths(NULL, paths);
    fm_path_list_unref(paths);
}

void Application::findFiles(QStringList paths) {
    // launch file searching utility.
    Fm::FileSearchDialog* dlg = new Fm::FileSearchDialog(paths);
    connect(dlg, &QDialog::accepted, this, &Application::onFindFileAccepted);
    dlg->setAttribute(Qt::WA_DeleteOnClose);
    dlg->show();
}

void Application::launchFiles(QString cwd, QStringList paths, bool inNewWindow) {
    FmPathList* pathList = fm_path_list_new();
    FmPath* cwd_path = NULL;
    QStringList::iterator it;
    Q_FOREACH(const QString& it, paths) {
        QByteArray pathName = it.toLocal8Bit();
        FmPath* path = NULL;
        if(pathName[0] == '/') // absolute path
            path = fm_path_new_for_path(pathName.constData());
        else if(pathName.contains(":/")) // URI
            path = fm_path_new_for_uri(pathName.constData());
        else if(pathName == "~") // special case for home dir
            path = fm_path_ref(fm_path_get_home());
        else // basename
        {
            if(Q_UNLIKELY(!cwd_path))
                cwd_path = fm_path_new_for_str(cwd.toLocal8Bit().constData());
            path = fm_path_new_relative(cwd_path, pathName.constData());
        }
        fm_path_list_push_tail(pathList, path);
        fm_path_unref(path);
    }
    if(cwd_path)
        fm_path_unref(cwd_path);

    Launcher(NULL).launchPaths(NULL, pathList);
    fm_path_list_unref(pathList);
}

void Application::openFolders(FmFileInfoList* files) {
    Launcher(NULL).launchFiles(NULL, files);
}

void Application::openFolderInTerminal(FmPath* path) {
    if(!settings_.terminal().isEmpty()) {
        char* cwd_str;
        if(fm_path_is_native(path))
            cwd_str = fm_path_to_str(path);
        else { // gio will map remote filesystems to local FUSE-mounted paths here.
            GFile* gf = fm_path_to_gfile(path);
            cwd_str = g_file_get_path(gf);
            g_object_unref(gf);
        }
        GError* err = NULL;
        if(!fm_terminal_launch(cwd_str, &err)) {
            QMessageBox::critical(NULL, tr("Error"), QString::fromUtf8(err->message));
            g_error_free(err);
        }
        g_free(cwd_str);
    }
    else {
        // show an error message and ask the user to set the command
        QMessageBox::critical(NULL, tr("Error"), tr("Terminal emulator is not set."));
        preferences("advanced");
    }
}

void Application::preferences(QString page) {
    // open preference dialog
    if(!preferencesDialog_) {
        preferencesDialog_ = new PreferencesDialog(page);
    }
    else {
        // TODO: set page
    }
    preferencesDialog_.data()->show();
    preferencesDialog_.data()->raise();
    preferencesDialog_.data()->activateWindow();
}

void Application::setWallpaper(QString path, QString modeString) {
    // Must match DesktopWindow::WallpaperMode's declaration order (see
    // desktopwindow.h) -- this array is indexed directly into that enum.
    static const char* valid_wallpaper_modes[] = {"none", "transparent", "stretch", "fit", "center", "tile"};
    DesktopWindow::WallpaperMode mode = settings_.wallpaperMode();
    bool changed = false;

    if(!path.isEmpty() && path != settings_.wallpaper()) {
        if(QFile(path).exists()) {
            settings_.setWallpaper(path);
            changed = true;
        }
    }
    // convert mode string to value
    for(int i = 0; i < G_N_ELEMENTS(valid_wallpaper_modes); ++i) {
        if(modeString == valid_wallpaper_modes[i]) {
            mode = (DesktopWindow::WallpaperMode)i;
            if(mode != settings_.wallpaperMode())
                changed = true;
            break;
        }
    }
    // update wallpaper
    if(changed) {
        settings_.setWallpaperMode(mode);
        settings_.save(); // save the settings to the config file
        static_cast<Application*>(qApp)->updateDesktopsFromSettings(); // Fixes https://github.com/helloSystem/Filer/issues/100
    }
}

/* This method receives a list of file:// URIs from DBus and opens windows
 * or tabs for each of them
 */
void Application::ShowFolders(const QStringList uriList, const QString startupId)
{
    // Absolute insane amount of effort is needed to convert QStringList to FmFileInfoList;
    // this is why we need to get rid of libfm-qt and use Qt only
    FmFileInfoList* files = fm_file_info_list_new();

    Q_EMIT openFolders(files);

    for (QString uri : uriList) {
        GFile* file = fm_file_new_for_uri(uri.toUtf8().constData());
        FmFileInfo* fileInfo = fm_file_info_new();
        FmPath* path = fm_path_new_for_gfile(file);
        fm_file_info_set_path(fileInfo, path);
        fm_file_info_list_push_tail(files, fileInfo);
        fm_file_info_unref(fileInfo);
        g_object_unref(file);
    }

    fm_file_info_list_unref(files);
}

/* This method receives a list of file:// URIs from DBus and opens windows
 * or tabs for each folder, highlighting all listed items within each. The
 * input list is not sorted or grouped so we need to marshal it into groups
 * by folder, then call our "reveal" method to show each group
 * --mszoek
 */
void Application::ShowItems(const QStringList uriList, const QString startupId)
{
    QMap<QString,QStringList> groups;
    
    for(QUrl u : uriList) {
        QFileInfo info(u.path());
        QString folder(QDir(info.dir()).absolutePath());
        if(info.exists()) {
            if(groups.empty() || !groups.contains(folder))
                groups[folder] = QStringList();
            groups[folder].append(info.filePath());
        }
    }

    for(QString k : groups.keys())
        Q_EMIT openFolderAndSelectItems(k, groups[k]);
}

void Application::ShowItemProperties(const QStringList uriList, const QString startupId)
{
}

void Application::onScreenResized(int num) {
    if(desktop()->isVirtualDesktop()) {
        // in virtual desktop mode, we only have one desktop window. that is the first one.
        DesktopWindow* window = desktopWindows_.at(0);
        window->setGeometry(desktop()->geometry());
    }
    else {
        DesktopWindow* window = desktopWindows_.at(num);
        QRect rect = desktop()->screenGeometry(num);
        window->setGeometry(rect);
    }
}

DesktopWindow* Application::desktopWindowForScreen(QScreen* screen) const {
    if(!screen)
        return nullptr;
    int index = screens().indexOf(screen);
    if(index < 0 || index >= desktopWindows_.size())
        return nullptr;
    return desktopWindows_.at(index);
}

DesktopWindow* Application::createDesktopWindow(int screenNum) {
    DesktopWindow* window = new DesktopWindow(screenNum);
    // filer-topbar covers the top strip of the screen; reserve space so
    // that the desktop icon area is placed below the bar.
    // Keep in sync with barHeight in ../filer-topbar/main.cpp.
    window->setReservedTopArea(34);
    if(screenNum == -1) { // one large virtual desktop only
        QRect rect = desktop()->geometry();
        window->setGeometry(rect);
    }
    else {
        QRect rect = desktop()->screenGeometry(screenNum);
        window->setGeometry(rect);
    }
    window->updateFromSettings(settings_);
    window->show();
    return window;
}

void Application::onScreenCountChanged(int newCount) {
    QDesktopWidget* desktopWidget = desktop();
    bool oldVirtual = (desktopWindows_.size() == 1 && desktopWindows_.at(0)->screenNum() == -1);
    bool isVirtual = desktopWidget->isVirtualDesktop();

    if(oldVirtual && isVirtual) {
        // if we are using virtual desktop mode previously, and the new mode is sitll virtual
        // no further change is needed, only do relayout.
        desktopWindows_.at(0)->queueRelayout();
        return;
    }

    // we used non-virtual mode originally, but now we're switched to virtual mode
    if(isVirtual)
        newCount = 1; // we only want one desktop window for all screens in virtual mode

    if(newCount > desktopWindows_.size()) {
        // add more desktop windows
        for(int i = desktopWindows_.size(); i < newCount; ++i) {
            DesktopWindow* desktop = createDesktopWindow(i);
            desktopWindows_.push_back(desktop);
        }
    }
    else if(newCount < desktopWindows_.size()) {
        // delete excessive desktop windows
        for(int i = newCount; i < desktopWindows_.size(); ++i) {
            DesktopWindow* desktop = desktopWindows_.at(i);
            delete desktop;
        }
        desktopWindows_.resize(newCount);
    }

    if(newCount == 1) { // now only 1 screen is in use
        DesktopWindow* desktop = desktopWindows_.at(0);
        if(isVirtual)
            desktop->setScreenNum(-1);
        else // non-virtual mode, and we only have 1 screen
            desktop->setScreenNum(0);
        desktop->updateWallpaper();
    }
}

// called when Settings is changed to update UI
void Application::updateFromSettings() {
    // if(iconTheme.isEmpty())
    //  Fm::IconTheme::setThemeName(settings_.fallbackIconThemeName());

    // update main windows and desktop windows
    QWidgetList windows = this->topLevelWidgets();
    QWidgetList::iterator it;
    for(it = windows.begin(); it != windows.end(); ++it) {
        QWidget* window = *it;
        if(window->inherits("Filer::MainWindow")) {
            MainWindow* mainWindow = static_cast<MainWindow*>(window);
            mainWindow->updateFromSettings(settings_);
        }
    }
    if(desktopManagerEnabled())
        updateDesktopsFromSettings();
}

void Application::updateDesktopsFromSettings() {
    QVector<DesktopWindow*>::iterator it;
    for(it = desktopWindows_.begin(); it != desktopWindows_.end(); ++it) {
        DesktopWindow* desktopWindow = static_cast<DesktopWindow*>(*it);
        desktopWindow->updateFromSettings(settings_);
    }
}

void Application::editBookmarks() {
    if(!editBookmarksialog_) {
        FmBookmarks* bookmarks = fm_bookmarks_dup();
        editBookmarksialog_ = new Fm::EditBookmarksDialog(bookmarks);
        g_object_unref(bookmarks);
    }
    editBookmarksialog_.data()->show();
}

void Application::initVolumeManager() {

    g_signal_connect(volumeMonitor, "volume-added", G_CALLBACK(onVolumeAdded), this);

    if(settings_.mountOnStartup()) {
        /* try to automount all volumes */
        GList* vols = g_volume_monitor_get_volumes(volumeMonitor);
        for(GList* l = vols; l; l = l->next) {
            GVolume* volume = G_VOLUME(l->data);
            if(g_volume_should_automount(volume))
                autoMountVolume(volume, false);
            g_object_unref(volume);
        }
        g_list_free(vols);
    }
}

bool Application::autoMountVolume(GVolume* volume, bool interactive) {
    if(!g_volume_should_automount(volume) || !g_volume_can_mount(volume))
        return FALSE;

    GMount* mount = g_volume_get_mount(volume);
    if(!mount) { // not mounted, automount is needed
        // try automount
        Fm::MountOperation* op = new Fm::MountOperation(interactive);
        op->mount(volume);
        if(!op->wait())
            return false;
        if(!interactive)
            return true;
        mount = g_volume_get_mount(volume);
    }

    if(mount) {
        if(interactive && settings_.autoRun()) { // show autorun dialog
            AutoRunDialog* dlg = new AutoRunDialog(volume, mount);
            dlg->show();
        }
        g_object_unref(mount);
    }
    return true;
}

// static
void Application::onVolumeAdded(GVolumeMonitor* monitor, GVolume* volume, Application* pThis) {
    if(pThis->settings_.mountRemovable())
        pThis->autoMountVolume(volume, true);
}

#if 0
bool Application::nativeEventFilter(const QByteArray & eventType, void * message, long * result) {
    if(eventType == "xcb_generic_event_t") { // XCB event
        // filter all native X11 events (xcb)
        xcb_generic_event_t* generic_event = reinterpret_cast<xcb_generic_event_t*>(message);
        // qDebug("XCB event: %d", generic_event->response_type & ~0x80);
        Q_FOREACH(DesktopWindow * window, desktopWindows_) {
        }
    }
    return false;
}
#endif

void Application::onScreenAdded(QScreen* newScreen) {
    if(enableDesktopManager_) {
        connect(newScreen, &QScreen::virtualGeometryChanged, this, &Application::onVirtualGeometryChanged);
        connect(newScreen, &QObject::destroyed, this, &Application::onScreenDestroyed);
    }
}

void Application::onScreenDestroyed(QObject* screenObj) {
    // NOTE by PCMan: This is a workaround for Qt 5 bug #40681.
    // With this very dirty workaround, we can fix lxde/lxde-qt bug #204, #205, and #206.
    // Qt 5 has two new regression bugs which breaks lxqt-panel in a multihead environment.
    // #40681: Regression bug: QWidget::winId() returns old value and QEvent::WinIdChange event is not emitted sometimes. (multihead setup)
    // #40791: Regression: QPlatformWindow, QWindow, and QWidget::winId() are out of sync.
    // Explanations for the workaround:
    // Internally, Qt mantains a list of QScreens and update it when XRandR configuration changes.
    // When the user turn off an monitor with xrandr --output <xxx> --off, this will destroy the QScreen
    // object which represent the output. If the QScreen being destroyed contains our panel widget,
    // Qt will call QWindow::setScreen(0) on the internal windowHandle() of our panel widget to move it
    // to the primary screen. However, moving a window to a different screen is more than just changing
    // its position. With XRandR, all screens are actually part of the same virtual desktop. However,
    // this is not the case in other setups, such as Xinerama and moving a window to another screen is
    // not possible unless you destroy the widget and create it again for a new screen.
    // Therefore, Qt destroy the widget and re-create it when moving our panel to a new screen.
    // Unfortunately, destroying the window also destroy the child windows embedded into it,
    // using XEMBED such as the tray icons. (#206)
    // Second, when the window is re-created, the winId of the QWidget is changed, but Qt failed to
    // generate QEvent::WinIdChange event so we have no way to know that. We have to set
    // some X11 window properties using the native winId() to make it a dock, but this stop working
    // because we cannot get the correct winId(), so this causes #204 and #205.
    //
    // The workaround is very simple. Just completely destroy the window before Qt has a chance to do
    // QWindow::setScreen() for it. Later, we recreate the window ourselves. So this can bypassing the Qt bugs.
    if(enableDesktopManager_) {
        bool reloadNeeded = false;
        // FIXME: add workarounds for Qt5 bug #40681 and #40791 here.
        Q_FOREACH(DesktopWindow* desktop, desktopWindows_) {
            if(desktop->windowHandle()->screen() == screenObj) {
                desktop->destroy(); // destroy the underlying native window
                reloadNeeded = true;
            }
        }
        if(reloadNeeded)
            QTimer::singleShot(0, this, SLOT(reloadDesktopsAsNeeded()));
    }
}

void Application::reloadDesktopsAsNeeded() {
    if(enableDesktopManager_) {
        // workarounds for Qt5 bug #40681 and #40791 here.
        Q_FOREACH(DesktopWindow* desktop, desktopWindows_) {
            if(!desktop->windowHandle()) {
                desktop->create(); // re-create the underlying native window
                desktop->queueRelayout();
                desktop->show();
            }
        }
    }
}

// This slot is for Qt 5 onlt, but the stupid Qt moc cannot do conditional compilation
// so we have to define it for Qt 4 as well.
void Application::onVirtualGeometryChanged(const QRect& rect) {
    // NOTE: the following is a workaround for Qt bug 32567.
    // https://bugreports.qt-project.org/browse/QTBUG-32567
    // Though the status of the bug report is closed, it's not yet fixed for X11.
    // In theory, QDesktopWidget should emit "workAreaResized()" signal when the work area
    // of any screen is changed, but in fact it does not do it.
    // However, QScreen provided since Qt5 does not have the bug and
    // virtualGeometryChanged() is emitted correctly when the workAreas changed.
    // So we use it in Qt5.
    if(enableDesktopManager_) {
        // qDebug() << "onVirtualGeometryChanged";
        Q_FOREACH(DesktopWindow* desktop, desktopWindows_) {
            desktop->queueRelayout();
        }
    }
}


static int sigterm_fd[2];

static void sigtermHandler(int) {
    char c = 1;
    ::write(sigterm_fd[0], &c, sizeof(c));
}

void Application::installSigtermHandler() {
    if(::socketpair(AF_UNIX, SOCK_STREAM, 0, sigterm_fd) == 0) {
        QSocketNotifier* notifier = new QSocketNotifier(sigterm_fd[1], QSocketNotifier::Read, this);
        connect(notifier, &QSocketNotifier::activated, this, &Application::onSigtermNotified);

        struct sigaction action = {};
        action.sa_handler = sigtermHandler;
        ::sigemptyset(&action.sa_mask);
        action.sa_flags |= SA_RESTART;
        if(::sigaction(SIGTERM, &action, 0) != 0) {
            qWarning("Couldn't install SIGTERM handler");
        }
    } else {
        qWarning("Couldn't create SIGTERM socketpair");
    }
}

void Application::onSigtermNotified() {
    if (QSocketNotifier* notifier = qobject_cast<QSocketNotifier*>(sender())) {
        notifier->setEnabled(false);
        char c;
        ::read(sigterm_fd[1], &c, sizeof(c));
        quit();
        notifier->setEnabled(true);
    }
}
