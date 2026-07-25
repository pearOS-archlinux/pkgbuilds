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
#include "common.h"

// kwineffects
#include <effect/effecthandler.h>
#include <effect/effectwindow.h>
#include <effect/timeline.h>
#include <scene/itemgeometry.h>

/**
 * Model for the magic lamp animation.
 **/
class Model {
public:
    struct Parameters {
        // The duration of the squash stage.
        std::chrono::milliseconds squashDuration;

        // The duration of the stretch stage.
        std::chrono::milliseconds stretchDuration;

        // The duration of the Stretch2 stage on unminimize (contains the
        // coupled bump-return). Defaults to stretchDuration.
        std::chrono::milliseconds unminimizeStretchDuration;

        // How long it takes to raise the window.
        std::chrono::milliseconds bumpDuration;

        // Defines shape of transformed windows.
        QEasingCurve shapeCurve;

        // The blend factor between Squash and Stretch stage.
        qreal shapeFactor;

        // How much the transformed window should be raised.
        qreal bumpDistance;
    };

    explicit Model(KWin::EffectWindow* window = nullptr);

    /**
     * This enum type is used to specify kind of the current animation.
     **/
    enum class AnimationKind {
        Minimize,
        Unminimize
    };

    /**
     * Starts animation with the given kind.
     *
     * @param kind Kind of the animation (minimize or unminimize).
     **/
    void start(AnimationKind kind);

    /**
     * Updates the model to @p milliseconds.
     **/
    void advance(std::chrono::milliseconds presentTime);

    /**
     * Returns whether the animation is complete.
     **/
    bool done() const;

    /**
     * Applies the current state of the model to the given list of
     * window quads.
     *
     * @param quads The list of window quads to be transformed.
     **/
    void apply(KWin::WindowQuadList& quads) const;

    /**
     * Returns the parameters of the model.
     **/
    Parameters parameters() const;

    /**
     * Sets parameters of the model.
     *
     * @param parameters The new paramaters.
     **/
    void setParameters(const Parameters& parameters);

    /**
     * Returns the associated window.
     **/
    KWin::EffectWindow* window() const;

    /**
     * Sets the associated window.
     *
     * @param window The window associated with this model.
     **/
    void setWindow(KWin::EffectWindow* window);

    /**
     * Overrides the icon geometry used for the animation target.
     * Call before start() to use a different target than iconGeometry().
     **/
    void setIconGeometry(const QRectF& rect);

    /**
     * Returns whether the painted result has to be clipped.
     *
     * @see clipRegion
     **/
    bool needsClip() const;

    /**
     * Returns the clip region.
     *
     * @see needsClip
     **/
    QRegion clipRegion() const;

private:
    void applyBump(KWin::WindowQuadList& quads) const;
    void applyStretch1(KWin::WindowQuadList& quads) const;
    void applyStretch2(KWin::WindowQuadList& quads) const;
    void applySquash(KWin::WindowQuadList& quads) const;

    void updateMinimizeStage();
    void updateUnminimizeStage();

    int computeBumpDistance() const;
    qreal computeShapeFactor() const;

    Parameters m_parameters;

    enum class AnimationStage {
        Bump,
        Stretch1,
        Stretch2,
        Squash
    };

    KWin::EffectWindow* m_window;
    QRectF m_iconGeometry;
    AnimationKind m_kind;
    AnimationStage m_stage;
    KWin::TimeLine m_timeLine;
    Direction m_direction;
    int m_bumpDistance;
    qreal m_shapeFactor;
    bool m_clip;
    bool m_done = false;
};
