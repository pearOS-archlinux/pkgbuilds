import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import QtQuick.Window
import QtCore

import org.kde.plasma.plasmoid
import org.kde.plasma.core as PlasmaCore
import org.kde.plasma.plasma5support as Plasma5Support
import org.kde.plasma.extras as PlasmaExtras
import org.kde.kirigami as Kirigami
import org.kde.ksvg as KSvg
import org.kde.taskmanager as TaskManager

import "../code/DockZoom.js" as DockZoom
import "../code/TaskTools.js" as TaskTools

Item {
    id: root

    required property var tasksRoot
    required property var dockRef
    required property int dockIndex

    readonly property bool isDockDecorator: true
    readonly property string title: i18n("Pinder")
    readonly property string iconName: "org.gnome.files"
    readonly property string toolTipSubText: ""

    property bool nautilusRunning: false
    property var nautilusWindowIds: []
    property var nautilusTaskIndexes: []
    property int currentWindowIndex: 0
    property var goToFolderDialogInstance: null

    readonly property bool inPanel: (Plasmoid.location === PlasmaCore.Types.TopEdge
        || Plasmoid.location === PlasmaCore.Types.RightEdge
        || Plasmoid.location === PlasmaCore.Types.BottomEdge
        || Plasmoid.location === PlasmaCore.Types.LeftEdge)

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
        NumberAnimation {
            duration: 200
            easing.type: Easing.OutCubic
        }
    }

    clip: false

    width: tasksRoot.vertical
        ? _baseSize
        : (_baseSize * zoomFactor)

    height: tasksRoot.vertical
        ? (_baseSize * zoomFactor)
        : tasksRoot.height

    x: {
        if (tasksRoot.vertical && tasksRoot.isLeftPanel) {
            return 0;
        }

        if (tasksRoot.vertical) {
            return (parent.width / 2) - (dockRef.spacing * 3);
        }

        return itemPos;
    }

    y: {
        if (tasksRoot.isTopPanel) {
            return 0;
        }

        if (tasksRoot.vertical) {
            return itemPos;
        }

        return 0;
    }

    property real itemPos: dockRef.centerOffset

    Plasma5Support.DataSource {
        id: executable
        engine: "executable"
        connectedSources: []
        onNewData: function(sourceName, data) {
            disconnectSource(sourceName)
        }
    }

    TaskManager.TasksModel {
        id: tasksModel
        filterByVirtualDesktop: false
        filterByScreen: false
        filterByActivity: false
        filterNotMinimized: false
        groupMode: TaskManager.TasksModel.GroupDisabled

        onDataChanged: updateNautilusWindows()
        onCountChanged: updateNautilusWindows()
    }

    function updateNautilusWindows() {
        var windows = []
        var indexes = []
        for (var i = 0; i < tasksModel.count; i++) {
            var index = tasksModel.index(i, 0)
            var appId = tasksModel.data(index, TaskManager.AbstractTasksModel.AppId)
            if (appId && (appId.includes("nautilus") || appId.includes("org.gnome.Nautilus"))) {
                var winIds = tasksModel.data(index, TaskManager.AbstractTasksModel.WinIdList)
                if (winIds && winIds.length > 0) {
                    for (var j = 0; j < winIds.length; j++) {
                        windows.push(winIds[j])
                    }
                    indexes.push(index)
                }
            }
        }
        root.nautilusWindowIds = windows
        root.nautilusTaskIndexes = indexes
        root.nautilusRunning = windows.length > 0
    }

    Plasma5Support.DataSource {
        id: processChecker
        engine: "executable"
        connectedSources: []
        onNewData: function(sourceName, data) {
            root.nautilusRunning = (data["exit code"] === 0)
            disconnectSource(sourceName)
        }
    }

    function checkNautilus() {
        updateNautilusWindows()
    }

    function showNautilusWindows() {
        if (nautilusTaskIndexes.length > 0) {
            var index = nautilusTaskIndexes[0]
            tasksModel.requestActivate(index)
        }
    }

    ListModel {
        id: recentFilesModel
    }

    Plasma5Support.DataSource {
        id: recentFilesReader
        engine: "executable"
        connectedSources: []
        onNewData: function(sourceName, data) {
            disconnectSource(sourceName)
            if (data["exit code"] === 0 && data["stdout"]) {
                var output = data["stdout"].toString().trim()
                if (output) {
                    var lines = output.split('\n')
                    recentFilesModel.clear()
                    var count = 0
                    var maxFiles = 10
                    for (var i = 0; i < lines.length && count < maxFiles; i++) {
                        var line = lines[i].trim()
                        if (line) {
                            var parts = line.split('|')
                            if (parts.length >= 2) {
                                recentFilesModel.append({
                                    name: parts[0],
                                    path: parts[1]
                                })
                                count++
                            }
                        }
                    }

                }
            }
        }
    }

    function loadRecentFiles() {
        var cmd = "python3 -c \"import xml.etree.ElementTree as ET, urllib.parse, os; tree = ET.parse(os.path.expanduser('~/.local/share/recently-used.xbel')); [print(os.path.basename(urllib.parse.unquote(b.get('href', '').replace('file://', ''))) + '|' + urllib.parse.unquote(b.get('href', '').replace('file://', ''))) for b in list(tree.findall('.//bookmark'))[:10] if b.get('href', '').startswith('file://') and os.path.exists(urllib.parse.unquote(b.get('href', '').replace('file://', '')))]\" 2>/dev/null"
        recentFilesReader.connectSource(cmd)
    }

    function openHome() {
        executable.connectSource("nautilus $HOME")
    }

    function openNewWindow() {
        executable.connectSource("nautilus --new-window")
    }

    function openGoToFolder() {
        if (goToFolderDialogInstance) {
            goToFolderDialogInstance.destroy()
        }
        goToFolderDialogInstance = goToFolderDialogComponent.createObject(root)
        if (goToFolderDialogInstance) {
            goToFolderDialogInstance.show()
        }
    }

    function openFolderPath(path) {
        if (path && path.trim() !== "") {
            var normalizedPath = path.trim()
            if (normalizedPath.startsWith("~")) {
                normalizedPath = normalizedPath.replace("~", "$HOME")
            }
            executable.connectSource("nautilus \"" + normalizedPath.replace(/"/g, '\\"') + "\"")
        }
    }

    Timer {
        id: activateWindowTimer
        interval: 150
        repeat: false
        onTriggered: {
            if (currentWindowIndex < nautilusTaskIndexes.length) {
                tasksModel.requestActivate(nautilusTaskIndexes[currentWindowIndex])
                currentWindowIndex++
                if (currentWindowIndex < nautilusTaskIndexes.length) {
                    activateWindowTimer.start()
                } else {
                    currentWindowIndex = 0
                }
            }
        }
    }

    function showAllWindows() {
        if (nautilusTaskIndexes.length > 0) {
            currentWindowIndex = 0
            activateWindowTimer.start()
        }
    }

    function openRecentFile(path) {
        executable.connectSource("nautilus \"" + path.replace(/"/g, '\\"') + "\"")
    }

    function handleActivated() {
        if (nautilusRunning && nautilusWindowIds.length > 0) {
            showNautilusWindows()
        } else {
            openHome()
        }
    }

    Timer {
        id: checkTimer
        interval: 1000
        running: true
        repeat: true
        onTriggered: root.checkNautilus()
    }

    Timer {
        id: reloadRecentFilesTimer
        interval: 5000
        running: true
        repeat: true
        onTriggered: {
            if (contextMenu.status !== PlasmaExtras.Menu.Open) root.loadRecentFiles()
        }
    }

    Component.onCompleted: {
        root.checkNautilus()
        root.loadRecentFiles()
    }

    Component {
        id: goToFolderDialogComponent

        Window {
            id: dialog
            flags: Qt.Dialog | Qt.WindowStaysOnTopHint
            title: i18n("Go to Folder")
            width: 500
            height: 150
            modality: Qt.WindowModal

            SystemPalette {
                id: systemPalette
                colorGroup: SystemPalette.Active
            }

            color: systemPalette.window

            Component.onCompleted: {
                var screen = Qt.application.screens[0]
                dialog.x = (screen.width - dialog.width) / 2
                dialog.y = (screen.height - dialog.height) / 2
            }

            Rectangle {
                anchors.fill: parent
                color: systemPalette.window

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: Kirigami.Units.largeSpacing
                    spacing: Kirigami.Units.smallSpacing

                    QQC2.Label {
                        text: i18n("Enter folder path:")
                        Layout.fillWidth: true
                    }

                    QQC2.TextField {
                        id: folderPathInput
                        Layout.fillWidth: true
                        placeholderText: i18n("e.g., /home/user/Documents or ~/Documents")
                        focus: true

                        Keys.onPressed: function(event) {
                            if (event.key === Qt.Key_Enter || event.key === Qt.Key_Return) {
                                root.openFolderPath(folderPathInput.text)
                                dialog.close()
                            } else if (event.key === Qt.Key_Escape) {
                                dialog.close()
                            }
                        }
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        Layout.alignment: Qt.AlignRight
                        spacing: Kirigami.Units.smallSpacing

                        QQC2.Button {
                            text: i18n("Cancel")
                            onClicked: dialog.close()
                        }

                        QQC2.Button {
                            text: i18n("OK")
                            onClicked: {
                                root.openFolderPath(folderPathInput.text)
                                dialog.close()
                            }
                        }
                    }
                }
            }

            onClosing: {
                dialog.destroy()
            }
        }
    }

    KSvg.FrameSvgItem {
        id: indicatorFrame
        visible: root.inPanel

        anchors {
            fill: parent
            topMargin: Math.round(parent.height - root._baseSize * root.zoomFactor) - Kirigami.Units.smallSpacing
            bottomMargin: -(Kirigami.Units.gridUnit / (tasksRoot.skinParams.positionTaskIndicator || 9))
        }

        imagePath: tasksRoot.skinParams.imagetask
        prefix: TaskTools.taskPrefix("normal", Plasmoid.location)
    }

    Item {
        id: iconBox

        width: _baseSize
        height: _baseSize

        anchors.centerIn: tasksRoot.vertical ? parent : undefined
        anchors.bottom: (!tasksRoot.vertical && Plasmoid.location === PlasmaCore.Types.BottomEdge) ? parent.bottom : parent.bottom
        anchors.bottomMargin: (!tasksRoot.vertical && Plasmoid.location === PlasmaCore.Types.BottomEdge)
            ? 0
            : Math.round((tasksRoot.height / 2) - (Kirigami.Units.iconSizes.small * 0.14))
        anchors.horizontalCenter: !tasksRoot.vertical ? parent.horizontalCenter : undefined

        property int baseRenderSize: _baseSize * 2

        scale: zoomFactor
        transformOrigin: {
            switch (Plasmoid.location) {
            case PlasmaCore.Types.BottomEdge:
                return Item.Bottom;
            case PlasmaCore.Types.TopEdge:
                return Item.Top;
            case PlasmaCore.Types.LeftEdge:
                return Item.Left;
            case PlasmaCore.Types.RightEdge:
                return Item.Right;
            default:
                return Item.Bottom;
            }
        }

        Kirigami.Icon {
            id: icon

            width: iconBox.baseRenderSize
            height: iconBox.baseRenderSize
            source: root.iconName
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.bottom: parent.bottom
            anchors.bottomMargin: tasksRoot.isTopPanel ? 0 : Kirigami.Units.smallSpacing
            anchors.topMargin: tasksRoot.isTopPanel ? Kirigami.Units.smallSpacing : 0
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
                case PlasmaCore.Types.LeftEdge:
                    return -width - Kirigami.Units.smallSpacing * 2.5
                case PlasmaCore.Types.RightEdge:
                    return iconBox.width + Kirigami.Units.smallSpacing * 2.5
                default:
                    return 0
                }
            }

            y: {
                switch (Plasmoid.location) {
                case PlasmaCore.Types.TopEdge:
                    return -height - Kirigami.Units.smallSpacing * 2
                case PlasmaCore.Types.BottomEdge:
                    return iconBox.height + Kirigami.Units.smallSpacing * 2
                default:
                    return 0
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
        activeFocusOnTab: true
        hoverEnabled: true
        acceptedButtons: Qt.LeftButton | Qt.RightButton

        onClicked: function(mouse) {
            if (mouse.button === Qt.RightButton) {
                root.buildContextMenu()
                contextMenu.open()
            } else {
                root.handleActivated()
            }
        }
    }

    PlasmaCore.ToolTipArea {
        anchors.fill: parent
        mainText: root.title
        subText: root.toolTipSubText
    }

    PlasmaExtras.Menu {
        id: contextMenu
        visualParent: mouseArea
        placement: {
            if (Plasmoid.location === PlasmaCore.Types.TopEdge)
                return PlasmaExtras.Menu.BottomPosedLeftAlignedPopup
            if (Plasmoid.location === PlasmaCore.Types.LeftEdge)
                return PlasmaExtras.Menu.RightPosedTopAlignedPopup
            if (Plasmoid.location === PlasmaCore.Types.RightEdge)
                return PlasmaExtras.Menu.LeftPosedTopAlignedPopup
            return PlasmaExtras.Menu.TopPosedLeftAlignedPopup
        }
    }

    function buildContextMenu() {
        contextMenu.clearMenuItems()

        function makeSep() {
            var s = Qt.createQmlObject('import org.kde.plasma.extras as PE; PE.MenuItem { separator: true }', contextMenu)
            contextMenu.addMenuItem(s)
        }

        function makeItem(text, iconName, cb) {
            var mi = Qt.createQmlObject('import org.kde.plasma.extras as PE; PE.MenuItem {}', contextMenu)
            mi.text = text
            if (iconName) mi.icon = iconName
            if (cb) mi.clicked.connect(cb)
            contextMenu.addMenuItem(mi)
            return mi
        }

        makeItem(i18n("New Files Window"), "window-new", function() { root.openNewWindow() })
        makeSep()
        makeItem(i18n("Go to Folder…"), "folder-open", function() { root.openGoToFolder() })

        if (recentFilesModel.count > 0) {
            makeSep()
            var maxShow = Math.min(recentFilesModel.count, 5)
            for (var i = 0; i < maxShow; i++) {
                ;(function(name, path) {
                    makeItem(name, "document-open", function() { root.openRecentFile(path) })
                })(recentFilesModel.get(i).name, recentFilesModel.get(i).path)
            }
            makeSep()
        }

        makeItem(i18n("Show All Windows"), "window", function() { root.showAllWindows() })
    }
}
