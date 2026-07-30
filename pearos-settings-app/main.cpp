#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickWindow>
#include <QFont>
#include <QFontDatabase>
#include <QSurfaceFormat>
#include <QLoggingCategory>
#include <stdio.h>

#include "backend/networkmanager.h"
#include "backend/bluetoothmanager.h"
#include "backend/audiomanager.h"
#include "backend/displaymanager.h"
#include "backend/batterymanager.h"
#include "backend/systeminfo.h"
#include "backend/wallpapermanager.h"
#include "backend/updatemanager.h"
#include "backend/appearancemanager.h"
#include "backend/keyboardmanager.h"
#include "backend/trackpadmanager.h"
#include "backend/usermanager.h"
#include "backend/datetimemanager.h"
#include "backend/privacymanager.h"
#include "backend/lockscreenmanager.h"
#include "backend/startupmanager.h"
#include "backend/pearidmanager.h"
#include "backend/pirimanager.h"
#include "backend/dockmanager.h"
#include "backend/touchidmanager.h"
#include "backend/sidereflection.h"
#include "backend/accessibilitymanager.h"
#include "backend/menubarmanager.h"
#include "backend/notificationsmanager.h"
#include "backend/focusmanager.h"
#include "backend/screentimemanager.h"
#include "backend/scrollbarmanager.h"
#include "backend/printersmanager.h"
#include "backend/spotlightmanager.h"
#include "backend/internetaccountsmanager.h"
#include "backend/filerconfigmanager.h"
#include "backend/hotcornersmanager.h"
#include "backend/missioncontrolmanager.h"
#include "backend/proxymanager.h"
#include "backend/kwalletmanager.h"

int main(int argc, char *argv[]) {
    qInstallMessageHandler([](QtMsgType type, const QMessageLogContext &ctx, const QString &msg) {
        fprintf(stderr, "[%s] %s\n",
            type == QtDebugMsg ? "D" : type == QtWarningMsg ? "W" : type == QtCriticalMsg ? "C" : "F",
            qPrintable(msg));
    });
    qputenv("QT_QPA_PLATFORM", "xcb");
    qunsetenv("SESSION_MANAGER"); // avoid Qt's session-management socket connection attempt/warning
    QQuickWindow::setDefaultAlphaBuffer(true);
    QSurfaceFormat fmt = QSurfaceFormat::defaultFormat();
    fmt.setAlphaBufferSize(8);
    QSurfaceFormat::setDefaultFormat(fmt);
    QGuiApplication::setApplicationName("systemsettings1");
    QGuiApplication::setOrganizationName("PearOS");
    QGuiApplication::setDesktopFileName("pearos-systemsettings");

    QGuiApplication app(argc, argv);

    // Load SF Pro if available, fall back to system sans-serif
    int fontId = QFontDatabase::addApplicationFont(":/fonts/SFPro.ttf");
    QString family = fontId != -1
        ? QFontDatabase::applicationFontFamilies(fontId).value(0)
        : "Sans Serif";
    app.setFont(QFont(family, 10));

    NetworkManager   network;
    BluetoothManager bluetooth;
    AudioManager     audio;
    DisplayManager   display;
    BatteryManager   battery;
    SystemInfo       sysinfo;
    WallpaperManager wallpaper;
    UpdateManager    updates;
    AppearanceManager appearance;
    KeyboardManager  keyboard;
    TrackpadManager  trackpad;
    UserManager      users;
    DateTimeManager  datetime;
    PrivacyManager   privacy;
    LockScreenManager lockscreen;
    StartupManager   startup;
    PearIDManager    pearid;
    PiriManager      piri;
    DockManager      dock;
    TouchIDManager   touchid;
    AccessibilityManager accessibility;
    MenuBarManager   menubar;
    NotificationsManager notifications;
    FocusManager     focus;
    ScreenTimeManager screentime;
    ScrollbarManager scrollbars;
    PrintersManager  printers;
    SpotlightManager spotlight;
    InternetAccountsManager inetaccounts;
    FilerConfigManager filerConfig;
    HotCornersManager hotcorners;
    MissionControlManager missioncontrol;
    ProxyManager     proxy;
    KWalletManager   kwallet;

    SidebarReflection* siderefl = new SidebarReflection();

    QQmlApplicationEngine engine;
    engine.addImageProvider("siderefl", new SidebarReflectionProvider(siderefl));
    QQmlContext *ctx = engine.rootContext();
    ctx->setContextProperty("SidebarRefl", siderefl);
    ctx->setContextProperty("Network",    &network);
    ctx->setContextProperty("Bluetooth",  &bluetooth);
    ctx->setContextProperty("Audio",      &audio);
    ctx->setContextProperty("Display",    &display);
    ctx->setContextProperty("Battery",    &battery);
    ctx->setContextProperty("SysInfo",    &sysinfo);
    ctx->setContextProperty("Wallpaper",  &wallpaper);
    ctx->setContextProperty("Updates",    &updates);
    ctx->setContextProperty("Appearance", &appearance);
    ctx->setContextProperty("Keyboard",   &keyboard);
    ctx->setContextProperty("Trackpad",   &trackpad);
    ctx->setContextProperty("Users",      &users);
    ctx->setContextProperty("DateTime",   &datetime);
    ctx->setContextProperty("Privacy",    &privacy);
    ctx->setContextProperty("LockScreen", &lockscreen);
    ctx->setContextProperty("Startup",    &startup);
    ctx->setContextProperty("PearID",     &pearid);
    ctx->setContextProperty("Piri",       &piri);
    ctx->setContextProperty("Dock",       &dock);
    ctx->setContextProperty("TouchID",    &touchid);
    ctx->setContextProperty("Accessibility", &accessibility);
    ctx->setContextProperty("MenuBarCfg", &menubar);
    ctx->setContextProperty("Notifications", &notifications);
    ctx->setContextProperty("Focus",      &focus);
    ctx->setContextProperty("ScreenTime", &screentime);
    ctx->setContextProperty("Scrollbars", &scrollbars);
    ctx->setContextProperty("Printers",   &printers);
    ctx->setContextProperty("Spotlight",  &spotlight);
    ctx->setContextProperty("InetAccounts", &inetaccounts);
    ctx->setContextProperty("FilerConfig", &filerConfig);
    ctx->setContextProperty("HotCorners", &hotcorners);
    ctx->setContextProperty("MissionControl", &missioncontrol);
    ctx->setContextProperty("Proxy",      &proxy);
    ctx->setContextProperty("KWallet",    &kwallet);

    engine.loadFromModule("PearOSSettings", "Main");

    if (engine.rootObjects().isEmpty()) return -1;

    // Enable KDE blur-behind for frosted glass effect (requires KWin compositor)
    if (auto *win = qobject_cast<QQuickWindow*>(engine.rootObjects().first())) {
        // Connect to first frame to ensure WId is valid before setting blur hint
        QObject::connect(win, &QQuickWindow::frameSwapped, win, [win]() {
            static bool done = false;
            if (done) return;
            done = true;
            WId wid = win->winId();
            // _KDE_NET_WM_BLUR_BEHIND_REGION with empty region = blur entire window
            QProcess::startDetached("bash", {"-c",
                QString("sleep 0.2 && xprop -id %1 -format _KDE_NET_WM_BLUR_BEHIND_REGION 32c "
                        "-set _KDE_NET_WM_BLUR_BEHIND_REGION '0,0,10000,10000' 2>/dev/null || true")
                    .arg(wid)});
        }, Qt::SingleShotConnection);
    }

    return app.exec();
}
