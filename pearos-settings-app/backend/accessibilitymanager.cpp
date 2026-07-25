#include "accessibilitymanager.h"
#include <QProcess>
#include <X11/Xlib.h>
#include <X11/XKBlib.h>

AccessibilityManager::AccessibilityManager(QObject *parent) : QObject(parent) {}

void AccessibilityManager::run(const QString &cmd, std::function<void(QString)> cb) {
    auto *proc = new QProcess(this);
    connect(proc, &QProcess::finished, this, [proc, cb](int, QProcess::ExitStatus) {
        cb(QString::fromUtf8(proc->readAllStandardOutput()).trimmed());
        proc->deleteLater();
    });
    connect(proc, &QProcess::errorOccurred, proc, &QProcess::deleteLater);
    proc->start("bash", {"-c", cmd});
}

void AccessibilityManager::notifyGlobalChange() {
    QProcess::startDetached("bash", {"-c",
        "qdbus6 org.kde.KWin /KWin reconfigure 2>/dev/null; "
        "qdbus6 org.kde.KGlobalSettings /KGlobalSettings"
        " org.kde.KGlobalSettings.notifyChange 0 0 2>/dev/null || true"});
}

// XkbGetControls auto-allocates desc->ctrls via XkbAllocControls when needed.
static XkbDescPtr xkbControls(Display *dpy) {
    XkbDescPtr desc = XkbAllocKeyboard();
    if (!desc) return nullptr;
    if (XkbGetControls(dpy, XkbAllControlsMask, desc) != Success) {
        XkbFreeKeyboard(desc, 0, True);
        return nullptr;
    }
    return desc;
}

// AccessX controls live in the XKB extension itself (same layer KDE's kaccess
// daemon writes to) — toggling them takes effect immediately, no daemon needed.
void AccessibilityManager::applyXkbControl(unsigned int mask, bool enable) {
    Display *dpy = XOpenDisplay(nullptr);
    if (!dpy) return;
    if (XkbDescPtr desc = xkbControls(dpy)) {
        if (enable) desc->ctrls->enabled_ctrls |= mask;
        else        desc->ctrls->enabled_ctrls &= ~mask;
        XkbSetControls(dpy, XkbAllControlsMask, desc);
        XkbFreeKeyboard(desc, 0, True);
    }
    XCloseDisplay(dpy);
}

void AccessibilityManager::refresh() {
    if (Display *dpy = XOpenDisplay(nullptr)) {
        if (XkbDescPtr desc = xkbControls(dpy)) {
            unsigned int c = desc->ctrls->enabled_ctrls;
            m_stickyKeys        = c & XkbStickyKeysMask;
            m_slowKeys          = c & XkbSlowKeysMask;
            m_bounceKeys        = c & XkbBounceKeysMask;
            m_mouseKeys         = c & XkbMouseKeysMask;
            m_audibleBell       = c & XkbAudibleBellMask;
            m_slowKeysDelay     = desc->ctrls->slow_keys_delay;
            m_bounceKeysDelay   = desc->ctrls->debounce_delay;
            m_mouseKeysMaxSpeed = desc->ctrls->mk_max_speed;
            XkbFreeKeyboard(desc, 0, True);
        }
        XCloseDisplay(dpy);
    }
    emit accessibilityChanged();

    run("kreadconfig6 --file kdeglobals --group KDE --key AnimationDurationFactor --default 1",
        [this](QString out) {
            bool ok; double v = out.toDouble(&ok);
            m_reduceMotion = ok && v <= 0.01;
            emit accessibilityChanged();
        });
    run("kreadconfig6 --file kdeglobals --group KDE --key contrast --default 4",
        [this](QString out) {
            bool ok; int v = out.toInt(&ok);
            if (ok) m_contrast = v;
            emit accessibilityChanged();
        });
    run("kreadconfig6 --file kcminputrc --group Mouse --key cursorSize --default 24",
        [this](QString out) {
            bool ok; int v = out.toInt(&ok);
            if (ok) m_cursorSize = v;
            emit accessibilityChanged();
        });
}

void AccessibilityManager::setStickyKeys(bool v) {
    m_stickyKeys = v; emit accessibilityChanged();
    applyXkbControl(XkbStickyKeysMask, v);
}

void AccessibilityManager::setSlowKeys(bool v) {
    m_slowKeys = v; emit accessibilityChanged();
    applyXkbControl(XkbSlowKeysMask, v);
}

void AccessibilityManager::setSlowKeysDelay(int ms) {
    m_slowKeysDelay = ms; emit accessibilityChanged();
    Display *dpy = XOpenDisplay(nullptr);
    if (!dpy) return;
    if (XkbDescPtr desc = xkbControls(dpy)) {
        desc->ctrls->slow_keys_delay = ms;
        XkbSetControls(dpy, XkbAllControlsMask, desc);
        XkbFreeKeyboard(desc, 0, True);
    }
    XCloseDisplay(dpy);
}

void AccessibilityManager::setBounceKeys(bool v) {
    m_bounceKeys = v; emit accessibilityChanged();
    applyXkbControl(XkbBounceKeysMask, v);
}

void AccessibilityManager::setBounceKeysDelay(int ms) {
    m_bounceKeysDelay = ms; emit accessibilityChanged();
    Display *dpy = XOpenDisplay(nullptr);
    if (!dpy) return;
    if (XkbDescPtr desc = xkbControls(dpy)) {
        desc->ctrls->debounce_delay = ms;
        XkbSetControls(dpy, XkbAllControlsMask, desc);
        XkbFreeKeyboard(desc, 0, True);
    }
    XCloseDisplay(dpy);
}

void AccessibilityManager::setMouseKeys(bool v) {
    m_mouseKeys = v; emit accessibilityChanged();
    Display *dpy = XOpenDisplay(nullptr);
    if (!dpy) return;
    if (XkbDescPtr desc = xkbControls(dpy)) {
        if (v) {
            desc->ctrls->enabled_ctrls |= XkbMouseKeysMask;
            // Sane defaults so the numpad actually moves the pointer —
            // a zeroed mk_delay/interval/time_to_max means no motion at all.
            if (desc->ctrls->mk_delay == 0)       desc->ctrls->mk_delay = 160;
            if (desc->ctrls->mk_interval == 0)    desc->ctrls->mk_interval = 20;
            if (desc->ctrls->mk_time_to_max == 0) desc->ctrls->mk_time_to_max = 20;
            if (desc->ctrls->mk_max_speed == 0)   desc->ctrls->mk_max_speed = m_mouseKeysMaxSpeed;
        } else {
            desc->ctrls->enabled_ctrls &= ~XkbMouseKeysMask;
        }
        XkbSetControls(dpy, XkbAllControlsMask, desc);
        XkbFreeKeyboard(desc, 0, True);
    }
    XCloseDisplay(dpy);
}

void AccessibilityManager::setMouseKeysMaxSpeed(int v) {
    m_mouseKeysMaxSpeed = v; emit accessibilityChanged();
    Display *dpy = XOpenDisplay(nullptr);
    if (!dpy) return;
    if (XkbDescPtr desc = xkbControls(dpy)) {
        desc->ctrls->mk_max_speed = v;
        XkbSetControls(dpy, XkbAllControlsMask, desc);
        XkbFreeKeyboard(desc, 0, True);
    }
    XCloseDisplay(dpy);
}

void AccessibilityManager::setAudibleBell(bool v) {
    m_audibleBell = v; emit accessibilityChanged();
    applyXkbControl(XkbAudibleBellMask, v);
}

void AccessibilityManager::setReduceMotion(bool v) {
    m_reduceMotion = v; emit accessibilityChanged();
    run(QString("kwriteconfig6 --file kdeglobals --group KDE --key AnimationDurationFactor %1")
            .arg(v ? "0" : "1"), [this](QString) { notifyGlobalChange(); emit accessibilityChanged(); });
}

void AccessibilityManager::setContrast(int v) {
    m_contrast = v; emit accessibilityChanged();
    run(QString("kwriteconfig6 --file kdeglobals --group KDE --key contrast %1").arg(v),
        [this](QString) { notifyGlobalChange(); emit accessibilityChanged(); });
}

void AccessibilityManager::setCursorSize(int v) {
    m_cursorSize = v; emit accessibilityChanged();
    run(QString("kwriteconfig6 --file kcminputrc --group Mouse --key cursorSize %1").arg(v),
        [this](QString) { notifyGlobalChange(); emit accessibilityChanged(); });
}
