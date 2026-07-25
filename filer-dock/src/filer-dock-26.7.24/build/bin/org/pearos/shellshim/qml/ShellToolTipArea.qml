import QtQuick
import org.pearos.shellshim

// Stand-in for `PlasmaCore.ToolTipArea` -- a plain Item that shows a small
// ShellDialog-backed popup after a short hover delay, with either custom
// content (`mainItem`) or a simple mainText/subText label pair (the default,
// covering PearAppTitle/PearClock/PearPrivacy/luisbocanegra's usage).
Item {
    id: root

    property bool active: true
    property bool interactive: false
    // Plasma::Types::Location (see filer-shell-common/CMakeLists.txt's
    // header comment for where these values come from) -- BottomEdge
    // default matches real ToolTipArea's own default.
    property int location: 4
    property Item mainItem: null
    property string mainText: ""
    property string subText: ""
    property var icon: undefined

    HoverHandler {
        id: hoverHandler
        enabled: root.active
    }

    Timer {
        id: showTimer
        interval: 500
        onTriggered: dialogLoader.active = true
    }

    onActiveChanged: if (!active) {
        showTimer.stop();
        dialogLoader.active = false;
    }

    Connections {
        target: hoverHandler
        function onHoveredChanged() {
            if (hoverHandler.hovered && root.active) {
                showTimer.start();
            } else {
                showTimer.stop();
                dialogLoader.active = false;
            }
        }
    }

    Component {
        id: defaultContent
        Column {
            padding: 8
            spacing: 2
            Text { text: root.mainText; font.bold: true; visible: text !== "" }
            Text { text: root.subText; visible: text !== "" }
        }
    }

    Loader {
        id: dialogLoader
        active: false
        sourceComponent: ShellDialog {
            visualParent: root
            location: root.location
            visible: true
            mainItem: (root.mainItem ? root.mainItem : defaultContentLoader.item)

            Loader {
                id: defaultContentLoader
                active: !root.mainItem
                sourceComponent: defaultContent
            }
        }
    }
}
