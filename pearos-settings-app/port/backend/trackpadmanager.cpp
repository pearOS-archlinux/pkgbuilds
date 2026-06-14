#include "trackpadmanager.h"
#include <QProcess>

static const QString KWIN_SERVICE = QStringLiteral("org.kde.KWin");
static const QString KWIN_IFACE   = QStringLiteral("org.kde.KWin.InputDevice");

TrackpadManager::TrackpadManager(QObject *parent) : QObject(parent) {}

void TrackpadManager::run(const QString &cmd, std::function<void(QString)> cb) {
    auto *proc = new QProcess(this);
    connect(proc, &QProcess::finished, this, [proc, cb](int, QProcess::ExitStatus) {
        cb(QString::fromUtf8(proc->readAllStandardOutput()).trimmed());
        proc->deleteLater();
    });
    connect(proc, &QProcess::errorOccurred, proc, &QProcess::deleteLater);
    proc->start("bash", {"-c", cmd});
}

void TrackpadManager::findDevicePath() {
    if (!m_devicePath.isEmpty()) return;

    QProcess p;
    p.start("bash", {"-c",
        "qdbus6 org.kde.KWin /org/kde/KWin/InputDevice 2>/dev/null"
        " | grep -oE 'event[0-9]+'"
    });
    p.waitForFinished(3000);

    const QString out = QString::fromUtf8(p.readAllStandardOutput());
    for (const QString &line : out.split('\n', Qt::SkipEmptyParts)) {
        const QString dev = line.trimmed();
        const QString path = "/org/kde/KWin/InputDevice/" + dev;
        QProcess q;
        q.start("qdbus6", {KWIN_SERVICE, path,
            "org.freedesktop.DBus.Properties.Get", KWIN_IFACE, "touchpad"});
        q.waitForFinished(1000);
        if (QString::fromUtf8(q.readAllStandardOutput()).trimmed() == "true") {
            m_devicePath = path;
            break;
        }
    }
}

QString TrackpadManager::getProp(const QString &prop) const {
    if (m_devicePath.isEmpty()) return {};
    QProcess p;
    p.start("qdbus6", {KWIN_SERVICE, m_devicePath,
        "org.freedesktop.DBus.Properties.Get", KWIN_IFACE, prop});
    p.waitForFinished(2000);
    return QString::fromUtf8(p.readAllStandardOutput()).trimmed();
}

void TrackpadManager::setProp(const QString &prop, const QString &value) {
    if (m_devicePath.isEmpty()) return;
    QProcess::startDetached("qdbus6", {KWIN_SERVICE, m_devicePath,
        "org.freedesktop.DBus.Properties.Set", KWIN_IFACE, prop, value});
}

void TrackpadManager::refresh() {
    findDevicePath();
    if (m_devicePath.isEmpty()) { emit trackpadChanged(); return; }

    m_enabled              = getProp("enabled")              == "true";
    m_tapToClick           = getProp("tapToClick")           == "true";
    m_leftHanded           = getProp("leftHanded")           == "false" ? false : (getProp("leftHanded") == "true");
    m_naturalScroll        = getProp("naturalScroll")        == "true";
    m_disableWhileTyping   = getProp("disableWhileTyping")   == "true";
    m_disableEventsOnExtMouse = getProp("disableEventsOnExternalMouse") == "true";
    m_tapAndDrag           = getProp("tapAndDrag")           == "true";
    m_tapDragLock          = getProp("tapDragLock")          == "true";
    m_middleEmulation      = getProp("middleEmulation")      == "true";
    m_lmrTapButtonMap      = getProp("lmrTapButtonMap")      == "true";

    bool ok;
    double accel = getProp("pointerAcceleration").toDouble(&ok);
    if (ok) m_pointerAcceleration = accel;

    double sf = getProp("scrollFactor").toDouble(&ok);
    if (ok) m_scrollFactor = sf;

    m_accelProfile = (getProp("pointerAccelerationProfileFlat") == "true") ? 1 : 2;
    m_scrollMethod = (getProp("scrollTwoFinger") == "true") ? 1 : 2;
    m_clickMethod  = (getProp("clickMethodAreas") == "true") ? 1 : 2;

    emit trackpadChanged();
}

void TrackpadManager::setEnabled(bool v) {
    m_enabled = v; emit trackpadChanged();
    setProp("enabled", v ? "true" : "false");
}

void TrackpadManager::setTapToClick(bool v) {
    m_tapToClick = v; emit trackpadChanged();
    setProp("tapToClick", v ? "true" : "false");
}

void TrackpadManager::setClickMethod(int v) {
    m_clickMethod = v; emit trackpadChanged();
    setProp("clickMethodAreas",       v == 1 ? "true" : "false");
    setProp("clickMethodClickfinger", v == 2 ? "true" : "false");
}

void TrackpadManager::setLeftHanded(bool v) {
    m_leftHanded = v; emit trackpadChanged();
    setProp("leftHanded", v ? "true" : "false");
}

void TrackpadManager::setPointerAcceleration(double v) {
    m_pointerAcceleration = v; emit trackpadChanged();
    setProp("pointerAcceleration", QString::number(v, 'f', 3));
}

void TrackpadManager::setAccelProfile(int v) {
    m_accelProfile = v; emit trackpadChanged();
    setProp("pointerAccelerationProfileFlat",     v == 1 ? "true" : "false");
    setProp("pointerAccelerationProfileAdaptive", v == 2 ? "true" : "false");
}

void TrackpadManager::setNaturalScroll(bool v) {
    m_naturalScroll = v; emit trackpadChanged();
    setProp("naturalScroll", v ? "true" : "false");
}

void TrackpadManager::setScrollMethod(int v) {
    m_scrollMethod = v; emit trackpadChanged();
    setProp("scrollTwoFinger", v == 1 ? "true" : "false");
    setProp("scrollEdge",      v == 2 ? "true" : "false");
}

void TrackpadManager::setScrollFactor(double v) {
    m_scrollFactor = v; emit trackpadChanged();
    setProp("scrollFactor", QString::number(v, 'f', 2));
}

void TrackpadManager::setDisableWhileTyping(bool v) {
    m_disableWhileTyping = v; emit trackpadChanged();
    setProp("disableWhileTyping", v ? "true" : "false");
}

void TrackpadManager::setDisableEventsOnExtMouse(bool v) {
    m_disableEventsOnExtMouse = v; emit trackpadChanged();
    setProp("disableEventsOnExternalMouse", v ? "true" : "false");
}

void TrackpadManager::setTapAndDrag(bool v) {
    m_tapAndDrag = v; emit trackpadChanged();
    setProp("tapAndDrag", v ? "true" : "false");
}

void TrackpadManager::setTapDragLock(bool v) {
    m_tapDragLock = v; emit trackpadChanged();
    setProp("tapDragLock", v ? "true" : "false");
}

void TrackpadManager::setMiddleEmulation(bool v) {
    m_middleEmulation = v; emit trackpadChanged();
    setProp("middleEmulation", v ? "true" : "false");
}

void TrackpadManager::setLmrTapButtonMap(bool v) {
    m_lmrTapButtonMap = v; emit trackpadChanged();
    setProp("lmrTapButtonMap", v ? "true" : "false");
}
