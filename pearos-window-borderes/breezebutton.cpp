/*
 * Copyright 2014  Martin Gräßlin <mgraesslin@kde.org>
 * Copyright 2014  Hugo Pereira Da Costa <hugo.pereira@free.fr>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "breezebutton.h"
#include "breezebusywatcher.h"

#include <KColorUtils>
#include <KLocalizedString>
#include <KDecoration3/DecoratedWindow>
//#include <KIconLoader>

#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusMessage>
#include <QDBusReply>
#include <QMouseEvent>
#include <QPainter>
#include <QGuiApplication>
#include <QMenu>
#include <QPainterPath>
#include <QTimer>
#include <QVariantAnimation>

namespace Breeze
{
    using KDecoration3::ColorGroup;
    using KDecoration3::ColorRole;
    using KDecoration3::DecorationButtonType;

    namespace
    {

        //* kwin effects interface, used to bypass the magic lamp animation
        QDBusInterface effectsInterface()
        {
            return QDBusInterface(QStringLiteral("org.kde.KWin"),
                                  QStringLiteral("/Effects"),
                                  QStringLiteral("org.kde.kwin.Effects"));
        }

    }


    //__________________________________________________________________
    Button::Button(DecorationButtonType type, Decoration* decoration, QObject* parent)
        : DecorationButton(type, decoration, parent)
        , m_animation(new QVariantAnimation(this))
    {

        // setup animation
        // It is important start and end value are of the same type, hence 0.0 and not just 0
        m_animation->setStartValue(0.0);
        m_animation->setEndValue(1.0);
        m_animation->setEasingCurve(QEasingCurve::InOutQuad);
        connect(m_animation, &QVariantAnimation::valueChanged, this, [this](const QVariant &value) {
            setOpacity(value.toReal());
        });

        // connections
        connect(decoration->window(), SIGNAL(iconChanged(QIcon)), this, SLOT(update()));
        connect(decoration->settings().get(), &KDecoration3::DecorationSettings::reconfigured, this, &Button::reconfigure);
        connect(this, &KDecoration3::DecorationButton::hoveredChanged, this, &Button::updateAnimationState);

        // hovering the maximize button for a moment offers the tiling options
        if (type == DecorationButtonType::Maximize)
        {
            m_hoverTimer = new QTimer(this);
            m_hoverTimer->setSingleShot(true);
            m_hoverTimer->setInterval(1500);
            connect(m_hoverTimer, &QTimer::timeout, this, &Button::showSnapMenu);
            connect(this, &KDecoration3::DecorationButton::hoveredChanged, this, [this, decoration](bool hovered) {
                if (hovered && decoration->internalSettings()->showSnapMenu()) m_hoverTimer->start();
                else m_hoverTimer->stop();
            });
        }

        // the close button carries the busy indicator
        if (type == DecorationButtonType::Close)
        {
            connect(BusyWatcher::self(), &BusyWatcher::busyChanged, this, [this]() { update(); });
            connect(decoration->window(), &KDecoration3::DecoratedWindow::captionChanged, this, [this]() { update(); });
        }

        reconfigure();

    }

    //__________________________________________________________________
    Button::Button(QObject *parent, const QVariantList &args)
        : Button(args.at(0).value<DecorationButtonType>(), args.at(1).value<Decoration*>(), parent)
    {
        setGeometry(QRectF(QPointF(0, 0), preferredSize()));
    }

    //__________________________________________________________________
    Button *Button::create(DecorationButtonType type, KDecoration3::Decoration *decoration, QObject *parent)
    {
        if (auto d = qobject_cast<Decoration*>(decoration))
        {
            Button *b = new Button(type, d, parent);
            const auto c = d->window();
            switch (type)
            {

                case DecorationButtonType::Close:
                b->setVisible(c->isCloseable());
                QObject::connect(c, &KDecoration3::DecoratedWindow::closeableChanged, b, &Breeze::Button::setVisible);
                break;

                case DecorationButtonType::Maximize:
                b->setVisible(c->isMaximizeable());
                QObject::connect(c, &KDecoration3::DecoratedWindow::maximizeableChanged, b, &Breeze::Button::setVisible);
                break;

                case DecorationButtonType::Minimize:
                b->setVisible(c->isMinimizeable());
                QObject::connect(c, &KDecoration3::DecoratedWindow::minimizeableChanged, b, &Breeze::Button::setVisible);
                break;

                case DecorationButtonType::ContextHelp:
                b->setVisible(c->providesContextHelp());
                QObject::connect(c, &KDecoration3::DecoratedWindow::providesContextHelpChanged, b, &Breeze::Button::setVisible);
                break;

                case DecorationButtonType::Shade:
                b->setVisible(c->isShadeable());
                QObject::connect(c, &KDecoration3::DecoratedWindow::shadeableChanged, b, &Breeze::Button::setVisible);
                break;

                case DecorationButtonType::Menu:
                QObject::connect(c, &KDecoration3::DecoratedWindow::iconChanged, b, [b]() { b->update(); });
                break;

                default: break;

            }

            return b;
        }

        return nullptr;

    }

    //__________________________________________________________________
    void Button::paint(QPainter *painter, const QRectF &repaintRegion)
    {
        Q_UNUSED(repaintRegion)

        if (!decoration()) return;

        painter->save();

        // menu button
        if (type() == DecorationButtonType::Menu)
        {
            const QRectF iconRect = geometry().marginsRemoved(m_padding);
            const auto w = decoration()->window();
            /*if (auto deco = qobject_cast<Decoration *>(decoration())) {
                const QPalette activePalette = KIconLoader::global()->customPalette();
                QPalette palette = w->palette();
                palette.setColor(QPalette::WindowText, deco->fontColor());
                KIconLoader::global()->setCustomPalette(palette);
                w->icon().paint(painter, iconRect.toRect());
                if (activePalette == QPalette()) {
                    KIconLoader::global()->resetPalette();
                } else {
                    KIconLoader::global()->setCustomPalette(palette);
                }
            } else {*/
                w->icon().paint(painter, iconRect.toRect());
            //}
        }
        else
            drawIcon(painter);

        painter->restore();

    }

    //__________________________________________________________________
    void Button::mousePressEvent(QMouseEvent *event)
    {

        auto d = qobject_cast<Decoration*>(decoration());
        if (type() == DecorationButtonType::Close
            && event->button() == Qt::LeftButton
            && d && d->internalSettings()->closeButtonMinimizes()
            && d->window()->isMinimizeable())
        {
            minimizeWindow();
            event->accept();
            return;
        }

        if (type() == DecorationButtonType::Maximize
            && event->button() == Qt::LeftButton
            && d && d->internalSettings()->maximizeButtonFullScreens()
            && d->window()->isActive()
            && fullScreenWindow())
        {
            event->accept();
            return;
        }

        KDecoration3::DecorationButton::mousePressEvent(event);

    }

    //__________________________________________________________________
    bool Button::fullScreenWindow() const
    {

        // KDecoration3 has no full screen request, so the kwin global shortcut is invoked
        // instead; it acts on the active window, hence the caller checks for that
        QDBusInterface shortcuts(QStringLiteral("org.kde.kglobalaccel"),
                                 QStringLiteral("/component/kwin"),
                                 QStringLiteral("org.kde.kglobalaccel.Component"));

        const QDBusMessage reply = shortcuts.call(QStringLiteral("invokeShortcut"), QStringLiteral("Window Fullscreen"));
        return reply.type() != QDBusMessage::ErrorMessage;

    }

    //__________________________________________________________________
    void Button::minimizeWindow()
    {

        auto d = qobject_cast<Decoration*>(decoration());
        if (!d) return;

        // the magic lamp animation is tied to the window geometry, and looks broken when a
        // window is minimized from the close button, so it is unloaded around the request
        bool restoreMagicLamp(false);
        if (d->internalSettings()->closeMinimizeSkipMagicLamp())
        {
            auto interface = effectsInterface();
            const QDBusReply<bool> loaded = interface.call(QStringLiteral("isEffectLoaded"), QStringLiteral("magiclamp"));
            if (loaded.isValid() && loaded.value())
            {
                interface.call(QStringLiteral("unloadEffect"), QStringLiteral("magiclamp"));
                restoreMagicLamp = true;
            }
        }

        d->requestMinimize();

        if (restoreMagicLamp)
        {
            QTimer::singleShot(500, this, []() {
                effectsInterface().call(QStringLiteral("loadEffect"), QStringLiteral("magiclamp"));
            });
        }

    }

    //__________________________________________________________________
    void Button::showSnapMenu()
    {

        auto d = qobject_cast<Decoration*>(decoration());
        if (!d || !isHovered()) return;

        QMenu *menu = new QMenu();
        menu->setAttribute(Qt::WA_DeleteOnClose);

        // KDecoration3 has no tiling request, so the kwin shortcuts are used; the
        // arrangements that move other windows live in the companion kwin script,
        // which registers them as shortcuts of its own
        auto addShortcut = [menu](const QString &title, const QString &shortcut) {
            menu->addAction(title, menu, [shortcut]() {
                QDBusInterface shortcuts(QStringLiteral("org.kde.kglobalaccel"),
                                         QStringLiteral("/component/kwin"),
                                         QStringLiteral("org.kde.kglobalaccel.Component"));
                shortcuts.call(QStringLiteral("invokeShortcut"), shortcut);
            });
        };

        addShortcut(i18n("Tile Left"), QStringLiteral("Window Quick Tile Left"));
        addShortcut(i18n("Tile Right"), QStringLiteral("Window Quick Tile Right"));
        addShortcut(i18n("Tile Top"), QStringLiteral("Window Quick Tile Top"));
        addShortcut(i18n("Tile Bottom"), QStringLiteral("Window Quick Tile Bottom"));

        menu->addSeparator();
        menu->addSection(i18n("Fill & Arrange"));

        menu->addAction(i18n("Maximize"), menu, [d]() { d->requestToggleMaximization(Qt::LeftButton); });
        addShortcut(i18n("Left && Right"), QStringLiteral("pearOS Arrange Two"));
        addShortcut(i18n("Left && Top, Bottom"), QStringLiteral("pearOS Arrange Three"));
        addShortcut(i18n("Four Windows"), QStringLiteral("pearOS Arrange Four"));

        menu->addSeparator();
        menu->addAction(i18n("Full Screen"), menu, [this]() { fullScreenWindow(); });

        // only worth offering when there is somewhere to move to
        if (QGuiApplication::screens().count() > 1)
        {
            menu->addSeparator();
            addShortcut(i18n("Move to Other Display"), QStringLiteral("Window to Next Screen"));
        }

        KDecoration3::Positioner positioner;
        positioner.setAnchorRect(geometry());
        d->popup(positioner, menu);

    }

    //__________________________________________________________________
    bool Button::drawMacOSButton(QPainter *painter) const
    {

        auto d = qobject_cast<Decoration*>(decoration());
        if (!d || !d->internalSettings()->macOSButtons()) return false;

        QColor fill;
        switch (type())
        {
            case DecorationButtonType::Close: fill = QColor(0xff, 0x5c, 0x60); break;
            case DecorationButtonType::Minimize: fill = QColor(0xfa, 0xc8, 0x00); break;
            case DecorationButtonType::Maximize: fill = QColor(0x35, 0xc7, 0x59); break;
            default: return false;
        }

        const bool active(d->window()->isActive());
        const QRectF rect(geometry().marginsRemoved(m_padding));

        painter->setRenderHints(QPainter::Antialiasing);

        // a dot marks a window that is running something, as an edited document is
        // marked on macOS; the symbol shown on hover takes precedence over it
        const bool busy(type() == DecorationButtonType::Close
            && !isHovered() && !isPressed()
            && d->internalSettings()->showBusyIndicator()
            && BusyWatcher::self()->isBusy(d->window()->caption()));

        const qreal dotRadius(rect.width()/5);

        // on inactive windows the circles are plain white at 16% opacity
        if (!active && !isHovered() && !isPressed())
        {
            painter->setPen(Qt::NoPen);
            painter->setBrush(QColor(255, 255, 255, 41));
            painter->drawEllipse(rect);

            if (busy)
            {
                // a dark dot would drown in the faint circle, so this one is lighter
                painter->setBrush(QColor(255, 255, 255, 120));
                painter->drawEllipse(rect.center(), dotRadius, dotRadius);
            }

            return true;
        }

        if (isPressed()) fill = fill.darker(115);

        painter->setPen(QPen(QColor(0, 0, 0, 115), 0.5));
        painter->setBrush(fill);
        painter->drawEllipse(rect.adjusted(0.25, 0.25, -0.25, -0.25));

        if (busy)
        {
            painter->setPen(Qt::NoPen);
            painter->setBrush(QColor(0, 0, 0, 145));
            painter->drawEllipse(rect.center(), dotRadius, dotRadius);
        }

        // the symbol only shows up on hover, as on macOS
        if (isHovered() || isPressed())
        {
            painter->save();
            painter->translate(rect.center());
            painter->scale(rect.width()/14, rect.width()/14);

            QPen pen(QColor(0, 0, 0, 145));
            pen.setWidthF(1.25);
            pen.setCapStyle(Qt::RoundCap);
            painter->setPen(pen);
            painter->setBrush(Qt::NoBrush);

            switch (type())
            {
                case DecorationButtonType::Close:
                painter->drawLine(QPointF(-3, -3), QPointF(3, 3));
                painter->drawLine(QPointF(-3, 3), QPointF(3, -3));
                break;

                case DecorationButtonType::Minimize:
                painter->drawLine(QPointF(-3.5, 0), QPointF(3.5, 0));
                break;

                case DecorationButtonType::Maximize:
                if (isChecked())
                {
                    painter->drawLine(QPointF(-3.5, 0), QPointF(3.5, 0));
                    painter->drawLine(QPointF(0, -3.5), QPointF(0, 3.5));
                }
                else
                {
                    painter->drawLine(QPointF(-3.5, 0), QPointF(3.5, 0));
                    painter->drawLine(QPointF(0, -3.5), QPointF(0, 3.5));
                }
                break;

                default: break;
            }

            painter->restore();
        }

        return true;

    }

    //__________________________________________________________________
    void Button::drawIcon(QPainter *painter) const
    {

        // the pearOS buttons are drawn flat, matching the design
        if (drawMacOSButton(painter)) return;

        painter->setRenderHints(QPainter::Antialiasing);

        /*
        scale painter so that its window matches QRect(-1, -1, 20, 20)
        this makes all further rendering and scaling simpler
        all further rendering is performed inside QRect(0, 0, 18, 18)
        */
        const QRectF rect = geometry().marginsRemoved(m_padding);
        painter->translate(rect.topLeft());

        const qreal width(rect.width());
        painter->scale(width/20, width/20);
        painter->translate(1, 1);

        // render background
        const QColor backgroundColor(this->backgroundColor());

        auto d = qobject_cast<Decoration*>(decoration());
        bool isInactive(d && !d->window()->isActive()
                        && !isHovered() && !isPressed()
                        && m_animation->state() != QAbstractAnimation::Running);
        QColor inactiveCol(Qt::gray);
        if (isInactive)
        {
            int gray = qGray(d->titleBarColor().rgb());
            if (gray <= 200) {
                gray += 55;
                gray = qMax(gray, 115);
            }
            else gray -= 45;
            inactiveCol = QColor(gray, gray, gray);
        }

        // render mark
        const QColor foregroundColor(this->foregroundColor(inactiveCol));
        if (foregroundColor.isValid())
        {

            // setup painter
            QPen pen(foregroundColor);
            pen.setCapStyle(Qt::RoundCap);
            pen.setJoinStyle(Qt::MiterJoin);
            pen.setWidthF(PenWidth::Symbol*qMax((qreal)1.0, 20/width));

            switch (type())
            {

                case DecorationButtonType::Close:
                {
                    if (!d || d->internalSettings()->macOSButtons()) {
                        QLinearGradient grad(QPointF(9, 2), QPointF(9, 16));
                        if (d && qGray(d->titleBarColor().rgb()) > 100)
                        {
                            grad.setColorAt(0, isInactive ? inactiveCol
                                                          : QColor(255, 92, 87));
                            grad.setColorAt(1, isInactive ? inactiveCol
                                                          : QColor(233, 84, 79));
                        }
                        else
                        {
                            grad.setColorAt(0, isInactive ? inactiveCol
                                                          : QColor(250, 100, 102));
                            grad.setColorAt(1, isInactive ? inactiveCol
                                                          : QColor(230, 92, 94));
                        }
                        painter->setBrush(QBrush(grad));
                        painter->setPen(Qt::NoPen);
                        painter->drawEllipse(QRectF(2, 2, 14, 14));
                        if (backgroundColor.isValid())
                        {
                            painter->setPen(Qt::NoPen);
                            painter->setBrush(backgroundColor);
                            qreal r = static_cast<qreal>(7)
                                      + (isPressed() ? 0.0
                                         : static_cast<qreal>(2) * m_animation->currentValue().toReal());
                            QPointF c(static_cast<qreal>(9), static_cast<qreal>(9));
                            painter->drawEllipse(c, r, r);
                        }
                    }
                    else {
                        if (backgroundColor.isValid())
                        {
                            painter->setPen(Qt::NoPen);
                            painter->setBrush(backgroundColor);
                            painter->drawEllipse(QRectF(0, 0, 18, 18));
                        }
                        painter->setPen(pen);
                        painter->setBrush(Qt::NoBrush);

                        painter->drawLine(QPointF(5, 5), QPointF(13, 13));
                        painter->drawLine(QPointF(5, 13), QPointF(13, 5));
                    }
                    break;
                }

                case DecorationButtonType::Maximize:
                {
                    if (!d || d->internalSettings()->macOSButtons()) {
                        QLinearGradient grad(QPointF(9, 2), QPointF(9, 16));
                        if (d && qGray(d->titleBarColor().rgb()) > 100)
                        {
                            grad.setColorAt(0, isChecked() ? isInactive ? inactiveCol
                                                                        : QColor(67, 198, 176)
                                                           : isInactive ? inactiveCol
                                                                        : QColor(40, 211, 63));
                            grad.setColorAt(1, isChecked() ? isInactive ? inactiveCol
                                                                        : QColor(60, 178, 159)
                                                           : isInactive ? inactiveCol
                                                                        : QColor(36, 191, 57));
                        }
                        else
                        {
                            grad.setColorAt(0, isChecked() ? isInactive ? inactiveCol
                                                                        : QColor(67, 198, 176)
                                                           : isInactive ? inactiveCol
                                                                        : QColor(124, 198, 67));
                            grad.setColorAt(1, isChecked() ? isInactive ? inactiveCol
                                                                        : QColor(60, 178, 159)
                                                           : isInactive ? inactiveCol
                                                                        : QColor(111, 178, 60));
                        }
                        painter->setBrush(QBrush(grad));
                        painter->setPen(Qt::NoPen);
                        painter->drawEllipse(QRectF(2, 2, 14, 14));
                        if (backgroundColor.isValid())
                        {
                            painter->setPen(Qt::NoPen);
                            painter->setBrush(backgroundColor);
                            qreal r = static_cast<qreal>(7)
                                      + (isPressed() ? 0.0
                                         : static_cast<qreal>(2) * m_animation->currentValue().toReal());
                            QPointF c(static_cast<qreal>(9), static_cast<qreal>(9));
                            painter->drawEllipse(c, r, r);
                        }
                    }
                    else {
                        if (backgroundColor.isValid())
                        {
                            painter->setPen(Qt::NoPen);
                            painter->setBrush(backgroundColor);
                            painter->drawEllipse(QRectF(0, 0, 18, 18));
                        }

                        if (isHovered())
                            pen.setWidthF(1.2*qMax((qreal)1.0, 20/width));
                        painter->setPen(pen);
                        painter->setBrush(Qt::NoBrush);

                        painter->drawPolyline(QPolygonF()
                                                << QPointF(5, 8) << QPointF(5, 13) << QPointF(10, 13));
                        if (isChecked())
                            painter->drawRect(QRectF(8.0, 5.0, 5.0, 5.0));
                        else {
                            painter->drawPolyline(QPolygonF()
                                                  << QPointF(8, 5) << QPointF(13, 5) << QPointF(13, 10));
                        }

                        if (isHovered())
                            pen.setWidthF(PenWidth::Symbol*qMax((qreal)1.0, 20/width));
                    }
                    break;
                }

                case DecorationButtonType::Minimize:
                {
                    if (!d || d->internalSettings()->macOSButtons()) {
                        QLinearGradient grad(QPointF(9, 2), QPointF(9, 16));
                        if (d && qGray(d->titleBarColor().rgb()) > 100)
                        { // yellow isn't good with light backgrounds
                            grad.setColorAt(0, isInactive ? inactiveCol
                                                          : QColor(243, 176, 43));
                            grad.setColorAt(1, isInactive ? inactiveCol
                                                          : QColor(223, 162, 39));
                        }
                        else
                        {
                            grad.setColorAt(0, isInactive ? inactiveCol
                                                          : QColor(237, 198, 81));
                            grad.setColorAt(1, isInactive ? inactiveCol
                                                          : QColor(217, 181, 74));
                        }
                        painter->setBrush(QBrush(grad));
                        painter->setPen(Qt::NoPen);
                        painter->drawEllipse(QRectF(2, 2, 14, 14));
                        if (backgroundColor.isValid())
                        {
                            painter->setPen(Qt::NoPen);
                            painter->setBrush(backgroundColor);
                            qreal r = static_cast<qreal>(7)
                                      + (isPressed() ? 0.0
                                         : static_cast<qreal>(2) * m_animation->currentValue().toReal());
                            QPointF c(static_cast<qreal>(9), static_cast<qreal>(9));
                            painter->drawEllipse(c, r, r);
                        }
                    }
                    else {
                        if (backgroundColor.isValid())
                        {
                            painter->setPen(Qt::NoPen);
                            painter->setBrush(backgroundColor);
                            painter->drawEllipse(QRectF(0, 0, 18, 18));
                        }

                        if (isHovered())
                            pen.setWidthF(1.2*qMax((qreal)1.0, 20/width));
                        painter->setPen(pen);
                        painter->setBrush(Qt::NoBrush);

                        painter->drawLine(QPointF(4, 9), QPointF(14, 9));

                        if (isHovered())
                            pen.setWidthF(PenWidth::Symbol*qMax((qreal)1.0, 20/width));
                    }
                    break;
                }

                case DecorationButtonType::OnAllDesktops:
                {
                    bool macOSBtn(!d || d->internalSettings()->macOSButtons());
                    if (macOSBtn && !isPressed()) {
                        QLinearGradient grad(QPointF(9, 2), QPointF(9, 16));
                        if (d && qGray(d->titleBarColor().rgb()) > 100)
                        { // yellow isn't good with light backgrounds
                            grad.setColorAt(0, isInactive ? inactiveCol
                                                          : QColor(103, 149, 210));
                            grad.setColorAt(1, isInactive ? inactiveCol
                                                          : QColor(93, 135, 190));
                        }
                        else
                        {
                            grad.setColorAt(0, isInactive ? inactiveCol
                                                          : QColor(135, 166, 220));
                            grad.setColorAt(1, isInactive ? inactiveCol
                                                          : QColor(122, 151, 200));
                        }
                        painter->setBrush(QBrush(grad));
                        painter->setPen(Qt::NoPen);
                        if (isChecked())
                            painter->drawEllipse(QRectF(0, 0, 18, 18));
                        else {
                            painter->drawEllipse(QRectF(2, 2, 14, 14));
                            if (backgroundColor.isValid())
                            {
                                painter->setPen(Qt::NoPen);
                                painter->setBrush(backgroundColor);
                                qreal r = static_cast<qreal>(7)
                                          + static_cast<qreal>(2) * m_animation->currentValue().toReal();
                                QPointF c(static_cast<qreal>(9), static_cast<qreal>(9));
                                painter->drawEllipse(c, r, r);
                            }
                        }
                    }
                    if (!macOSBtn || isPressed() || isHovered() || isChecked()) {
                        painter->setPen(Qt::NoPen);
                        if ((!macOSBtn  || isPressed()) && backgroundColor.isValid())
                        {
                            painter->setBrush(backgroundColor);
                            painter->drawEllipse(QRectF(0, 0, 18, 18));
                        }
                        painter->setBrush(foregroundColor);

                        if (macOSBtn)
                            painter->drawEllipse(QRectF(6, 6, 6, 6));
                        else {
                            if (isChecked()) {

                                // outer ring
                                painter->drawEllipse(QRectF(3, 3, 12, 12));

                                // center dot
                                QColor backgroundColor(this->backgroundColor());
                                if (!backgroundColor.isValid() && d) backgroundColor = d->titleBarColor();

                                if (backgroundColor.isValid())
                                {
                                    painter->setBrush(backgroundColor);
                                    painter->drawEllipse(QRectF(8, 8, 2, 2));
                                }

                            }
                            else {
                                painter->drawPolygon(QPolygonF()
                                    << QPointF(6.5, 8.5)
                                    << QPointF(12, 3)
                                    << QPointF(15, 6)
                                    << QPointF(9.5, 11.5));

                                painter->setPen(pen);
                                painter->drawLine(QPointF(5.5, 7.5), QPointF(10.5, 12.5));
                                painter->drawLine(QPointF(12, 6), QPointF(4.5, 13.5));
                            }
                        }
                    }
                    break;
                }

                case DecorationButtonType::Shade:
                {
                    bool macOSBtn(!d || d->internalSettings()->macOSButtons());
                    if (macOSBtn && !isPressed()) {
                        QLinearGradient grad(QPointF(9, 2), QPointF(9, 16));
                        if (d && qGray(d->titleBarColor().rgb()) > 100)
                        { // yellow isn't good with light backgrounds
                            grad.setColorAt(0, isInactive ? inactiveCol
                                                          : QColor(103, 149, 210));
                            grad.setColorAt(1, isInactive ? inactiveCol
                                                          : QColor(93, 135, 190));
                        }
                        else
                        {
                            grad.setColorAt(0, isInactive ? inactiveCol
                                                          : QColor(135, 166, 220));
                            grad.setColorAt(1, isInactive ? inactiveCol
                                                          : QColor(122, 151, 200));
                        }
                        painter->setBrush(QBrush(grad));
                        painter->setPen(Qt::NoPen);
                        if (isChecked())
                            painter->drawEllipse(QRectF(0, 0, 18, 18));
                        else {
                            painter->drawEllipse(QRectF(2, 2, 14, 14));
                            if (backgroundColor.isValid())
                            {
                                painter->setPen(Qt::NoPen);
                                painter->setBrush(backgroundColor);
                                qreal r = static_cast<qreal>(7)
                                          + static_cast<qreal>(2) * m_animation->currentValue().toReal();
                                QPointF c(static_cast<qreal>(9), static_cast<qreal>(9));
                                painter->drawEllipse(c, r, r);
                            }
                        }
                    }
                    if (!macOSBtn || isPressed() || isHovered() || isChecked()) {
                        if ((!macOSBtn  || isPressed()) && backgroundColor.isValid())
                        {
                            painter->setPen(Qt::NoPen);
                            painter->setBrush(backgroundColor);
                            painter->drawEllipse(QRectF(0, 0, 18, 18));
                        }
                        painter->setPen(pen);
                        painter->setBrush(Qt::NoBrush);

                        painter->drawLine(5, 6, 13, 6);
                        if (isChecked()) {
                            painter->drawPolyline(QPolygonF()
                                << QPointF(5, 9)
                                << QPointF(9, 13)
                                << QPointF(13, 9));

                        }
                        else {
                            painter->drawPolyline(QPolygonF()
                                << QPointF(5, 13)
                                << QPointF(9, 9)
                                << QPointF(13, 13));
                        }
                    }

                    break;

                }

                case DecorationButtonType::KeepBelow:
                {
                    bool macOSBtn(!d || d->internalSettings()->macOSButtons() || isChecked());
                    if (macOSBtn && !isPressed()) {
                        QLinearGradient grad(QPointF(9, 2), QPointF(9, 16));
                        if (d && qGray(d->titleBarColor().rgb()) > 100)
                        { // yellow isn't good with light backgrounds
                            grad.setColorAt(0, isInactive ? inactiveCol
                                                          : QColor(103, 149, 210));
                            grad.setColorAt(1, isInactive ? inactiveCol
                                                          : QColor(93, 135, 190));
                        }
                        else
                        {
                            grad.setColorAt(0, isInactive ? inactiveCol
                                                          : QColor(135, 166, 220));
                            grad.setColorAt(1, isInactive ? inactiveCol
                                                          : QColor(122, 151, 200));
                        }
                        painter->setBrush(QBrush(grad));
                        painter->setPen(Qt::NoPen);
                        if (isChecked())
                            painter->drawEllipse(QRectF(0, 0, 18, 18));
                        else {
                            painter->drawEllipse(QRectF(2, 2, 14, 14));
                            if (backgroundColor.isValid())
                            {
                                painter->setPen(Qt::NoPen);
                                painter->setBrush(backgroundColor);
                                qreal r = static_cast<qreal>(7)
                                          + static_cast<qreal>(2) * m_animation->currentValue().toReal();
                                QPointF c(static_cast<qreal>(9), static_cast<qreal>(9));
                                painter->drawEllipse(c, r, r);
                            }
                        }
                    }
                    if (!macOSBtn || isPressed() || isHovered() || isChecked()) {
                        if ((!macOSBtn  || isPressed()) && backgroundColor.isValid())
                        {
                            painter->setPen(Qt::NoPen);
                            painter->setBrush(backgroundColor);
                            painter->drawEllipse(QRectF(0, 0, 18, 18));
                        }
                        painter->setPen(pen);
                        painter->setBrush(Qt::NoBrush);

                        if (macOSBtn) {
                            painter->drawPolyline(QPolygonF()
                                << QPointF(6, 6)
                                << QPointF(9, 9)
                                << QPointF(12, 6));

                            painter->drawPolyline(QPolygonF()
                                << QPointF(6, 10)
                                << QPointF(9, 13)
                                << QPointF(12, 10));
                        }
                        else {
                            painter->drawPolyline(QPolygonF()
                                << QPointF(5, 5)
                                << QPointF(9, 9)
                                << QPointF(13, 5));

                            painter->drawPolyline(QPolygonF()
                                << QPointF(5, 9)
                                << QPointF(9, 13)
                                << QPointF(13, 9));
                        }
                    }
                    break;

                }

                case DecorationButtonType::KeepAbove:
                {
                    bool macOSBtn(!d || d->internalSettings()->macOSButtons());
                    if (macOSBtn && !isPressed()) {
                        QLinearGradient grad(QPointF(9, 2), QPointF(9, 16));
                        if (d && qGray(d->titleBarColor().rgb()) > 100)
                        { // yellow isn't good with light backgrounds
                            grad.setColorAt(0, isInactive ? inactiveCol
                                                          : QColor(103, 149, 210));
                            grad.setColorAt(1, isInactive ? inactiveCol
                                                          : QColor(93, 135, 190));
                        }
                        else
                        {
                            grad.setColorAt(0, isInactive ? inactiveCol
                                                          : QColor(135, 166, 220));
                            grad.setColorAt(1, isInactive ? inactiveCol
                                                          : QColor(122, 151, 200));
                        }
                        painter->setBrush(QBrush(grad));
                        painter->setPen(Qt::NoPen);
                        if (isChecked())
                            painter->drawEllipse(QRectF(0, 0, 18, 18));
                        else {
                            painter->drawEllipse(QRectF(2, 2, 14, 14));
                            if (backgroundColor.isValid())
                            {
                                painter->setPen(Qt::NoPen);
                                painter->setBrush(backgroundColor);
                                qreal r = static_cast<qreal>(7)
                                          + static_cast<qreal>(2) * m_animation->currentValue().toReal();
                                QPointF c(static_cast<qreal>(9), static_cast<qreal>(9));
                                painter->drawEllipse(c, r, r);
                            }
                        }
                    }
                    if (!macOSBtn || isPressed() || isHovered() || isChecked()) {
                        if ((!macOSBtn  || isPressed()) && backgroundColor.isValid())
                        {
                            painter->setPen(Qt::NoPen);
                            painter->setBrush(backgroundColor);
                            painter->drawEllipse(QRectF(0, 0, 18, 18));
                        }
                        painter->setPen(pen);
                        painter->setBrush(Qt::NoBrush);

                        if (macOSBtn) {
                            painter->drawPolyline(QPolygonF()
                                << QPointF(6, 8)
                                << QPointF(9, 5)
                                << QPointF(12, 8));

                            painter->drawPolyline(QPolygonF()
                                << QPointF(6, 12)
                                << QPointF(9, 9)
                                << QPointF(12, 12));
                        }
                        else {
                            painter->drawPolyline(QPolygonF()
                                << QPointF(5, 9)
                                << QPointF(9, 5)
                                << QPointF(13, 9));

                            painter->drawPolyline(QPolygonF()
                                << QPointF(5, 13)
                                << QPointF(9, 9)
                                << QPointF(13, 13));
                        }
                    }
                    break;
                }


                case DecorationButtonType::ApplicationMenu:
                {
                    bool macOSBtn(!d || d->internalSettings()->macOSButtons());
                    if (macOSBtn && !isPressed()) {
                        QLinearGradient grad(QPointF(9, 2), QPointF(9, 16));
                        if (d && qGray(d->titleBarColor().rgb()) > 100)
                        { // yellow isn't good with light backgrounds
                            grad.setColorAt(0, isInactive ? inactiveCol
                                                          : QColor(230, 129, 67));
                            grad.setColorAt(1, isInactive ? inactiveCol
                                                          : QColor(210, 118, 61));
                        }
                        else
                        {
                            grad.setColorAt(0, isInactive ? inactiveCol
                                                          : QColor(250, 145, 100));
                            grad.setColorAt(1, isInactive ? inactiveCol
                                                          : QColor(230, 131, 92));
                        }
                        painter->setBrush(QBrush(grad));
                        painter->setPen(Qt::NoPen);
                        painter->drawEllipse(QRectF(2, 2, 14, 14));
                        if (backgroundColor.isValid())
                        {
                            painter->setPen(Qt::NoPen);
                            painter->setBrush(backgroundColor);
                            qreal r = static_cast<qreal>(7)
                                      + static_cast<qreal>(2) * m_animation->currentValue().toReal();
                            QPointF c(static_cast<qreal>(9), static_cast<qreal>(9));
                            painter->drawEllipse(c, r, r);
                        }
                    }
                    if (!macOSBtn || isPressed() || isHovered()) {
                        if ((!macOSBtn  || isPressed()) && backgroundColor.isValid())
                        {
                            painter->setPen(Qt::NoPen);
                            painter->setBrush(backgroundColor);
                            painter->drawEllipse(QRectF(0, 0, 18, 18));
                        }
                        painter->setPen(pen);
                        painter->setBrush(Qt::NoBrush);

                        if (macOSBtn) {
                            painter->drawLine(QPointF(4.5, 6), QPointF(13.5, 6));
                            painter->drawLine(QPointF(4.5, 9), QPointF(13.5, 9));
                            painter->drawLine(QPointF(4.5, 12), QPointF(13.5, 12));
                        }
                        else {
                            painter->drawLine(QPointF(3.5, 5), QPointF(14.5, 5));
                            painter->drawLine(QPointF(3.5, 9), QPointF(14.5, 9));
                            painter->drawLine(QPointF(3.5, 13), QPointF(14.5, 13));
                        }
                    }
                    break;
                }

                case DecorationButtonType::ContextHelp:
                {
                    bool macOSBtn(!d || d->internalSettings()->macOSButtons());
                    if (macOSBtn && !isPressed()) {
                        QLinearGradient grad(QPointF(9, 2), QPointF(9, 16));
                        if (d && qGray(d->titleBarColor().rgb()) > 100)
                        { // yellow isn't good with light backgrounds
                            grad.setColorAt(0, isInactive ? inactiveCol
                                                          : QColor(103, 149, 210));
                            grad.setColorAt(1, isInactive ? inactiveCol
                                                          : QColor(93, 135, 190));
                        }
                        else
                        {
                            grad.setColorAt(0, isInactive ? inactiveCol
                                                          : QColor(135, 166, 220));
                            grad.setColorAt(1, isInactive ? inactiveCol
                                                          : QColor(122, 151, 200));
                        }
                        painter->setBrush(QBrush(grad));
                        painter->setPen(Qt::NoPen);
                        painter->drawEllipse(QRectF(2, 2, 14, 14));
                        if (backgroundColor.isValid())
                        {
                            painter->setPen(Qt::NoPen);
                            painter->setBrush(backgroundColor);
                            qreal r = static_cast<qreal>(7)
                                      + static_cast<qreal>(2) * m_animation->currentValue().toReal();
                            QPointF c(static_cast<qreal>(9), static_cast<qreal>(9));
                            painter->drawEllipse(c, r, r);
                        }
                    }
                    if (!macOSBtn || isPressed() || isHovered()) {
                        if ((!macOSBtn  || isPressed()) && backgroundColor.isValid())
                        {
                            painter->setPen(Qt::NoPen);
                            painter->setBrush(backgroundColor);
                            painter->drawEllipse(QRectF(0, 0, 18, 18));
                        }
                        painter->setPen(pen);
                        painter->setBrush(Qt::NoBrush);

                        QPainterPath path;
                        path.moveTo(5, 6);
                        path.arcTo(QRectF(5, 3.5, 8, 5), 180, -180);
                        path.cubicTo(QPointF(12.5, 9.5), QPointF(9, 7.5), QPointF(9, 11.5));
                        painter->drawPath(path);

                        painter->drawPoint(9, 15);
                    }

                    break;
                }

                default: break;

            }

        }

    }

    //__________________________________________________________________
    QColor Button::foregroundColor(const QColor& inactiveCol) const
    {
        auto d = qobject_cast<Decoration*>(decoration());
        if (!d || d->internalSettings()->macOSButtons()) {
            QColor col;
            if (d && !d->window()->isActive()
                && !isHovered() && !isPressed()
                && m_animation->state() != QAbstractAnimation::Running)
            {
                int v = qGray(inactiveCol.rgb());
                if (v > 127) v -= 127;
                else v += 128;
                col = QColor(v, v, v);
            }
            else
            {
                if (d && qGray(d->titleBarColor().rgb()) > 100)
                    col = QColor(250, 250, 250);
                else
                    col = QColor(40, 40, 40);
            }
            return col;
        }
        else if (!d) {

            return QColor();

        } else if (isPressed()) {

            return d->titleBarColor();

        /*} else if (type() == DecorationButtonType::Close && d->internalSettings()->outlineCloseButton()) {

            return d->titleBarColor();*/

        } else if ((type() == DecorationButtonType::KeepBelow || type() == DecorationButtonType::KeepAbove) && isChecked()) {

            return d->titleBarColor();

        } else if (m_animation->state() == QAbstractAnimation::Running) {

            return KColorUtils::mix(d->fontColor(), d->titleBarColor(), m_opacity);

        } else if (isHovered()) {

            return d->titleBarColor();

        } else {

            return d->fontColor();

        }

    }

    //__________________________________________________________________
    QColor Button::backgroundColor() const
    {
        auto d = qobject_cast<Decoration*>(decoration());
        if (!d) {

            return QColor();

        }

        if (d->internalSettings()->macOSButtons()) {
            if (isPressed()) {

                QColor col;
                if (type() == DecorationButtonType::Close)
                {
                    if (qGray(d->titleBarColor().rgb()) > 100)
                        col = QColor(254, 73, 66);
                    else
                        col = QColor(240, 77, 80);
                }
                else if (type() == DecorationButtonType::Maximize)
                {
                    if (qGray(d->titleBarColor().rgb()) > 100)
                        col = isChecked() ? QColor(0, 188, 154) : QColor(7, 201, 33);
                    else
                        col = isChecked() ? QColor(0, 188, 154) : QColor(101, 188, 34);
                }
                else if (type() == DecorationButtonType::Minimize)
                {
                    if (qGray(d->titleBarColor().rgb()) > 100)
                        col = QColor(233, 160, 13);
                    else
                        col = QColor(227, 185, 59);
                }
                else if (type() == DecorationButtonType::ApplicationMenu) {
                    if (qGray(d->titleBarColor().rgb()) > 100)
                        col = QColor(220, 124, 64);
                    else
                        col = QColor(240, 139, 96);
                }
                else {
                    if (qGray(d->titleBarColor().rgb()) > 100)
                        col = QColor(83, 121, 170);
                    else
                        col = QColor(110, 136, 180);
                }
                if (col.isValid())
                    return col;
                else return KColorUtils::mix(d->titleBarColor(), d->fontColor(), 0.3);

            } else if (m_animation->state() == QAbstractAnimation::Running) {

                QColor col;
                if (type() == DecorationButtonType::Close)
                {
                    if (qGray(d->titleBarColor().rgb()) > 100)
                        col = QColor(254, 95, 87);
                    else
                        col = QColor(240, 96, 97);
                }
                else if (type() == DecorationButtonType::Maximize)
                {
                    if (qGray(d->titleBarColor().rgb()) > 100)
                        col = isChecked() ? QColor(64, 188, 168) : QColor(39, 201, 63);
                    else
                        col = isChecked() ? QColor(64, 188, 168) : QColor(116, 188, 64);
                }
                else if (type() == DecorationButtonType::Minimize)
                {
                    if (qGray(d->titleBarColor().rgb()) > 100)
                        col = QColor(233, 172, 41);
                    else
                        col = QColor(227, 191, 78);
                }
                else if (type() == DecorationButtonType::ApplicationMenu) {
                    if (qGray(d->titleBarColor().rgb()) > 100)
                        col = QColor(220, 124, 64);
                    else
                        col = QColor(240, 139, 96);
                }
                else {
                    if (qGray(d->titleBarColor().rgb()) > 100)
                        col = QColor(98, 141, 200);
                    else
                        col = QColor(128, 157, 210);
                }
                if (col.isValid())
                    return col;
                else {

                    col = d->fontColor();
                    col.setAlpha(col.alpha()*m_opacity);
                    return col;

                }

            } else if (isHovered()) {

                QColor col;
                if (type() == DecorationButtonType::Close)
                {
                    if (qGray(d->titleBarColor().rgb()) > 100)
                        col = QColor(254, 95, 87);
                    else
                        col = QColor(240, 96, 97);
                }
                else if (type() == DecorationButtonType::Maximize)
                {
                    if (qGray(d->titleBarColor().rgb()) > 100)
                        col = isChecked() ? QColor(64, 188, 168) : QColor(39, 201, 63);
                    else
                        col = isChecked() ? QColor(64, 188, 168) : QColor(116, 188, 64);
                }
                else if (type() == DecorationButtonType::Minimize)
                {
                    if (qGray(d->titleBarColor().rgb()) > 100)
                        col = QColor(233, 172, 41);
                    else
                        col = QColor(227, 191, 78);
                }
                else if (type() == DecorationButtonType::ApplicationMenu) {
                    if (qGray(d->titleBarColor().rgb()) > 100)
                        col = QColor(220, 124, 64);
                    else
                        col = QColor(240, 139, 96);
                }
                else {
                    if (qGray(d->titleBarColor().rgb()) > 100)
                        col = QColor(98, 141, 200);
                    else
                        col = QColor(128, 157, 210);
                }
                if (col.isValid())
                    return col;
                else return d->fontColor();

            } else {

                return QColor();

            }
        }
        else {
            auto c = d->window();
            if (isPressed()) {

                if (type() == DecorationButtonType::Close) return c->color(ColorGroup::Warning, ColorRole::Foreground);
                else
                {
                    QColor col;
                    if (qGray(d->titleBarColor().rgb()) > 100)
                        col = QColor(0, 0, 0, 190);
                    else
                        col = QColor(255, 255, 255, 210);
                    return col;
                }

            } else if ((type() == DecorationButtonType::KeepBelow || type() == DecorationButtonType::KeepAbove) && isChecked()) {

                    QColor col;
                    if (qGray(d->titleBarColor().rgb()) > 100)
                        col = QColor(0, 0, 0, 165);
                    else
                        col = QColor(255, 255, 255, 180);
                    return col;

            } else if (m_animation->state() == QAbstractAnimation::Running) {

                if (type() == DecorationButtonType::Close)
                {

                    QColor color(c->color(ColorGroup::Warning, ColorRole::Foreground).lighter());
                    color.setAlpha(color.alpha()*m_opacity);
                    return color;

                } else {

                    QColor col;
                    if (qGray(d->titleBarColor().rgb()) > 100)
                        col = QColor(0, 0, 0, 165);
                    else
                        col = QColor(255, 255, 255, 180);
                    col.setAlpha(col.alpha()*m_opacity);
                    return col;

                }

            } else if (isHovered()) {

                if (type() == DecorationButtonType::Close) return c->color(ColorGroup::Warning, ColorRole::Foreground).lighter();
                else
                {

                    QColor col;
                    if (qGray(d->titleBarColor().rgb()) > 100)
                        col = QColor(0, 0, 0, 165);
                    else
                        col = QColor(255, 255, 255, 180);
                    return col;

                }

            } else {

                return QColor();

            }
        }

    }

    //________________________________________________________________
    void Button::reconfigure()
    {

        // animation
        if (auto d = qobject_cast<Decoration*>(decoration()))
        {
            m_animation->setDuration(d->internalSettings()->animationsDuration());
            setPreferredSize(QSizeF(d->buttonSize(), d->buttonSize()));
        }

    }

    //__________________________________________________________________
    void Button::updateAnimationState(bool hovered)
    {

        auto d = qobject_cast<Decoration*>(decoration());
        if (!(d && d->internalSettings()->animationsEnabled())) return;

        QAbstractAnimation::Direction dir = hovered ? QAbstractAnimation::Forward : QAbstractAnimation::Backward;
        if (m_animation->state() == QAbstractAnimation::Running && m_animation->direction() != dir)
            m_animation->stop();
        m_animation->setDirection(dir);
        if (m_animation->state() != QAbstractAnimation::Running) m_animation->start();

    }

} // namespace
