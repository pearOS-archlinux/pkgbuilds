#include "appearancemanager.h"
#include <QProcess>
#include <QDir>
#include <QFileInfoList>

AppearanceManager::AppearanceManager(QObject *parent) : QObject(parent) {}

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
    run("kreadconfig5 --group 'General' --key 'ColorScheme' 2>/dev/null", [this](QString out) {
        m_colorScheme = out.trimmed();
    });
    run("kreadconfig5 --group 'General' --key 'font' 2>/dev/null", [this](QString out) {
        // KDE font string: "Sans Serif,10,..."
        QString f = out.trimmed();
        QStringList parts = f.split(',');
        m_fontFamily = parts.value(0);
        m_fontSize   = parts.value(1, "10");
    });
    run("kreadconfig5 --group 'Icons' --key 'Theme' 2>/dev/null", [this](QString out) {
        m_iconTheme = out.trimmed();
    });

    // Enumerate color schemes
    run("ls /usr/share/color-schemes/*.colors ~/.local/share/color-schemes/*.colors 2>/dev/null", [this](QString out) {
        m_colorSchemes.clear();
        for (const QString &line : out.split('\n')) {
            QString f = line.trimmed();
            if (f.isEmpty()) continue;
            QVariantMap s; s["path"] = f; s["name"] = QFileInfo(f).baseName();
            m_colorSchemes.append(s);
        }
    });

    // Enumerate icon themes
    run("ls -d /usr/share/icons/*/ ~/.local/share/icons/*/ 2>/dev/null | xargs -I{} basename {}", [this](QString out) {
        m_iconThemes.clear();
        for (const QString &line : out.split('\n')) {
            QString n = line.trimmed();
            if (!n.isEmpty()) m_iconThemes.append(n);
        }
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

void AppearanceManager::setColorScheme(const QString &scheme) {
    m_colorScheme = scheme;
    run(QString("plasma-apply-colorscheme %1 2>/dev/null").arg(scheme), [this](QString) { emit appearanceChanged(); });
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
    run(QString("plasma-apply-lookandfeel --resetLayout 2>/dev/null; "
                "kwriteconfig5 --group Icons --key Theme '%1'").arg(theme), [this](QString) {
        run("qdbus org.kde.KWin /KWin reconfigure 2>/dev/null", [this](QString) { emit appearanceChanged(); });
    });
}

void AppearanceManager::writeLgKey(const QString &key, const QString &value) {
    QProcess proc;
    proc.start("kwriteconfig6", {"--file", "kwinrc", "--group", "Effect-blurplus", "--key", key, value});
    proc.waitForFinished(3000);
}

void AppearanceManager::reconfigureLiquidGel() {
    QProcess::startDetached("qdbus6", {
        "org.kde.KWin", "/Effects",
        "org.kde.kwin.Effects.reconfigureEffect", "forceblur_x11"
    });
}

void AppearanceManager::setLgEnabled(bool enabled) {
    m_lgEnabled = enabled;
    emit appearanceChanged();
    QProcess proc;
    proc.start("kwriteconfig6", {"--file", "kwinrc", "--group", "Plugins", "--key",
                                  "forceblurEnabled", enabled ? "true" : "false"});
    proc.waitForFinished(3000);
    reconfigureLiquidGel();
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
