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

#pragma once

// Own
#include "Model.h"
#include "common.h"

// kwineffects
#include <effect/offscreeneffect.h>

// Qt
#include <QPointer>

struct AnimationData {
    Model model;
    KWin::EffectWindowVisibleRef visibleRef;
};

class PearosMagicLampEffect : public KWin::OffscreenEffect {
    Q_OBJECT

public:
    PearosMagicLampEffect();
    ~PearosMagicLampEffect() override;

    void reconfigure(ReconfigureFlags flags) override;

    // KWin 6.7 removed the presentation timestamp from the pre-paint hook
    // signatures. The CMake probe KWIN_PREPAINT_HAS_PRESENT_TIME picks the
    // signature that matches the installed KWin so `override` stays correct.
#ifdef KWIN_PREPAINT_HAS_PRESENT_TIME
    void prePaintScreen(KWin::ScreenPrePaintData& data, std::chrono::milliseconds presentTime) override;
    void prePaintWindow(KWin::RenderView* view, KWin::EffectWindow* w, KWin::WindowPrePaintData& data, std::chrono::milliseconds presentTime) override;
#else
    void prePaintScreen(KWin::ScreenPrePaintData& data) override;
    void prePaintWindow(KWin::RenderView* view, KWin::EffectWindow* w, KWin::WindowPrePaintData& data) override;
#endif
    void postPaintScreen() override;

    void paintWindow(const KWin::RenderTarget& renderTarget, const KWin::RenderViewport& viewport,
        KWin::EffectWindow* w, int mask, const KWin::Region& region,
        KWin::WindowPaintData& data) override;

    bool isActive() const override;
    int requestedEffectChainPosition() const override;

    static bool supported();

protected:
    void apply(KWin::EffectWindow* window, int mask, KWin::WindowPaintData& data, KWin::WindowQuadList& quads) override;

private Q_SLOTS:
    void slotWindowAdded(KWin::EffectWindow* w);
    void slotMinimizedChanged(KWin::EffectWindow* w);
    void slotWindowDeleted(KWin::EffectWindow* w);
    void slotActiveFullScreenEffectChanged();

private:
    void startMinimize(KWin::EffectWindow* w);
    void startUnminimize(KWin::EffectWindow* w);

    Model::Parameters m_modelParameters;
    int m_gridResolution;

    QMap<KWin::EffectWindow*, AnimationData> m_animations;
    // Windows waiting for iconGeometry to become valid before animation starts
    QMap<KWin::EffectWindow*, KWin::EffectWindowVisibleRef> m_pendingMinimize;
};

inline int PearosMagicLampEffect::requestedEffectChainPosition() const
{
    return 50;
}
