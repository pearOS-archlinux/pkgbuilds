/*
 * Copyright (C) 2018 Vlad Zagorodniy <vladzzag@gmail.com>
 * Copyright (C) 2026 Roy Bock <dev@si13n7.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

// Own
#include "PearosMagicLampEffect.h"
#include "Model.h"
#include "kwincompat.h"

// Auto-generated
#include "PearosMagicLampConfig.h"

// kwineffects
#include <core/renderviewport.h>

// Qt
#include <QTimer>

// std
#include <cmath>

// Returns which screen edge a dock panel is closest to.
static Direction dockEdge(KWin::EffectWindow* dock)
{
    KWin::LogicalOutput* screen = KWin::effects->screenAt(
        dock->frameGeometry().center().toPoint());
    const QRectF screenRect = KWinCompat::fullScreenArea(KWin::effects, screen);
    const QPointF c = dock->frameGeometry().center();

    const qreal l = qAbs(c.x() - screenRect.left());
    const qreal t = qAbs(c.y() - screenRect.top());
    const qreal r = qAbs(screenRect.right() - c.x());
    const qreal b = qAbs(screenRect.bottom() - c.y());
    const qreal m = qMin(qMin(l, t), qMin(r, b));

    if (qFuzzyCompare(l, m))
        return Direction::Left;
    if (qFuzzyCompare(t, m))
        return Direction::Top;
    if (qFuzzyCompare(r, m))
        return Direction::Right;
    return Direction::Bottom;
}

// Returns the icon geometry on the screen that contains the window's center.
// When iconGeometry() points to a panel on a different screen (race condition
// with multiple task managers writing _NET_WM_ICON_GEOMETRY), the icon is
// remapped to the correct screen.
//
// Screen identity is determined via dock window pointers (always in global
// coordinates) rather than screenAt(iconRect.center()), which is unreliable
// when iconGeometry() uses screen-local coordinates (as on Wayland).
// When the target screen has multiple panels, the panel on the same screen
// edge as the source panel is preferred (picks task bar over status bar).
// Position is translated by the screen-origin offset so that an icon at local
// position (x, y) within the source panel maps to the same local position in
// the target panel — exact for identical panel layouts, reasonable otherwise.
static QRectF resolveIconGeometry(KWin::EffectWindow* w)
{
    const QRectF iconRect = w->iconGeometry();

    // Window center determines the authoritative screen (robust against decorations
    // extending a few pixels into an adjacent screen).
    KWin::LogicalOutput* primaryScreen = KWin::effects->screenAt(
        w->frameGeometry().center().toPoint());
    if (!primaryScreen) {
        return iconRect;
    }

    // Find which dock (panel) the iconGeometry falls into.
    // Dock frameGeometry is always in global coordinates, making this reliable
    // regardless of whether iconGeometry itself uses global or screen-local coords.
    KWin::EffectWindow* sourceDock = nullptr;
    for (KWin::EffectWindow* candidate : KWin::effects->stackingOrder()) {
        if (!candidate->isDock()) {
            continue;
        }
        // Use intersects, not contains: Plasma sometimes publishes icon geometry
        // slightly larger than the panel (KDE's own Magic Lamp has the same note).
        if (candidate->frameGeometry().intersects(iconRect)) {
            sourceDock = candidate;
            break;
        }
    }

    // If the dock containing the icon is already on the primary screen, the
    // iconGeometry is correct — use it directly.
    if (sourceDock) {
        KWin::LogicalOutput* sourceDockScreen = KWin::effects->screenAt(
            sourceDock->frameGeometry().center().toPoint());
        if (sourceDockScreen == primaryScreen) {
            return iconRect;
        }
    }

    // Icon is on the wrong screen. Find the best-matching dock on primaryScreen:
    // prefer the dock on the same edge as the source (so we pick the task bar,
    // not an unrelated status bar on screens with multiple panels).
    // Determine which edge of the target screen the task-manager panel is on.
    // Strategy: use screen-adjacency to mirror the source panel's relative position.
    //   - Find which edge of the source screen faces the target screen (srcFacingEdge).
    //   - Find which edge of the target screen faces the source screen (tgtFacingEdge).
    //   - If source panel is on srcFacingEdge  → target panel is on tgtFacingEdge.
    //   - If source panel is on the opposite edge (non-facing) → target panel is on
    //     the opposite of tgtFacingEdge (i.e. the non-facing edge of the target screen).
    //
    // Example: upper screen (source) has task bar at TOP (non-facing, away from lower screen).
    //   srcFacingEdge=Bottom, tgtFacingEdge=Top, sourceDockEdge=Top ≠ Bottom
    //   → preferredEdge = opposite(Top) = Bottom  → picks the BOTTOM panel of lower screen ✓
    Direction preferredEdge = Direction::Bottom;
    if (sourceDock) {
        KWin::LogicalOutput* srcScreen = KWin::effects->screenAt(
            sourceDock->frameGeometry().center().toPoint());
        const QRectF srcRect = KWinCompat::fullScreenArea(KWin::effects, srcScreen);
        const QRectF tgtRect = KWinCompat::fullScreenArea(KWin::effects, primaryScreen);

        const auto closestEdge = [](const QRectF& rect, const QPointF& p) -> Direction {
            const qreal l = qAbs(p.x() - rect.left());
            const qreal t = qAbs(p.y() - rect.top());
            const qreal r = qAbs(rect.right() - p.x());
            const qreal b = qAbs(rect.bottom() - p.y());
            const qreal m = qMin(qMin(l, t), qMin(r, b));
            if (qFuzzyCompare(l, m))
                return Direction::Left;
            if (qFuzzyCompare(t, m))
                return Direction::Top;
            if (qFuzzyCompare(r, m))
                return Direction::Right;
            return Direction::Bottom;
        };
        const auto opposite = [](Direction d) -> Direction {
            switch (d) {
            case Direction::Top:
                return Direction::Bottom;
            case Direction::Bottom:
                return Direction::Top;
            case Direction::Left:
                return Direction::Right;
            default:
                return Direction::Left;
            }
        };

        const Direction srcFacingEdge = closestEdge(srcRect, tgtRect.center());
        const Direction tgtFacingEdge = closestEdge(tgtRect, srcRect.center());
        const Direction sourceDockEdge = dockEdge(sourceDock);
        preferredEdge = (sourceDockEdge == srcFacingEdge) ? tgtFacingEdge : opposite(tgtFacingEdge);
    }

    KWin::EffectWindow* targetDock = nullptr;
    KWin::EffectWindow* fallbackDock = nullptr;
    for (KWin::EffectWindow* candidate : KWin::effects->stackingOrder()) {
        if (!candidate->isDock()) {
            continue;
        }
        if (KWin::effects->screenAt(candidate->frameGeometry().center().toPoint()) != primaryScreen) {
            continue;
        }
        if (dockEdge(candidate) == preferredEdge) {
            targetDock = candidate;
            break;
        }
        if (!fallbackDock) {
            fallbackDock = candidate;
        }
    }
    if (!targetDock) {
        targetDock = fallbackDock;
    }
    if (!targetDock) {
        return iconRect;
    }

    // Translate the icon by the offset between the two screens' origins so that
    // a button at local position (lx, ly) within the source panel appears at the
    // same local position within the target panel.
    // Falls back to dock center when sourceDock is unknown.
    if (!sourceDock) {
        const QRectF tgt = targetDock->frameGeometry();
        return QRectF(tgt.center().x() - iconRect.width() / 2,
            tgt.center().y() - iconRect.height() / 2,
            iconRect.width(), iconRect.height());
    }

    KWin::LogicalOutput* sourceDockScreen = KWin::effects->screenAt(
        sourceDock->frameGeometry().center().toPoint());
    const QRectF srcScreenRect = KWinCompat::fullScreenArea(KWin::effects, sourceDockScreen);
    const QRectF tgtScreenRect = KWinCompat::fullScreenArea(KWin::effects, primaryScreen);

    const QPointF screenOffset = tgtScreenRect.topLeft() - srcScreenRect.topLeft();
    const QRectF translatedRect = iconRect.translated(screenOffset);

    // Clamp to the target dock so the icon stays within the panel.
    const QRectF tgt = targetDock->frameGeometry();
    const qreal cx = qBound(tgt.left() + iconRect.width() / 2,
        translatedRect.center().x(),
        tgt.right() - iconRect.width() / 2);
    const qreal cy = qBound(tgt.top() + iconRect.height() / 2,
        translatedRect.center().y(),
        tgt.bottom() - iconRect.height() / 2);
    return QRectF(cx - iconRect.width() / 2, cy - iconRect.height() / 2,
        iconRect.width(), iconRect.height());
}

enum ShapeCurve {
    Linear = 0,
    Quad = 1,
    Cubic = 2,
    Quart = 3,
    Quint = 4,
    Sine = 5,
    Circ = 6,
    Bounce = 7,
    Bezier = 8
};

PearosMagicLampEffect::PearosMagicLampEffect()
{
    reconfigure(ReconfigureAll);

    for (KWin::EffectWindow* w : KWin::effects->stackingOrder()) {
        slotWindowAdded(w);
    }

    connect(KWin::effects, &KWin::EffectsHandler::windowAdded,
        this, &PearosMagicLampEffect::slotWindowAdded);
    connect(KWin::effects, &KWin::EffectsHandler::windowDeleted,
        this, &PearosMagicLampEffect::slotWindowDeleted);
    connect(KWin::effects, &KWin::EffectsHandler::activeFullScreenEffectChanged,
        this, &PearosMagicLampEffect::slotActiveFullScreenEffectChanged);
}

PearosMagicLampEffect::~PearosMagicLampEffect()
{
}

void PearosMagicLampEffect::reconfigure(ReconfigureFlags flags)
{
    Q_UNUSED(flags)

    PearosMagicLampConfig::self()->read();

    QEasingCurve curve;
    const auto shapeCurve = static_cast<ShapeCurve>(PearosMagicLampConfig::shapeCurve());
    switch (shapeCurve) {
    case ShapeCurve::Linear:
        curve.setType(QEasingCurve::Linear);
        break;

    case ShapeCurve::Quad:
        curve.setType(QEasingCurve::InOutQuad);
        break;

    case ShapeCurve::Cubic:
        curve.setType(QEasingCurve::InOutCubic);
        break;

    case ShapeCurve::Quart:
        curve.setType(QEasingCurve::InOutQuart);
        break;

    case ShapeCurve::Quint:
        curve.setType(QEasingCurve::InOutQuint);
        break;

    case ShapeCurve::Sine:
        curve.setType(QEasingCurve::InOutSine);
        break;

    case ShapeCurve::Circ:
        curve.setType(QEasingCurve::InOutCirc);
        break;

    case ShapeCurve::Bounce:
        curve.setType(QEasingCurve::InOutBounce);
        break;

    case ShapeCurve::Bezier:
        // With the cubic bezier curve, "0" corresponds to the furtherst edge
        // of a window, "1" corresponds to the closest edge.
        curve.setType(QEasingCurve::BezierSpline);
        curve.addCubicBezierSegment(
            QPointF(0.3, 0.0),
            QPointF(0.7, 1.0),
            QPointF(1.0, 1.0));
        break;
    default:
        // Fallback to the sine curve.
        curve.setType(QEasingCurve::InOutSine);
        break;
    }
    m_modelParameters.shapeCurve = curve;

    const auto baseDuration = KWinCompat::toMilliseconds(
        animationTime<PearosMagicLampConfig>(std::chrono::milliseconds(250)));
    const auto configuredSquashDuration = PearosMagicLampConfig::squashDuration();
    m_modelParameters.squashDuration = configuredSquashDuration > 0
        ? std::chrono::milliseconds(configuredSquashDuration)
        : baseDuration;
    const auto configuredStretchDuration = PearosMagicLampConfig::stretchDuration();
    m_modelParameters.stretchDuration = configuredStretchDuration > 0
        ? std::chrono::milliseconds(configuredStretchDuration)
        : std::chrono::milliseconds(qMax(qRound(baseDuration.count() * 0.7), 1));
    const auto configuredUnminimizeStretchDuration = PearosMagicLampConfig::unminimizeStretchDuration();
    m_modelParameters.unminimizeStretchDuration = configuredUnminimizeStretchDuration > 0
        ? std::chrono::milliseconds(configuredUnminimizeStretchDuration)
        : m_modelParameters.stretchDuration;
    const auto configuredBumpDuration = PearosMagicLampConfig::bumpDuration();
    m_modelParameters.bumpDuration = configuredBumpDuration > 0
        ? std::chrono::milliseconds(configuredBumpDuration)
        : baseDuration;
    m_modelParameters.shapeFactor = PearosMagicLampConfig::initialShapeFactor();
    m_modelParameters.bumpDistance = PearosMagicLampConfig::maxBumpDistance();

    m_gridResolution = PearosMagicLampConfig::gridResolution();
}

#ifdef KWIN_PREPAINT_HAS_PRESENT_TIME
void PearosMagicLampEffect::prePaintScreen(KWin::ScreenPrePaintData& data, std::chrono::milliseconds presentTime)
{
    for (AnimationData& animData : m_animations) {
        animData.model.advance(presentTime);
    }

    data.mask |= PAINT_SCREEN_WITH_TRANSFORMED_WINDOWS;

    KWinCompat::prePaintScreen(KWin::effects, data, presentTime);
}

void PearosMagicLampEffect::prePaintWindow(KWin::RenderView* view, KWin::EffectWindow* w, KWin::WindowPrePaintData& data, std::chrono::milliseconds presentTime)
{
    if (m_animations.contains(w)) {
        data.mask |= PAINT_WINDOW_TRANSFORMED;
    }
    KWinCompat::prePaintWindow(KWin::effects, view, w, data, presentTime);
}
#else
void PearosMagicLampEffect::prePaintScreen(KWin::ScreenPrePaintData& data)
{
    const auto presentTime = KWinCompat::monotonicTimestamp();
    for (AnimationData& animData : m_animations) {
        animData.model.advance(presentTime);
    }

    data.mask |= PAINT_SCREEN_WITH_TRANSFORMED_WINDOWS;

    KWinCompat::prePaintScreen(KWin::effects, data, presentTime);
}

void PearosMagicLampEffect::prePaintWindow(KWin::RenderView* view, KWin::EffectWindow* w, KWin::WindowPrePaintData& data)
{
    if (m_animations.contains(w)) {
        data.mask |= PAINT_WINDOW_TRANSFORMED;
    }
    KWinCompat::prePaintWindow(KWin::effects, view, w, data, KWinCompat::monotonicTimestamp());
}
#endif

void PearosMagicLampEffect::postPaintScreen()
{
    for (auto it = m_animations.begin(); it != m_animations.end();) {
        if (it->model.done()) {
            unredirect(it.key());
            it = m_animations.erase(it);
        } else {
            ++it;
        }
    }

    KWin::effects->addRepaintFull();
    KWin::effects->postPaintScreen();
}

void PearosMagicLampEffect::paintWindow(const KWin::RenderTarget& renderTarget,
    const KWin::RenderViewport& viewport,
    KWin::EffectWindow* w, int mask,
    const KWin::Region& region,
    KWin::WindowPaintData& data)
{
    auto it = m_animations.constFind(w);
    if (it != m_animations.constEnd() && it->model.needsClip()) {
        const KWin::Region clipLogical(it->model.clipRegion());
        const KWin::Region clip = viewport.mapToDeviceCoordinatesAligned(clipLogical);
        KWin::effects->paintWindow(renderTarget, viewport, w, mask, clip, data);
        return;
    }

    KWin::effects->paintWindow(renderTarget, viewport, w, mask, region, data);
}

void PearosMagicLampEffect::apply(KWin::EffectWindow* window, int mask, KWin::WindowPaintData& data, KWin::WindowQuadList& quads)
{
    Q_UNUSED(mask)
    Q_UNUSED(data)

    auto it = m_animations.constFind(window);
    if (it == m_animations.constEnd()) {
        return;
    }

    quads = quads.makeGrid(m_gridResolution);
    (*it).model.apply(quads);
}

bool PearosMagicLampEffect::isActive() const
{
    return !m_animations.isEmpty();
}

bool PearosMagicLampEffect::supported()
{
    if (!KWin::effects->animationsSupported()) {
        return false;
    }
    if (KWin::effects->isOpenGLCompositing()) {
        return true;
    }
    return false;
}

void PearosMagicLampEffect::slotWindowAdded(KWin::EffectWindow* w)
{
    connect(w, &KWin::EffectWindow::minimizedChanged,
        this, &PearosMagicLampEffect::slotMinimizedChanged);
}

void PearosMagicLampEffect::slotMinimizedChanged(KWin::EffectWindow* w)
{
    if (w->isMinimized()) {
        startMinimize(w);
    } else {
        startUnminimize(w);
    }
}

void PearosMagicLampEffect::startMinimize(KWin::EffectWindow* w)
{
    if (KWin::effects->activeFullScreenEffect()) {
        return;
    }

    const QRectF iconRect = w->iconGeometry();
    if (!iconRect.isValid()) {
        // On Wayland secondary screens the Plasma taskbar sets iconGeometry
        // asynchronously (after QML layout). Keep the window visible and retry
        // once the geometry arrives.
        if (m_pendingMinimize.contains(w)) {
            return;
        }
        m_pendingMinimize[w] = KWin::EffectWindowVisibleRef(w, KWin::EffectWindow::PAINT_DISABLED_BY_MINIMIZE);
        QPointer<KWin::EffectWindow> weakRef(w);
        QTimer::singleShot(150, this, [this, weakRef]() {
            KWin::EffectWindow* win = weakRef.data();
            m_pendingMinimize.remove(win);
            if (win && win->isMinimized()) {
                startMinimize(win);
            }
        });
        return;
    }

    AnimationData& animData = m_animations[w];
    animData.model.setWindow(w);
    animData.model.setParameters(m_modelParameters);
    animData.model.setIconGeometry(resolveIconGeometry(w));
    animData.model.start(Model::AnimationKind::Minimize);
    animData.visibleRef = KWin::EffectWindowVisibleRef(w, KWin::EffectWindow::PAINT_DISABLED_BY_MINIMIZE);

    redirect(w);

    KWin::effects->addRepaintFull();
}

void PearosMagicLampEffect::startUnminimize(KWin::EffectWindow* w)
{
    if (KWin::effects->activeFullScreenEffect()) {
        return;
    }

    const QRectF iconRect = w->iconGeometry();
    if (!iconRect.isValid()) {
        return;
    }

    AnimationData& animData = m_animations[w];
    animData.model.setWindow(w);
    animData.model.setParameters(m_modelParameters);
    animData.model.setIconGeometry(resolveIconGeometry(w));
    animData.model.start(Model::AnimationKind::Unminimize);

    redirect(w);

    KWin::effects->addRepaintFull();
}

void PearosMagicLampEffect::slotWindowDeleted(KWin::EffectWindow* w)
{
    m_pendingMinimize.remove(w);
    m_animations.remove(w);
}

void PearosMagicLampEffect::slotActiveFullScreenEffectChanged()
{
    if (KWin::effects->activeFullScreenEffect() != nullptr) {
        m_pendingMinimize.clear();
        for (auto it = m_animations.constBegin(); it != m_animations.constEnd(); ++it) {
            unredirect(it.key());
        }
        m_animations.clear();
    }
}
