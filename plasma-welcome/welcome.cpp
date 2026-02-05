#include <QApplication>
#include <QMainWindow>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QCheckBox>
#include <QDialog>
#include <QDesktopServices>
#include <QUrl>
#include <QIcon>
#include <QPixmap>
#include <QScreen>
#include <QTimer>
#include <QMessageBox>
#include <QFile>
#include <QDir>
#include <QProcess>
#include <QThread>
#include <QObject>
#include <QStandardPaths>
#include <QMoveEvent>
#include <QWebEngineView>
#include <QWebEnginePage>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QEventLoop>
#include <QDirIterator>

#include <functional>
#include <cstdio>
#include <cstring>
#include <unistd.h>
#include <sys/stat.h>

static const int WINDOW_WIDTH = 480;
static const int WINDOW_HEIGHT = 550;
static const int LOGO_SIZE = 60;
static const int ICON_SIZE = 20;
static const char INSTALL_PREFIX[] = "/usr/share/pearos-welcome";

/** Returns true if any window with "electron" in its title is present (e.g. Electron apps). */
static bool is_electron_window_visible(const QString &basePath) {
    const bool wayland = !qEnvironmentVariable("WAYLAND_DISPLAY").isEmpty()
        || qEnvironmentVariable("XDG_SESSION_TYPE") == QLatin1String("wayland");

    if (wayland) {
        // KWin on Wayland: run a small script via D-Bus, get window titles from journalctl
        const QString scriptPath = basePath + QLatin1String("/list_windows.js");
        if (!QFile::exists(scriptPath))
            return false;

        QProcess loadProc;
        loadProc.setProgram(QStringLiteral("dbus-send"));
        loadProc.setArguments(QStringList()
            << QStringLiteral("--print-reply")
            << QStringLiteral("--dest=org.kde.KWin")
            << QStringLiteral("/Scripting")
            << QStringLiteral("org.kde.kwin.Scripting.loadScript")
            << QStringLiteral("string:") + scriptPath);
        loadProc.start(QProcess::ReadOnly);
        if (!loadProc.waitForFinished(3000) || loadProc.exitCode() != 0)
            return false;

        const QByteArray loadOut = loadProc.readAllStandardOutput();
        // Reply like "   uint32 0" or "   string \"0\"" - take last token as id
        QList<QByteArray> tokens = loadOut.trimmed().split(' ');
        if (tokens.isEmpty()) return false;
        QString scriptId = QString::fromUtf8(tokens.last()).trimmed();
        scriptId.remove(QLatin1Char('"'));

        QProcess runProc;
        runProc.setProgram(QStringLiteral("dbus-send"));
        runProc.setArguments(QStringList()
            << QStringLiteral("--print-reply")
            << QStringLiteral("--dest=org.kde.KWin")
            << (QStringLiteral("/Scripting/Script") + scriptId)
            << QStringLiteral("org.kde.kwin.Script.run"));
        runProc.start(QProcess::ReadOnly);
        runProc.waitForFinished(2000);

        QThread::msleep(400);

        QProcess journalProc;
        journalProc.setProgram(QStringLiteral("journalctl"));
        journalProc.setArguments(QStringList()
            << QStringLiteral("_COMM=kwin_wayland")
            << QStringLiteral("-o")
            << QStringLiteral("cat")
            << QStringLiteral("-n")
            << QStringLiteral("500")
            << QStringLiteral("--no-pager"));
        journalProc.start(QProcess::ReadOnly);
        if (!journalProc.waitForFinished(3000))
            return false;

        const QString journal = QString::fromUtf8(journalProc.readAllStandardOutput());
        const QStringList lines = journal.split(QLatin1Char('\n'));
        for (const QString &line : lines) {
            QString t = line.trimmed();
            if (t.startsWith(QLatin1String("js: ")))
                t = t.mid(4);
            if (t.contains(QLatin1String("electron"), Qt::CaseInsensitive))
                return true;
        }

        runProc.setArguments(QStringList()
            << QStringLiteral("--dest=org.kde.KWin")
            << (QStringLiteral("/Scripting/Script") + scriptId)
            << QStringLiteral("org.kde.kwin.Script.stop"));
        runProc.start(QProcess::ReadOnly);
        runProc.waitForFinished(1000);
        return false;
    }

    // X11: xdotool
    QProcess p;
    p.setProgram(QStringLiteral("xdotool"));
    p.setArguments(QStringList() << QStringLiteral("search") << QStringLiteral("--name") << QStringLiteral("electron"));
    p.start(QProcess::ReadOnly);
    if (!p.waitForFinished(2000))
        return false;
    return (p.exitStatus() == QProcess::NormalExit && p.exitCode() == 0);
}

/** Return true if we have internet (reachability of the What's New page). */
static bool is_online() {
    QNetworkAccessManager manager;
    QEventLoop loop;
    bool online = false;
    QNetworkReply *reply = manager.get(QNetworkRequest(QUrl(QStringLiteral("https://new.pearos.xyz/"))));
    QObject::connect(reply, &QNetworkReply::finished, &loop, [&]() {
        if (reply->error() == QNetworkReply::NoError)
            online = true;
        loop.quit();
    });
    QTimer::singleShot(3000, &loop, &QEventLoop::quit);
    loop.exec();
    return online;
}

/** Base path for assets/styles: next to executable, or INSTALL_PREFIX when installed. */
static QString get_base_path() {
    QString base = QCoreApplication::applicationDirPath();
    if (QFile::exists(base + "/styles.qss"))
        return base;
    if (QFile::exists(QLatin1String(INSTALL_PREFIX) + "/styles.qss"))
        return QLatin1String(INSTALL_PREFIX);
    return base;
}

// --- Helpers (mirror utils/extras.py, autostart, themes, etc.) ---

static QString get_desktop_environment() {
    QString de = qEnvironmentVariable("XDG_CURRENT_DESKTOP", "");
    return de.toLower();
}

static bool check_if_live_iso() {
    return QFile::exists("/run/archiso");
}

static void open_url(const QString &url) {
    if (!QDesktopServices::openUrl(QUrl(url))) {
        QProcess::startDetached("xdg-open", QStringList() << url);
    }
}

static bool autostart_file_exists() {
    QString path = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation)
        + "/autostart/welcome.desktop";
    return QFile::exists(path);
}

static void toggle_autostart(bool enable) {
    QString autostartDir = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + "/autostart";
    QString autostartFile = autostartDir + "/welcome.desktop";
    QString sourceFile = "/usr/share/applications/welcome.desktop";

    if (enable) {
        if (QFile::exists(autostartFile)) return;
        QDir().mkpath(autostartDir);
        if (QFile::exists(sourceFile))
            QFile::copy(sourceFile, autostartFile);
    } else {
        if (!QFile::exists(autostartFile)) return;
        QProcess::execute("pkexec", QStringList() << "rm" << autostartFile);
    }
}

static QString read_theme_state() {
    QFile f("/usr/share/extras/system-settings/themeswitcher/state");
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) return QString();
    QString state = QString::fromUtf8(f.readAll()).trimmed().toLower();
    return (state == "dark" || state == "light") ? state : QString();
}

static bool is_dark_theme() {
    return read_theme_state() == "dark";
}

static void toggle_theme(bool dark) {
    QString script = "/usr/share/extras/system-settings/themeswitcher/kde-theme-switch.sh";
    QString flag = dark ? "--dark" : "--light";
    QProcess::startDetached("bash", QStringList() << script << flag);
}

static void screen_resolution(const QString &desktop) {
    if (desktop == "xfce")
        QProcess::startDetached("bash", QStringList() << "-c" << "xfce4-display-settings");
    else if (desktop == "gnome")
        QProcess::startDetached("gnome-control-center", QStringList() << "display");
    else if (desktop == "kde") {
        QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
        env.remove("LD_LIBRARY_PATH");
        QProcess p;
        p.setProcessEnvironment(env);
        p.setProgram("kcmshell6");
        p.setArguments(QStringList() << "kcm_kscreen");
        p.startDetached();
    }
}

static void update_system(const QString &desktop) {
    if (desktop == "xfce")
        QProcess::startDetached("xfce4-terminal", QStringList() << "-x" << "pkexec" << "pacman" << "--noconfirm" << "-Syu");
    else if (desktop == "gnome")
        QProcess::startDetached("gnome-terminal", QStringList() << "--" << "sudo" << "pacman" << "--noconfirm" << "-Syu");
    else if (desktop == "kde") {
        QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
        env.remove("LD_LIBRARY_PATH");
        env.remove("QT_PLUGIN_PATH");
        env.remove("QT_QPA_PLATFORM_THEME");
        QProcess p;
        p.setProcessEnvironment(env);
        p.setProgram("konsole");
        p.setArguments(QStringList() << "-e" << "sudo" << "pacman" << "--noconfirm" << "-Syu");
        p.startDetached();
    }
}

// Run bash bin_install in background (no calamares "running" check per your changes)
static void run_bin_install() {
    QProcess *proc = new QProcess();
    proc->setWorkingDirectory(QDir::currentPath());
    QObject::connect(proc, &QProcess::finished, proc, [proc](int code) {
        if (code != 0)
            fprintf(stderr, "bin_install exit code: %d\n", code);
        proc->deleteLater();
    });
    proc->start("bash", QStringList() << "bin_install");
}

// --- About dialog (mirror utils/aboutus.py) ---

class AboutUsDialog : public QDialog {
public:
    explicit AboutUsDialog(QWidget *parent = nullptr) : QDialog(parent) {
        if (parent) setStyleSheet(parent->styleSheet());
        setWindowTitle("About Us");
        setFixedSize(400, 350);
        setModal(true);
        setAttribute(Qt::WA_TranslucentBackground);

        QVBoxLayout *layout = new QVBoxLayout(this);
        layout->setSpacing(10);
        layout->setContentsMargins(20, 20, 20, 20);

        QLabel *appName = new QLabel("<a href=\"https://pear-software.com\" style=\"text-decoration: none; color: inherit;\">Pear Software and Services S.R.L.</a>");
        QFont f = appName->font();
        f.setPointSize(18);
        f.setBold(true);
        appName->setFont(f);
        appName->setAlignment(Qt::AlignCenter);
        appName->setOpenExternalLinks(true);
        appName->setTextFormat(Qt::RichText);
        layout->addWidget(appName);

        QLabel *version = new QLabel("Version 25.12.10");
        version->setAlignment(Qt::AlignCenter);
        layout->addWidget(version);

        QLabel *desc = new QLabel("pearOS welcome was made to help you onboard quickly");
        desc->setWordWrap(true);
        desc->setAlignment(Qt::AlignJustify);
        layout->addWidget(desc);

        layout->addWidget(new QLabel("<b>Developers:</b>"));
        QLabel *dev = new QLabel("Alexandru Balan under Pear Software and Services S.R.L.");
        dev->setWordWrap(true);
        dev->setAlignment(Qt::AlignLeft);
        layout->addWidget(dev);

        QPushButton *website = new QPushButton("Visit our website");
        website->setFocusPolicy(Qt::NoFocus);
        connect(website, &QPushButton::clicked, this, []() { open_url("https://pear-software.com"); });
        layout->addWidget(website);

        QLabel *license = new QLabel("Distributed under the MIT License.");
        license->setWordWrap(true);
        license->setAlignment(Qt::AlignCenter);
        layout->addWidget(license);

        layout->addStretch();
    }
};

// --- Quick Update: intercept click on .../QuickUpdate, download zip, extract, chmod +x .sh, run ---

static const char QUICKUPDATE_URL[] = "https://new.pearos.xyz/QuickUpdate";
static const char QUICKUPDATE_ZIP[] = "https://new.pearos.xyz/QuickUpdate/update.zip";
static const char QUICKUPDATE_DIR[] = "/tmp/pearos-update";

class QuickUpdatePage : public QWebEnginePage {
    Q_OBJECT
public:
    explicit QuickUpdatePage(QObject *parent = nullptr) : QWebEnginePage(parent) {}

    bool acceptNavigationRequest(const QUrl &url, QWebEnginePage::NavigationType type, bool isMainFrame) override {
        if (!isMainFrame)
            return QWebEnginePage::acceptNavigationRequest(url, type, isMainFrame);
        QString u = url.toString();
        if (!u.startsWith(QLatin1String(QUICKUPDATE_URL)))
            return QWebEnginePage::acceptNavigationRequest(url, type, isMainFrame);

        // Intercept: download update.zip, extract, chmod +x *.sh, run first .sh
        QNetworkAccessManager *mgr = new QNetworkAccessManager(this);
        QNetworkReply *reply = mgr->get(QNetworkRequest(QUrl(QLatin1String(QUICKUPDATE_ZIP))));
        QObject::connect(reply, &QNetworkReply::finished, this, [this, reply, mgr]() {
            QObject::connect(reply, &QNetworkReply::destroyed, mgr, &QObject::deleteLater);
            if (reply->error() != QNetworkReply::NoError) {
                reply->deleteLater();
                return;
            }
            QDir().mkpath(QLatin1String(QUICKUPDATE_DIR));
            QFile out(QLatin1String(QUICKUPDATE_DIR) + QStringLiteral("/update.zip"));
            if (!out.open(QIODevice::WriteOnly)) {
                reply->deleteLater();
                return;
            }
            out.write(reply->readAll());
            out.close();
            reply->deleteLater();

            QProcess *unzipProc = new QProcess(this);
            unzipProc->setWorkingDirectory(QLatin1String(QUICKUPDATE_DIR));
            unzipProc->start(QStringLiteral("unzip"), QStringList() << QStringLiteral("-o")
                << QStringLiteral("/tmp/pearos-update/update.zip") << QStringLiteral("-d") << QLatin1String(QUICKUPDATE_DIR));
            QObject::connect(unzipProc, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, [unzipProc](int, QProcess::ExitStatus) {
                QDirIterator it(QLatin1String(QUICKUPDATE_DIR), QStringList() << QStringLiteral("*.sh"),
                    QDir::Files, QDirIterator::Subdirectories);
                QStringList shFiles;
                while (it.hasNext())
                    shFiles << it.next();
                for (const QString &f : shFiles)
                    QProcess::execute(QStringLiteral("chmod"), QStringList() << QStringLiteral("+x") << f);
                if (!shFiles.isEmpty())
                    QProcess::startDetached(QStringLiteral("bash"), QStringList() << shFiles.first());
                unzipProc->deleteLater();
            });
        });
        return false; // block navigation
    }
};

// --- What's New? window (webview, attached to main window) ---

class WhatsNewWindow : public QWidget {
    Q_OBJECT
public:
    explicit WhatsNewWindow(QWidget *parent = nullptr) : QWidget(parent, Qt::Window) {
        setWindowTitle(tr("What's New?"));
        setMinimumSize(420, 500);
        resize(420, 550);
        setAttribute(Qt::WA_DeleteOnClose, false);

        QVBoxLayout *layout = new QVBoxLayout(this);
        layout->setContentsMargins(0, 0, 0, 0);
        QWebEngineView *view = new QWebEngineView(this);
        view->setPage(new QuickUpdatePage(view));
        view->setUrl(QUrl(QStringLiteral("https://new.pearos.xyz/")));
        layout->addWidget(view);
    }
};

// --- Main window ---

static const int WHATSNEW_WIDTH = 420;
static const int WHATSNEW_GAP = 4;

class WelcomeWindow : public QMainWindow {
    Q_OBJECT
public:
    QString desktop_env;
    bool is_live_iso;
    QPushButton *install_button = nullptr;
    QPushButton *update_system_button = nullptr;
    QPushButton *update_mirrorlist_button = nullptr;
    QCheckBox *autostart_switch = nullptr;
    QCheckBox *theme_switch = nullptr;
    WhatsNewWindow *whatsNewWindow = nullptr;

    explicit WelcomeWindow(QWidget *parent = nullptr)
        : QMainWindow(parent)
        , desktop_env(get_desktop_environment())
        , is_live_iso(check_if_live_iso())
    {
        setWindowTitle("pearOS welcome");
        setFixedSize(WINDOW_WIDTH, WINDOW_HEIGHT);
        setAttribute(Qt::WA_TranslucentBackground);

        QString basePath = get_base_path();
        QString iconPath = basePath + "/assets/welcome.png";
        if (QFile::exists(iconPath))
            setWindowIcon(QIcon(iconPath));

        QScreen *screen = QApplication::primaryScreen();
        if (screen) {
            QRect geo = screen->geometry();
            move(geo.x() + (geo.width() - WINDOW_WIDTH) / 2,
                 geo.y() + (geo.height() - WINDOW_HEIGHT) / 2);
        }

        loadStylesheet(basePath);
        setupUi(basePath);
    }

    void repositionWhatsNew() {
        if (whatsNewWindow && whatsNewWindow->isVisible()) {
            whatsNewWindow->move(x() + width() + WHATSNEW_GAP, y());
            whatsNewWindow->resize(WHATSNEW_WIDTH, height());
        }
    }

    void openWhatsNew() {
        if (!is_online())
            return;
        if (!whatsNewWindow) {
            whatsNewWindow = new WhatsNewWindow(this);
            whatsNewWindow->setStyleSheet(styleSheet());
        }
        repositionWhatsNew();
        whatsNewWindow->move(x() + width() + WHATSNEW_GAP, y());
        whatsNewWindow->resize(WHATSNEW_WIDTH, height());
        whatsNewWindow->show();
        whatsNewWindow->raise();
        whatsNewWindow->activateWindow();
    }

    void moveEvent(QMoveEvent *event) override {
        QMainWindow::moveEvent(event);
        repositionWhatsNew();
    }

    void loadStylesheet(const QString &basePath) {
        QString qssPath = basePath + "/styles.qss";
        if (QFile::exists(qssPath)) {
            QFile f(qssPath);
            if (f.open(QIODevice::ReadOnly | QIODevice::Text))
                setStyleSheet(QString::fromUtf8(f.readAll()));
        }
    }

    void setupUi(const QString &basePath) {
        QWidget *central = new QWidget(this);
        central->setAttribute(Qt::WA_TranslucentBackground);
        central->setObjectName("centralwidget");
        setCentralWidget(central);

        QVBoxLayout *mainLayout = new QVBoxLayout(central);
        mainLayout->setSpacing(10);
        mainLayout->setContentsMargins(20, 10, 20, 10);

        // Header
        QHBoxLayout *headerLayout = new QHBoxLayout();
        headerLayout->setSpacing(10);
        QString logoPath = basePath + "/assets/welcome.png";
        if (QFile::exists(logoPath)) {
            QLabel *logoLabel = new QLabel();
            QPixmap pix(logoPath);
            logoLabel->setPixmap(pix.scaled(LOGO_SIZE, LOGO_SIZE, Qt::KeepAspectRatio, Qt::SmoothTransformation));
            headerLayout->addWidget(logoLabel);
        }
        QLabel *welcomeLabel = new QLabel("Welcome to pearOS NiceC0re 25.12");
        welcomeLabel->setObjectName("header");
        welcomeLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        headerLayout->addWidget(welcomeLabel, 1);
        mainLayout->addLayout(headerLayout);

        // Install & Setup
        mainLayout->addWidget(newSectionLabel("Install & Setup"));
        QGridLayout *grid1 = new QGridLayout();
        grid1->setSpacing(10);

        auto addButton = [this, basePath](const QString &label, const QString &iconName, bool fromFile, std::function<void()> slot) {
            QPushButton *btn = new QPushButton(label);
            btn->setFocusPolicy(Qt::NoFocus);
            if (fromFile) {
                QString path = basePath + "/" + iconName;
                if (QFile::exists(path)) {
                    btn->setIcon(QIcon(path));
                    btn->setIconSize(QSize(ICON_SIZE, ICON_SIZE));
                }
            } else {
                btn->setIcon(QIcon::fromTheme(iconName));
                btn->setIconSize(QSize(ICON_SIZE, ICON_SIZE));
            }
            btn->setLayoutDirection(Qt::RightToLeft);
            connect(btn, &QPushButton::clicked, this, [slot]() { slot(); });
            return btn;
        };

        int row = 0, col = 0;
        if (is_live_iso) {
            install_button = addButton("Install pearOS ", "system-software-install", false, [this]() {
                run_bin_install();
            });
            grid1->addWidget(install_button, row, col); if (++col == 2) { col = 0; row++; }
        }
        QPushButton *resBtn = addButton("Screen Resolution ", "video-display", false, [this]() { screen_resolution(desktop_env); });
        grid1->addWidget(resBtn, row, col); if (++col == 2) { col = 0; row++; }

        update_system_button = addButton("Update System ", "system-software-update", false, [this]() { update_system(desktop_env); });
        grid1->addWidget(update_system_button, row, col); if (++col == 2) { col = 0; row++; }

        update_mirrorlist_button = addButton("Update Mirrorlist ", "view-refresh", false, [this]() {
            QProcess::startDetached("pkexec", QStringList() << "pacman-mirrors" << "-f" << "5");
        });
        grid1->addWidget(update_mirrorlist_button, row, col); if (++col == 2) { col = 0; row++; }

        if (!is_live_iso) {
            QPushButton *yt = addButton("YouTube ", "assets/youtube.svg", true, []() { open_url("https://youtube.com/pearOS"); });
            grid1->addWidget(yt, row, col);
        }
        mainLayout->addLayout(grid1);

        // Donate
        mainLayout->addWidget(newSectionLabel("Donate"));
        QGridLayout *gridDonate = new QGridLayout();
        gridDonate->setSpacing(10);
        gridDonate->addWidget(addButton("PayPal", "assets/paypal.svg", true, []() { open_url("https://paypal.me/alxb421"); }), 0, 0);
        gridDonate->addWidget(addButton("GitHub Sponsors", "assets/github.svg", true, []() { open_url("https://github.com/sponsors/pearOS-archlinux?o=esb"); }), 0, 1);
        gridDonate->addWidget(addButton("10 eur/mo", "assets/patreon.png", true, []() { open_url("https://www.patreon.com/checkout/PearSoftwareandServices?rid=27197016"); }), 0, 2);
        mainLayout->addLayout(gridDonate);

        // Social Media
        mainLayout->addWidget(newSectionLabel("Social Media Links"));
        QGridLayout *gridSocial = new QGridLayout();
        gridSocial->setSpacing(10);
        gridSocial->addWidget(addButton("GitHub  ", "assets/github.svg", true, []() { open_url("https://github.com/pearOS-archlinux"); }), 0, 0);
        gridSocial->addWidget(addButton("Discord ", "assets/discord.svg", true, []() { open_url("https://discord.gg/pearos-697456171631509515"); }), 0, 1);
        mainLayout->addLayout(gridSocial);

        // More Options
        mainLayout->addWidget(newSectionLabel("More Options"));
        QHBoxLayout *hopt = new QHBoxLayout();
        hopt->setSpacing(10);
        QWidget *autostartWidget = new QWidget();
        QHBoxLayout *ah = new QHBoxLayout(autostartWidget);
        ah->setContentsMargins(0, 0, 0, 0);
        ah->setSpacing(5);
        ah->addWidget(new QLabel("AutoStart:"));
        autostart_switch = new QCheckBox();
        autostart_switch->setFocusPolicy(Qt::NoFocus);
        autostart_switch->setChecked(autostart_file_exists());
        connect(autostart_switch, &QCheckBox::toggled, this, [](bool checked) { toggle_autostart(checked); });
        ah->addStretch();
        ah->addWidget(autostart_switch);
        hopt->addWidget(autostartWidget);

        QWidget *themeWidget = new QWidget();
        QHBoxLayout *th = new QHBoxLayout(themeWidget);
        th->setContentsMargins(0, 0, 0, 0);
        th->setSpacing(5);
        th->addWidget(new QLabel("Dark Theme:"));
        theme_switch = new QCheckBox();
        theme_switch->setFocusPolicy(Qt::NoFocus);
        theme_switch->setChecked(is_dark_theme());
        connect(theme_switch, &QCheckBox::toggled, this, [this](bool checked) { toggle_theme(checked); });
        th->addStretch();
        th->addWidget(theme_switch);
        hopt->addWidget(themeWidget);
        mainLayout->addLayout(hopt);

        // What's New? and About Us
        QPushButton *whatsNewBtn = new QPushButton(tr("What's New?"));
        whatsNewBtn->setFocusPolicy(Qt::NoFocus);
        connect(whatsNewBtn, &QPushButton::clicked, this, [this]() { openWhatsNew(); });
        mainLayout->addWidget(whatsNewBtn);

        QPushButton *aboutBtn = new QPushButton("About Us");
        aboutBtn->setFocusPolicy(Qt::NoFocus);
        connect(aboutBtn, &QPushButton::clicked, this, [this]() {
            AboutUsDialog dlg(this);
            dlg.exec();
        });
        mainLayout->addWidget(aboutBtn);
    }

    QLabel *newSectionLabel(const QString &text) {
        QLabel *l = new QLabel(text);
        l->setAlignment(Qt::AlignCenter);
        return l;
    }
};

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    app.setApplicationName("Welcome to pearOS");

    if (is_electron_window_visible(get_base_path())) {
        return 0;  // do not start when a window with "electron" in title is open
    }

    WelcomeWindow w;
    w.show();
    QTimer::singleShot(0, &w, [&w]() { if (is_online()) w.openWhatsNew(); });

    return app.exec();
}

#include "welcome.moc"
