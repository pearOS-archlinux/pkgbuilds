#include "NotchWindow.h"
#include "MprisWatcher.h"
#include "Config.h"
#include "SettingsDialog.h"

#include <QPainter>
#include <QPainterPath>
#include <QScreen>
#include <QGuiApplication>
#include <QMouseEvent>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QDrag>
#include <QMimeData>
#include <QDateTime>
#include <QWindow>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QSettings>
#include <QProcess>
#include <QMenu>
#include <QContextMenuEvent>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusMessage>
#include <QDBusReply>
#include <QDBusObjectPath>
#include <QDebug>

#ifdef WITH_LAYER_SHELL
#include <LayerShellQt/window.h>
#endif

#include <QtGui/qguiapplication_platform.h>
#include <xcb/xcb.h>
#include <cstring>

// ── Helpers ───────────────────────────────────────────────────────────────────

static QColor extensionColor(const QString& ext) {
    static const QHash<QString, QColor> map = {
        {"PDF",  {220,60,60}},
        {"ZIP",  {200,160,40}}, {"RAR",{200,160,40}}, {"7Z",{200,160,40}},
        {"TAR",  {200,160,40}}, {"GZ", {200,160,40}}, {"XZ",{200,160,40}},
        {"PNG",  {60,140,220}}, {"JPG",{60,140,220}}, {"JPEG",{60,140,220}},
        {"GIF",  {60,140,220}}, {"WEBP",{60,140,220}}, {"SVG",{60,140,220}},
        {"MP4",  {140,60,220}}, {"MKV",{140,60,220}}, {"AVI",{140,60,220}},
        {"MOV",  {140,60,220}}, {"WEBM",{140,60,220}},
        {"MP3",  {60,180,120}}, {"FLAC",{60,180,120}}, {"WAV",{60,180,120}},
        {"OGG",  {60,180,120}}, {"AAC",{60,180,120}},
        {"DOC",  {60,100,200}}, {"DOCX",{60,100,200}}, {"ODT",{60,100,200}},
        {"XLS",  {60,180,60}},  {"XLSX",{60,180,60}},  {"CSV",{60,180,60}},
        {"PPT",  {220,100,40}}, {"PPTX",{220,100,40}},
        {"SH",   {150,150,150}}, {"BIN",{150,150,150}}, {"APPIMAGE",{150,150,150}},
    };
    return map.value(ext, QColor(90, 90, 90));
}

// ── Constructor / Destructor ──────────────────────────────────────────────────

NotchWindow::NotchWindow(SettingsDialog* settingsDlg, QWidget* parent)
    : QWidget(parent)
    , m_settingsDlg(settingsDlg)
{
    setAttribute(Qt::WA_NativeWindow);
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_NoSystemBackground);
    setAcceptDrops(true);
    setMouseTracking(true);

    applyWindowHints();

    m_svgPrev     = new QSvgRenderer(QString(":/icons/prev.svg"),    this);
    m_svgNext     = new QSvgRenderer(QString(":/icons/next.svg"),    this);
    m_svgPlay     = new QSvgRenderer(QString(":/icons/play.svg"),    this);
    m_svgPause    = new QSvgRenderer(QString(":/icons/pause.svg"),   this);
    m_svgSettings = new QSvgRenderer(QString(":/icons/settings.svg"),this);
    m_svgHome     = new QSvgRenderer(QString(":/icons/home.svg"),    this);
    m_svgTrayIcon = new QSvgRenderer(QString(":/icons/tray.svg"),    this);
    m_svgBrowser  = new QSvgRenderer(QString(":/icons/browser.svg"), this);

    m_mpris = new MprisWatcher(this);
    connect(m_mpris, &MprisWatcher::infoChanged,     this, &NotchWindow::onMprisChanged);
    connect(m_mpris, &MprisWatcher::positionChanged, this, &NotchWindow::onPositionChanged);

    m_sizeAnim = new QPropertyAnimation(this, "notchSize", this);

    m_hoverTimer = new QTimer(this);
    m_hoverTimer->setSingleShot(true);
    connect(m_hoverTimer, &QTimer::timeout, this, &NotchWindow::onHoverExpand);

    m_vizTimer = new QTimer(this);
    m_vizTimer->setInterval(16);
    connect(m_vizTimer, &QTimer::timeout, this, &NotchWindow::onVizTimer);

    Config& cfg = Config::instance();
    connect(&cfg, &Config::modeChanged,        this, &NotchWindow::reinitForMode);
    connect(&cfg, &Config::smallWChanged,      this, [this](){ if (!m_expanded) reinitForMode(); });
    connect(&cfg, &Config::smallHChanged,      this, [this](){ if (!m_expanded) reinitForMode(); });
    connect(&cfg, &Config::bigWChanged,        this, [this](){ if (m_expanded)  reinitForMode(); });
    connect(&cfg, &Config::bigHChanged,        this, [this](){ if (m_expanded)  reinitForMode(); });
    connect(&cfg, &Config::screenGapChanged,   this, &NotchWindow::positionWindow);
    connect(&cfg, &Config::hoverDelayMsChanged,this, [this](int v){ m_hoverTimer->setInterval(v); });

    setFixedSize(cfg.smallW(), cfg.smallH());
    positionWindow();
    loadTrayFromSettings();
}

NotchWindow::~NotchWindow() = default;

// ── Platform ──────────────────────────────────────────────────────────────────

bool NotchWindow::isWayland() const {
    return QGuiApplication::platformName() == "wayland";
}

void NotchWindow::applyWindowHints() {
    if (isWayland())
        setWindowFlags(Qt::FramelessWindowHint | Qt::Window);
    else
        setWindowFlags(Qt::FramelessWindowHint | Qt::BypassWindowManagerHint | Qt::Window);
}

void NotchWindow::setX11Hints() {
    auto* x11 = qApp->nativeInterface<QNativeInterface::QX11Application>();
    if (!x11) return;
    xcb_connection_t* conn = x11->connection();
    xcb_window_t win = (xcb_window_t)winId();

    auto intern = [&](const char* name) {
        auto* r = xcb_intern_atom_reply(conn,
            xcb_intern_atom(conn, 0, (uint16_t)strlen(name), name), nullptr);
        xcb_atom_t a = r ? r->atom : 0; free(r); return a;
    };

    xcb_atom_t wmType = intern("_NET_WM_WINDOW_TYPE");
    xcb_atom_t dock   = intern("_NET_WM_WINDOW_TYPE_DOCK");
    if (wmType && dock)
        xcb_change_property(conn, XCB_PROP_MODE_REPLACE, win, wmType, XCB_ATOM_ATOM, 32, 1, &dock);

    auto zeroAtom = [&](const char* name) {
        xcb_atom_t a = intern(name);
        if (a) { uint32_t z=0; xcb_change_property(conn, XCB_PROP_MODE_REPLACE, win, a, XCB_ATOM_CARDINAL, 32, 1, &z); }
    };
    zeroAtom("_COMPTON_SHADOW");
    zeroAtom("_KDE_NET_WM_SHADOW");
    zeroAtom("_KDE_NET_WM_BLUR_BEHIND_REGION");
    xcb_flush(conn);
}

void NotchWindow::applyLayerShell(bool /*updateOnly*/) {
#ifdef WITH_LAYER_SHELL
    if (!isWayland()) return;
    QWindow* wh = windowHandle();
    if (!wh) return;
    auto* lw = LayerShellQt::Window::get(wh);
    if (!lw) return;

    lw->setLayer(LayerShellQt::Window::LayerOverlay);
    lw->setAnchors(LayerShellQt::Window::AnchorTop);
    lw->setKeyboardInteractivity(LayerShellQt::Window::KeyboardInteractivityNone);
    lw->setExclusiveZone(-1);
    lw->setDesiredSize(size());

    int gap = (Config::instance().mode() == "island") ? Config::instance().screenGap() : 0;
    lw->setMargins(QMargins(0, gap, 0, 0));
#endif
}

void NotchWindow::disableKWinEffects() {
    QWindow* wh = windowHandle();
    if (!wh) return;
    wh->setProperty("_NET_WM_STATE_SKIP_TASKBAR", true);
    wh->setProperty("_NET_WM_STATE_SKIP_PAGER",   true);
}

void NotchWindow::showEvent(QShowEvent* e) {
    QWidget::showEvent(e);
    if (isWayland()) applyLayerShell(false);
    else             setX11Hints();
    disableKWinEffects();
}

// ── Q_PROPERTY ───────────────────────────────────────────────────────────────

QSize NotchWindow::notchSize() const { return size(); }

void NotchWindow::setNotchSize(const QSize& sz) {
    Config& cfg = Config::instance();
    int w = qMax(cfg.smallW(), sz.width());
    int h = qMax(cfg.smallH(), sz.height());
    if (QSize(w, h) == size()) return;
    setFixedSize(w, h);
    positionWindow();
#ifdef WITH_LAYER_SHELL
    if (isWayland()) applyLayerShell(true);
#endif
    update();
}

// ── Animation ─────────────────────────────────────────────────────────────────

void NotchWindow::startSizeAnim(const QSize& target, bool expanding) {
    if (m_sizeAnim->state() == QAbstractAnimation::Running)
        m_sizeAnim->stop();
    m_sizeAnim->setEasingCurve(expanding ? QEasingCurve::OutCubic : QEasingCurve::InOutQuad);
    m_sizeAnim->setDuration(Config::instance().animDurationMs());
    m_sizeAnim->setStartValue(size());
    m_sizeAnim->setEndValue(target);
    m_sizeAnim->start();
}

void NotchWindow::startVizAnim(bool playing) {
    m_vizTarget = playing ? 1.0 : 0.0;
    if (!m_vizTimer->isActive()) m_vizTimer->start();
    update();
}

void NotchWindow::onVizTimer() {
    constexpr double STEP = 1.0 / 20.0;
    double diff = m_vizTarget - m_vizAlpha;
    m_vizAlpha = (qAbs(diff) <= STEP) ? m_vizTarget : m_vizAlpha + (diff > 0 ? STEP : -STEP);
    update();
    if (m_vizAlpha < 0.005 && m_vizTarget < 0.005)
        m_vizTimer->stop();
}

bool NotchWindow::isExpanded() const {
    return height() > Config::instance().smallH() + 5;
}

// ── Window positioning ────────────────────────────────────────────────────────

void NotchWindow::positionWindow() {
    QScreen* screen = QGuiApplication::primaryScreen();
    if (!screen) return;
    QRect sg = screen->geometry();
    int x = sg.x() + (sg.width() - width()) / 2;
    int y = sg.y() + (Config::instance().mode() == "island" ? Config::instance().screenGap() : 0);

#ifdef WITH_LAYER_SHELL
    if (isWayland()) { applyLayerShell(true); return; }
#endif
    move(x, y);
}

void NotchWindow::reinitForMode() {
    m_expanded = false;
    Config& cfg = Config::instance();
    setFixedSize(cfg.smallW(), cfg.smallH());
    positionWindow();
    update();
}

// ── Tray ─────────────────────────────────────────────────────────────────────

QString NotchWindow::trayStorageDir() {
    return QDir::homePath() + "/.local/share/pearos-notch/tray/";
}

void NotchWindow::loadTrayFromSettings() {
    QSettings s("pearos-notch", "tray");
    m_trayPaths = s.value("paths").toStringList();
    m_trayPaths.erase(std::remove_if(m_trayPaths.begin(), m_trayPaths.end(),
        [](const QString& p){ return !QFile::exists(p); }), m_trayPaths.end());
}

void NotchWindow::saveTrayToSettings() {
    QSettings s("pearos-notch", "tray");
    s.setValue("paths", m_trayPaths);
}

void NotchWindow::addToTray(const QString& path) {
    if (m_trayPaths.contains(path) || m_trayPaths.size() >= 5) return;
    if (!QFile::exists(path)) return;
    m_trayPaths.append(path);
    saveTrayToSettings();
}

void NotchWindow::removeFromTray(int index) {
    if (index < 0 || index >= m_trayPaths.size()) return;
    m_trayPaths.removeAt(index);
    if (m_hoveredTrayItem >= m_trayPaths.size()) m_hoveredTrayItem = -1;
    saveTrayToSettings();
    update();
}

void NotchWindow::startTrayItemDrag(int index) {
    if (index < 0 || index >= m_trayPaths.size()) return;

    QString path = m_trayPaths[index];
    QDrag* drag = new QDrag(this);
    QMimeData* mime = new QMimeData;
    mime->setUrls({QUrl::fromLocalFile(path)});
    drag->setMimeData(mime);

    QFileInfo fi(path);
    QString label = fi.fileName();
    QFont f; f.setPixelSize(11);
    QFontMetrics fm(f);
    int lw = qMin(fm.horizontalAdvance(label) + 16, 180);
    QPixmap pm(lw, 22);
    pm.fill(Qt::transparent);
    QPainter p(&pm);
    p.setRenderHint(QPainter::Antialiasing);
    p.setBrush(QColor(40, 40, 40, 220));
    p.setPen(Qt::NoPen);
    p.drawRoundedRect(pm.rect(), 5, 5);
    p.setPen(Qt::white);
    p.setFont(f);
    p.drawText(pm.rect().adjusted(8,0,-8,0), Qt::AlignVCenter|Qt::AlignLeft,
               fm.elidedText(label, Qt::ElideMiddle, lw - 16));
    drag->setPixmap(pm);

    m_dragLock = true;
    Qt::DropAction result = drag->exec(Qt::CopyAction | Qt::MoveAction, Qt::MoveAction);
    m_dragLock = false;

    if (result != Qt::IgnoreAction) {
        m_trayPaths.removeAt(index);
        saveTrayToSettings();
    }
    update();
}

// ── Events: hover / expand ────────────────────────────────────────────────────

void NotchWindow::enterEvent(QEnterEvent* e) {
    QWidget::enterEvent(e);
    if (!m_expanded) {
        m_hoverTimer->setInterval(Config::instance().hoverDelayMs());
        m_hoverTimer->start();
    }
    setCursor(Qt::ArrowCursor);
}

void NotchWindow::leaveEvent(QEvent* e) {
    QWidget::leaveEvent(e);
    m_hoverTimer->stop();
    m_hoveredTrayItem = -1;

    if (m_dragLock || m_incomingDrag) return;

    if (m_expanded) {
        m_expanded = false;
        Config& cfg = Config::instance();
        startSizeAnim(QSize(cfg.smallW(), cfg.smallH()), false);
    }
    setCursor(Qt::BlankCursor);
}

void NotchWindow::onHoverExpand() {
    if (m_expanded) return;
    m_expanded = true;
    Config& cfg = Config::instance();
    startSizeAnim(QSize(cfg.bigW(), cfg.bigH()), true);
}

void NotchWindow::mouseMoveEvent(QMouseEvent* e) {
    QWidget::mouseMoveEvent(e);

    if (!m_expanded || m_currentTab != Tab::Tray) {
        if (m_hoveredTrayItem != -1) { m_hoveredTrayItem = -1; update(); }
        if (m_pressedTrayItem >= 0 && (e->buttons() & Qt::LeftButton)) {
            if ((e->pos() - m_dragStartPos).manhattanLength() > 8) {
                int idx = m_pressedTrayItem;
                m_pressedTrayItem = -1;
                startTrayItemDrag(idx);
            }
        }
        return;
    }

    int prev = m_hoveredTrayItem;
    m_hoveredTrayItem = -1;
    for (int i = 0; i < m_trayPaths.size(); i++) {
        if (trayItemRect(i, width(), height()).contains(e->pos())) {
            m_hoveredTrayItem = i; break;
        }
    }
    if (m_hoveredTrayItem != prev) update();

    if (m_pressedTrayItem >= 0 && (e->buttons() & Qt::LeftButton)) {
        if ((e->pos() - m_dragStartPos).manhattanLength() > 8) {
            int idx = m_pressedTrayItem;
            m_pressedTrayItem = -1;
            startTrayItemDrag(idx);
        }
    }
}

void NotchWindow::mouseReleaseEvent(QMouseEvent* e) {
    QWidget::mouseReleaseEvent(e);
    m_pressedTrayItem = -1;
}

// ── Events: clicks ────────────────────────────────────────────────────────────

void NotchWindow::mousePressEvent(QMouseEvent* e) {
    if (!m_expanded) { QWidget::mousePressEvent(e); return; }

    QPoint pos = e->pos();
    int w = width(), h = height();

    if (e->button() == Qt::LeftButton) {
        for (int t = 0; t < 3; t++) {
            if (tabIconRect((Tab)t, w).contains(pos)) {
                m_currentTab = (Tab)t;
                update();
                return;
            }
        }

        if (settingsIconRect(w).contains(pos)) {
            if (m_settingsDlg) {
                m_settingsDlg->show();
                m_settingsDlg->raise();
                m_settingsDlg->activateWindow();
            }
            return;
        }

        if (m_currentTab == Tab::Tray) {
            for (int i = 0; i < m_trayPaths.size(); i++) {
                if (trayItemCloseRect(i, w, h).contains(pos)) {
                    removeFromTray(i);
                    return;
                }
            }
            for (int i = 0; i < m_trayPaths.size(); i++) {
                if (trayItemRect(i, w, h).contains(pos)) {
                    m_pressedTrayItem = i;
                    m_dragStartPos    = pos;
                    return;
                }
            }
        }

        if (m_currentTab == Tab::Music) {
            if (prevButtonRect(w, h).contains(pos)) { m_mpris->previous(); return; }
            if (playButtonRect(w, h).contains(pos)) { m_mpris->playPause(); return; }
            if (nextButtonRect(w, h).contains(pos)) { m_mpris->next();      return; }

            const MediaInfo& info = m_mpris->info();
            if (info.lengthUs > 0) {
                auto L = expandedLayout(w, h);
                QRectF barRect(L.textX + 16.0, L.barY, L.textW - 32.0, 8.0);
                if (barRect.contains(pos)) {
                    double frac = qBound(0.0, (pos.x() - barRect.left()) / barRect.width(), 1.0);
                    qint64 seekUs = (qint64)(frac * info.lengthUs);
                    QDBusInterface dbus("org.freedesktop.DBus", "/",
                                        "org.freedesktop.DBus", QDBusConnection::sessionBus());
                    QDBusReply<QStringList> reply = dbus.call("ListNames");
                    QString player;
                    if (reply.isValid())
                        for (const QString& n : reply.value())
                            if (n.startsWith("org.mpris.MediaPlayer2.")) { player = n; break; }
                    if (!player.isEmpty()) {
                        QDBusConnection::sessionBus().call(
                            QDBusMessage::createMethodCall(player, "/org/mpris/MediaPlayer2",
                                "org.mpris.MediaPlayer2.Player", "SetPosition")
                            << QVariant::fromValue(QDBusObjectPath("/org/mpris/MediaPlayer2/Track/0"))
                            << seekUs, QDBus::NoBlock);
                    }
                    return;
                }
            }
        }
    }

    QWidget::mousePressEvent(e);
}

void NotchWindow::contextMenuEvent(QContextMenuEvent* e) {
    if (!m_expanded || m_currentTab != Tab::Tray) return;

    QPoint pos = e->pos();
    int w = width(), h = height();
    for (int i = 0; i < m_trayPaths.size(); i++) {
        if (trayItemRect(i, w, h).contains(pos)) {
            const QString path = m_trayPaths[i];
            QMenu menu(this);
            menu.addAction("Show in Files", this, [path](){
                QProcess::startDetached("nautilus", {"--select", path});
            });
            menu.addSeparator();
            menu.addAction("Remove from Tray", this, [this, i](){
                removeFromTray(i);
            });
            menu.exec(e->globalPos());
            return;
        }
    }
}

// ── Events: drag-and-drop ─────────────────────────────────────────────────────

void NotchWindow::dragEnterEvent(QDragEnterEvent* e) {
    if (!e->mimeData()->hasUrls()) return;
    m_incomingDrag = true;
    if (!m_expanded) {
        m_hoverTimer->stop();
        onHoverExpand();
    }
    m_currentTab = Tab::Tray;
    e->acceptProposedAction();
    update();
}

void NotchWindow::dragLeaveEvent(QDragLeaveEvent* e) {
    QWidget::dragLeaveEvent(e);
    m_incomingDrag = false;
    QTimer::singleShot(200, this, [this](){
        if (!underMouse() && !m_dragLock) {
            m_expanded = false;
            Config& cfg = Config::instance();
            startSizeAnim(QSize(cfg.smallW(), cfg.smallH()), false);
        }
    });
}

void NotchWindow::dragMoveEvent(QDragMoveEvent* e) {
    if (e->mimeData()->hasUrls())
        e->acceptProposedAction();
}

void NotchWindow::dropEvent(QDropEvent* e) {
    m_incomingDrag = false;
    for (const QUrl& url : e->mimeData()->urls()) {
        if (url.isLocalFile())
            addToTray(url.toLocalFile());
    }
    e->acceptProposedAction();
    update();
}

// ── MPRIS callbacks ───────────────────────────────────────────────────────────

void NotchWindow::onMprisChanged() {
    bool playing = m_mpris->info().playing;
    qDebug() << "[Notch] onMprisChanged playing=" << playing
             << "title=" << m_mpris->info().title.left(40);
    startVizAnim(playing);
    update();
}

void NotchWindow::onPositionChanged(qint64) { update(); }

// ── Layout helpers ────────────────────────────────────────────────────────────

NotchWindow::ExpandedLayout NotchWindow::expandedLayout(int w, int h) const {
    constexpr double ART_M   = 10.0;
    constexpr double ART_PAD = 12.0;
    constexpr double PAD_V   = 8.0;

    ExpandedLayout L;
    double contentTop = TAB_H + PAD_V;
    double contentBot = (double)h - PAD_V;
    L.artSize = qMin(contentBot - contentTop, 80.0);
    L.artX    = ART_M;
    L.artY    = contentTop + (contentBot - contentTop - L.artSize) / 2.0;
    L.textX   = L.artX + L.artSize + ART_PAD;
    L.textW   = (double)w - L.textX - 30.0;

    double sH    = (contentBot - contentTop) / 4.0;
    L.titleY  = contentTop + sH * 0.70;
    L.artistY = contentTop + sH * 1.70;
    L.barY    = contentTop + sH * 2.55;
    L.btnCY   = contentTop + sH * 3.55;
    return L;
}

QRectF NotchWindow::tabIconRect(Tab t, int w) const {
    constexpr double SZ  = 16.0;
    constexpr double GAP = 20.0;
    double totalW = 3 * SZ + 2 * GAP;
    double startX = (w - totalW) / 2.0;
    double y      = (TAB_H - SZ) / 2.0;
    return QRectF(startX + (int)t * (SZ + GAP), y, SZ, SZ);
}

QRectF NotchWindow::settingsIconRect(int w) const {
    constexpr double SZ = 16.0;
    return QRectF(w - SZ - 8.0, (TAB_H - SZ) / 2.0, SZ, SZ);
}

QRectF NotchWindow::trayItemRect(int i, int w, int h) const {
    double top  = TAB_H + 4.0;
    double avail = h - top - 4.0;
    double rowH = qMax(16.0, avail / 5.0);
    return QRectF(8.0, top + i * rowH, w - 16.0, rowH);
}

QRectF NotchWindow::trayItemCloseRect(int i, int w, int h) const {
    QRectF row = trayItemRect(i, w, h);
    double sz  = 14.0;
    return QRectF(row.right() - sz - 2.0,
                  row.center().y() - sz / 2.0, sz, sz);
}

static void btnGeom(int w, int h, double& bx, double& by, double& SZ, double& GAP,
                    double tabH) {
    constexpr double ART_M   = 10.0;
    constexpr double ART_PAD = 12.0;
    constexpr double PAD_V   = 8.0;
    SZ = 22.0; GAP = 14.0;
    double artSize = qMin((double)h - tabH - 2*PAD_V, 80.0);
    double textX   = ART_M + artSize + ART_PAD;
    double textW   = (double)w - textX - 30.0;
    double contentTop = tabH + PAD_V;
    double contentBot = (double)h - PAD_V;
    double sH      = (contentBot - contentTop) / 4.0;
    double btnCY   = contentTop + sH * 3.55;
    double totalW  = 3*SZ + 2*GAP;
    bx = textX + (textW - totalW) / 2.0;
    by = btnCY - SZ / 2.0;
}

QRectF NotchWindow::prevButtonRect(int w, int h) const {
    double bx, by, SZ, GAP;
    btnGeom(w, h, bx, by, SZ, GAP, TAB_H);
    return QRectF(bx, by, SZ, SZ);
}
QRectF NotchWindow::playButtonRect(int w, int h) const {
    double bx, by, SZ, GAP;
    btnGeom(w, h, bx, by, SZ, GAP, TAB_H);
    return QRectF(bx+SZ+GAP, by, SZ, SZ);
}
QRectF NotchWindow::nextButtonRect(int w, int h) const {
    double bx, by, SZ, GAP;
    btnGeom(w, h, bx, by, SZ, GAP, TAB_H);
    return QRectF(bx+2*(SZ+GAP), by, SZ, SZ);
}

// ── Drawing ───────────────────────────────────────────────────────────────────

QPainterPath NotchWindow::notchShape(int w, int h) {
    const QString& mode = Config::instance().mode();
    if (mode == "island") {
        QPainterPath p; p.addRoundedRect(0, 0, w, h, h/2.0, h/2.0); return p;
    }
    double r = qMin(15.0, qMin(h*0.45, w*0.45));
    QPainterPath p;
    p.moveTo(0, 0); p.lineTo(w, 0);
    p.lineTo(w, h-r); p.arcTo(w-2*r, h-2*r, 2*r, 2*r, 0, -90);
    p.lineTo(r, h);   p.arcTo(0, h-2*r, 2*r, 2*r, 270, -90);
    p.closeSubpath(); return p;
}

void NotchWindow::drawSvgWhite(QPainter& p, QSvgRenderer* svg, const QRectF& rect, double opacity) {
    if (!svg || !svg->isValid() || rect.width() < 1 || rect.height() < 1) return;
    double prev = p.opacity();
    p.setOpacity(prev * opacity);
    svg->render(&p, rect);
    p.setOpacity(prev);
}

void NotchWindow::drawTabBar(QPainter& p, int w, int /*h*/) {
    p.setPen(QPen(QColor(60, 60, 60), 0.5));
    p.drawLine(QPointF(10, TAB_H - 0.5), QPointF(w - 10, TAB_H - 0.5));
    p.setPen(Qt::NoPen);

    struct { Tab t; QSvgRenderer* svg; } tabs[] = {
        { Tab::Music,   m_svgHome     },
        { Tab::Tray,    m_svgTrayIcon },
        { Tab::Browser, m_svgBrowser  },
    };
    for (auto& [t, svg] : tabs) {
        bool active = (m_currentTab == t);
        drawSvgWhite(p, svg, tabIconRect(t, w), active ? 1.0 : 0.40);
        if (active) {
            QRectF r = tabIconRect(t, w);
            p.setBrush(QColor(255, 255, 255, 200));
            p.drawEllipse(QPointF(r.center().x(), TAB_H - 3.0), 2.0, 2.0);
        }
    }
    drawSvgWhite(p, m_svgSettings, settingsIconRect(w));
}

void NotchWindow::drawVisualizer(QPainter& p, int w, int h) {
    if (m_vizAlpha < 0.01) return;

    constexpr double BAR_W   = 3.0;
    constexpr double BAR_GAP = 3.0;
    constexpr double MARGIN  = 12.0;
    constexpr int    COUNT   = 4;
    const double PHASES[4]   = {0.0, 1.57, 3.14, 4.71};

    double t   = QDateTime::currentMSecsSinceEpoch() / 1000.0;
    double cy  = h / 2.0;
    double amp = (h / 2.0 - 4.0) * 0.85;

    for (int i = 0; i < COUNT; i++) {
        double v  = 0.5 + 0.5 * qSin(t * 3.5 + PHASES[i]);
        double bh = BAR_W + amp * v;
        double rx = BAR_W / 2.0;
        double cx = w - MARGIN - (COUNT - 1 - i) * (BAR_W + BAR_GAP) - rx;

        QPainterPath bar;
        bar.addRoundedRect(cx - rx, cy - bh/2.0, BAR_W, bh, rx, rx);
        p.fillPath(bar, QColor(200, 200, 200, (int)(m_vizAlpha * 220)));
    }
}

void NotchWindow::drawAlbumArt(QPainter& p, int w, int h) {
    const QPixmap& art = m_mpris->info().art;
    if (art.isNull()) return;

    double artX, artY, artSize, r;
    if (isExpanded() && m_currentTab == Tab::Music) {
        auto L = expandedLayout(w, h);
        artX = L.artX; artY = L.artY; artSize = L.artSize; r = 8.0;
    } else if (!isExpanded()) {
        artSize = 18.0; artX = 8.0; artY = (h - artSize)/2.0; r = 3.0;
    } else {
        return;
    }

    QPainterPath clip;
    clip.addRoundedRect(artX, artY, artSize, artSize, r, r);
    p.save();
    p.setClipPath(clip);
    p.drawPixmap(QRectF(artX, artY, artSize, artSize), art, art.rect());
    p.restore();
}

void NotchWindow::drawMusicContent(QPainter& p, int w, int h) {
    const MediaInfo& info = m_mpris->info();
    auto L = expandedLayout(w, h);

    if (!info.title.isEmpty()) {
        QFont f = p.font(); f.setBold(true); f.setPointSizeF(8.5);
        p.setFont(f); p.setPen(QColor(240, 240, 240));
        QFontMetricsF fm(f);
        p.drawText(QPointF(L.textX, L.titleY),
                   fm.elidedText(info.title, Qt::ElideRight, L.textW));
    }

    if (!info.artist.isEmpty()) {
        QFont f = p.font(); f.setBold(false); f.setPointSizeF(7.0);
        p.setFont(f); p.setPen(QColor(145, 145, 145));
        QFontMetricsF fm(f);
        p.drawText(QPointF(L.textX, L.artistY),
                   fm.elidedText(info.artist, Qt::ElideRight, L.textW));
    }

    // Progress bar — inset 16px each side
    {
        constexpr double BAR_H  = 3.0;
        constexpr double INSET  = 16.0;
        double barX = L.textX + INSET;
        double barW = L.textW - 2.0 * INSET;
        p.setPen(Qt::NoPen);
        p.setBrush(QColor(55, 55, 55));
        p.drawRoundedRect(QRectF(barX, L.barY, barW, BAR_H), 1.5, 1.5);

        if (info.lengthUs > 0) {
            double frac = qBound(0.0, (double)info.posUs / info.lengthUs, 1.0);
            if (frac > 0) {
                p.setBrush(QColor(200, 200, 200));
                p.drawRoundedRect(QRectF(barX, L.barY, barW * frac, BAR_H), 1.5, 1.5);
            }
        }
    }

    // Buttons
    {
        constexpr double SZ = 22.0, GAP = 14.0;
        double total = 3*SZ + 2*GAP;
        double bx = L.textX + (L.textW - total) / 2.0;
        double by = L.btnCY - SZ / 2.0;
        QSvgRenderer* playIcon = info.playing ? m_svgPause : m_svgPlay;
        drawSvgWhite(p, m_svgPrev, QRectF(bx,           by, SZ, SZ));
        drawSvgWhite(p, playIcon,  QRectF(bx+SZ+GAP,    by, SZ, SZ));
        drawSvgWhite(p, m_svgNext, QRectF(bx+2*(SZ+GAP),by, SZ, SZ));
    }
}

void NotchWindow::drawTrayContent(QPainter& p, int w, int h) {
    if (m_trayPaths.isEmpty()) {
        if (m_incomingDrag) {
            p.setPen(QPen(QColor(100, 160, 255, 180), 1.5, Qt::DashLine));
            p.setBrush(QColor(100, 160, 255, 25));
            p.drawRoundedRect(QRectF(10, TAB_H + 4, w - 20, h - TAB_H - 8), 6, 6);
            p.setPen(QColor(100, 160, 255, 180));
        } else {
            p.setPen(QColor(80, 80, 80));
        }
        QFont f = p.font(); f.setPointSizeF(7.5); p.setFont(f);
        p.drawText(QRectF(10, TAB_H, w-20, h-TAB_H),
                   Qt::AlignCenter, "Drop files here\n(max 5)");
        return;
    }

    for (int i = 0; i < m_trayPaths.size(); i++) {
        QRectF row = trayItemRect(i, w, h);
        QFileInfo fi(m_trayPaths[i]);
        bool hovered = (m_hoveredTrayItem == i);

        if (hovered) {
            p.setPen(Qt::NoPen);
            p.setBrush(QColor(255, 255, 255, 18));
            p.drawRoundedRect(row.adjusted(2, 1, -2, -1), 4, 4);
        }

        QString ext = fi.suffix().toUpper();
        QString extLabel = ext.isEmpty() ? "FILE" : ext.left(4);
        constexpr double PILL_W = 34.0, PILL_H = 14.0;
        double pillX = row.left() + 6.0;
        double pillY = row.center().y() - PILL_H / 2.0;
        p.setPen(Qt::NoPen);
        p.setBrush(extensionColor(ext));
        p.drawRoundedRect(QRectF(pillX, pillY, PILL_W, PILL_H), 3, 3);

        QFont fe; fe.setPixelSize(8); fe.setBold(true);
        p.setFont(fe); p.setPen(Qt::white);
        p.drawText(QRectF(pillX, pillY, PILL_W, PILL_H), Qt::AlignCenter, extLabel);

        QFont fn; fn.setPointSizeF(7.5); p.setFont(fn);
        p.setPen(hovered ? QColor(235, 235, 235) : QColor(200, 200, 200));
        double nameX = pillX + PILL_W + 8.0;
        double nameW = row.right() - nameX - (hovered ? 22.0 : 6.0);
        QFontMetricsF fm(fn);
        p.drawText(QRectF(nameX, row.top(), nameW, row.height()),
                   Qt::AlignVCenter | Qt::AlignLeft,
                   fm.elidedText(fi.fileName(), Qt::ElideMiddle, nameW));

        if (hovered) {
            QRectF xr = trayItemCloseRect(i, w, h);
            p.setPen(Qt::NoPen);
            p.setBrush(QColor(180, 60, 60, 180));
            p.drawRoundedRect(xr, 3, 3);
            QFont xf; xf.setPixelSize(11); xf.setBold(true);
            p.setFont(xf); p.setPen(Qt::white);
            p.drawText(xr, Qt::AlignCenter, "×");
        }
    }

    if (m_incomingDrag && m_trayPaths.size() < 5) {
        p.setPen(QPen(QColor(100, 160, 255, 120), 1.0, Qt::DashLine));
        p.setBrush(Qt::NoBrush);
        double lastBottom = trayItemRect(m_trayPaths.size()-1, w, h).bottom();
        p.drawRoundedRect(QRectF(10, lastBottom + 2, w - 20,
                                 h - lastBottom - 6), 4, 4);
    }
}

void NotchWindow::drawBrowserContent(QPainter& p, int w, int h) {
    p.setPen(QColor(80, 80, 80));
    QFont f = p.font(); f.setPointSizeF(7.5); p.setFont(f);
    p.drawText(QRectF(0, TAB_H, w, h - TAB_H), Qt::AlignCenter,
               "Browser coming soon");
}

void NotchWindow::paintEvent(QPaintEvent*) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    int w = width(), h = height();

    p.setCompositionMode(QPainter::CompositionMode_Source);
    p.fillRect(rect(), Qt::transparent);
    p.setCompositionMode(QPainter::CompositionMode_SourceOver);

    p.setPen(Qt::NoPen);
    p.setBrush(QColor(0, 0, 0));
    p.drawPath(notchShape(w, h));

    p.setClipPath(notchShape(w, h));

    drawAlbumArt(p, w, h);
    drawVisualizer(p, w, h);

    if (isExpanded()) {
        drawTabBar(p, w, h);
        switch (m_currentTab) {
        case Tab::Music:   drawMusicContent(p, w, h);   break;
        case Tab::Tray:    drawTrayContent(p, w, h);    break;
        case Tab::Browser: drawBrowserContent(p, w, h); break;
        }
    }
}
