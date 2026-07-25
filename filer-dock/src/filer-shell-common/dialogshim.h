#ifndef DIALOGSHIM_H
#define DIALOGSHIM_H

#include <QHash>
#include <QQmlEngine>
#include <QQuickItem>
#include <QQuickWindow>
#include <QRectF>
#include <QRegion>

// Stand-in for `PlasmaCore.Dialog` -- a floating, frameless popup window
// positioned relative to a `visualParent` item, with no libplasma dependency
// at all (a plain QQuickWindow). Registered as `ShellDialog` (not
// `PlasmaCore.Dialog`) to avoid any ambiguity with the real
// org.kde.plasma.core module, which is still imported elsewhere in these
// files for everything else (Part B of the rewrite plan, not yet done).
//
// Only the properties/behavior actually used by the 6 plasmoid call sites
// that assign real PlasmaCore.Dialog today are implemented: mainItem,
// visualParent, location (Plasma::Types::Location int, for opening
// downward/upward depending on which screen edge the host is anchored to),
// backgroundHints (cosmetic only -- honors NoBackground vs anything else),
// type (accepted but not behaviorally distinguished -- `Popup` is the only
// value ever assigned), hideOnWindowDeactivate, and requestActivate()
// (already inherited from QWindow, works unmodified).
class DialogShim : public QQuickWindow
{
    Q_OBJECT
    QML_NAMED_ELEMENT(ShellDialog)

    Q_PROPERTY(QQuickItem *mainItem READ mainItem WRITE setMainItem NOTIFY mainItemChanged)
    Q_PROPERTY(QQuickItem *visualParent READ visualParent WRITE setVisualParent NOTIFY visualParentChanged)
    Q_PROPERTY(int location READ location WRITE setLocation NOTIFY locationChanged)
    Q_PROPERTY(int backgroundHints READ backgroundHints WRITE setBackgroundHints NOTIFY backgroundHintsChanged)
    Q_PROPERTY(int type READ type WRITE setType NOTIFY typeChanged)
    Q_PROPERTY(bool hideOnWindowDeactivate READ hideOnWindowDeactivate WRITE setHideOnWindowDeactivate NOTIFY hideOnWindowDeactivateChanged)
    // `ShellDialog.Popup`/`ShellDialog.AppletPopup` -- the only two
    // Plasma::Types::WindowType values any of these plasmoids actually
    // assign to `type:`. Not behaviorally distinguished (both are already
    // just frameless popup windows here), only exposed so the assignment
    // itself resolves instead of erroring on an unknown identifier.
    Q_PROPERTY(int Popup READ popupConstant CONSTANT)
    Q_PROPERTY(int AppletPopup READ appletPopupConstant CONSTANT)

public:
    explicit DialogShim(QWindow *parent = nullptr);

    QQuickItem *mainItem() const { return m_mainItem; }
    void setMainItem(QQuickItem *item);

    QQuickItem *visualParent() const { return m_visualParent; }
    void setVisualParent(QQuickItem *item);

    int location() const { return m_location; }
    void setLocation(int location);

    int backgroundHints() const { return m_backgroundHints; }
    void setBackgroundHints(int hints);

    int type() const { return m_type; }
    void setType(int type) { m_type = type; }

    bool hideOnWindowDeactivate() const { return m_hideOnWindowDeactivate; }
    void setHideOnWindowDeactivate(bool hide) { m_hideOnWindowDeactivate = hide; }

    static constexpr int popupConstant() { return 0; }
    static constexpr int appletPopupConstant() { return 1; }

    // Called by individual "glass" widgets (Card.qml's glassEffect rects) to
    // report their own window-local geometry, so KWindowEffects::enableBlurBehind()
    // only blurs those specific rectangles instead of the whole popup --
    // matching Filer's own updateSidebarBlurRegion(), which restricts KWin's
    // blur to just the sidebar column rather than the whole main window.
    // `token` is any QObject identifying the caller (typically `this` from
    // QML); its rect is dropped automatically when it's destroyed.
    Q_INVOKABLE void registerGlassRect(QObject *token, const QRectF &rect);
    Q_INVOKABLE void unregisterGlassRect(QObject *token);

signals:
    void mainItemChanged();
    void visualParentChanged();
    void locationChanged();
    void backgroundHintsChanged();
    void typeChanged();
    void hideOnWindowDeactivateChanged();

private:
    void reposition();
    void updateBlurRegion();

    QQuickItem *m_mainItem = nullptr;
    QQuickItem *m_visualParent = nullptr;
    int m_location = 4; // Plasma::Types::BottomEdge
    int m_backgroundHints = 1; // Plasma::Types::StandardBackground
    int m_type = 0;
    bool m_hideOnWindowDeactivate = false;
    QHash<QObject *, QRectF> m_glassRects;
};

#endif // DIALOGSHIM_H
