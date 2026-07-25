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
#include "Model.h"
#include "kwincompat.h"

static inline std::chrono::milliseconds durationFraction(std::chrono::milliseconds duration, qreal fraction)
{
    return std::chrono::milliseconds(qMax(qRound(duration.count() * fraction), 1));
}

Model::Model(KWin::EffectWindow* window)
    : m_window(window)
{
}

// Mirrors KDE's own Magic Lamp logic exactly:
// 1. Find panel via intersects() — Plasma sometimes publishes icon geometry
//    slightly larger than the panel, so contains() misses it.
// 2. Use clientArea(ScreenArea, window) — the WINDOW's screen center — to
//    determine direction. This correctly handles cross-screen icons: even if
//    the icon is on a different screen, comparing to the window's screen
//    center always gives the right direction.
static Direction realizeDirection(const KWin::EffectWindow* window, const QRectF& iconRect)
{
    KWin::EffectWindow* panel = nullptr;
    for (KWin::EffectWindow* candidate : KWin::effects->stackingOrder()) {
        if (!candidate->isDock()) {
            continue;
        }
        if (candidate->frameGeometry().intersects(iconRect)) {
            panel = candidate;
            break;
        }
    }

    if (panel) {
        const QRectF windowScreen = KWin::effects->clientArea(KWin::ScreenArea, window);
        if (panel->frameGeometry().width() >= panel->frameGeometry().height()) {
            return iconRect.center().y() <= windowScreen.center().y()
                ? Direction::Top
                : Direction::Bottom;
        } else {
            return iconRect.center().x() <= windowScreen.center().x()
                ? Direction::Left
                : Direction::Right;
        }
    }

    // No panel found — fall back to closest screen edge (KDE uses ScreenArea intersect here).
    KWin::LogicalOutput* screen = KWin::effects->screenAt(iconRect.center().toPoint());
    const QRectF screenRect = KWinCompat::fullScreenArea(KWin::effects, screen);
    const QPointF c = iconRect.center();
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

void Model::start(AnimationKind kind)
{
    m_kind = kind;

    if (m_timeLine.running()) {
        m_timeLine.toggleDirection();
        return;
    }

    if (!m_iconGeometry.isValid()) {
        m_iconGeometry = m_window->iconGeometry();
    }
    m_direction = realizeDirection(m_window, m_iconGeometry);
    m_bumpDistance = computeBumpDistance();
    m_shapeFactor = computeShapeFactor();

    switch (m_kind) {
    case AnimationKind::Minimize:
        if (m_bumpDistance != 0) {
            m_stage = AnimationStage::Bump;
            m_timeLine.reset();
            m_timeLine.setDirection(KWin::TimeLine::Forward);
            m_timeLine.setDuration(m_parameters.bumpDuration);
            m_timeLine.setEasingCurve(QEasingCurve::Linear);
            m_clip = false;
        } else {
            m_stage = AnimationStage::Stretch1;
            m_timeLine.reset();
            m_timeLine.setDirection(KWin::TimeLine::Forward);
            m_timeLine.setDuration(durationFraction(m_parameters.stretchDuration, m_shapeFactor));
            m_timeLine.setEasingCurve(QEasingCurve::Linear);
            m_clip = true;
        }
        break;

    case AnimationKind::Unminimize:
        m_stage = AnimationStage::Squash;
        m_timeLine.reset();
        m_timeLine.setDirection(KWin::TimeLine::Backward);
        m_timeLine.setDuration(m_parameters.squashDuration);
        m_timeLine.setEasingCurve(QEasingCurve::Linear);
        m_clip = true;
        break;

    default:
        Q_UNREACHABLE();
    }
}

void Model::advance(std::chrono::milliseconds presentTime)
{
    m_timeLine.advance(presentTime);
    if (!m_timeLine.done()) {
        return;
    }

    switch (m_kind) {
    case AnimationKind::Minimize:
        updateMinimizeStage();
        break;

    case AnimationKind::Unminimize:
        updateUnminimizeStage();
        break;

    default:
        Q_UNREACHABLE();
    }
}

void Model::updateMinimizeStage()
{
    switch (m_stage) {
    case AnimationStage::Bump:
        m_stage = AnimationStage::Stretch1;
        m_timeLine.reset();
        m_timeLine.setDirection(KWin::TimeLine::Forward);
        m_timeLine.setDuration(
            durationFraction(m_parameters.stretchDuration, m_shapeFactor));
        m_timeLine.setEasingCurve(QEasingCurve::Linear);
        m_clip = true;
        m_done = false;
        return;

    case AnimationStage::Stretch1:
    case AnimationStage::Stretch2:
        m_stage = AnimationStage::Squash;
        m_timeLine.reset();
        m_timeLine.setDirection(KWin::TimeLine::Forward);
        m_timeLine.setDuration(m_parameters.squashDuration);
        m_timeLine.setEasingCurve(QEasingCurve::Linear);
        m_clip = true;
        m_done = false;
        return;

    case AnimationStage::Squash:
        m_done = true;
        return;

    default:
        Q_UNREACHABLE();
    }
}

void Model::updateUnminimizeStage()
{
    switch (m_stage) {
    case AnimationStage::Bump:
        m_done = true;
        return;

    case AnimationStage::Stretch1:
        if (m_bumpDistance == 0) {
            m_done = true;
            return;
        }
        m_stage = AnimationStage::Bump;
        m_timeLine.reset();
        m_timeLine.setDirection(KWin::TimeLine::Backward);
        m_timeLine.setDuration(m_parameters.bumpDuration);
        m_timeLine.setEasingCurve(QEasingCurve::Linear);
        m_clip = false;
        m_done = false;
        return;

    case AnimationStage::Stretch2:
        m_done = true;
        return;

    case AnimationStage::Squash:
        m_stage = AnimationStage::Stretch2;
        m_timeLine.reset();
        m_timeLine.setDirection(KWin::TimeLine::Backward);
        m_timeLine.setDuration(
            durationFraction(m_parameters.unminimizeStretchDuration, m_shapeFactor));
        m_timeLine.setEasingCurve(QEasingCurve::Linear);
        m_clip = false;
        m_done = false;
        return;

    default:
        Q_UNREACHABLE();
    }
}

bool Model::done() const
{
    return m_done;
}

struct TransformParameters {
    QEasingCurve shapeCurve;
    Direction direction;
    QRectF iconRect;
    qreal stretchProgress;
    qreal squashProgress;
    qreal bumpProgress;
    qreal bumpDistance;
};

static inline qreal interpolate(qreal from, qreal to, qreal t)
{
    return from * (1.0 - t) + to * t;
}

static void transformQuadsLeft(
    const KWin::EffectWindow* window,
    const TransformParameters& params,
    KWin::WindowQuadList& quads)
{
    const QRectF iconRect = params.iconRect;
    const QRectF windowRect = window->frameGeometry();

    const qreal distance = windowRect.right() - iconRect.right() + params.bumpDistance;

    for (int i = 0; i < quads.count(); ++i) {
        KWin::WindowQuad& quad = quads[i];

        const qreal leftOffset = quad[0].x() - interpolate(0.0, distance, params.squashProgress);
        const qreal rightOffset = quad[2].x() - interpolate(0.0, distance, params.squashProgress);

        const qreal leftScale = params.stretchProgress * params.shapeCurve.valueForProgress((windowRect.width() - leftOffset) / distance);
        const qreal rightScale = params.stretchProgress * params.shapeCurve.valueForProgress((windowRect.width() - rightOffset) / distance);

        const qreal targetTopLeftY = iconRect.y() + iconRect.height() * quad[0].y() / windowRect.height();
        const qreal targetTopRightY = iconRect.y() + iconRect.height() * quad[1].y() / windowRect.height();
        const qreal targetBottomRightY = iconRect.y() + iconRect.height() * quad[2].y() / windowRect.height();
        const qreal targetBottomLeftY = iconRect.y() + iconRect.height() * quad[3].y() / windowRect.height();

        quad[0].setY(quad[0].y() + leftScale * (targetTopLeftY - (windowRect.y() + quad[0].y())));
        quad[3].setY(quad[3].y() + leftScale * (targetBottomLeftY - (windowRect.y() + quad[3].y())));
        quad[1].setY(quad[1].y() + rightScale * (targetTopRightY - (windowRect.y() + quad[1].y())));
        quad[2].setY(quad[2].y() + rightScale * (targetBottomRightY - (windowRect.y() + quad[2].y())));

        const qreal targetLeftOffset = leftOffset + params.bumpDistance * params.bumpProgress;
        const qreal targetRightOffset = rightOffset + params.bumpDistance * params.bumpProgress;

        quad[0].setX(targetLeftOffset);
        quad[3].setX(targetLeftOffset);
        quad[1].setX(targetRightOffset);
        quad[2].setX(targetRightOffset);
    }
}

static void transformQuadsTop(
    const KWin::EffectWindow* window,
    const TransformParameters& params,
    KWin::WindowQuadList& quads)
{
    const QRectF iconRect = params.iconRect;
    const QRectF windowRect = window->frameGeometry();

    const qreal distance = windowRect.bottom() - iconRect.bottom() + params.bumpDistance;

    for (int i = 0; i < quads.count(); ++i) {
        KWin::WindowQuad& quad = quads[i];

        const qreal topOffset = quad[0].y() - interpolate(0.0, distance, params.squashProgress);
        const qreal bottomOffset = quad[2].y() - interpolate(0.0, distance, params.squashProgress);

        const qreal topScale = params.stretchProgress * params.shapeCurve.valueForProgress((windowRect.height() - topOffset) / distance);
        const qreal bottomScale = params.stretchProgress * params.shapeCurve.valueForProgress((windowRect.height() - bottomOffset) / distance);

        const qreal targetTopLeftX = iconRect.x() + iconRect.width() * quad[0].x() / windowRect.width();
        const qreal targetTopRightX = iconRect.x() + iconRect.width() * quad[1].x() / windowRect.width();
        const qreal targetBottomRightX = iconRect.x() + iconRect.width() * quad[2].x() / windowRect.width();
        const qreal targetBottomLeftX = iconRect.x() + iconRect.width() * quad[3].x() / windowRect.width();

        quad[0].setX(quad[0].x() + topScale * (targetTopLeftX - (windowRect.x() + quad[0].x())));
        quad[1].setX(quad[1].x() + topScale * (targetTopRightX - (windowRect.x() + quad[1].x())));
        quad[2].setX(quad[2].x() + bottomScale * (targetBottomRightX - (windowRect.x() + quad[2].x())));
        quad[3].setX(quad[3].x() + bottomScale * (targetBottomLeftX - (windowRect.x() + quad[3].x())));

        const qreal targetTopOffset = topOffset + params.bumpDistance * params.bumpProgress;
        const qreal targetBottomOffset = bottomOffset + params.bumpDistance * params.bumpProgress;

        quad[0].setY(targetTopOffset);
        quad[1].setY(targetTopOffset);
        quad[2].setY(targetBottomOffset);
        quad[3].setY(targetBottomOffset);
    }
}

static void transformQuadsRight(
    const KWin::EffectWindow* window,
    const TransformParameters& params,
    KWin::WindowQuadList& quads)
{
    const QRectF iconRect = params.iconRect;
    const QRectF windowRect = window->frameGeometry();

    const qreal distance = iconRect.left() - windowRect.left() + params.bumpDistance;

    for (int i = 0; i < quads.count(); ++i) {
        KWin::WindowQuad& quad = quads[i];

        const qreal leftOffset = quad[0].x() + interpolate(0.0, distance, params.squashProgress);
        const qreal rightOffset = quad[2].x() + interpolate(0.0, distance, params.squashProgress);

        const qreal leftScale = params.stretchProgress * params.shapeCurve.valueForProgress(leftOffset / distance);
        const qreal rightScale = params.stretchProgress * params.shapeCurve.valueForProgress(rightOffset / distance);

        const qreal targetTopLeftY = iconRect.y() + iconRect.height() * quad[0].y() / windowRect.height();
        const qreal targetTopRightY = iconRect.y() + iconRect.height() * quad[1].y() / windowRect.height();
        const qreal targetBottomRightY = iconRect.y() + iconRect.height() * quad[2].y() / windowRect.height();
        const qreal targetBottomLeftY = iconRect.y() + iconRect.height() * quad[3].y() / windowRect.height();

        quad[0].setY(quad[0].y() + leftScale * (targetTopLeftY - (windowRect.y() + quad[0].y())));
        quad[3].setY(quad[3].y() + leftScale * (targetBottomLeftY - (windowRect.y() + quad[3].y())));
        quad[1].setY(quad[1].y() + rightScale * (targetTopRightY - (windowRect.y() + quad[1].y())));
        quad[2].setY(quad[2].y() + rightScale * (targetBottomRightY - (windowRect.y() + quad[2].y())));

        const qreal targetLeftOffset = leftOffset - params.bumpDistance * params.bumpProgress;
        const qreal targetRightOffset = rightOffset - params.bumpDistance * params.bumpProgress;

        quad[0].setX(targetLeftOffset);
        quad[3].setX(targetLeftOffset);
        quad[1].setX(targetRightOffset);
        quad[2].setX(targetRightOffset);
    }
}

static void transformQuadsBottom(
    const KWin::EffectWindow* window,
    const TransformParameters& params,
    KWin::WindowQuadList& quads)
{
    const QRectF iconRect = params.iconRect;
    const QRectF windowRect = window->frameGeometry();

    const qreal distance = iconRect.top() - windowRect.top() + params.bumpDistance;

    for (int i = 0; i < quads.count(); ++i) {
        KWin::WindowQuad& quad = quads[i];

        const qreal topOffset = quad[0].y() + interpolate(0.0, distance, params.squashProgress);
        const qreal bottomOffset = quad[2].y() + interpolate(0.0, distance, params.squashProgress);

        const qreal topScale = params.stretchProgress * params.shapeCurve.valueForProgress(topOffset / distance);
        const qreal bottomScale = params.stretchProgress * params.shapeCurve.valueForProgress(bottomOffset / distance);

        const qreal targetTopLeftX = iconRect.x() + iconRect.width() * quad[0].x() / windowRect.width();
        const qreal targetTopRightX = iconRect.x() + iconRect.width() * quad[1].x() / windowRect.width();
        const qreal targetBottomRightX = iconRect.x() + iconRect.width() * quad[2].x() / windowRect.width();
        const qreal targetBottomLeftX = iconRect.x() + iconRect.width() * quad[3].x() / windowRect.width();

        quad[0].setX(quad[0].x() + topScale * (targetTopLeftX - (windowRect.x() + quad[0].x())));
        quad[1].setX(quad[1].x() + topScale * (targetTopRightX - (windowRect.x() + quad[1].x())));
        quad[2].setX(quad[2].x() + bottomScale * (targetBottomRightX - (windowRect.x() + quad[2].x())));
        quad[3].setX(quad[3].x() + bottomScale * (targetBottomLeftX - (windowRect.x() + quad[3].x())));

        const qreal targetTopOffset = topOffset - params.bumpDistance * params.bumpProgress;
        const qreal targetBottomOffset = bottomOffset - params.bumpDistance * params.bumpProgress;

        quad[0].setY(targetTopOffset);
        quad[1].setY(targetTopOffset);
        quad[2].setY(targetBottomOffset);
        quad[3].setY(targetBottomOffset);
    }
}

static void transformQuads(
    const KWin::EffectWindow* window,
    const TransformParameters& params,
    KWin::WindowQuadList& quads)
{
    switch (params.direction) {
    case Direction::Left:
        transformQuadsLeft(window, params, quads);
        break;

    case Direction::Top:
        transformQuadsTop(window, params, quads);
        break;

    case Direction::Right:
        transformQuadsRight(window, params, quads);
        break;

    case Direction::Bottom:
        transformQuadsBottom(window, params, quads);
        break;

    default:
        Q_UNREACHABLE();
    }
}

void Model::apply(KWin::WindowQuadList& quads) const
{
    switch (m_stage) {
    case AnimationStage::Bump:
        applyBump(quads);
        break;

    case AnimationStage::Stretch1:
        applyStretch1(quads);
        break;

    case AnimationStage::Stretch2:
        applyStretch2(quads);
        break;

    case AnimationStage::Squash:
        applySquash(quads);
        break;
    }
}

void Model::applyBump(KWin::WindowQuadList& quads) const
{
    TransformParameters params;
    params.shapeCurve = m_parameters.shapeCurve;
    params.direction = m_direction;
    params.iconRect = m_iconGeometry;
    params.squashProgress = 0.0;
    params.stretchProgress = 0.0;
    params.bumpProgress = m_timeLine.value();
    params.bumpDistance = m_bumpDistance;
    transformQuads(m_window, params, quads);
}

void Model::applyStretch1(KWin::WindowQuadList& quads) const
{
    TransformParameters params;
    params.shapeCurve = m_parameters.shapeCurve;
    params.direction = m_direction;
    params.iconRect = m_iconGeometry;
    params.squashProgress = 0.0;
    params.stretchProgress = m_shapeFactor * m_timeLine.value();
    params.bumpProgress = 1.0;
    params.bumpDistance = m_bumpDistance;
    transformQuads(m_window, params, quads);
}

void Model::applyStretch2(KWin::WindowQuadList& quads) const
{
    TransformParameters params;
    params.shapeCurve = m_parameters.shapeCurve;
    params.direction = m_direction;
    params.iconRect = m_iconGeometry;
    params.squashProgress = 0.0;
    params.stretchProgress = m_shapeFactor * m_timeLine.value();
    params.bumpProgress = params.stretchProgress;
    params.bumpDistance = m_bumpDistance;
    transformQuads(m_window, params, quads);
}

void Model::applySquash(KWin::WindowQuadList& quads) const
{
    TransformParameters params;
    params.shapeCurve = m_parameters.shapeCurve;
    params.direction = m_direction;
    params.iconRect = m_iconGeometry;
    params.squashProgress = m_timeLine.value();
    params.stretchProgress = qMin(m_shapeFactor + params.squashProgress, 1.0);
    params.bumpProgress = 1.0;
    params.bumpDistance = m_bumpDistance;
    transformQuads(m_window, params, quads);
}

Model::Parameters Model::parameters() const
{
    return m_parameters;
}

void Model::setParameters(const Parameters& parameters)
{
    m_parameters = parameters;
}

KWin::EffectWindow* Model::window() const
{
    return m_window;
}

void Model::setWindow(KWin::EffectWindow* window)
{
    m_window = window;
    m_iconGeometry = QRectF(); // reset so start() re-reads from window or override
}

void Model::setIconGeometry(const QRectF& rect)
{
    m_iconGeometry = rect;
}

bool Model::needsClip() const
{
    return m_clip;
}

QRegion Model::clipRegion() const
{
    const QRectF iconRect = m_iconGeometry;
    QRectF clipRect = m_window->expandedGeometry();

    switch (m_direction) {
    case Direction::Top:
        clipRect.translate(0, m_bumpDistance);
        clipRect.setTop(iconRect.top());
        clipRect.setLeft(qMin(iconRect.left(), clipRect.left()));
        clipRect.setRight(qMax(iconRect.right(), clipRect.right()));
        break;

    case Direction::Right:
        clipRect.translate(-m_bumpDistance, 0);
        clipRect.setRight(iconRect.right());
        clipRect.setTop(qMin(iconRect.top(), clipRect.top()));
        clipRect.setBottom(qMax(iconRect.bottom(), clipRect.bottom()));
        break;

    case Direction::Bottom:
        clipRect.translate(0, -m_bumpDistance);
        clipRect.setBottom(iconRect.bottom());
        clipRect.setLeft(qMin(iconRect.left(), clipRect.left()));
        clipRect.setRight(qMax(iconRect.right(), clipRect.right()));
        break;

    case Direction::Left:
        clipRect.translate(m_bumpDistance, 0);
        clipRect.setLeft(iconRect.left());
        clipRect.setTop(qMin(iconRect.top(), clipRect.top()));
        clipRect.setBottom(qMax(iconRect.bottom(), clipRect.bottom()));
        break;

    default:
        Q_UNREACHABLE();
    }

    return clipRect.toAlignedRect();
}

int Model::computeBumpDistance() const
{
    const QRectF windowRect = m_window->frameGeometry();
    const QRectF iconRect = m_iconGeometry;

    qreal bumpDistance = 0;
    switch (m_direction) {
    case Direction::Top:
        bumpDistance = std::max(qreal(0), iconRect.y() + iconRect.height() - windowRect.y());
        break;

    case Direction::Right:
        bumpDistance = std::max(qreal(0), windowRect.x() + windowRect.width() - iconRect.x());
        break;

    case Direction::Bottom:
        bumpDistance = std::max(qreal(0), windowRect.y() + windowRect.height() - iconRect.y());
        break;

    case Direction::Left:
        bumpDistance = std::max(qreal(0), iconRect.x() + iconRect.width() - windowRect.x());
        break;

    default:
        Q_UNREACHABLE();
    }

    bumpDistance += std::min(bumpDistance, m_parameters.bumpDistance);

    return bumpDistance;
}

qreal Model::computeShapeFactor() const
{
    const QRectF windowRect = m_window->frameGeometry();
    const QRectF iconRect = m_iconGeometry;

    int movingExtent = 0;
    int distanceToIcon = 0;
    switch (m_direction) {
    case Direction::Top:
        movingExtent = windowRect.height();
        distanceToIcon = windowRect.bottom() - iconRect.bottom() + m_bumpDistance;
        break;

    case Direction::Right:
        movingExtent = windowRect.width();
        distanceToIcon = iconRect.left() - windowRect.left() + m_bumpDistance;
        break;

    case Direction::Bottom:
        movingExtent = windowRect.height();
        distanceToIcon = iconRect.top() - windowRect.top() + m_bumpDistance;
        break;

    case Direction::Left:
        movingExtent = windowRect.width();
        distanceToIcon = windowRect.right() - iconRect.right() + m_bumpDistance;
        break;

    default:
        Q_UNREACHABLE();
    }

    const qreal minimumShapeFactor = static_cast<qreal>(movingExtent) / distanceToIcon;
    return qMax(m_parameters.shapeFactor, minimumShapeFactor);
}
