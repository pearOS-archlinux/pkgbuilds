#include "dockmanager.h"
#include <QProcess>
#include <QFile>
#include <QDir>
#include <QTextStream>
#include <QRegularExpression>

DockManager::DockManager(QObject *parent) : QObject(parent) {
    // Scan available skins
    QDir skinsDir("/home/alxb421/Desktop/pkgbuilds/pearos-dock/package/contents/skins");
    if (!skinsDir.exists())
        skinsDir.setPath("/usr/share/plasma/plasmoids/PearDock/contents/skins");
    for (const QFileInfo &fi : skinsDir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot))
        m_availableSkins.append(fi.fileName());
    if (m_availableSkins.isEmpty())
        m_availableSkins = {"Tahoe Dark", "Tahoe", "Big Sur Light", "Big Sur Night"};
}

void DockManager::run(const QString &cmd, std::function<void(QString)> cb) {
    auto *proc = new QProcess(this);
    connect(proc, &QProcess::finished, this, [proc, cb](int, QProcess::ExitStatus) {
        cb(QString::fromUtf8(proc->readAllStandardOutput()).trimmed());
        proc->deleteLater();
    });
    connect(proc, &QProcess::errorOccurred, proc, &QProcess::deleteLater);
    proc->start("bash", {"-c", cmd});
}

// Find the [Containments][N][Applets][M] section whose plugin=PearDock
void DockManager::findGroupPath() {
    if (!m_groupPath.isEmpty()) return;

    QString cfgPath = QDir::homePath() + "/.config/plasma-org.kde.plasma.desktop-appletsrc";
    QFile f(cfgPath);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) return;

    static QRegularExpression sectionRe(R"(^\[Containments\]\[(\d+)\]\[Applets\]\[(\d+)\]$)");
    QString containment, applet;
    QString curSection;

    QTextStream in(&f);
    while (!in.atEnd()) {
        QString line = in.readLine();
        auto m = sectionRe.match(line);
        if (m.hasMatch()) {
            containment = m.captured(1);
            applet      = m.captured(2);
            curSection  = line;
        } else if (!containment.isEmpty() && line.trimmed() == "plugin=PearDock") {
            m_groupPath = QString("Containments/%1/Applets/%2/Configuration/General")
                              .arg(containment, applet);
            break;
        }
    }
}

static QStringList groupArgs(const QString &groupPath) {
    QStringList args;
    args << "--file" << "plasma-org.kde.plasma.desktop-appletsrc";
    for (const QString &g : groupPath.split('/'))
        args << "--group" << g;
    return args;
}

QString DockManager::readKey(const QString &key, const QString &defaultVal) const {
    if (m_groupPath.isEmpty()) return defaultVal;
    QStringList args = groupArgs(m_groupPath);
    args << "--key" << key;
    QProcess proc;
    proc.start("kreadconfig6", args);
    proc.waitForFinished(2000);
    QString out = QString::fromUtf8(proc.readAllStandardOutput()).trimmed();
    return out.isEmpty() ? defaultVal : out;
}

void DockManager::writeKey(const QString &key, const QString &value) const {
    if (m_groupPath.isEmpty()) {
        fprintf(stderr, "[DockManager] writeKey: groupPath empty, key=%s\n", qPrintable(key));
        return;
    }
    QStringList args = groupArgs(m_groupPath);
    args << "--key" << key << value;
    QProcess proc;
    proc.start("kwriteconfig6", args);
    proc.waitForFinished(3000);
    if (proc.exitCode() != 0)
        fprintf(stderr, "[DockManager] kwriteconfig6 failed: %s\n",
                qPrintable(proc.readAllStandardError()));
}

void DockManager::notifyPlasmashell(const QString &key, const QString &value) const {
    QString escaped = value;
    escaped.replace("\\", "\\\\").replace("'", "\\'");

    QString script = QString(
        "var allPanels = panels();"
        "for (var i = 0; i < allPanels.length; i++) {"
        "  var ws = allPanels[i].widgets();"
        "  for (var j = 0; j < ws.length; j++) {"
        "    if (ws[j].type === 'PearDock') {"
        "      ws[j].currentConfigGroup = ['General'];"
        "      ws[j].writeConfig('%1', '%2');"
        "      ws[j].reloadConfig();"
        "    }"
        "  }"
        "}").arg(key, escaped);

    QProcess::startDetached("qdbus6", {
        "org.kde.plasmashell",
        "/PlasmaShell",
        "org.kde.PlasmaShell.evaluateScript",
        script
    });
}

void DockManager::refresh() {
    findGroupPath();

    m_skinName      = readKey("skinName",    "Tahoe Dark");
    m_iconSize      = readKey("iconSize",    "50").toInt();
    m_magnification = readKey("magnification", "80").toDouble();
    m_amplitud      = readKey("amplitud",    "1.5").toDouble();
    m_iconSpacing   = readKey("iconSpacing", "1").toInt();
    m_showReflection = readKey("showReflection", "false") == "true";

    m_skinBlur              = readKey("skinBlur",              "true")  == "true";
    m_skinBlurRadius        = readKey("skinBlurRadius",        "24").toInt();
    m_skinLiquidGelEffect   = readKey("skinLiquidGelEffect",   "true")  == "true";
    m_skinRefractionStrength = readKey("skinRefractionStrength","8").toDouble();
    m_skinRgbFringing       = readKey("skinRgbFringing",       "2").toDouble();
    m_skinPositionTaskIndicator = readKey("skinPositionTaskIndicator", "3").toInt();

    m_showToolTips              = readKey("showToolTips",              "true")  == "true";
    m_highlightWindows          = readKey("highlightWindows",          "false") == "true";
    m_indicateAudioStreams      = readKey("indicateAudioStreams",      "true")  == "true";
    m_interactiveMute           = readKey("interactiveMute",           "true")  == "true";
    m_tooltipControls           = readKey("tooltipControls",           "true")  == "true";
    m_minimizeActiveTaskOnClick = readKey("minimizeActiveTaskOnClick", "true")  == "true";
    m_middleClickAction         = readKey("middleClickAction",         "2").toInt();
    m_wheelEnabled              = readKey("wheelEnabled",              "0").toInt();
    m_wheelSkipMinimized        = readKey("wheelSkipMinimized",        "true")  == "true";
    m_groupingStrategy          = readKey("groupingStrategy",          "1").toInt();
    m_groupedTaskVisualization  = readKey("groupedTaskVisualization",  "0").toInt();
    m_sortingStrategy           = readKey("sortingStrategy",           "1").toInt();

    m_skinLeftMargin          = readKey("skinLeftMargin",          "20").toInt();
    m_skinTopMargin           = readKey("skinTopMargin",           "20").toInt();
    m_skinRightMargin         = readKey("skinRightMargin",         "20").toInt();
    m_skinBottomMargin        = readKey("skinBottomMargin",        "20").toInt();
    m_skinOutsideLeftMargin   = readKey("skinOutsideLeftMargin",   "20").toInt();
    m_skinOutsideTopMargin    = readKey("skinOutsideTopMargin",    "0").toInt();
    m_skinOutsideRightMargin  = readKey("skinOutsideRightMargin",  "20").toInt();
    m_skinOutsideBottomMargin = readKey("skinOutsideBottomMargin", "-10").toInt();

    m_showOnlyCurrentDesktop  = readKey("showOnlyCurrentDesktop",  "true")  == "true";
    m_showOnlyCurrentActivity = readKey("showOnlyCurrentActivity", "true")  == "true";
    m_showOnlyCurrentScreen   = readKey("showOnlyCurrentScreen",   "false") == "true";
    m_showOnlyMinimized       = readKey("showOnlyMinimized",       "false") == "true";
    m_unhideOnAttention       = readKey("unhideOnAttention",       "true")  == "true";

    emit changed();
}

void DockManager::set(const QString &key, const QVariant &value) {
    findGroupPath();

    // Update in-memory state immediately for responsive UI
    if (key == "skinName")     { m_skinName     = value.toString();  }
    if (key == "iconSize")     { m_iconSize      = value.toInt();     }
    if (key == "magnification"){ m_magnification = value.toDouble();  }
    if (key == "amplitud")     { m_amplitud      = value.toDouble();  }
    if (key == "iconSpacing")  { m_iconSpacing   = value.toInt();     }
    if (key == "showReflection"){ m_showReflection = value.toBool();  }

    if (key == "skinBlur")             { m_skinBlur             = value.toBool();   }
    if (key == "skinBlurRadius")       { m_skinBlurRadius       = value.toInt();    }
    if (key == "skinLiquidGelEffect")  { m_skinLiquidGelEffect  = value.toBool();   }
    if (key == "skinRefractionStrength"){ m_skinRefractionStrength = value.toDouble(); }
    if (key == "skinRgbFringing")      { m_skinRgbFringing      = value.toDouble(); }
    if (key == "skinPositionTaskIndicator") { m_skinPositionTaskIndicator = value.toInt(); }

    if (key == "showToolTips")              { m_showToolTips              = value.toBool(); }
    if (key == "highlightWindows")          { m_highlightWindows          = value.toBool(); }
    if (key == "indicateAudioStreams")      { m_indicateAudioStreams      = value.toBool(); }
    if (key == "interactiveMute")           { m_interactiveMute           = value.toBool(); }
    if (key == "tooltipControls")           { m_tooltipControls           = value.toBool(); }
    if (key == "minimizeActiveTaskOnClick") { m_minimizeActiveTaskOnClick = value.toBool(); }
    if (key == "middleClickAction")         { m_middleClickAction         = value.toInt();  }
    if (key == "wheelEnabled")              { m_wheelEnabled              = value.toInt();  }
    if (key == "wheelSkipMinimized")        { m_wheelSkipMinimized        = value.toBool(); }
    if (key == "groupingStrategy")          { m_groupingStrategy          = value.toInt();  }
    if (key == "groupedTaskVisualization")  { m_groupedTaskVisualization  = value.toInt();  }
    if (key == "sortingStrategy")           { m_sortingStrategy           = value.toInt();  }

    if (key == "skinLeftMargin")          { m_skinLeftMargin          = value.toInt(); }
    if (key == "skinTopMargin")           { m_skinTopMargin           = value.toInt(); }
    if (key == "skinRightMargin")         { m_skinRightMargin         = value.toInt(); }
    if (key == "skinBottomMargin")        { m_skinBottomMargin        = value.toInt(); }
    if (key == "skinOutsideLeftMargin")   { m_skinOutsideLeftMargin   = value.toInt(); }
    if (key == "skinOutsideTopMargin")    { m_skinOutsideTopMargin    = value.toInt(); }
    if (key == "skinOutsideRightMargin")  { m_skinOutsideRightMargin  = value.toInt(); }
    if (key == "skinOutsideBottomMargin") { m_skinOutsideBottomMargin = value.toInt(); }

    if (key == "showOnlyCurrentDesktop")  { m_showOnlyCurrentDesktop  = value.toBool(); }
    if (key == "showOnlyCurrentActivity") { m_showOnlyCurrentActivity = value.toBool(); }
    if (key == "showOnlyCurrentScreen")   { m_showOnlyCurrentScreen   = value.toBool(); }
    if (key == "showOnlyMinimized")       { m_showOnlyMinimized       = value.toBool(); }
    if (key == "unhideOnAttention")       { m_unhideOnAttention       = value.toBool(); }

    emit changed();

    // Persist and notify dock
    QString strVal;
    if (value.typeId() == QMetaType::Bool)
        strVal = value.toBool() ? "true" : "false";
    else
        strVal = value.toString();
    writeKey(key, strVal);
    notifyPlasmashell(key, strVal);
}
