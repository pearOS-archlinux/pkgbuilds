/*
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "PearosTinterEffect.h"

KWIN_EFFECT_FACTORY_SUPPORTED_ENABLED(PearosTinterEffect,
    "metadata.json",
    return PearosTinterEffect::supported();
    ,
    return false;)

#include "plugin.moc"
