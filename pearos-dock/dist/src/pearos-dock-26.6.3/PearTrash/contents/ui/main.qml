import QtQuick
import QtQuick.Layouts

import org.kde.plasma.plasmoid
import org.kde.plasma.core as PlasmaCore
import org.kde.plasma.extras as PlasmaExtras
import org.kde.draganddrop as DragDrop
import org.kde.plasma.plasma5support as Plasma5Support
import org.kde.kirigami as Kirigami

import org.kde.kcmutils as KCM
import org.kde.config as KConfig

PlasmoidItem {
    id: root

    readonly property bool inPanel: (Plasmoid.location === PlasmaCore.Types.TopEdge
        || Plasmoid.location === PlasmaCore.Types.RightEdge
        || Plasmoid.location === PlasmaCore.Types.BottomEdge
        || Plasmoid.location === PlasmaCore.Types.LeftEdge)
    readonly property bool hasContents: dirModelCount > 0

    property bool containsAcceptableDrag: false

    Plasmoid.title: i18nc("@title the name of the Trash widget", "Trash")
    toolTipSubText: hasContents
        ? i18ncp("@info:status The trash contains this many items in it", "One item", "%1 items", dirModelCount)
        : i18nc("@info:status The trash is empty", "Empty")

    Plasmoid.backgroundHints: PlasmaCore.Types.NoBackground
    Plasmoid.icon: hasContents ? "/usr/share/extras/user-trash-full.svg" : "/usr/share/extras/user-trash.svg"

    Plasmoid.onActivated: {
        // Open Nautilus with trash://
        openTrashSource.connectSource("nautilus trash://")
    }

    Keys.onPressed: {
        switch (event.key) {
        case Qt.Key_Space:
        case Qt.Key_Enter:
        case Qt.Key_Return:
        case Qt.Key_Select:
            Plasmoid.activated();
            break;
        }
    }
    Accessible.name: Plasmoid.title
    Accessible.description: toolTipSubText
    Accessible.role: Accessible.Button

    Plasma5Support.DataSource {
        id: dirModelSource
        engine: "executable"
        connectedSources: []
        
        function checkTrash() {
            dirModelSource.connectSource("kioclient5 ls trash:/")
        }
        
        onNewData: function(source, data) {
            disconnectSource(source)
            if (data.stdout) {
                var lines = data.stdout.split('\n').filter(line => {
                    var trimmed = line.trim()
                    // Filter out empty lines and the current directory indicator (.)
                    return trimmed.length > 0 && trimmed !== '.'
                })
                root.dirModelCount = lines.length
            } else {
                root.dirModelCount = 0
            }
        }
    }
    
    property int dirModelCount: 0
    
    Timer {
        id: trashCheckTimer
        interval: 1000
        running: false
        repeat: true
        onTriggered: dirModelSource.checkTrash()
    }

    Plasmoid.contextualActions: [
        PlasmaCore.Action {
            text: i18nc("@action:inmenu Open the trash", "Open")
            icon.name: "document-open"
            onTriggered: Plasmoid.activated()
        },
        PlasmaCore.Action {
            text: i18nc("@action:inmenu Empty the trash", "Empty")
            icon.name: "trash-empty"
            enabled: hasContents
            onTriggered: {
                emptyTrashSource.connectSource("sh -c 'rm -rf ~/.local/share/Trash/files/* ~/.local/share/Trash/info/* 2>/dev/null || true'")
            }
        },
        PlasmaCore.Action {
            text: i18nc("@action:inmenu", "Trash Settings…")
            icon.name: "configure"
            visible: KConfig.KAuthorized.authorizeControlModule("kcm_trash")
            onTriggered: KCM.KCMLauncher.open("kcm_trash")
        }
    ]

    Plasma5Support.DataSource {
        id: openTrashSource
        engine: "executable"
        connectedSources: []
        onNewData: function(source, data) {
            disconnectSource(source)
        }
    }
    
    Plasma5Support.DataSource {
        id: emptyTrashSource
        engine: "executable"
        connectedSources: []
        onNewData: function(source, data) {
            disconnectSource(source)
            dirModelSource.checkTrash()
        }
    }
    
    Plasma5Support.DataSource {
        id: trashSource
        engine: "executable"
        connectedSources: []
        onNewData: function(source, data) {
            disconnectSource(source)
            dirModelSource.checkTrash()
        }
    }

    Component.onCompleted: {
        Plasmoid.removeInternalAction("configure");
        dirModelSource.checkTrash()
        trashCheckTimer.start()
    }

    // Only exists because the default CompactRepresentation doesn't:
    // - allow defining a custom drop handler
    // - expose the ability to show text below or beside the icon
    // TODO remove once it gains those features
    preferredRepresentation: fullRepresentation
    fullRepresentation: MouseArea {
        id: mouseArea

        activeFocusOnTab: true
        hoverEnabled: true

        onClicked: Plasmoid.activated()

        DragDrop.DropArea {
            anchors.fill: parent
            preventStealing: true
            onDragEnter: {
                // Accept all URLs for now - KIO will handle validation
                root.containsAcceptableDrag = event.mimeData.urls.length > 0
            }
            onDragLeave: root.containsAcceptableDrag = false

            onDrop: {
                root.containsAcceptableDrag = false

                var urls = event.mimeData.urls
                if (urls.length > 0) {
                    // Use kioclient5 to move files to trash
                    for (var i = 0; i < urls.length; i++) {
                        trashSource.connectSource("kioclient5 move " + urls[i] + " trash:/")
                    }
                    event.accept(Qt.MoveAction)
                } else {
                    event.accept(Qt.IgnoreAction) // prevent Plasma from spawning an applet
                }
            }
        }

        Kirigami.Icon {
            source: Plasmoid.icon
            anchors {
                left: parent.left
                right: parent.right
                top: parent.top
                bottom: root.inPanel ? parent.bottom: text.top
            }
            active: root.containsAcceptableDrag
        }

        PlasmaExtras.ShadowedLabel {
            id: text
            anchors {
                horizontalCenter: parent.horizontalCenter
                bottom: parent.bottom
            }
            width: Math.round(text.implicitWidth + Kirigami.Units.smallSpacing) // make sure label is not blurry
            text: Plasmoid.title + "\n" + root.toolTipSubText
            horizontalAlignment: Text.AlignHCenter
            visible: !root.inPanel
        }

        PlasmaCore.ToolTipArea {
            anchors.fill: parent
            mainText: Plasmoid.title
            subText:  root.toolTipSubText
        }
    }
}
