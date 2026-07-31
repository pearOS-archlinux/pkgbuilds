// Ported from ../welcome.cpp -- see welcome.h's header comment for what was
// deliberately left out (is_electron_window_visible()/is_online(), the
// QuickUpdatePage webview flow). Everything else here is the same shell
// commands/QProcess calls as the real Qt6 app, unchanged.

#include "welcome.h"

#include <QCoreApplication>
#include <QDesktopServices>
#include <QUrl>
#include <QFile>
#include <QDir>
#include <QProcess>
#include <QProcessEnvironment>
#include <QStandardPaths>
#include <QDateTime>

static const char INSTALL_PREFIX[] = "/usr/share/pearos-welcome";

/** Read VERSION or IMAGE_VERSION from /etc/os-release. */
QString get_os_version() {
    QFile f(QStringLiteral("/etc/os-release"));
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text))
        return QString();
    QString version, imageVersion;
    while (!f.atEnd()) {
        QByteArray line = f.readLine().trimmed();
        QByteArray val;
        if (line.startsWith("VERSION="))
            val = line.mid(8);
        else if (line.startsWith("IMAGE_VERSION="))
            val = line.mid(14);
        if (!val.isEmpty()) {
            if (val.startsWith('"') && val.endsWith('"'))
                val = val.mid(1, val.size() - 2);
            QString v = QString::fromUtf8(val);
            if (line.startsWith("VERSION="))
                version = v;
            else
                imageVersion = v;
        }
    }
    return !version.isEmpty() ? version : imageVersion;
}

/** Base path for assets/styles: next to executable, or INSTALL_PREFIX when installed. */
QString get_base_path() {
    QString base = QCoreApplication::applicationDirPath();
    if (QFile::exists(base + "/styles.qss"))
        return base;
    if (QFile::exists(QLatin1String(INSTALL_PREFIX) + "/styles.qss"))
        return QLatin1String(INSTALL_PREFIX);
    // Dev fallback: run straight from qt5/build/ without an install step,
    // same directory-resolution trick as MainWindow's own assetsBasePath_.
    if (QFile::exists(base + "/../styles.qss"))
        return base + "/..";
    return base;
}

QString get_desktop_environment() {
    QString de = qEnvironmentVariable("XDG_CURRENT_DESKTOP", "");
    return de.toLower();
}

bool check_if_live_iso() {
    return QFile::exists("/run/archiso");
}

void open_url(const QString &url) {
    if (!QDesktopServices::openUrl(QUrl(url))) {
        QProcess::startDetached("xdg-open", QStringList() << url);
    }
}

bool autostart_file_exists() {
    QString path = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation)
        + "/autostart/welcome.desktop";
    return QFile::exists(path);
}

void toggle_autostart(bool enable) {
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
        QFile::remove(autostartFile);
    }
}

void screen_resolution(const QString &) {
    QProcess::startDetached(QStringLiteral("systemsettings"),
        QStringList() << QStringLiteral("kcm_kscreen"));
}

static QString bash_single_quoted(const QString &s) {
    QString r = s;
    r.replace(QLatin1Char('\''), QLatin1String("'\\''"));
    return QLatin1Char('\'') + r + QLatin1Char('\'');
}

static QString escape_for_sh_double_quotes(const QString &s) {
    QString e;
    for (QChar c : s) {
        if (c == QLatin1Char('\\') || c == QLatin1Char('"') || c == QLatin1Char('$') || c == QLatin1Char('`'))
            e += QLatin1Char('\\');
        e += c;
    }
    return e;
}

/** Shell command: pacman-key setup (root), updater.sh, reset plasma appletsrc, log cp, then full upgrade.
 *  @param alsoResyncLiquidGel dacă true, un singur pkexec pentru -Syu și pearos-liquidgel (mai puține prompturi). */
static QString update_system_prep_and_upgrade_cmd(bool alsoResyncLiquidGel = false) {
    const QString gpgPath = get_base_path() + QStringLiteral("/assets/pearos.gpg");
    const QString updaterPath = get_base_path() + QStringLiteral("/updater.sh");
    const QString logPath = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation)
        + QStringLiteral("/update-")
        + QDateTime::currentDateTime().toString(QStringLiteral("yyyyMMdd-HHmmss"))
        + QStringLiteral(".log");

    const QString inner = QStringLiteral("pacman-key --init && pacman-key --add %1 && pacman-key --lsign-key 4C1A9F3C131ACA95 && pacman-key --populate")
        .arg(bash_single_quoted(gpgPath));
    const QString rootKeyBlock = QStringLiteral("pkexec sh -c \"%1\"").arg(escape_for_sh_double_quotes(inner));

    const QString pacmanTail = alsoResyncLiquidGel
        ? QStringLiteral("pkexec sh -c \"%1\"").arg(escape_for_sh_double_quotes(
            QStringLiteral("pacman --noconfirm -Syu && pacman -Sy pearos-liquidgel --noconfirm")))
        : QStringLiteral("pkexec pacman --noconfirm -Syu");

    return QStringLiteral("%1 && bash %2 && rm -rf \"$HOME/.config/plasma-org.kde.plasma.desktop-appletsrc\" && "
        "cp -r /etc/skel/.config/plasma-org.kde.plasma.desktop-appletsrc \"$HOME/.config/plasma-org.kde.plasma.desktop-appletsrc\" >> %3 && "
        "%4")
        .arg(rootKeyBlock)
        .arg(bash_single_quoted(updaterPath))
        .arg(bash_single_quoted(logPath))
        .arg(pacmanTail);
}

static void run_shell_in_desktop_terminal(const QString &desktop, const QString &cmd) {
    if (desktop == QLatin1String("xfce"))
        QProcess::startDetached(QStringLiteral("xfce4-terminal"), QStringList()
            << QStringLiteral("-x") << QStringLiteral("bash") << QStringLiteral("-lc") << cmd);
    else if (desktop == QLatin1String("gnome"))
        QProcess::startDetached(QStringLiteral("gnome-terminal"), QStringList()
            << QStringLiteral("--") << QStringLiteral("bash") << QStringLiteral("-lc") << cmd);
    else if (desktop == QLatin1String("kde")) {
        QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
        env.remove(QStringLiteral("LD_LIBRARY_PATH"));
        env.remove(QStringLiteral("QT_PLUGIN_PATH"));
        env.remove(QStringLiteral("QT_QPA_PLATFORM_THEME"));
        QProcess p;
        p.setProcessEnvironment(env);
        p.setProgram(QStringLiteral("konsole"));
        p.setArguments(QStringList() << QStringLiteral("-e") << QStringLiteral("bash") << QStringLiteral("-lc") << cmd);
        p.startDetached();
    }
}

void update_system(const QString &desktop) {
    run_shell_in_desktop_terminal(desktop, update_system_prep_and_upgrade_cmd());
}

/** Același flux ca Update System, apoi pearos-liquidgel în același pkexec cu -Syu (o singură parolă în plus față de chei). */
void fix_liquid_gel_after_upgrade(const QString &desktop) {
    run_shell_in_desktop_terminal(desktop, update_system_prep_and_upgrade_cmd(true));
}

/** Reset pacman.d/gnupg + re-populate + re-sign the archive key, all inside
 *  one pkexec sh -c so it's a single root prompt, not one per command. */
void fix_pacman_keys(const QString &desktop) {
    const QString inner = QStringLiteral(
        "rm -rf /etc/pacman.d/gnupg && "
        "pacman-key --init && "
        "pacman-key --populate && "
        "pacman-key -a /root/archive.key && "
        "pacman-key --lsign-key 4C1A9F3C131ACA95 && "
        "pacman -Syy");
    const QString cmd = QStringLiteral("pkexec sh -c \"%1\"").arg(escape_for_sh_double_quotes(inner));
    run_shell_in_desktop_terminal(desktop, cmd);
}

// Run bash bin_install in background (no calamares "running" check per real Welcome's changes)
bool fix_layout(QStringList *failed) {
    const QString skel = QStringLiteral("/etc/skel/.config");
    const QString dest = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation);

    static const char *plainFiles[] = {
        "plasma-org.kde.plasma.desktop-appletsrc",
        "plasmarc",
        "plasmashellrc",
        "kdeglobals",
        "gtkrc",
        "gtkrc-2.0",
        "Trolltech.conf",
        nullptr
    };

    static const char *dirs[] = {
        "gtk-2.0",
        "gtk-3.0",
        "gtk-4.0",
        nullptr
    };

    bool allOk = true;

    for (int i = 0; plainFiles[i]; ++i) {
        QString src = skel + QLatin1Char('/') + plainFiles[i];
        QString dst = dest + QLatin1Char('/') + plainFiles[i];
        if (!QFile::exists(src))
            continue;
        if (QFile::exists(dst) && !QFile::remove(dst)) {
            allOk = false;
            if (failed) *failed << QLatin1String(plainFiles[i]);
            continue;
        }
        if (!QFile::copy(src, dst)) {
            allOk = false;
            if (failed) *failed << QLatin1String(plainFiles[i]);
        }
    }

    for (int i = 0; dirs[i]; ++i) {
        QString src = skel + QLatin1Char('/') + dirs[i];
        QString dst = dest + QLatin1Char('/') + dirs[i];
        if (!QDir(src).exists())
            continue;
        QDir(dst).removeRecursively();
        QProcess proc;
        proc.start(QStringLiteral("cp"), QStringList() << QStringLiteral("-r") << src << dst);
        proc.waitForFinished(10000);
        if (proc.exitCode() != 0 || !QDir(dst).exists()) {
            allOk = false;
            if (failed) *failed << QLatin1String(dirs[i]);
        }
    }

    return allOk;
}

void run_bin_install() {
    QProcess *proc = new QProcess();
    proc->setWorkingDirectory(QDir::currentPath());
    QObject::connect(proc, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), proc,
        [proc](int code, QProcess::ExitStatus) {
            if (code != 0)
                fprintf(stderr, "bin_install exit code: %d\n", code);
            proc->deleteLater();
        });
    QObject::connect(proc, &QProcess::errorOccurred, proc, [proc](QProcess::ProcessError) {
        proc->deleteLater();
    });
    proc->start("bash", QStringList() << "bin_install");
}
