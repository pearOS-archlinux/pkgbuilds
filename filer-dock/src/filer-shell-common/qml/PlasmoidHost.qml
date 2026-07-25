import QtQuick
import QtQuick.Layouts

// Stand-in for org.kde.plasma.plasmoid's PlasmoidItem, used as every hosted
// plasmoid's new QML root (replacing `PlasmoidItem { ... }`) now that there
// is no real Plasma::Applet to provide it. Plain `Item` -- so `Layout.*`
// attached properties a plasmoid sets directly on its root (e.g.
// luisbocanegra.panelspacer.extended's `Layout.fillWidth: ...`) still work
// unchanged once parented into RowHost.qml's RowLayout. `Plasmoid.foo:
// value` attached-property assignment syntax (PearClock's `Plasmoid.title:
// ...` etc) resolves independently of this file's root type -- see
// plasmoidattachroot.h's PlasmoidAttachedProvider, which registers `Plasmoid`
// itself as the attachable type name (QML attached properties attach to any
// QObject, regardless of its own class, so the root here doesn't need to be
// anything special).
//
// Most of the hosted plasmoids put their real content directly as children
// of the root (never touching compact/full/preferredRepresentation at all,
// same as PlasmoidItem itself would in that case) -- those just work as-is.
// The few that DO assign `fullRepresentation:`/`preferredRepresentation:`
// (PearClock, PearControlCentre, ...) get it loaded here, sized to fill.
Item {
    id: root

    property Component compactRepresentation
    property Component fullRepresentation
    property Component preferredRepresentation

    implicitWidth: loader.item ? loader.item.implicitWidth : 0
    implicitHeight: loader.item ? loader.item.implicitHeight : 0

    // fullRepresentation/preferredRepresentation items (PearClock,
    // PearControlCentre) set their own Layout.preferredWidth/minimumWidth
    // expecting to be a direct RowLayout child, as they would be if this
    // were the real PlasmoidItem -- but they're actually one level down,
    // inside this Loader. Forward those attached values up to root's own
    // Layout property so RowHost.qml's RowLayout sizes root correctly
    // instead of falling back to implicitWidth (0, since a plain Item's
    // implicitWidth is never set just because Layout.preferredWidth was).
    Layout.preferredWidth: loader.item && loader.item.Layout.preferredWidth > 0
        ? loader.item.Layout.preferredWidth : implicitWidth
    Layout.minimumWidth: loader.item ? loader.item.Layout.minimumWidth : 0

    // Deliberately NOT forwarding Layout.preferredHeight/minimumHeight the
    // same way: xyz.pearos.pearmenu's MainMenuButton sets
    // `Layout.preferredHeight: root.height` where "root" (via the QML
    // context-climbing name resolution this project already relies on
    // elsewhere) means THIS root item -- forwarding that value back up
    // creates a real circular binding (root.height depends on
    // loader.item.Layout.preferredHeight depends on root.height), which Qt
    // resolves to 0, collapsing the whole button. Every hosted plasmoid in
    // this fixed-height top bar is meant to fill the row's height anyway,
    // so just do that directly instead.
    Layout.fillHeight: true

    Loader {
        id: loader
        anchors.fill: parent
        active: root.preferredRepresentation || root.fullRepresentation || root.compactRepresentation
        sourceComponent: root.preferredRepresentation
            ? root.preferredRepresentation
            : (root.fullRepresentation ? root.fullRepresentation : root.compactRepresentation)
    }
}
