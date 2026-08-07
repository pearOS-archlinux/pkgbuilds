/*
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include <effect/offscreeneffect.h>

#include <QColor>
#include <QHash>
#include <QSet>
#include <QTimer>

#include <memory>

class WallpaperSampler;

namespace KWin
{
class GLShader;
}

/**
 * Tints window backgrounds with the average color of the wallpaper behind
 * the window, the way Filer tints its sidebar (see
 * MainWindow::updateSidebarWallpaperTint(), ~/Desktop/Filer/src/mainwindow.cpp:3648)
 * -- except here it applies to every window, because the compositor knows
 * every window's position without a KWin script relaying it over D-Bus.
 *
 * The compositor only ever sees a flat, already-composed client buffer: it
 * cannot tell background from text the way AppKit can, where the tint lives
 * in an NSVisualEffectView *below* the content layers. So the shader
 * approximates that separation with two guards -- chroma (icons, photos) and
 * local contrast (text, edges). See README.md.
 */
class PearosTinterEffect : public KWin::OffscreenEffect
{
    Q_OBJECT

public:
    PearosTinterEffect();
    ~PearosTinterEffect() override;

    static bool supported();

    void reconfigure(ReconfigureFlags flags) override;
    void drawWindow(const KWin::RenderTarget &renderTarget,
                    const KWin::RenderViewport &viewport,
                    KWin::EffectWindow *window,
                    int mask,
                    const KWin::Region &deviceRegion,
                    KWin::WindowPaintData &data) override;

    int requestedEffectChainPosition() const override;
    bool isActive() const override;

private Q_SLOTS:
    void slotWindowAdded(KWin::EffectWindow *window);
    void slotWindowDeleted(KWin::EffectWindow *window);

private:
    struct WindowState
    {
        QColor tint;
        QSizeF textureSize;
    };

    bool shouldTint(KWin::EffectWindow *window) const;
    /** Connected once per window and kept for the window's lifetime. */
    void setupConnections(KWin::EffectWindow *window);
    /** Applies the current eligibility verdict to @p window. */
    void refresh(KWin::EffectWindow *window);
    /**
     * Forces OffscreenEffect to re-render the window into its offscreen
     * texture, which it otherwise only does on real client damage.
     */
    void invalidateTexture(KWin::EffectWindow *window);
    /**
     * Re-invalidates and repaints @p window over the next few frames.
     *
     * A single repaint request issued from a geometry/full-screen signal is
     * not enough: at that point the client has not committed its new buffer
     * yet, so the request is spent on stale content and KWin then only
     * repaints whatever the client damages afterwards -- which is why the
     * window used to recover in patches, following the mouse.
     */
    void scheduleResettle(KWin::EffectWindow *window);
    void startTinting(KWin::EffectWindow *window);
    void stopTinting(KWin::EffectWindow *window);
    void retintAll();
    /** Returns true if the stored tint for @p window actually changed. */
    bool updateTint(KWin::EffectWindow *window);
    void updateKeyColor();

    std::unique_ptr<KWin::GLShader> m_shader;
    std::unique_ptr<WallpaperSampler> m_sampler;
    QHash<KWin::EffectWindow *, WindowState> m_windows;
    /** Windows temporarily not tinted because the user is resizing them. */
    QSet<KWin::EffectWindow *> m_suspended;
    QSet<KWin::EffectWindow *> m_resettling;
    QTimer m_resettleTimer;
    int m_resettleTicks = 0;

    qreal m_tintAmount = 0.0;
    qreal m_chromaGuard = 1.0;
    QVector2D m_chromaRange;
    qreal m_edgeGuard = 1.0;
    QVector2D m_edgeRange;
    qreal m_keyStrength = 0.0;
    QColor m_keyColor;
    QVector2D m_keyRange;
    bool m_includeDialogs = true;
    bool m_skipFullScreen = true;
    bool m_suspendDuringResize = true;
    QStringList m_blacklist;
};
