#include "scroll_bar_fade.h"
#include <QScrollArea>
#include <QScrollBar>
#include <QGraphicsOpacityEffect>
#include <QTimer>
#include <QPropertyAnimation>

void applyScrollBarFade(QScrollArea* area) {
    if (!area) return;
    QScrollBar* bar = area->verticalScrollBar();
    if (!bar) return;

    QGraphicsOpacityEffect* effect = new QGraphicsOpacityEffect(bar);
    bar->setGraphicsEffect(effect);
    effect->setOpacity(0.0);

    QTimer* hideTimer = new QTimer(area);
    hideTimer->setSingleShot(true);

    QPropertyAnimation* fadeOut = new QPropertyAnimation(effect, "opacity", area);
    fadeOut->setDuration(400);
    fadeOut->setEndValue(0.0);

    QPropertyAnimation* fadeIn = new QPropertyAnimation(effect, "opacity", area);
    fadeIn->setDuration(200);
    fadeIn->setEndValue(1.0);

    QObject::connect(bar, &QScrollBar::valueChanged, area, [effect, fadeIn, fadeOut, hideTimer]() {
        fadeOut->stop();
        fadeIn->stop();
        fadeIn->setStartValue(effect->opacity());
        fadeIn->setEndValue(1.0);
        fadeIn->start();
        hideTimer->start(1000);
    });

    QObject::connect(hideTimer, &QTimer::timeout, area, [effect, fadeOut]() {
        fadeOut->setStartValue(effect->opacity());
        fadeOut->setEndValue(0.0);
        fadeOut->start();
    });
}
