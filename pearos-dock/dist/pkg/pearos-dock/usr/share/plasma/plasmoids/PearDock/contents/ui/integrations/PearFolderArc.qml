import QtQuick
import QtQuick.Controls as QQC2
import Qt.labs.folderlistmodel
import QtCore

import org.kde.plasma.plasmoid
import org.kde.plasma.core as PlasmaCore
import org.kde.kirigami as Kirigami
import org.kde.plasma.plasma5support as Plasma5Support

import "../code/DockZoom.js" as DockZoom

Item {
    id: root

    required property var tasksRoot
    required property var dockRef
    required property int dockIndex

    readonly property bool isDockDecorator: true
    readonly property string title: {
        let p = folderPath.replace(/\/+$/, "")
        return p.split("/").pop() || "Downloads"
    }
    readonly property string toolTipSubText: ""

    property string folderPath: "~/Downloads"
    property int maxItems: 10
    readonly property int itemSpacing: 40

    property bool isOpen: false
    property bool isClosing: false
    property int lastFileCount: -1
    readonly property int remainingItems: Math.max(0, dirModel.count - maxItems)

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

    property real itemPos: dockRef ? dockRef.afterTasksFolderArcPos : 0

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

    function resolvePath() {
        var path = folderPath.trim()
        if (path === "~/Downloads" || path === "")
            return StandardPaths.writableLocation(StandardPaths.DownloadLocation).toString()
        if (path.indexOf("~") === 0) {
            var homeUrl = StandardPaths.writableLocation(StandardPaths.HomeLocation).toString()
            if (homeUrl.startsWith("file://")) homeUrl = homeUrl.substring(7)
            path = homeUrl + path.substring(1)
        }
        if (path.indexOf("file://") !== 0) {
            if (path.indexOf("/") === 0) path = "file://" + path
            else {
                var home = StandardPaths.writableLocation(StandardPaths.HomeLocation).toString()
                if (home.startsWith("file://")) home = home.substring(7)
                path = "file://" + home + "/" + path
            }
        }
        return path
    }

    Plasma5Support.DataSource {
        id: execSource
        engine: "executable"
        connectedSources: []
        onNewData: (sourceName, data) => { disconnectSource(sourceName) }
    }

    function runNautilus(urlPath) {
        var path = urlPath.toString()
        if (path.startsWith("file://")) path = path.substring(7)
        execSource.connectSource("nautilus \"" + path + "\"")
    }

    function openFileSmart(url, isDir) {
        if (isDir) runNautilus(url)
        else Qt.openUrlExternally(url)
    }

    function openFolderInFiles() { runNautilus(dirModel.folder) }

    Timer {
        id: closingTimer
        interval: 350
        repeat: false
        onTriggered: {
            root.isOpen = false
            root.isClosing = false
        }
    }

    FolderListModel {
        id: dirModel
        folder: ""
        showDirs: true
        showFiles: true
        showDotAndDotDot: false
        sortField: FolderListModel.Time
        sortReversed: true
        onCountChanged: {
            if (root.lastFileCount === -1) return
            if (count > root.lastFileCount) flyInAnim.restart()
            root.lastFileCount = count
        }
    }

    Timer {
        id: initTimer
        interval: 300; running: true; repeat: false
        onTriggered: {
            root.lastFileCount = -1
            dirModel.folder = root.resolvePath()
            modelReadyTimer.restart()
        }
    }

    Timer {
        id: modelReadyTimer
        interval: 500; repeat: false
        onTriggered: root.lastFileCount = dirModel.count
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
            source: root.isOpen ? "folder-open" : "folder-download"
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
                if (root.isOpen) { root.isClosing = true; closingTimer.restart() }
                else { root.isClosing = false; root.isOpen = true }
            }
        }
    }

    PlasmaCore.ToolTipArea {
        anchors.fill: parent
        mainText: root.title
        subText: root.toolTipSubText
    }

    QQC2.Menu {
        id: contextMenu
        QQC2.MenuItem {
            text: i18n("Open Folder")
            icon.name: "folder-open"
            onTriggered: root.openFolderInFiles()
        }
    }

    PlasmaCore.Dialog {
        id: customDialog
        visible: root.isOpen
        visualParent: root
        location: Plasmoid.location
        backgroundHints: PlasmaCore.Types.NoBackground
        flags: Qt.FramelessWindowHint | Qt.Popup
        type: PlasmaCore.Dialog.Popup

        onVisibleChanged: {
            if (visible) customDialog.requestActivate()
            else if (!closingTimer.running) { root.isOpen = false; root.isClosing = false }
        }

        mainItem: Item {
            id: contentItem
            width: (Kirigami.Units.gridUnit * 30) + 300
            height: (root.maxItems * root.itemSpacing) + 200
            MouseArea { anchors.fill: parent }

            Item {
                id: emptyStateItem
                visible: dirModel.count === 0
                width: contentItem.width; height: root._baseSize
                opacity: 0.0
                anchors.right: parent.right; anchors.bottom: parent.bottom
                anchors.bottomMargin: 20; anchors.rightMargin: 390

                NumberAnimation { id: emptyOpenAnim; target: emptyStateItem; property: "opacity"; from: 0.0; to: 1.0; duration: 300; easing.type: Easing.OutQuad }
                NumberAnimation { id: emptyCloseAnim; target: emptyStateItem; property: "opacity"; to: 0.0; duration: 250; easing.type: Easing.InQuad }

                Connections {
                    target: root
                    function onIsClosingChanged() {
                        if (root.isOpen && dirModel.count === 0) {
                            if (root.isClosing) { emptyOpenAnim.stop(); emptyCloseAnim.restart() }
                            else { emptyCloseAnim.stop(); emptyStateItem.opacity = 0.0; emptyOpenAnim.restart() }
                        }
                    }
                    function onIsOpenChanged() {
                        if (root.isOpen && !root.isClosing && dirModel.count === 0) { emptyStateItem.opacity = 0.0; emptyOpenAnim.restart() }
                    }
                }

                Row {
                    layoutDirection: Qt.RightToLeft; spacing: 12; anchors.right: parent.right
                    Rectangle {
                        width: root._baseSize; height: root._baseSize; color: "transparent"
                        Kirigami.Icon { anchors.fill: parent; source: "folder-open"; smooth: true; opacity: 0.7 }
                    }
                    Rectangle {
                        height: emptyLabel.implicitHeight + 8; width: emptyLabel.implicitWidth + 20; radius: 6
                        color: "#cc202020"; border.color: "#30ffffff"; border.width: 1
                        anchors.verticalCenter: parent.verticalCenter
                        QQC2.Label { id: emptyLabel; text: i18n("No items"); color: "#eeeeee"; anchors.centerIn: parent; font.weight: Font.Medium }
                    }
                }
            }

            Repeater {
                model: dirModel
                delegate: Item {
                    id: delegateItem
                    visible: index < root.maxItems
                    width: contentItem.width; height: root.itemSpacing
                    readonly property real arcRadius: 600
                    readonly property real startAngle: 0
                    readonly property real angleStep: 0.09
                    readonly property real targetAngle: startAngle + (index * angleStep)
                    property real animatedAngle: 0.0
                    opacity: 0.0

                    ParallelAnimation {
                        id: openAnimation
                        SequentialAnimation {
                            PauseAnimation { id: openPauseAngle; duration: 0 }
                            NumberAnimation { target: delegateItem; property: "animatedAngle"; from: 0.0; to: delegateItem.targetAngle; duration: 300; easing.type: Easing.OutBack; easing.overshoot: 0.8 }
                        }
                        SequentialAnimation {
                            PauseAnimation { id: openPauseOpacity; duration: 0 }
                            NumberAnimation { target: delegateItem; property: "opacity"; from: 0.0; to: 1.0; duration: 200; easing.type: Easing.OutQuad }
                        }
                    }
                    ParallelAnimation {
                        id: closeAnimation
                        SequentialAnimation {
                            PauseAnimation { id: closePauseAngle; duration: 0 }
                            NumberAnimation { target: delegateItem; property: "animatedAngle"; to: 0.0; duration: 250; easing.type: Easing.InQuad }
                        }
                        SequentialAnimation {
                            PauseAnimation { id: closePauseOpacity; duration: 0 }
                            NumberAnimation { target: delegateItem; property: "opacity"; to: 0.0; duration: 200; easing.type: Easing.InQuad }
                        }
                    }

                    Connections {
                        target: root
                        function onIsClosingChanged() {
                            if (root.isOpen) {
                                if (root.isClosing) {
                                    var d = Math.max(0, (root.maxItems - index) * 20)
                                    closePauseAngle.duration = d
                                    closePauseOpacity.duration = d
                                    openAnimation.stop(); closeAnimation.restart()
                                } else {
                                    var openD = Math.max(0, index * 25)
                                    openPauseAngle.duration = openD
                                    openPauseOpacity.duration = openD
                                    closeAnimation.stop(); delegateItem.animatedAngle = 0.0; delegateItem.opacity = 0.0; openAnimation.restart()
                                }
                            }
                        }
                        function onIsOpenChanged() {
                            if (root.isOpen && !root.isClosing) {
                                var openD = Math.max(0, index * 25)
                                openPauseAngle.duration = openD
                                openPauseOpacity.duration = openD
                                delegateItem.animatedAngle = 0.0; delegateItem.opacity = 0.0; openAnimation.restart()
                            }
                        }
                    }

                    anchors.right: parent.right; anchors.bottom: parent.bottom
                    anchors.bottomMargin: 20 + (Math.sin(animatedAngle) * arcRadius)
                    anchors.rightMargin: 390 - ((1.0 - Math.cos(animatedAngle)) * (arcRadius * 0.6))

                    Item {
                        id: fileContainer
                        anchors.right: parent.right
                        width: fileRowGroup.width; height: root.itemSpacing
                        opacity: parent.opacity
                        transform: Rotation {
                            origin.x: fileContainer.width; origin.y: fileContainer.height / 2
                            angle: (delegateItem.angleStep > 0) ? (delegateItem.animatedAngle / delegateItem.angleStep) * 3.5 : 0
                        }

                        Row {
                            id: fileRowGroup; layoutDirection: Qt.RightToLeft; spacing: 12

                            MouseArea {
                                id: dragArea
                                width: root._baseSize; height: root._baseSize
                                hoverEnabled: true
                                drag.target: dragVisual

                                onClicked: {
                                    if (!drag.active) {
                                        root.openFileSmart(fileUrl, fileIsDir)
                                        root.isClosing = true
                                        closingTimer.restart()
                                    }
                                }

                                Item {
                                    id: dragVisual
                                    anchors.fill: parent
                                    Drag.active: dragArea.drag.active
                                    Drag.hotSpot.x: width / 2
                                    Drag.hotSpot.y: height / 2
                                    Drag.mimeData: { "text/uri-list": fileUrl }
                                    Drag.supportedActions: Qt.CopyAction | Qt.MoveAction
                                    Drag.dragType: Drag.Automatic

                                    Kirigami.Icon {
                                        anchors.fill: parent
                                        source: fileIsDir ? "folder" : (fileName.endsWith(".png") || fileName.endsWith(".jpg") || fileName.endsWith(".jpeg") ? fileUrl : "application-x-zerosize")
                                        smooth: true
                                    }
                                    opacity: Drag.active ? 0.7 : 1.0
                                    scale: Drag.active ? 1.1 : 1.0
                                    onXChanged: if (!Drag.active) x = 0
                                    onYChanged: if (!Drag.active) y = 0
                                }
                            }

                            Rectangle {
                                height: fileLabel.implicitHeight + 8
                                width: Math.min(fileLabel.implicitWidth + 20, 200)
                                radius: 6; color: "#cc202020"; border.color: "#30ffffff"; border.width: 1
                                anchors.verticalCenter: parent.verticalCenter
                                QQC2.Label {
                                    id: fileLabel; text: fileName; color: "#eeeeee"
                                    elide: Text.ElideMiddle; anchors.centerIn: parent
                                    width: parent.width - 16; horizontalAlignment: Text.AlignRight
                                    font.weight: Font.Medium
                                }
                            }
                        }
                    }
                }
            }

            Item {
                id: moreItem
                visible: root.remainingItems > 0
                width: contentItem.width; height: root.itemSpacing
                readonly property real arcRadius: 600
                readonly property real startAngle: 0
                readonly property real angleStep: 0.09
                property real itemIndex: root.maxItems
                readonly property real targetAngle: startAngle + (itemIndex * angleStep)
                property real animatedAngle: 0.0
                opacity: 0.0

                ParallelAnimation {
                    id: moreOpenAnimation
                    SequentialAnimation {
                        PauseAnimation { duration: moreItem.itemIndex * 25 }
                        NumberAnimation { target: moreItem; property: "animatedAngle"; from: 0.0; to: moreItem.targetAngle; duration: 300; easing.type: Easing.OutBack; easing.overshoot: 0.8 }
                    }
                    SequentialAnimation {
                        PauseAnimation { duration: moreItem.itemIndex * 25 }
                        NumberAnimation { target: moreItem; property: "opacity"; from: 0.0; to: 1.0; duration: 200; easing.type: Easing.OutQuad }
                    }
                }
                ParallelAnimation {
                    id: moreCloseAnimation
                    SequentialAnimation { NumberAnimation { target: moreItem; property: "animatedAngle"; to: 0.0; duration: 250; easing.type: Easing.InQuad } }
                    SequentialAnimation { NumberAnimation { target: moreItem; property: "opacity"; to: 0.0; duration: 200; easing.type: Easing.InQuad } }
                }

                Connections {
                    target: root
                    function onIsClosingChanged() {
                        if (root.isOpen) {
                            if (root.isClosing) { moreOpenAnimation.stop(); moreCloseAnimation.restart() }
                            else { moreCloseAnimation.stop(); moreItem.animatedAngle = 0.0; moreItem.opacity = 0.0; moreOpenAnimation.restart() }
                        }
                    }
                    function onIsOpenChanged() {
                        if (root.isOpen && !root.isClosing) { moreItem.animatedAngle = 0.0; moreItem.opacity = 0.0; moreOpenAnimation.restart() }
                    }
                }

                anchors.right: parent.right; anchors.bottom: parent.bottom
                anchors.bottomMargin: 20 + (Math.sin(animatedAngle) * arcRadius)
                anchors.rightMargin: 390 - ((1.0 - Math.cos(animatedAngle)) * (arcRadius * 0.6))

                Item {
                    id: moreContainer
                    anchors.right: parent.right
                    width: moreRowGroup.width; height: root.itemSpacing
                    opacity: parent.opacity
                    transform: Rotation {
                        origin.x: moreContainer.width; origin.y: moreContainer.height / 2
                        angle: (moreItem.angleStep > 0) ? (moreItem.animatedAngle / moreItem.angleStep) * 3.5 : 0
                    }

                    Row {
                        id: moreRowGroup; layoutDirection: Qt.RightToLeft; spacing: 12
                        MouseArea {
                            width: root._baseSize; height: root._baseSize; hoverEnabled: true
                            onClicked: { root.openFolderInFiles(); root.isClosing = true; closingTimer.restart() }
                            Rectangle {
                                anchors.fill: parent; anchors.margins: 4; radius: width / 2; color: "#e0e0e0"
                                Kirigami.Icon { anchors.centerIn: parent; width: 24; height: 24; source: "view-list-details"; isMask: true; color: "black" }
                            }
                        }
                        Rectangle {
                            height: 24; width: showMoreLabel.implicitWidth + 20; radius: 6
                            color: "#cc202020"; border.color: "#30ffffff"; border.width: 1
                            anchors.verticalCenter: parent.verticalCenter
                            QQC2.Label {
                                id: showMoreLabel
                                text: i18np("Show %1 more...", "Show %1 more...", root.remainingItems)
                                color: "#eeeeee"; anchors.centerIn: parent; font.pixelSize: 12; font.weight: Font.Medium
                            }
                        }
                    }
                }
            }
        }
    }

    PlasmaCore.Dialog {
        id: flyInDialog
        visible: flyInAnim.running
        visualParent: root
        location: Plasmoid.location
        flags: Qt.FramelessWindowHint | Qt.WindowStaysOnTopHint | Qt.WindowTransparentForInput
        backgroundHints: PlasmaCore.Types.NoBackground
        type: PlasmaCore.Dialog.Notification

        mainItem: Item {
            id: flyContainer
            width: 100; height: Screen.height * 0.7
            Kirigami.Icon {
                id: flyingIcon; source: "document-new"; width: 64; height: 64
                anchors.horizontalCenter: parent.horizontalCenter
                y: 0; opacity: 0.0; scale: 2.0
            }
            SequentialAnimation {
                id: flyInAnim
                ScriptAction { script: { flyingIcon.y = 0; flyingIcon.opacity = 0.0; flyingIcon.scale = 2.5 } }
                ParallelAnimation {
                    NumberAnimation { target: flyingIcon; property: "opacity"; to: 1.0; duration: 200 }
                    NumberAnimation { target: flyingIcon; property: "y"; to: flyContainer.height - 50; duration: 800; easing.type: Easing.InBack; easing.overshoot: 0.8 }
                    NumberAnimation { target: flyingIcon; property: "scale"; to: 0.2; duration: 800; easing.type: Easing.InQuad }
                }
                ScriptAction { script: { flyingIcon.opacity = 0.0 } }
            }
        }
    }
}
