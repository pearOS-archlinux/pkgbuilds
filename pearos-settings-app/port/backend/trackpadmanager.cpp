#include "trackpadmanager.h"
#include <QProcess>
#include <QRegularExpression>

TrackpadManager::TrackpadManager(QObject *parent) : QObject(parent) {}

void TrackpadManager::run(const QString &cmd, std::function<void(QString)> cb) {
    auto *proc = new QProcess(this);
    connect(proc, &QProcess::finished, this, [proc, cb](int, QProcess::ExitStatus) {
        cb(QString::fromUtf8(proc->readAllStandardOutput()));
        proc->deleteLater();
    });
    connect(proc, &QProcess::errorOccurred, proc, &QProcess::deleteLater);
    proc->start("bash", {"-c", cmd});
}

void TrackpadManager::applyProperty(const QString &prop, const QString &value) {
    if (m_deviceId.isEmpty()) return;
    run(QString("xinput set-prop %1 \"%2\" %3 2>/dev/null").arg(m_deviceId, prop, value), [](QString) {});
}

void TrackpadManager::refresh() {
    run("xinput list 2>/dev/null | grep -i 'touchpad\\|trackpad' | head -1 | grep -oP 'id=\\K\\d+'", [this](QString out) {
        m_deviceId = out.trimmed();
        if (m_deviceId.isEmpty()) { emit trackpadChanged(); return; }

        run(QString("xinput list-props %1 2>/dev/null").arg(m_deviceId), [this](QString props) {
            static QRegularExpression nsRe("Natural Scrolling Enabled[^:]*:\\s*(\\d)");
            static QRegularExpression tpRe("Tapping Enabled[^:]*:\\s*(\\d)");
            static QRegularExpression tfRe("Two Finger Scrolling Enabled[^:]*:\\s*(\\d)");
            static QRegularExpression spRe("Accel Speed[^:]*:\\s*([\\d.+-]+)");

            auto nm = nsRe.match(props); m_naturalScroll   = nm.hasMatch() && nm.captured(1) == "1";
            auto tm = tpRe.match(props); m_tapToClick      = !tm.hasMatch() || tm.captured(1) == "1";
            auto fm = tfRe.match(props); m_twoFingerScroll = !fm.hasMatch() || fm.captured(1) == "1";
            auto sm = spRe.match(props); m_speed = sm.hasMatch() ? sm.captured(1).toDouble() : 0.0;
            emit trackpadChanged();
        });
    });
}

void TrackpadManager::setNaturalScroll(bool enabled) {
    m_naturalScroll = enabled;
    applyProperty("libinput Natural Scrolling Enabled", enabled ? "1" : "0");
    emit trackpadChanged();
}

void TrackpadManager::setTapToClick(bool enabled) {
    m_tapToClick = enabled;
    applyProperty("libinput Tapping Enabled", enabled ? "1" : "0");
    emit trackpadChanged();
}

void TrackpadManager::setTwoFingerScroll(bool enabled) {
    m_twoFingerScroll = enabled;
    applyProperty("libinput Two Finger Scrolling Enabled", enabled ? "1" : "0");
    emit trackpadChanged();
}

void TrackpadManager::setSpeed(double speed) {
    m_speed = speed;
    applyProperty("libinput Accel Speed", QString::number(speed, 'f', 2));
    emit trackpadChanged();
}
