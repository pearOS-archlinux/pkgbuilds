#include "displaymanager.h"
#include <QProcess>
#include <QProcessEnvironment>
#include <QRegularExpression>
#include <cmath>

DisplayManager::DisplayManager(QObject *parent) : QObject(parent) {}

void DisplayManager::run(const QString &cmd, std::function<void(QString)> cb) {
    auto *proc = new QProcess(this);
    connect(proc, &QProcess::finished, this, [proc, cb](int, QProcess::ExitStatus) {
        cb(QString::fromUtf8(proc->readAllStandardOutput()));
        proc->deleteLater();
    });
    connect(proc, &QProcess::errorOccurred, proc, &QProcess::deleteLater);
    proc->start("bash", {"-c", cmd});
}

// kscreen-doctor hangs when it inherits QT_QPA_PLATFORM=xcb from our process
void DisplayManager::runKScreen(const QString &cmd, std::function<void(QString)> cb) {
    auto *proc = new QProcess(this);
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.remove("QT_QPA_PLATFORM");
    proc->setProcessEnvironment(env);
    connect(proc, &QProcess::finished, this, [proc, cb](int, QProcess::ExitStatus) {
        cb(QString::fromUtf8(proc->readAllStandardOutput()));
        proc->deleteLater();
    });
    connect(proc, &QProcess::errorOccurred, proc, &QProcess::deleteLater);
    proc->start("bash", {"-c", cmd});
}

void DisplayManager::refreshDisplays() {
    runKScreen("kscreen-doctor -o 2>/dev/null", [this](QString raw) {
        // Strip ANSI escape codes
        raw.replace(QRegularExpression("\x1b\\[[0-9;]*m"), "");

        m_displays.clear();
        QVariantMap cur;
        QVariantList modes;
        int outputIdx = 0;
        bool inOutput = false;

        static QRegularExpression outputRe(R"(^Output:\s+(\d+)\s+(\S+))");
        static QRegularExpression modesRe(R"((\d+):(\d+)x(\d+)@([\d.]+)(\*?)(!?))");
        static QRegularExpression geomRe(R"(Geometry:\s+[\d,]+\s+(\d+)x(\d+))");
        static QRegularExpression scaleRe(R"(Scale:\s+([\d.]+))");
        static QRegularExpression rotRe(R"(Rotation:\s+(\d+))");
        static QRegularExpression priorityRe(R"(priority\s+(\d+))");

        for (const QString &rawLine : raw.split('\n')) {
            QString line = rawLine.trimmed();
            if (line.isEmpty()) continue;

            auto om = outputRe.match(line);
            if (om.hasMatch()) {
                if (inOutput && !cur.isEmpty()) {
                    cur["modes"] = modes;
                    m_displays.append(cur);
                }
                cur.clear(); modes.clear();
                inOutput = true;
                cur["id"]        = om.captured(1).toInt();
                cur["name"]      = om.captured(2);
                cur["isBuiltIn"] = (outputIdx == 0);
                cur["enabled"]   = false;
                cur["connected"] = false;
                cur["isPrimary"] = false;
                cur["currentWidth"]       = 0;
                cur["currentHeight"]      = 0;
                cur["currentRefreshRate"] = 0.0;
                cur["currentModeId"]      = -1;
                cur["scale"]     = 1.0;
                cur["rotation"]  = 0;
                outputIdx++;
                continue;
            }

            if (!inOutput) continue;

            if (line == "enabled")   { cur["enabled"] = true; continue; }
            if (line == "connected") { cur["connected"] = true; continue; }

            auto pm = priorityRe.match(line);
            if (pm.hasMatch()) { cur["isPrimary"] = (pm.captured(1).toInt() == 1); continue; }

            if (line.startsWith("Modes:")) {
                QRegularExpressionMatchIterator it = modesRe.globalMatch(line);
                while (it.hasNext()) {
                    auto mm = it.next();
                    int    id   = mm.captured(1).toInt();
                    int    w    = mm.captured(2).toInt();
                    int    h    = mm.captured(3).toInt();
                    double hz   = mm.captured(4).toDouble();
                    bool active    = mm.captured(5) == "*";
                    bool preferred = mm.captured(6) == "!";

                    QVariantMap mode;
                    mode["id"]          = id;
                    mode["width"]       = w;
                    mode["height"]      = h;
                    mode["refreshRate"] = hz;
                    mode["isActive"]    = active;
                    mode["isPreferred"] = preferred;
                    modes.append(mode);

                    if (active) {
                        cur["currentWidth"]       = w;
                        cur["currentHeight"]      = h;
                        cur["currentRefreshRate"] = hz;
                        cur["currentModeId"]      = id;
                    }
                }
                continue;
            }

            auto gm = geomRe.match(line);
            if (gm.hasMatch() && cur["currentWidth"].toInt() == 0) {
                cur["currentWidth"]  = gm.captured(1).toInt();
                cur["currentHeight"] = gm.captured(2).toInt();
                continue;
            }

            auto sm = scaleRe.match(line);
            if (sm.hasMatch()) { cur["scale"] = sm.captured(1).toDouble(); continue; }

            auto rm = rotRe.match(line);
            if (rm.hasMatch()) { cur["rotation"] = rm.captured(1).toInt(); continue; }
        }

        if (inOutput && !cur.isEmpty()) {
            cur["modes"] = modes;
            m_displays.append(cur);
        }

        emit displaysChanged();
    });

    // Read Night Light state from KWin DBus (enabled property)
    run("qdbus6 org.kde.KWin.NightLight /org/kde/KWin/NightLight"
        " org.freedesktop.DBus.Properties.Get org.kde.KWin.NightLight enabled 2>/dev/null",
        [this](QString out) {
            bool enabled = out.trimmed().contains("true", Qt::CaseInsensitive);
            if (m_nightLight != enabled) {
                m_nightLight = enabled;
                emit nightLightChanged();
            }
        });
}

void DisplayManager::setMode(const QString &outputName, int modeId) {
    runKScreen(QString("kscreen-doctor output.%1.mode.%2 2>/dev/null")
                   .arg(outputName).arg(modeId),
               [this](QString) { refreshDisplays(); });
}

void DisplayManager::setScale(const QString &outputName, double scale) {
    runKScreen(QString("kscreen-doctor output.%1.scale.%2 2>/dev/null")
                   .arg(outputName).arg(scale, 0, 'g', 4),
               [this](QString) { refreshDisplays(); });
}

void DisplayManager::setDisplayEnabled(const QString &outputName, bool enabled) {
    runKScreen(QString("kscreen-doctor output.%1.%2 2>/dev/null")
                   .arg(outputName, enabled ? "enable" : "disable"),
               [this](QString) { refreshDisplays(); });
}

void DisplayManager::refreshBrightness(const QString &displayName) {
    Q_UNUSED(displayName)
    run("qdbus6 org.kde.Solid.PowerManagement "
        "/org/kde/Solid/PowerManagement/Actions/BrightnessControl "
        "org.kde.Solid.PowerManagement.Actions.BrightnessControl.brightnessMax 2>/dev/null",
        [this](QString maxOut) {
            int maxVal = maxOut.trimmed().toInt();
            if (maxVal <= 0) maxVal = 100;
            m_brightnessMax = maxVal;
            run("qdbus6 org.kde.Solid.PowerManagement "
                "/org/kde/Solid/PowerManagement/Actions/BrightnessControl "
                "org.kde.Solid.PowerManagement.Actions.BrightnessControl.brightness 2>/dev/null",
                [this, maxVal](QString out) {
                    int raw = out.trimmed().toInt();
                    m_brightness = maxVal > 0 ? qRound(raw * 100.0 / maxVal) : 50;
                    emit brightnessChanged();
                });
        });
}

void DisplayManager::setBrightness(int pct, const QString &displayName) {
    Q_UNUSED(displayName)
    m_brightness = qBound(0, pct, 100);
    emit brightnessChanged();
    int raw = qRound(pct * m_brightnessMax / 100.0);
    run(QString("qdbus6 org.kde.Solid.PowerManagement "
                "/org/kde/Solid/PowerManagement/Actions/BrightnessControl "
                "org.kde.Solid.PowerManagement.Actions.BrightnessControl.setBrightness %1 2>/dev/null").arg(raw),
        [](QString) {});
}

void DisplayManager::setNightLight(bool enabled) {
    m_nightLight = enabled;
    emit nightLightChanged();
    run(QString("kwriteconfig6 --file kwinrc --group NightColor --key Active %1 2>/dev/null"
                " && qdbus6 org.kde.KWin /KWin reconfigure 2>/dev/null || true")
            .arg(enabled ? "true" : "false"),
        [](QString) {});
}
