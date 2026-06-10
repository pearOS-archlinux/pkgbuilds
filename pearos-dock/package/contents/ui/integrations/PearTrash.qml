import QtQuick
import QtQuick.Controls as QQC2

import org.kde.plasma.plasmoid
import org.kde.plasma.core as PlasmaCore
import org.kde.draganddrop as DragDrop
import org.kde.plasma.plasma5support as Plasma5Support
import org.kde.kirigami as Kirigami

import "../code/DockZoom.js" as DockZoom

Item {
    id: root

    required property var tasksRoot
    required property var dockRef
    required property int dockIndex

    readonly property bool isDockDecorator: true
    readonly property string title: i18nc("@title the name of the Trash widget", "Trash")
    readonly property string toolTipSubText: hasContents
        ? i18ncp("@info:status The trash contains this many items in it", "One item", "%1 items", dirModelCount)
        : i18nc("@info:status The trash is empty", "Empty")

    readonly property bool hasContents: dirModelCount > 0
    property int dirModelCount: 0
    property bool containsAcceptableDrag: false

    readonly property string iconName: hasContents ? "user-trash-full" : "user-trash"

    readonly property real _baseSize: Plasmoid.configuration.iconSize
    readonly property real zoomFactor: DockZoom.calculateZoomFactor(
        dockRef,
        Plasmoid.configuration.iconSize,
        Plasmoid.configuration.amplitud,
        Plasmoid.configuration.magnification,
        tasksRoot.vertical,
        dockIndex,
        entryProgress
    )

    property real entryProgress: (dockRef && dockRef.insideDock) ? 1.0 : 0.0

    Behavior on entryProgress {
        NumberAnimation { duration: 200; easing.type: Easing.OutCubic }
    }

    clip: false

    width: tasksRoot.vertical ? _baseSize : (_baseSize * zoomFactor)
    height: tasksRoot.vertical ? (_baseSize * zoomFactor) : tasksRoot.height

    property real itemPos: dockRef ? dockRef.afterTasksSecondPos : 0

    x: {
        if (tasksRoot.vertical && tasksRoot.isLeftPanel) return 0;
        if (tasksRoot.vertical) return (parent.width / 2) - (dockRef.spacing * 3);
        return itemPos;
    }

    y: {
        if (tasksRoot.isTopPanel) return 0;
        if (tasksRoot.vertical) return itemPos;
        return 0;
    }

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
                var lines = data.stdout.split('\n').filter(function(line) {
                    var trimmed = line.trim()
                    return trimmed.length > 0 && trimmed !== '.'
                })
                root.dirModelCount = lines.length
            } else {
                root.dirModelCount = 0
            }
        }
    }

    Plasma5Support.DataSource {
        id: openTrashSource
        engine: "executable"
        connectedSources: []
        onNewData: function(source, data) { disconnectSource(source) }
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

    Timer {
        id: trashCheckTimer
        interval: 1000
        running: true
        repeat: true
        onTriggered: dirModelSource.checkTrash()
    }

    function openTrash() {
        openTrashSource.connectSource("nautilus trash://")
    }

    function emptyTrash() {
        emptyTrashSource.connectSource("sh -c 'rm -rf ~/.local/share/Trash/files/* ~/.local/share/Trash/info/* 2>/dev/null || true'")
    }

    Component.onCompleted: {
        dirModelSource.checkTrash()
    }

    Item {
        id: iconBox

        width: _baseSize
        height: _baseSize

        anchors.centerIn: tasksRoot.vertical ? parent : undefined
        anchors.bottom: parent.bottom
        anchors.bottomMargin: (!tasksRoot.vertical && Plasmoid.location === PlasmaCore.Types.BottomEdge)
            ? 0
            : Math.round((tasksRoot.height / 2) - (Kirigami.Units.iconSizes.small * 0.14))
        anchors.horizontalCenter: !tasksRoot.vertical ? parent.horizontalCenter : undefined

        property int baseRenderSize: _baseSize * 2

        scale: zoomFactor
        transformOrigin: {
            switch (Plasmoid.location) {
            case PlasmaCore.Types.BottomEdge: return Item.Bottom;
            case PlasmaCore.Types.TopEdge: return Item.Top;
            case PlasmaCore.Types.LeftEdge: return Item.Left;
            case PlasmaCore.Types.RightEdge: return Item.Right;
            default: return Item.Bottom;
            }
        }

        Kirigami.Icon {
            id: icon
            width: iconBox.baseRenderSize
            height: iconBox.baseRenderSize
            source: root.iconName
            active: root.containsAcceptableDrag
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.bottom: parent.bottom
            anchors.bottomMargin: tasksRoot.isTopPanel ? 0 : Kirigami.Units.smallSpacing
            transformOrigin: Item.Bottom
            scale: 1 / (iconBox.baseRenderSize / iconBox.width)
            smooth: true
            antialiasing: true
        }

        Item {
            id: reflectionContainer
            visible: Plasmoid.configuration.showReflection
            opacity: 0.4
            clip: true
            z: -5

            width: tasksRoot.vertical ? iconBox.width / 2 : iconBox.width
            height: tasksRoot.vertical ? iconBox.height : iconBox.height / 2

            x: {
                switch (Plasmoid.location) {
                case PlasmaCore.Types.LeftEdge: return -width - Kirigami.Units.smallSpacing * 2.5
                case PlasmaCore.Types.RightEdge: return iconBox.width + Kirigami.Units.smallSpacing * 2.5
                default: return 0
                }
            }

            y: {
                switch (Plasmoid.location) {
                case PlasmaCore.Types.TopEdge: return -height - Kirigami.Units.smallSpacing * 2
                case PlasmaCore.Types.BottomEdge: return iconBox.height + Kirigami.Units.smallSpacing * 2
                default: return 0
                }
            }

            Kirigami.Icon {
                id: reflectionIcon
                width: icon.width
                height: icon.height
                source: icon.source
                smooth: true
                antialiasing: true
                anchors.centerIn: parent
                scale: icon.scale
                transform: Scale {
                    origin.x: reflectionIcon.width / 2
                    origin.y: reflectionIcon.height / 2
                    xScale: tasksRoot.vertical ? -1 : 1
                    yScale: tasksRoot.vertical ? 1 : -1
                }
            }
        }

        Rectangle {
            id: indicator
            anchors {
                horizontalCenter: parent.horizontalCenter
                bottom: parent.bottom
                bottomMargin: -5
            }
            width: 6
            height: 6
            radius: width / 2
            color: Kirigami.Theme.highlightColor
            visible: root.hasContents
        }
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true
        acceptedButtons: Qt.LeftButton | Qt.RightButton

        onClicked: function(mouse) {
            if (mouse.button === Qt.RightButton) {
                contextMenu.popup(mouseArea, mouse.x, mouse.y)
            } else {
                root.openTrash()
            }
        }

        DragDrop.DropArea {
            anchors.fill: parent
            preventStealing: true

            onDragEnter: {
                root.containsAcceptableDrag = event.mimeData.urls.length > 0
            }
            onDragLeave: root.containsAcceptableDrag = false

            onDrop: {
                root.containsAcceptableDrag = false
                var urls = event.mimeData.urls
                if (urls.length > 0) {
                    for (var i = 0; i < urls.length; i++) {
                        trashSource.connectSource("kioclient5 move " + urls[i] + " trash:/")
                    }
                    event.accept(Qt.MoveAction)
                } else {
                    event.accept(Qt.IgnoreAction)
                }
            }
        }
    }

    PlasmaCore.ToolTipArea {
        anchors.fill: parent
        mainText: root.title
        subText: ""
    }

    QQC2.Menu {
        id: contextMenu

        QQC2.MenuItem {
            text: i18nc("@action:inmenu Open the trash", "Open")
            icon.name: "document-open"
            onTriggered: root.openTrash()
        }

        QQC2.MenuSeparator {}

        QQC2.MenuItem {
            text: i18nc("@action:inmenu Empty the trash", "Empty Trash")
            icon.name: "trash-empty"
            enabled: root.hasContents
            onTriggered: root.emptyTrash()
        }
    }
}
