#include "appearancemanager.h"
#include <QProcess>
#include <QDir>
#include <QFile>
#include <QMap>
#include <QTextStream>
#include <QFileInfo>
#include <QRegularExpression>
#include <QTime>

AppearanceManager::AppearanceManager(QObject *parent) : QObject(parent) {}

static QString systemdUserDir() {
    return QDir::homePath() + "/.config/systemd/user";
}

bool AppearanceManager::autoThemeEnabled() {
    QProcess p;
    p.start("systemctl", {"--user", "is-enabled", "pearos-theme-light.timer"});
    p.waitForFinished(3000);
    return p.exitCode() == 0;
}

void AppearanceManager::enableAutoTheme() {
    QString script = themeSwitcherScript();
    QString dir = systemdUserDir();
    QDir().mkpath(dir);

    auto writeUnit = [](const QString &path, const QString &content) {
        QFile f(path);
        if (f.open(QIODevice::WriteOnly | QIODevice::Text))
            QTextStream(&f) << content;
    };

    writeUnit(dir + "/pearos-theme-light.service",
        "[Unit]\nDescription=PearOS Auto Theme - Light\n\n"
        "[Service]\nType=oneshot\nExecStart=" + script + " --light\n");

    writeUnit(dir + "/pearos-theme-light.timer",
        "[Unit]\nDescription=PearOS Auto Theme - Light at 08:00\n\n"
        "[Timer]\nOnCalendar=*-*-* 08:00:00\nPersistent=true\n\n"
        "[Install]\nWantedBy=timers.target\n");

    writeUnit(dir + "/pearos-theme-dark.service",
        "[Unit]\nDescription=PearOS Auto Theme - Dark\n\n"
        "[Service]\nType=oneshot\nExecStart=" + script + " --dark\n");

    writeUnit(dir + "/pearos-theme-dark.timer",
        "[Unit]\nDescription=PearOS Auto Theme - Dark at 18:00\n\n"
        "[Timer]\nOnCalendar=*-*-* 18:00:00\nPersistent=true\n\n"
        "[Install]\nWantedBy=timers.target\n");

    run("systemctl --user daemon-reload && "
        "systemctl --user enable --now pearos-theme-light.timer pearos-theme-dark.timer",
        [](QString) {});

    // Apply appropriate theme immediately based on current hour
    QString immediateMode = (QTime::currentTime().hour() >= 8 && QTime::currentTime().hour() < 18)
                            ? "light" : "dark";
    run(QString("bash \"%1\" --%2 2>/dev/null").arg(script, immediateMode),
        [this, immediateMode](QString) {
            applyIconThemeForAccent(m_accent, immediateMode);
            maybeUpdateWallpaper(immediateMode);
        });

    m_colorScheme = "auto";
    emit appearanceChanged();
}

void AppearanceManager::disableAutoTheme() {
    run("systemctl --user disable --now pearos-theme-light.timer pearos-theme-dark.timer 2>/dev/null || true;"
        " systemctl --user daemon-reload 2>/dev/null || true",
        [](QString) {});

    QString dir = systemdUserDir();
    const QStringList units = {
        "pearos-theme-light.service", "pearos-theme-light.timer",
        "pearos-theme-dark.service",  "pearos-theme-dark.timer"
    };
    for (const QString &u : units)
        QFile::remove(dir + "/" + u);
}

QString AppearanceManager::themeSwitcherDir() {
    static const QStringList candidates = {
        "/usr/share/extras/system-settings/themeswitcher",
        "/usr/share/extras/pearos-themesw",
    };
    for (const QString &d : candidates) {
        if (QDir(d).exists())
            return d;
    }
    return candidates.first();
}

QString AppearanceManager::themeSwitcherScript() {
    return themeSwitcherDir() + "/kde-theme-switch.sh";
}

static QString readFile(const QString &path) {
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text))
        return {};
    return QTextStream(&f).readAll().trimmed();
}

void AppearanceManager::run(const QString &cmd, std::function<void(QString)> cb) {
    auto *proc = new QProcess(this);
    connect(proc, &QProcess::finished, this, [proc, cb](int, QProcess::ExitStatus) {
        cb(QString::fromUtf8(proc->readAllStandardOutput()));
        proc->deleteLater();
    });
    connect(proc, &QProcess::errorOccurred, proc, &QProcess::deleteLater);
    proc->start("bash", {"-c", cmd});
}

void AppearanceManager::refresh() {
    // Auto mode is canonical if systemd timers are installed
    if (autoThemeEnabled()) {
        m_colorScheme = "auto";
    } else {
        QString state = readFile(themeSwitcherDir() + "/state");
        m_colorScheme = (state == "dark" || state == "light") ? state : "light";
    }

    // Accent from accent file
    QString acc = readFile(themeSwitcherDir() + "/accent");
    if (!acc.isEmpty())
        m_accent = acc;

    run("kreadconfig6 --file kdeglobals --group 'General' --key 'font' 2>/dev/null", [this](QString out) {
        QString f = out.trimmed();
        QStringList parts = f.split(',');
        m_fontFamily = parts.value(0);
        m_fontSize   = parts.value(1, "10");
    });
    run("kreadconfig6 --file kdeglobals --group 'Icons' --key 'Theme' 2>/dev/null", [this](QString out) {
        m_iconTheme = out.trimmed();
    });

    run("ls /usr/share/color-schemes/*.colors ~/.local/share/color-schemes/*.colors 2>/dev/null", [this](QString out) {
        m_colorSchemes.clear();
        for (const QString &line : out.split('\n')) {
            QString f = line.trimmed();
            if (f.isEmpty()) continue;
            QVariantMap s; s["path"] = f; s["name"] = QFileInfo(f).baseName();
            m_colorSchemes.append(s);
        }
    });

    run("ls -d /usr/share/icons/*/ ~/.local/share/icons/*/ 2>/dev/null | xargs -I{} basename {}", [this](QString out) {
        m_iconThemes.clear();
        for (const QString &line : out.split('\n')) {
            QString n = line.trimmed();
            if (!n.isEmpty()) m_iconThemes.append(n);
        }
        emit appearanceChanged();
    });

    // Wallpaper tint
    run("kreadconfig6 --file pearos-settingsrc --group Appearance --key TintEnabled --default false",
        [this](QString out) {
            m_tintEnabled = out.trimmed() == "true";
            emit appearanceChanged();
        });

    // Liquid Gel settings — read all in one shot to avoid multiple incomplete emits
    run("kreadconfig6 --file kwinrc --group Plugins --key forceblurEnabled && "
        "kreadconfig6 --file kwinrc --group Effect-blurplus --key BlurStrength && "
        "kreadconfig6 --file kwinrc --group Effect-blurplus --key NoiseStrength && "
        "kreadconfig6 --file kwinrc --group Effect-blurplus --key RefractionStrength && "
        "kreadconfig6 --file kwinrc --group Effect-blurplus --key RefractionEdgeSize && "
        "kreadconfig6 --file kwinrc --group Effect-blurplus --key RefractionRGBFringing",
        [this](QString out) {
            QStringList vals = out.trimmed().split('\n');
            if (vals.size() >= 1) m_lgEnabled = vals[0].trimmed() == "true";
            bool ok;
            if (vals.size() >= 2) { int v = vals[1].trimmed().toInt(&ok); if (ok) m_lgBlurStrength = v; }
            if (vals.size() >= 3) { int v = vals[2].trimmed().toInt(&ok); if (ok) m_lgNoiseStrength = v; }
            if (vals.size() >= 4) { int v = vals[3].trimmed().toInt(&ok); if (ok) m_lgRefractionStrength = v; }
            if (vals.size() >= 5) { int v = vals[4].trimmed().toInt(&ok); if (ok) m_lgRefractionEdgeSize = v; }
            if (vals.size() >= 6) { int v = vals[5].trimmed().toInt(&ok); if (ok) m_lgRGBFringing = v; }
            emit appearanceChanged();
        });
}

void AppearanceManager::applyIconThemeForAccent(const QString &accent, const QString &colorScheme) {
    // Map accent preset names to available pearOS icon theme variants
    static const QMap<QString, QString> accentToIcon = {
        {"blue",        "blue"},
        {"purple",      "purple"},
        {"lila",        "purple"},
        {"dark-purple", "purple"},
        {"orange",      "orange"},
        {"yellow",      "yellow"},
        {"green",       "green"},
        {"grey",        "grey"},
        {"azul",        "blue"},
        {"magenta",     "red"},
    };

    QString iconAccent = accentToIcon.value(accent, "blue");
    QString suffix = (colorScheme == "dark") ? "-dark" : (colorScheme == "light") ? "-light" : "";
    QString theme = "pearOS-" + iconAccent + suffix;

    // Fall back to base variant if the suffixed theme doesn't exist
    if (!QDir("/usr/share/icons/" + theme).exists())
        theme = "pearOS-" + iconAccent;
    // Fall back to pearOS base if accent variant doesn't exist either
    if (!QDir("/usr/share/icons/" + theme).exists())
        theme = "pearOS" + suffix;

    m_iconTheme = theme;
    emit appearanceChanged();

    // plasma-changeicons is the KDE 6 tool that writes the theme to kdeglobals
    // AND notifies running apps via the proper KF6 mechanism (no plasmashell restart)
    run(QString("/usr/lib/plasma-changeicons '%1' 2>/dev/null || "
                "kwriteconfig6 --file kdeglobals --group Icons --key Theme '%1' 2>/dev/null")
            .arg(theme), [](QString) {});
}

void AppearanceManager::maybeUpdateWallpaper(const QString &mode) {
    // Read the actual current wallpaper from plasma desktop config
    run("grep '^Image=' ~/.config/plasma-org.kde.plasma.desktop-appletsrc 2>/dev/null | head -1",
        [mode](QString out) {
            QString img = out.trimmed().remove(QRegularExpression("^Image=file://")).remove(QRegularExpression("^Image="));
            QString base = QFileInfo(img).fileName();
            if (base != "dark-mode.jpg" && base != "light-mode.jpg")
                return;
            QString target = (mode == "dark")
                ? "/usr/share/extras/wallpapers/Default/dark-mode.jpg"
                : "/usr/share/extras/wallpapers/Default/light-mode.jpg";
            QProcess::startDetached("plasma-apply-wallpaperimage", {target});
        });
}

void AppearanceManager::setColorScheme(const QString &mode) {
    if (mode == "auto") {
        enableAutoTheme();
        return;
    }
    if (mode != "dark" && mode != "light")
        return;

    disableAutoTheme();

    m_colorScheme = mode;
    emit appearanceChanged();
    QString script = themeSwitcherScript();
    run(QString("bash \"%1\" --%2 2>/dev/null").arg(script, mode), [this, mode](QString) {
        emit appearanceChanged();
        applyIconThemeForAccent(m_accent, m_colorScheme);
        maybeUpdateWallpaper(mode);
    });
}

void AppearanceManager::setAccent(const QString &accentName) {
    static const QMap<QString, QString> accentHex = {
        {"purple",      "#8B5CF6"}, {"magenta",     "#EC4899"},
        {"orange",      "#F97316"}, {"yellow",      "#EAB308"},
        {"green",       "#22C55E"}, {"azul",        "#06B6D4"},
        {"blue",        "#3B82F6"}, {"lila",        "#A855F7"},
        {"dark-purple", "#6B21A8"}, {"grey",        "#6B7280"},
    };
    m_accent = accentName;
    emit appearanceChanged();

    QString hex = accentHex.value(accentName, "#3B82F6");
    bool ok;
    uint argb = 0xFF000000u | hex.mid(1).toUInt(&ok, 16);

    // Set accent via DBus — no plasmashell restart needed
    run(QString("qdbus6 org.kde.plasmashell.accentColor /AccentColor"
                " org.kde.plasmashell.accentColor.setAccentColor %1 2>/dev/null")
            .arg(argb), [](QString) {});

    // Persist accent name for themeswitcher state file
    run(QString("echo '%1' > '%2/accent' 2>/dev/null")
            .arg(accentName, themeSwitcherDir()), [](QString) {});

    // Reload KWin decorations + notify Qt apps — no restart
    run("qdbus6 org.kde.KWin /KWin reconfigure 2>/dev/null;"
        " qdbus6 org.kde.KGlobalSettings /KGlobalSettings"
        " org.kde.KGlobalSettings.notifyChange 0 0 2>/dev/null || true",
        [this, accentName](QString) {
            emit appearanceChanged();
            applyIconThemeForAccent(accentName, m_colorScheme);
        });
}

void AppearanceManager::setFontSize(const QString &size) {
    m_fontSize = size;
    run(QString("kwriteconfig5 --group 'General' --key 'font' '%1,%2,-1,5,50,0,0,0,0,0'")
            .arg(m_fontFamily, size), [this](QString) {
        run("qdbus org.kde.KWin /KWin reconfigure 2>/dev/null", [this](QString) { emit appearanceChanged(); });
    });
}

void AppearanceManager::setFontFamily(const QString &family) {
    m_fontFamily = family;
    run(QString("kwriteconfig5 --group 'General' --key 'font' '%1,%2,-1,5,50,0,0,0,0,0'")
            .arg(family, m_fontSize), [this](QString) {
        run("qdbus org.kde.KWin /KWin reconfigure 2>/dev/null", [this](QString) { emit appearanceChanged(); });
    });
}

void AppearanceManager::setIconTheme(const QString &theme) {
    m_iconTheme = theme;
    emit appearanceChanged();
    run(QString("/usr/lib/plasma-changeicons '%1' 2>/dev/null || "
                "kwriteconfig6 --file kdeglobals --group Icons --key Theme '%1' 2>/dev/null")
            .arg(theme), [](QString) {});
}

void AppearanceManager::setTintEnabled(bool enabled) {
    m_tintEnabled = enabled;
    emit appearanceChanged();
    run(QString("kwriteconfig6 --file pearos-settingsrc --group Appearance --key TintEnabled %1")
            .arg(enabled ? "true" : "false"), [](QString) {});
}

void AppearanceManager::writeLgKey(const QString &key, const QString &value) {
    QProcess proc;
    proc.start("kwriteconfig6", {"--file", "kwinrc", "--group", "Effect-blurplus", "--key", key, value});
    proc.waitForFinished(3000);
}

void AppearanceManager::reconfigureLiquidGel() {
    QProcess::startDetached("qdbus6", {
        "org.kde.KWin", "/Effects",
        "org.kde.kwin.Effects.reconfigureEffect", "forceblur"
    });
}

void AppearanceManager::setLgEnabled(bool enabled) {
    m_lgEnabled = enabled;
    emit appearanceChanged();
    QProcess proc;
    proc.start("kwriteconfig6", {"--file", "kwinrc", "--group", "Plugins", "--key",
                                  "forceblurEnabled", enabled ? "true" : "false"});
    proc.waitForFinished(3000);
    // Load/unload the effect immediately without requiring a Plasma restart
    QProcess::startDetached("qdbus6", {
        "org.kde.KWin", "/Effects",
        enabled ? "org.kde.kwin.Effects.loadEffect" : "org.kde.kwin.Effects.unloadEffect",
        "forceblur"
    });
}

void AppearanceManager::setLgBlurStrength(int v) {
    m_lgBlurStrength = v; emit appearanceChanged();
    writeLgKey("BlurStrength", QString::number(v));
    reconfigureLiquidGel();
}

void AppearanceManager::setLgNoiseStrength(int v) {
    m_lgNoiseStrength = v; emit appearanceChanged();
    writeLgKey("NoiseStrength", QString::number(v));
    reconfigureLiquidGel();
}

void AppearanceManager::setLgRefractionStrength(int v) {
    m_lgRefractionStrength = v; emit appearanceChanged();
    writeLgKey("RefractionStrength", QString::number(v));
    reconfigureLiquidGel();
}

void AppearanceManager::setLgRefractionEdgeSize(int v) {
    m_lgRefractionEdgeSize = v; emit appearanceChanged();
    writeLgKey("RefractionEdgeSize", QString::number(v));
    reconfigureLiquidGel();
}

void AppearanceManager::setLgRGBFringing(int v) {
    m_lgRGBFringing = v; emit appearanceChanged();
    writeLgKey("RefractionRGBFringing", QString::number(v));
    reconfigureLiquidGel();
}
