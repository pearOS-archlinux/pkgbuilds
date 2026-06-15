#include "appearancemanager.h"
#include <QProcess>
#include <QDir>
#include <QFile>
#include <QMap>
#include <QTextStream>
#include <QFileInfo>

AppearanceManager::AppearanceManager(QObject *parent) : QObject(parent) {}

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
    // Dark/light mode from state file
    QString state = readFile(themeSwitcherDir() + "/state");
    if (state == "dark" || state == "light")
        m_colorScheme = state;
    else
        m_colorScheme = "auto";

    // Accent from accent file
    QString acc = readFile(themeSwitcherDir() + "/accent");
    if (!acc.isEmpty())
        m_accent = acc;

    run("kreadconfig5 --group 'General' --key 'font' 2>/dev/null", [this](QString out) {
        QString f = out.trimmed();
        QStringList parts = f.split(',');
        m_fontFamily = parts.value(0);
        m_fontSize   = parts.value(1, "10");
    });
    run("kreadconfig5 --group 'Icons' --key 'Theme' 2>/dev/null", [this](QString out) {
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

    // Liquid Gel settings
    run("kreadconfig6 --file kwinrc --group Plugins --key forceblurEnabled", [this](QString out) {
        m_lgEnabled = out.trimmed() == "true";
    });
    run("kreadconfig6 --file kwinrc --group Effect-blurplus --key BlurStrength", [this](QString out) {
        bool ok; int v = out.trimmed().toInt(&ok); if (ok) m_lgBlurStrength = v;
    });
    run("kreadconfig6 --file kwinrc --group Effect-blurplus --key NoiseStrength", [this](QString out) {
        bool ok; int v = out.trimmed().toInt(&ok); if (ok) m_lgNoiseStrength = v;
    });
    run("kreadconfig6 --file kwinrc --group Effect-blurplus --key RefractionStrength", [this](QString out) {
        bool ok; int v = out.trimmed().toInt(&ok); if (ok) m_lgRefractionStrength = v;
    });
    run("kreadconfig6 --file kwinrc --group Effect-blurplus --key RefractionEdgeSize", [this](QString out) {
        bool ok; int v = out.trimmed().toInt(&ok); if (ok) m_lgRefractionEdgeSize = v;
    });
    run("kreadconfig6 --file kwinrc --group Effect-blurplus --key RefractionRGBFringing", [this](QString out) {
        bool ok; int v = out.trimmed().toInt(&ok); if (ok) m_lgRGBFringing = v;
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

void AppearanceManager::setColorScheme(const QString &mode) {
    // "auto" — no system command (like Electron); just record locally
    if (mode == "auto") {
        m_colorScheme = "auto";
        emit appearanceChanged();
        applyIconThemeForAccent(m_accent, m_colorScheme);
        return;
    }
    if (mode != "dark" && mode != "light")
        return;
    m_colorScheme = mode;
    emit appearanceChanged();
    QString script = themeSwitcherScript();
    run(QString("bash \"%1\" --%2 2>/dev/null").arg(script, mode), [this](QString) {
        emit appearanceChanged();
        applyIconThemeForAccent(m_accent, m_colorScheme);
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
