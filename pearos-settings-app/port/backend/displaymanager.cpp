#include "displaymanager.h"
#include <QProcess>
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

void DisplayManager::refreshDisplays() {
    run("xrandr 2>/dev/null", [this](QString out) {
        m_displays.clear();
        QVariantMap cur;
        QVariantList curRes;

        for (const QString &line : out.split('\n')) {
            static QRegularExpression dispRe("^([A-Za-z0-9-]+)\\s+(connected|disconnected)");
            static QRegularExpression resRe("^\\s+(\\d+x\\d+)");
            auto dm = dispRe.match(line);
            if (dm.hasMatch()) {
                if (!cur.isEmpty()) { cur["availableResolutions"] = curRes; m_displays.append(cur); }
                cur.clear(); curRes.clear();
                cur["name"] = dm.captured(1);
                cur["isConnected"] = dm.captured(2) == "connected";
                cur["isBuiltIn"] = m_displays.isEmpty();
                cur["isEnabled"] = line.contains('*');
                cur["isPrimary"] = line.contains("primary");
                cur["resolution"] = QString();
            } else {
                auto rm = resRe.match(line);
                if (rm.hasMatch() && !cur.isEmpty()) {
                    QString res = rm.captured(1);
                    bool active = line.contains('*');
                    auto parts = res.split('x');
                    int w = parts.value(0).toInt(), h = parts.value(1).toInt();
                    double diag = std::sqrt(w*w + h*h) / 96.0;
                    QVariantMap r;
                    r["resolution"] = res; r["width"] = w; r["height"] = h;
                    r["totalPixels"] = w * h; r["isActive"] = active;
                    curRes.append(r);
                    if (active) {
                        cur["resolution"] = res;
                        cur["isEnabled"] = true;
                        cur["inches"] = QString::number(diag, 'f', 1) + "\"";
                    }
                }
            }
        }
        if (!cur.isEmpty()) { cur["availableResolutions"] = curRes; m_displays.append(cur); }
        emit displaysChanged();
    });
}

void DisplayManager::refreshBrightness(const QString &displayName) {
    Q_UNUSED(displayName)
    run("qdbus org.kde.Solid.PowerManagement "
        "/org/kde/Solid/PowerManagement/Actions/BrightnessControl "
        "org.kde.Solid.PowerManagement.Actions.BrightnessControl.brightnessMax 2>/dev/null",
        [this](QString maxOut) {
            int maxVal = maxOut.trimmed().toInt();
            if (maxVal <= 0) maxVal = 100;
            m_brightnessMax = maxVal;
            run("qdbus org.kde.Solid.PowerManagement "
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
    m_brightness = qBound(0, pct, 100);
    emit brightnessChanged();
    if (displayName.isEmpty() || displayName.startsWith("eDP")) {
        int raw = qRound(pct * m_brightnessMax / 100.0);
        run(QString("qdbus org.kde.Solid.PowerManagement "
                    "/org/kde/Solid/PowerManagement/Actions/BrightnessControl "
                    "org.kde.Solid.PowerManagement.Actions.BrightnessControl.setBrightness %1 2>/dev/null").arg(raw),
            [this, displayName, pct](QString err) {
                if (!err.trimmed().isEmpty()) return;
                if (!displayName.isEmpty())
                    run(QString("xrandr --output %1 --brightness %2").arg(displayName).arg(pct / 100.0), [](QString) {});
            });
    } else {
        run(QString("xrandr --output %1 --brightness %2").arg(displayName).arg(pct / 100.0), [](QString) {});
    }
}

void DisplayManager::setResolution(const QString &displayName, const QString &resolutionType) {
    run("xrandr 2>/dev/null", [this, displayName, resolutionType](QString out) {
        QStringList modes;
        bool inDisplay = false;
        for (const QString &line : out.split('\n')) {
            if (line.startsWith(displayName + " ")) { inDisplay = true; continue; }
            if (inDisplay && line.startsWith(' ')) {
                static QRegularExpression re("(\\d+x\\d+)");
                auto m = re.match(line.trimmed());
                if (m.hasMatch()) modes.append(m.captured(1));
            } else if (inDisplay) break;
        }
        if (modes.isEmpty()) return;
        QString mode = resolutionType == "default" ? modes.first()
                     : resolutionType == "larger" ? modes.last()
                     : modes.value(modes.size() / 2);
        run(QString("xrandr --output %1 --mode %2").arg(displayName, mode), [this](QString) { refreshDisplays(); });
    });
}

void DisplayManager::setDisplayEnabled(const QString &displayName, bool enabled, const QString &modeId) {
    QString cmd;
    if (enabled)
        cmd = QString("xrandr --output %1 %2").arg(displayName, modeId.isEmpty() ? "--auto" : "--mode " + modeId);
    else
        cmd = QString("xrandr --output %1 --off").arg(displayName);
    run(cmd, [this](QString) { refreshDisplays(); });
}

void DisplayManager::setDisplayPrimary(const QString &displayName) {
    run(QString("xrandr --output %1 --primary").arg(displayName), [this](QString) { refreshDisplays(); });
}

void DisplayManager::setNightLight(bool enabled) {
    if (enabled) {
        // Try kscreen-doctor first, fall back to redshift
        run("kscreen-doctor dpms.on 2>/dev/null || true", [this, enabled](QString) {
            run("busctl call org.kde.kwin /ColorManager org.kde.kwin.ColorManager setTemperature u 4500 2>/dev/null"
                " || redshift -O 4500 2>/dev/null || true", [](QString) {});
        });
    } else {
        run("busctl call org.kde.kwin /ColorManager org.kde.kwin.ColorManager setTemperature u 6500 2>/dev/null"
            " || redshift -x 2>/dev/null || true", [](QString) {});
    }
}
