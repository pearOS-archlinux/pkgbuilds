/*
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "PearosTinterEffect.h"
#include "PearosTinterConfig.h"
#include "WallpaperSampler.h"

#include <effect/effecthandler.h>
#include <core/renderviewport.h>
#include <opengl/glshader.h>
#include <opengl/glshadermanager.h>

#include <KColorScheme>

#include <QVector3D>

using namespace KWin;

namespace
{
QVector3D toVector(const QColor &color)
{
    return QVector3D(color.redF(), color.greenF(), color.blueF());
}

// Ignore sub-percent wobble (thumbnail repaints, drag jitter) so a nearly
// stationary window doesn't queue a repaint every frame. Same threshold
// idea as Filer's channelDelta() > 4.
bool colorsDiffer(const QColor &a, const QColor &b)
{
    if (!a.isValid() || !b.isValid()) {
        return a.isValid() != b.isValid();
    }
    return qAbs(a.red() - b.red()) > 4 || qAbs(a.green() - b.green()) > 4 || qAbs(a.blue() - b.blue()) > 4;
}
}

PearosTinterEffect::PearosTinterEffect()
    : m_sampler(std::make_unique<WallpaperSampler>())
{
    m_shader = ShaderManager::instance()->generateShaderFromFile(
        ShaderTrait::MapTexture | ShaderTrait::Modulate | ShaderTrait::AdjustSaturation,
        QString(),
        QStringLiteral(":/pearos-tinter/shaders/tint.frag"));

    if (!m_shader) {
        qWarning("pearos-tinter: tint shader failed to compile, effect stays inert");
        m_shader.reset();
        return;
    }

    // ~6 frames at 60Hz: long enough for a client to commit a new buffer
    // after a resize or a full-screen toggle, short enough to be invisible.
    m_resettleTimer.setInterval(50);
    connect(&m_resettleTimer, &QTimer::timeout, this, [this] {
        for (EffectWindow *window : std::as_const(m_resettling)) {
            invalidateTexture(window);
            updateTint(window);
            window->addRepaintFull();
        }
        if (!m_resettling.isEmpty()) {
            // The window's own damage region is not enough when the stale
            // pixels are already on screen -- the screen area has to be
            // redrawn too.
            effects->addRepaintFull();
        }
        if (++m_resettleTicks >= 6) {
            m_resettleTimer.stop();
            m_resettling.clear();
        }
    });

    connect(effects, &EffectsHandler::windowAdded, this, &PearosTinterEffect::slotWindowAdded);
    connect(effects, &EffectsHandler::windowDeleted, this, &PearosTinterEffect::slotWindowDeleted);
    connect(effects, &EffectsHandler::screenAdded, this, &PearosTinterEffect::retintAll);
    connect(effects, &EffectsHandler::screenRemoved, this, &PearosTinterEffect::retintAll);
    connect(m_sampler.get(), &WallpaperSampler::changed, this, &PearosTinterEffect::retintAll);

    PearosTinterEffect::reconfigure(ReconfigureAll);

    const auto windows = effects->stackingOrder();
    for (EffectWindow *window : windows) {
        slotWindowAdded(window);
    }
}

PearosTinterEffect::~PearosTinterEffect()
{
    const auto tracked = m_windows.keys();
    for (EffectWindow *window : tracked) {
        unredirect(window);
    }
}

bool PearosTinterEffect::supported()
{
    return effects->isOpenGLCompositing() && OffscreenEffect::supported();
}

int PearosTinterEffect::requestedEffectChainPosition() const
{
    // Late enough that the window content is final, early enough that
    // window-transforming effects (present windows, wobbly, magic lamp)
    // still see a tinted window rather than fighting over the texture.
    return 60;
}

bool PearosTinterEffect::isActive() const
{
    return m_shader && m_tintAmount > 0.0 && !m_windows.isEmpty();
}

void PearosTinterEffect::updateKeyColor()
{
    if (PearosTinterConfig::keyFromColorScheme()) {
        // NOTE: this is KWin's own color scheme, not the client's. It is a
        // decent guess for Breeze/Qt apps and simply wrong for GTK, Electron
        // and self-skinned apps -- which is why KeyStrength defaults to 0.
        const KColorScheme scheme(QPalette::Normal, KColorScheme::Window);
        m_keyColor = scheme.background().color();
    } else {
        m_keyColor = PearosTinterConfig::keyColor();
    }
}

void PearosTinterEffect::reconfigure(ReconfigureFlags flags)
{
    Q_UNUSED(flags)

    PearosTinterConfig::self()->read();

    m_tintAmount = PearosTinterConfig::tintAmount() / 100.0;
    m_chromaGuard = PearosTinterConfig::chromaGuard() / 100.0;
    m_chromaRange = QVector2D(PearosTinterConfig::chromaLow() / 100.0f, PearosTinterConfig::chromaHigh() / 100.0f);
    m_edgeGuard = PearosTinterConfig::edgeGuard() / 100.0;
    m_edgeRange = QVector2D(PearosTinterConfig::edgeLow() / 100.0f, PearosTinterConfig::edgeHigh() / 100.0f);
    m_keyStrength = PearosTinterConfig::keyStrength() / 100.0;
    m_keyRange = QVector2D(PearosTinterConfig::keyTolerance() / 100.0f, PearosTinterConfig::keySoftness() / 100.0f);
    m_includeDialogs = PearosTinterConfig::includeDialogs();
    m_skipFullScreen = PearosTinterConfig::skipFullScreen();
    m_blacklist = PearosTinterConfig::blacklist();
    m_suspendDuringResize = PearosTinterConfig::suspendDuringResize();

    updateKeyColor();
    m_sampler->setSampleResolution(PearosTinterConfig::sampleResolution());

    // Filtering rules may have changed, so re-evaluate every window rather
    // than only the ones currently tracked.
    const auto windows = effects->stackingOrder();
    for (EffectWindow *window : windows) {
        refresh(window);
    }
    retintAll();
}

bool PearosTinterEffect::shouldTint(EffectWindow *window) const
{
    if (!m_shader || m_tintAmount <= 0.0) {
        return false;
    }
    if (window->isDesktop() || window->isDock() || window->isLockScreen() || window->isOutline() || window->isPopupWindow()) {
        return false;
    }
    if (m_skipFullScreen && window->isFullScreen()) {
        return false;
    }
    if (!window->isNormalWindow() && !(m_includeDialogs && window->isDialog())) {
        return false;
    }

    const QString windowClass = window->windowClass();
    for (const QString &entry : m_blacklist) {
        const QString trimmed = entry.trimmed();
        if (!trimmed.isEmpty() && windowClass.contains(trimmed, Qt::CaseInsensitive)) {
            return false;
        }
    }
    return true;
}

void PearosTinterEffect::slotWindowAdded(EffectWindow *window)
{
    setupConnections(window);
    refresh(window);
}

void PearosTinterEffect::slotWindowDeleted(EffectWindow *window)
{
    m_windows.remove(window);
    m_suspended.remove(window);
    m_resettling.remove(window);
}

// Connections live for the window's lifetime, not for as long as the window
// happens to be tinted: a window that stopped being eligible (went full
// screen, is being resized) still has to be able to tell us when it becomes
// eligible again.
void PearosTinterEffect::setupConnections(EffectWindow *window)
{
    // The tint follows the window: moving it across the wallpaper is the
    // whole point of the effect.
    connect(window, &EffectWindow::windowFrameGeometryChanged, this, [this, window](EffectWindow *, const RectF &oldGeometry) {
        const bool resized = window->frameGeometry().size() != oldGeometry.size();
        const bool retinted = updateTint(window);
        if (resized) {
            // OffscreenData::maybeRender() reallocates its texture when the
            // window size changes, but only *fills* it when m_isDirty is set
            // -- and that flag is set from EffectWindow::windowDamaged alone.
            // A resize on its own therefore leaves an empty texture on screen
            // until the client happens to damage itself.
            scheduleResettle(window);
        } else if (retinted) {
            window->addRepaintFull();
        }
    });

    // Going full screen changes eligibility (SkipFullScreen). Without this
    // the window kept its stale redirected texture across the transition.
    // Shadow/expanded geometry feeds the offscreen texture size too.
    connect(window, &EffectWindow::windowExpandedGeometryChanged, this, [this, window] {
        scheduleResettle(window);
    });

    connect(window, &EffectWindow::windowFullScreenChanged, this, [this, window] {
        refresh(window);
        scheduleResettle(window);
    });
    connect(window, &EffectWindow::windowMaximizedStateChanged, this, [this, window] {
        refresh(window);
        scheduleResettle(window);
    });

    // Interactive resize renders through a different path frame by frame and
    // redirecting it offscreen is what glitches. Drop out of the way while
    // the user drags, restore on release.
    connect(window, &EffectWindow::windowStartUserMovedResized, this, [this, window] {
        if (m_suspendDuringResize && window->isUserResize()) {
            m_suspended.insert(window);
            refresh(window);
        }
    });
    connect(window, &EffectWindow::windowFinishUserMovedResized, this, [this, window] {
        if (m_suspended.remove(window)) {
            refresh(window);
        }
    });
}

void PearosTinterEffect::refresh(EffectWindow *window)
{
    if (shouldTint(window) && !m_suspended.contains(window)) {
        startTinting(window);
        updateTint(window);
    } else {
        stopTinting(window);
    }
}

void PearosTinterEffect::scheduleResettle(EffectWindow *window)
{
    if (!m_windows.contains(window)) {
        return;
    }
    m_resettling.insert(window);
    m_resettleTicks = 0;
    if (!m_resettleTimer.isActive()) {
        m_resettleTimer.start();
    }
    invalidateTexture(window);
    window->addRepaintFull();
    effects->addRepaintFull();
}

void PearosTinterEffect::invalidateTexture(EffectWindow *window)
{
    if (!m_windows.contains(window)) {
        return;
    }
    // OffscreenData has no public way to be marked dirty, so drop it and let
    // redirect() build a fresh one -- those start out dirty.
    unredirect(window);
    redirect(window);
    setShader(window, m_shader.get());
}

void PearosTinterEffect::startTinting(EffectWindow *window)
{
    if (m_windows.contains(window)) {
        return;
    }

    m_windows.insert(window, WindowState{});
    redirect(window);
    setShader(window, m_shader.get());
    // A freshly redirected window has an empty offscreen texture until
    // something damages it -- without this the window stays blank/stale
    // until an unrelated repaint (alt-tab, another window overlapping) comes
    // along.
    window->addRepaintFull();
}

void PearosTinterEffect::stopTinting(EffectWindow *window)
{
    if (!m_windows.contains(window)) {
        return;
    }
    unredirect(window);
    m_windows.remove(window);
    m_resettling.remove(window);
    window->addRepaintFull();
    effects->addRepaintFull();
}

bool PearosTinterEffect::updateTint(EffectWindow *window)
{
    auto state = m_windows.find(window);
    if (state == m_windows.end()) {
        return false;
    }

    QColor tint;

    if (LogicalOutput *output = window->screen()) {
        const QRect screenGeometry = output->geometry();
        const QRect windowGeometry = window->frameGeometry().toRect();
        const QRect localRect = windowGeometry.translated(-screenGeometry.topLeft()).intersected(QRect(QPoint(0, 0), screenGeometry.size()));

        if (!localRect.isEmpty()) {
            const int screenIndex = effects->screens().indexOf(output);
            tint = m_sampler->averageColor(screenIndex, localRect, screenGeometry.size());
        }
    }

    if (!colorsDiffer(tint, state->tint)) {
        return false;
    }
    state->tint = tint;
    return true;
}

void PearosTinterEffect::retintAll()
{
    for (auto it = m_windows.begin(); it != m_windows.end(); ++it) {
        EffectWindow *window = it.key();
        if (updateTint(window)) {
            window->addRepaintFull();
        }
    }
}

void PearosTinterEffect::drawWindow(const RenderTarget &renderTarget,
                                    const RenderViewport &viewport,
                                    EffectWindow *window,
                                    int mask,
                                    const Region &deviceRegion,
                                    WindowPaintData &data)
{
    auto state = m_windows.constFind(window);
    if (state != m_windows.constEnd() && state->tint.isValid()) {
        const QRectF geometry = window->expandedGeometry();
        const qreal scale = viewport.scale();
        const QSizeF textureSize(qMax(1.0, geometry.width() * scale), qMax(1.0, geometry.height() * scale));

        // Uniform values live in the program object, so setting them with the
        // shader briefly bound here is enough -- OffscreenEffect::drawWindow()
        // binds the same program again to do the actual draw.
        ShaderManager::instance()->pushShader(m_shader.get());
        m_shader->setUniform("tintColor", toVector(state->tint));
        m_shader->setUniform("tintAmount", float(m_tintAmount));
        m_shader->setUniform("texelSize", QVector2D(1.0f / textureSize.width(), 1.0f / textureSize.height()));
        m_shader->setUniform("chromaGuard", float(m_chromaGuard));
        m_shader->setUniform("chromaRange", m_chromaRange);
        m_shader->setUniform("edgeGuard", float(m_edgeGuard));
        m_shader->setUniform("edgeRange", m_edgeRange);
        m_shader->setUniform("keyStrength", float(m_keyStrength));
        m_shader->setUniform("keyColor", toVector(m_keyColor));
        m_shader->setUniform("keyRange", m_keyRange);
        ShaderManager::instance()->popShader();
    }

    OffscreenEffect::drawWindow(renderTarget, viewport, window, mask, deviceRegion, data);
}
