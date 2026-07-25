/*
 *  SPDX-FileCopyrightText: 2013 Giorgos Tsiapaliokas <terietor@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef VIEW_H
#define VIEW_H

#include <QQuickView>

// Plain QQuickView loading pearos-dock-src's main.qml directly, instead of
// hosting it as a Plasma::Applet inside a Plasma::Containment/Corona (see
// plasmoidshim.h for what replaces the `Plasmoid` attached property that
// hosting used to provide). Keeps only what filer-dock's own main.cpp
// actually needs on top of QQuickView: syncing Qt's idea of this window's
// on-screen position for popups (see syncQtWindowPosition()), same as
// before.
class View : public QQuickView
{
    Q_OBJECT

public:
    explicit View(QWindow *parent = nullptr);
    ~View() override;

    void setBottomMargin(int margin);
    void syncQtWindowPosition();

protected:
    void resizeEvent(QResizeEvent *event) override;

private:
    int m_bottomMargin = 0;
};

#endif // VIEW_H
