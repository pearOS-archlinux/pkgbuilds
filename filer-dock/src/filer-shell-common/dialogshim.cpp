#include "dialogshim.h"

#include <QGuiApplication>
#include <QQuickItem>
#include <QScreen>

#include <KWindowEffects>

DialogShim::DialogShim(QWindow *parent)
    : QQuickWindow(parent)
{
    setFlags(Qt::FramelessWindowHint | Qt::Tool);
    setColor(QColor(Qt::transparent));

    // Real compositor blur-behind (same KWindowEffects::enableBlurBehind()
    // API Filer's own file manager uses for its sidebar and QMenus -- see
    // mainwindow.cpp's updateSidebarBlurRegion() / application.cpp's
    // eventFilter()), layered underneath this window's own painted "glass"
    // tint/bevel (Card.qml's glassEffect) so popups like PearControlCentre
    // get real live-blurred desktop content behind their translucent
    // buttons, not just a static gradient standing in for one.
    connect(this, &QWindow::visibleChanged, this, [this](bool visible) {
        if (visible) {
            reposition();
            updateBlurRegion();
        }
    });

    connect(this, &QWindow::activeChanged, this, [this]() {
        if (m_hideOnWindowDeactivate && !isActive() && isVisible()) {
            setVisible(false);
        }
    });
}

void DialogShim::setMainItem(QQuickItem *item)
{
    if (m_mainItem == item) {
        return;
    }
    m_mainItem = item;
    if (item) {
        item->setParentItem(contentItem());
        // Size the window to the content, same as real Dialog.
        connect(item, &QQuickItem::widthChanged, this, [this, item] { resize(item->width(), height()); });
        connect(item, &QQuickItem::heightChanged, this, [this, item] { resize(width(), item->height()); });
        resize(item->width() > 0 ? item->width() : 1, item->height() > 0 ? item->height() : 1);
    }
    emit mainItemChanged();
}

void DialogShim::setVisualParent(QQuickItem *item)
{
    if (m_visualParent == item) {
        return;
    }
    m_visualParent = item;
    emit visualParentChanged();
    if (isVisible()) {
        reposition();
    }
}

void DialogShim::setLocation(int location)
{
    if (m_location == location) {
        return;
    }
    m_location = location;
    emit locationChanged();
}

void DialogShim::setBackgroundHints(int hints)
{
    if (m_backgroundHints == hints) {
        return;
    }
    m_backgroundHints = hints;
    emit backgroundHintsChanged();
}

void DialogShim::registerGlassRect(QObject *token, const QRectF &rect)
{
    if (!m_glassRects.contains(token)) {
        connect(token, &QObject::destroyed, this, [this, token] {
            m_glassRects.remove(token);
            updateBlurRegion();
        });
    }
    m_glassRects[token] = rect;
    updateBlurRegion();
}

void DialogShim::unregisterGlassRect(QObject *token)
{
    if (m_glassRects.remove(token) > 0) {
        updateBlurRegion();
    }
}

void DialogShim::updateBlurRegion()
{
    if (!isVisible()) {
        return;
    }
    QRegion region;
    for (const QRectF &rect : std::as_const(m_glassRects)) {
        region += rect.toRect();
    }
    KWindowEffects::enableBlurBehind(this, !region.isEmpty(), region);
}

// Opens downward from visualParent for a top-anchored host (TopEdge, the
// only case filer-topbar actually needs), upward for a bottom-anchored one
// (BottomEdge, filer-dock's case) -- clamped so it doesn't run off the
// screen edge horizontally. Plasma::Types::Location values: TopEdge=3,
// BottomEdge=4 (see filer-shell-common/CMakeLists.txt's header comment for
// where these come from).
void DialogShim::reposition()
{
    if (!m_visualParent) {
        return;
    }
    QScreen *screen = m_visualParent->window() ? m_visualParent->window()->screen() : QGuiApplication::primaryScreen();
    if (!screen) {
        return;
    }
    const QPointF topLeft = m_visualParent->mapToGlobal(QPointF(0, 0));
    const QRect screenGeom = screen->geometry();

    int x = static_cast<int>(topLeft.x());
    int y;
    if (m_location == 4) { // BottomEdge: open upward, above the anchor
        y = static_cast<int>(topLeft.y()) - height();
    } else { // TopEdge (and default): open downward, below the anchor
        y = static_cast<int>(topLeft.y()) + m_visualParent->height();
    }

    // Keep entirely on-screen horizontally.
    x = qBound(screenGeom.left(), x, screenGeom.right() - width());
    y = qBound(screenGeom.top(), y, screenGeom.bottom() - height());

    setPosition(x, y);
}
