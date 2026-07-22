/*
    PearLauncher — self-contained app launcher dock integration for PearDock.
    Inlines all PearLauncher UI so Kicker context variables stay in scope.
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import Qt5Compat.GraphicalEffects
import org.kde.plasma.plasmoid
import org.kde.plasma.core as PlasmaCore
import org.kde.plasma.components 3.0 as PlasmaComponents
import org.kde.plasma.extras as PlasmaExtras
import org.kde.kirigami as Kirigami
import org.kde.ksvg as KSvg
import org.kde.plasma.private.kicker 0.1 as Kicker

import "../code/DockZoom.js" as DockZoom
import "../code/TaskTools.js" as TaskTools
import "launchertools.js" as Tools

Item {
    id: launcherIntegration

    required property var tasksRoot
    required property var dockRef
    required property int dockIndex

    readonly property bool isDockDecorator: true
    readonly property string title: i18n("App Launcher")

    // ── Kicker context objects (must be id-accessible from inline sub-items) ──
    property QtObject globalFavorites: rootModel.favoritesModel
    property QtObject systemFavorites: rootModel.systemFavoritesModel
    property var dragSourceItem: null

    // ── Kicker models ────────────────────────────────────────────────────────
    Kicker.RootModel {
        id: rootModel
        appletInterface: launcherIntegration.tasksRoot
        autoPopulate: false
        flat: true
        sorted: true
        showAllApps: true
        showRecentApps: true
        showRecentDocs: true
    }

    Kicker.RunnerModel {
        id: runnerModel
        favoritesModel: launcherIntegration.globalFavorites
        appletInterface: launcherIntegration.tasksRoot
        mergeResults: true
    }

    Kicker.DragHelper {
        id: launcherDragHelper
        dragIconSize: Kirigami.Units.iconSizes.medium
    }

    // ── SVG helpers — invisible measurement items ─────────────────────────────
    KSvg.FrameSvgItem {
        id: highlightItemSvg
        imagePath: "widgets/viewitem"
        prefix: "hover"
        visible: false; width: 0; height: 0
    }
    KSvg.FrameSvgItem {
        id: panelSvg
        imagePath: "widgets/panel-background"
        visible: false; width: 0; height: 0
    }
    KSvg.FrameSvgItem {
        id: dialogSvg
        imagePath: "dialogs/background"
        visible: false; width: 0; height: 0
    }

    // ── Zoom ──────────────────────────────────────────────────────────────────
    readonly property real _baseSize: Plasmoid.configuration.iconSize
    readonly property real zoomFactor: DockZoom.calculateZoomFactor(
        dockRef, Plasmoid.configuration.iconSize, Plasmoid.configuration.amplitud,
        Plasmoid.configuration.magnification, tasksRoot.vertical, dockIndex, entryProgress
    )

    property real entryProgress: (dockRef && dockRef.insideDock) ? 1.0 : 0.0
    Behavior on entryProgress {
        NumberAnimation { duration: 200; easing.type: Easing.OutCubic }
    }

    // ── Size & position ───────────────────────────────────────────────────────
    clip: false
    width:  tasksRoot.vertical ? _baseSize : (_baseSize * zoomFactor)
    height: tasksRoot.vertical ? (_baseSize * zoomFactor) : tasksRoot.height

    readonly property bool inPanel: (Plasmoid.location === PlasmaCore.Types.TopEdge
        || Plasmoid.location === PlasmaCore.Types.RightEdge
        || Plasmoid.location === PlasmaCore.Types.BottomEdge
        || Plasmoid.location === PlasmaCore.Types.LeftEdge)

    x: {
        if (tasksRoot.vertical && tasksRoot.isLeftPanel) return 0
        if (tasksRoot.vertical) return (parent.width / 2) - (dockRef.spacing * 3)
        return itemPos
    }
    y: {
        if (tasksRoot.isTopPanel) return 0
        if (tasksRoot.vertical) return itemPos
        return 0
    }

    property real itemPos: {
        let pos = dockRef ? dockRef.centerOffset : 0
        let finder = dockRef ? dockRef.pearFinderItem : null
        if (finder && finder.width > 0) {
            pos += (tasksRoot.vertical ? finder.height : finder.width) + (dockRef ? dockRef.spacing : 0)
        }
        return pos
    }

    // ── Task indicator — intentionally hidden for launcher (not a running window) ──
    KSvg.FrameSvgItem {
        id: indicatorFrame
        visible: false
    }

    // ── Icon ──────────────────────────────────────────────────────────────────
    Item {
        id: iconBox
        width: _baseSize
        height: _baseSize
        anchors.centerIn: tasksRoot.vertical ? parent : undefined
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 0
        anchors.horizontalCenter: !tasksRoot.vertical ? parent.horizontalCenter : undefined

        property int baseRenderSize: _baseSize * 2
        scale: zoomFactor
        transformOrigin: {
            switch (Plasmoid.location) {
            case PlasmaCore.Types.BottomEdge: return Item.Bottom
            case PlasmaCore.Types.TopEdge:    return Item.Top
            case PlasmaCore.Types.LeftEdge:   return Item.Left
            case PlasmaCore.Types.RightEdge:  return Item.Right
            default:                          return Item.Bottom
            }
        }

        Kirigami.Icon {
            id: dockIcon
            width: iconBox.baseRenderSize
            height: iconBox.baseRenderSize
            source: Qt.resolvedUrl("icons/appicons/launchpad_light.png")
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
            width:  tasksRoot.vertical ? iconBox.width / 2 : iconBox.width
            height: tasksRoot.vertical ? iconBox.height   : iconBox.height / 2
            x: {
                switch (Plasmoid.location) {
                case PlasmaCore.Types.LeftEdge:  return -width - Kirigami.Units.smallSpacing * 2.5
                case PlasmaCore.Types.RightEdge: return iconBox.width + Kirigami.Units.smallSpacing * 2.5
                default: return 0
                }
            }
            y: {
                switch (Plasmoid.location) {
                case PlasmaCore.Types.TopEdge:    return -height - Kirigami.Units.smallSpacing * 2
                case PlasmaCore.Types.BottomEdge: return iconBox.height + Kirigami.Units.smallSpacing * 2
                default: return 0
                }
            }
            Kirigami.Icon {
                width: dockIcon.width; height: dockIcon.height
                source: dockIcon.source
                smooth: true; antialiasing: true
                anchors.centerIn: parent
                scale: dockIcon.scale
                transform: Scale {
                    origin.x: width / 2; origin.y: height / 2
                    xScale: tasksRoot.vertical ? -1 : 1
                    yScale: tasksRoot.vertical ? 1 : -1
                }
            }
        }
    }

    // ── Mouse / tooltip ───────────────────────────────────────────────────────
    MouseArea {
        anchors.fill: parent
        activeFocusOnTab: true
        hoverEnabled: true
        acceptedButtons: Qt.LeftButton | Qt.RightButton
        onClicked: function(mouse) {
            if (mouse.button === Qt.LeftButton) launcherDialog.toggle()
        }
    }

    PlasmaCore.ToolTipArea {
        anchors.fill: parent
        mainText: launcherIntegration.title
    }

    // open the popup in search-only mode (keyboard shortcut) — centered on screen
    function openSearch() {
        if (launcherDialog.visible && !launcherDialog.searchOnlyMode) {
            launcherDialog.closeAnimated()
        }
        launcherDialog.searchOnlyMode = true
        launcherDialog.visible = true
        Qt.callLater(function() { searchTextField.forceActiveFocus() })
    }

    // ==========================================================================
    //  POPUP  (MenuRepresentation + MainView + all sub-views inlined)
    // ==========================================================================
    PlasmaCore.Dialog {
        id: launcherDialog

        objectName: "launcherPopup"
        flags: Qt.WindowStaysOnTopHint | Qt.FramelessWindowHint
        location: "Floating"
        hideOnWindowDeactivate: true
        backgroundHints: PlasmaCore.Dialog.StandardBackground

        property bool isClosing: false
        property bool searchOnlyMode: false   // true = keyboard shortcut (search-only, centered); false = click (full view, above dock)
        property int  iconSize:  Kirigami.Units.iconSizes.large   // 64 px
        property int  columns:   7

        property int cellSizeHeight: iconSize
            + Kirigami.Units.gridUnit * 2
            + 2 * Math.max(
                highlightItemSvg.margins.top  + highlightItemSvg.margins.bottom,
                highlightItemSvg.margins.left + highlightItemSvg.margins.right)
        property int cellSizeWidth: cellSizeHeight

        onVisibleChanged: {
            if (!visible) {
                isClosing = false
                searchOnlyMode = false
                launcherDialog.reset()
            } else {
                isClosing = false
                Qt.callLater(function() {
                    var p = popupPosition(launcherFs.width, launcherFs.height)
                    launcherDialog.x = p.x
                    launcherDialog.y = p.y
                    launcherDialog.requestActivate()
                })
            }
        }

        onHeightChanged: {
            if (visible) {
                var p = popupPosition(launcherFs.width, launcherFs.height)
                launcherDialog.x = p.x
                launcherDialog.y = p.y
            }
        }

        function toggle() {
            if (visible) closeAnimated()
            else { searchOnlyMode = false; visible = true }
        }
        function closeAnimated() {
            if (visible && !isClosing) isClosing = true
        }
        function reset() {
            if (launcherMain) launcherMain.reset()
        }
        function popupPosition(w, h) {
            var screen = Plasmoid.containment.screenGeometry
            var px     = screen.x + (screen.width  - w) / 2
            var py     = screen.y + (screen.height - h) / 2
            return Qt.point(px, py)
        }

        // ── FocusScope set via mainItem so PlasmaCore.Dialog registers content ─
        mainItem: FocusScope {
            id: launcherFs
            focus: true

            property real innerPadding: 15

            width:  launcherDialog.searchOnlyMode
                ? 600
                : (launcherDialog.cellSizeWidth * launcherDialog.columns) + innerPadding * 2
            height: launcherDialog.searchOnlyMode
                ? (launcherMain.searching ? 600 : 60)
                : 40 + 2 + 40 + (launcherDialog.cellSizeHeight * 5) + innerPadding

            opacity: launcherDialog.isClosing ? 0.0 : 1.0
            Behavior on opacity {
                NumberAnimation {
                    duration: 200
                    easing.type: Easing.OutCubic
                    onRunningChanged: {
                        if (!running && launcherDialog.isClosing) {
                            launcherDialog.visible = false
                            launcherDialog.isClosing = false
                        }
                    }
                }
            }

            Keys.onPressed: event => {
                if (event.key === Qt.Key_Escape) launcherDialog.closeAnimated()
            }

            // offset by dialogSvg margins so content aligns with the SVG border (not needed in search mode)
            Item {
                id: svgOffsetItem
                x: launcherDialog.searchOnlyMode ? 0 : -dialogSvg.margins.left
                y: launcherDialog.searchOnlyMode ? 0 : -dialogSvg.margins.top
                width:  launcherDialog.searchOnlyMode ? parent.width  : parent.width  + dialogSvg.margins.left + dialogSvg.margins.right
                height: launcherDialog.searchOnlyMode ? parent.height : parent.height + dialogSvg.margins.top  + dialogSvg.margins.bottom

                // ── MainView ─────────────────────────────────────────────────
                Item {
                    id: launcherMain

                    x: launcherFs.innerPadding
                    y: launcherDialog.searchOnlyMode ? 0 : launcherFs.innerPadding
                    width:  svgOffsetItem.width  - launcherFs.innerPadding
                    height: launcherDialog.searchOnlyMode
                        ? svgOffsetItem.height
                        : svgOffsetItem.height - launcherFs.innerPadding * 2

                    property bool searching: searchTextField.text !== ""

                    readonly property color textColor:      Kirigami.Theme.textColor
                    readonly property string textFont:      Kirigami.Theme.defaultFont
                    readonly property real   textSize:      10
                    readonly property color  bgColor:       Kirigami.Theme.backgroundColor
                    readonly property color  highlightColor:Kirigami.Theme.highlightColor

                    property bool isDarkTheme: {
                        var lum = 0.2126 * bgColor.r + 0.7152 * bgColor.g + 0.0722 * bgColor.b
                        return lum <= 0.179
                    }
                    property color contrastBgColor: isDarkTheme
                        ? Qt.rgba(1, 1, 1, 0.15)
                        : Qt.rgba(0, 0, 0, 0.10)
                    property color dimmedTextColor: Qt.rgba(textColor.r, textColor.g, textColor.b, 0.7)

                    property bool showAllApps: true

                    function reload() { searchTextField.clear(); launcherAppList.reset() }
                    function reset()  { searchTextField.clear(); launcherAppList.reset() }

                    ColumnLayout {
                        anchors.fill: parent
                        spacing: 0

                        // ── SearchBar ─────────────────────────────────────────
                        RowLayout {
                            id: launcherSearchBar
                            Layout.fillWidth: true
                            Layout.preferredHeight: 40
                            Layout.maximumHeight:   40
                            Layout.rightMargin: launcherFs.innerPadding

                            property alias textField: searchTextField

                            Kirigami.Icon {
                                id: searchModeBtn
                                source: {
                                    if (launcherDialog.searchOnlyMode)
                                        return Qt.resolvedUrl("icons/feather/search.svg")
                                    if (searchIconArea.containsMouse && launcherMain.showAllApps)
                                        return "go-previous-symbolic"
                                    return launcherMain.showAllApps
                                        ? Qt.resolvedUrl("icons/AppsIcon.svg")
                                        : "favorite-symbolic"
                                }
                                isMask: launcherDialog.searchOnlyMode || launcherMain.showAllApps
                                color: launcherMain.dimmedTextColor
                                Layout.preferredWidth:  20
                                Layout.preferredHeight: 20
                                MouseArea {
                                    id: searchIconArea
                                    anchors.fill: parent
                                    hoverEnabled: true
                                    cursorShape: Qt.PointingHandCursor
                                    enabled: !launcherDialog.searchOnlyMode
                                    onClicked: {
                                        if (launcherMain.showAllApps) {
                                            launcherDialog.searchOnlyMode = true
                                            searchTextField.forceActiveFocus()
                                        } else {
                                            launcherMain.showAllApps = true
                                            searchTextField.forceActiveFocus(Qt.BacktabFocusReason)
                                        }
                                    }
                                }
                            }

                            TextField {
                                id: searchTextField
                                Layout.fillHeight: true
                                Layout.fillWidth:  true
                                font.pointSize: 18
                                placeholderText: launcherDialog.searchOnlyMode ? "Spotlight Search" : "Applications"
                                placeholderTextColor: launcherMain.dimmedTextColor
                                background: Rectangle { color: "transparent" }
                                focus: true
                                onTextChanged: {
                                    forceActiveFocus(Qt.ShortcutFocusReason)
                                    runnerModel.query = text
                                }
                                Keys.onPressed: event => {
                                    if (event.key === Qt.Key_Escape) {
                                        event.accepted = true
                                        if (launcherMain.searching) clear()
                                        else launcherDialog.toggle()
                                    }
                                }
                            }

                            RoundButton {
                                id: menuBtn
                                flat: true
                                visible: !launcherDialog.searchOnlyMode
                                down: viewMenu.status === PlasmaExtras.Menu.Open || pressed
                                icon.name: "application-menu"
                                background: Rectangle {
                                    color: menuBtn.down ? launcherMain.contrastBgColor : "transparent"
                                    radius: height / 2
                                }
                                onPressed: viewMenu.openRelative()
                            }
                            PlasmaExtras.Menu {
                                id: viewMenu
                                visualParent: menuBtn
                                placement: PlasmaExtras.Menu.BottomPosedLeftAlignedPopup

                                PlasmaExtras.MenuItem {
                                    text: i18n("Grid")
                                    icon: launcherMain.showAllApps ? "checkmark-symbolic" : ""
                                    onClicked: launcherMain.showAllApps = true
                                }
                                PlasmaExtras.MenuItem {
                                    text: i18n("List")
                                    icon: !launcherMain.showAllApps ? "checkmark-symbolic" : ""
                                    onClicked: launcherMain.showAllApps = false
                                }
                            }
                        }

                        // ── Separator ─────────────────────────────────────────
                        Rectangle {
                            Layout.fillWidth: true
                            Layout.rightMargin: launcherFs.innerPadding
                            height: 1.5
                            color: launcherMain.contrastBgColor
                            visible: launcherDialog.searchOnlyMode ? launcherMain.searching : true
                        }

                        // ── AllAppsList ───────────────────────────────────────
                        ColumnLayout {
                            id: launcherAppList
                            Layout.fillHeight: true
                            Layout.fillWidth:  true
                            spacing: 0

                            visible: !launcherDialog.searchOnlyMode && opacity > 0
                            opacity: launcherMain.searching ? 0.0 : 1.0
                            Behavior on opacity {
                                NumberAnimation { duration: 100; easing.type: Easing.OutQuart }
                            }

                            property QtObject allAppsModel:  rootModel.modelForRow(2)
                            property QtObject recentAppsModel: null
                            property QtObject currentModel:  rootModel.modelForRow(2)

                            property var appsCategoriesList: {
                                var cats = []
                                for (var i = 2; i < rootModel.count - 2; i++) {
                                    cats.push({
                                        name:       rootModel.data(rootModel.index(i, 0), Qt.DisplayRole),
                                        modelIndex: i,
                                        icon:       rootModel.data(rootModel.index(i, 0), Qt.DecorationRole)
                                    })
                                }
                                allAppsModel  = rootModel.modelForRow(2)
                                currentModel  = rootModel.modelForRow(2)
                                return cats
                            }

                            function updateShowedModel(idx) { currentModel = rootModel.modelForRow(idx) }
                            function reset() { /* nothing to reset for grid */ }

                            Connections {
                                target: rootModel
                                function onRefreshed() {
                                    var r = rootModel.modelForRow(0)
                                    if (r) launcherAppList.recentAppsModel = r
                                }
                            }
                            Component.onCompleted: {
                                var r = rootModel.modelForRow(0)
                                if (r) launcherAppList.recentAppsModel = r
                            }

                            // Category tab strip
                            ScrollView {
                                Layout.preferredWidth:  parent.width - launcherFs.innerPadding
                                Layout.preferredHeight: visible ? 40 : 0
                                visible: launcherMain.showAllApps
                                ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
                                ScrollBar.vertical.policy:   ScrollBar.AlwaysOff

                                ListView {
                                    id: catList
                                    spacing: 7
                                    orientation: ListView.Horizontal
                                    anchors.fill: parent
                                    model: launcherAppList.appsCategoriesList

                                    onCurrentIndexChanged: {
                                        if (currentItem && currentIndex >= 0)
                                            launcherAppList.updateShowedModel(model[currentIndex].modelIndex)
                                    }

                                    delegate: Text {
                                        required property var model
                                        required property int index
                                        property bool selected: catList.currentIndex === index
                                        topPadding: 4; bottomPadding: 4
                                        leftPadding: 8; rightPadding: 8
                                        font.pointSize: 10
                                        text: model.name
                                        color: selected ? "#000000" : launcherMain.textColor
                                        opacity: selected ? 0.8 : 0.4
                                        Rectangle {
                                            anchors.fill: parent; z: -1; radius: 8
                                            color: parent.selected
                                                ? Qt.rgba(1,1,1,0.5)
                                                : (launcherMain.isDarkTheme
                                                    ? Qt.rgba(1,1,1,0.2)
                                                    : Qt.rgba(1,1,1,0.5))
                                        }
                                        MouseArea {
                                            anchors.fill: parent
                                            onClicked: catList.currentIndex = index
                                        }
                                    }
                                }

                                MouseArea {
                                    anchors.fill: parent
                                    acceptedButtons: Qt.NoButton
                                    hoverEnabled: true
                                    onWheel: wheel => {
                                        if (wheel.angleDelta.y !== 0) {
                                            catList.flick(wheel.angleDelta.y * 15, 0)
                                            wheel.accepted = true
                                        }
                                    }
                                }
                            }

                            // ── Apps GridView (scrollable, like original AppGridView) ──
                            GridView {
                                id: appsGrid

                                Layout.fillHeight: true
                                Layout.fillWidth:  true

                                visible: launcherMain.showAllApps

                                property bool movedWithKeyboard: false
                                property bool movedWithWheel:    false

                                focus: true
                                clip:  true
                                currentIndex: count > 0 ? 0 : -1
                                interactive: height < contentHeight
                                boundsBehavior: Flickable.StopAtBounds
                                keyNavigationEnabled: false
                                keyNavigationWraps:   false
                                highlightMoveDuration: 0

                                cellWidth:  launcherDialog.cellSizeWidth
                                cellHeight: launcherDialog.cellSizeHeight

                                model: launcherAppList.currentModel

                                Connections {
                                    target: launcherDialog
                                    function onVisibleChanged() {
                                        if (!launcherDialog.visible) {
                                            appsGrid.currentIndex = 0
                                            appsGrid.positionViewAtBeginning()
                                        }
                                    }
                                }

                                delegate: Item {
                                    id: gridDelegate
                                    required property var model
                                    required property int index

                                    property var triggerModel: appsGrid.model
                                    property bool isDraging: false
                                    property bool hasActionList: (
                                        (model.favoriteId !== null) ||
                                        (("hasActionList" in model) && model.hasActionList !== null)
                                    )

                                    width:  launcherDialog.cellSizeWidth
                                    height: launcherDialog.cellSizeHeight

                                    function trigger() {
                                        triggerModel.trigger(index, "", null)
                                        launcherDialog.toggle()
                                    }

                                    Kirigami.Icon {
                                        id: appIcon
                                        y: Kirigami.Units.smallSpacing
                                        anchors.horizontalCenter: parent.horizontalCenter
                                        width:  launcherDialog.iconSize
                                        height: launcherDialog.iconSize
                                        source: model.decoration
                                    }

                                    PlasmaComponents.Label {
                                        text: ("name" in model ? model.name : model.display)
                                        font.family:    launcherMain.textFont
                                        font.pointSize: launcherMain.textSize
                                        font.weight: 650
                                        color: launcherMain.textColor
                                        anchors {
                                            top: appIcon.bottom
                                            left: parent.left; right: parent.right
                                            topMargin:   Kirigami.Units.largeSpacing * 1.2
                                            leftMargin:  Kirigami.Units.smallSpacing
                                            rightMargin: Kirigami.Units.smallSpacing
                                        }
                                        textFormat: Text.PlainText
                                        elide: Text.ElideMiddle
                                        horizontalAlignment: Text.AlignHCenter
                                        maximumLineCount: 2
                                        wrapMode: Text.Wrap
                                    }

                                    DropShadow {
                                        anchors.fill: appIcon
                                        horizontalOffset: 0; verticalOffset: 0
                                        radius: 15.0; samples: 16
                                        color: "#33000000"
                                        source: appIcon
                                    }

                                    MouseArea {
                                        anchors.fill: parent
                                        z: parent.z + 1
                                        acceptedButtons: Qt.LeftButton | Qt.RightButton
                                        cursorShape: Qt.PointingHandCursor
                                        hoverEnabled: !appsGrid.movedWithWheel
                                        onClicked: mouse => {
                                            if (mouse.button === Qt.RightButton) {
                                                if (gridDelegate.hasActionList) {
                                                    var al = model.actionList || []
                                                    Tools.fillActionMenu(i18n, gridActionMenu, al,
                                                        launcherIntegration.globalFavorites, model.favoriteId)
                                                    gridActionMenu.visualParent = gridDelegate
                                                    gridActionMenu.open(mouse.x, mouse.y)
                                                }
                                            } else {
                                                gridDelegate.trigger()
                                            }
                                        }
                                        onEntered: {
                                            if (!appsGrid.activeFocus) appsGrid.forceActiveFocus(Qt.MouseFocusReason)
                                            appsGrid.currentIndex = index
                                        }
                                        onPositionChanged: mouse => {
                                            isDraging = pressed
                                            if (pressed) {
                                                if ("pluginName" in model)
                                                    launcherDragHelper.startDrag(launcherIntegration.tasksRoot,
                                                        model.url, model.decoration,
                                                        "text/x-plasmoidservicename", model.pluginName)
                                                else
                                                    launcherDragHelper.startDrag(launcherIntegration.tasksRoot,
                                                        model.url, model.decoration)
                                            }
                                        }
                                    }

                                    Item {
                                        id: gridActionMenu
                                        property QtObject menu: null
                                        property Item visualParent: gridDelegate
                                        property variant actionList
                                        onActionListChanged: refreshMenu()
                                        function open(x, y) {
                                            if (!menu) return
                                            if (x !== undefined && y !== undefined) menu.open(x, y)
                                            else menu.open()
                                        }
                                        function refreshMenu() {
                                            if (menu) menu.destroy()
                                            if (!actionList) return
                                            menu = gridMenuComp.createObject(gridActionMenu)
                                            fillMenu(menu, actionList)
                                        }
                                        function fillMenu(m, items) {
                                            if (!items) return
                                            items.forEach(function(ai) {
                                                if (ai.subActions) {
                                                    var sub = gridSubMenuComp.createObject(m, {"actionItem": ai})
                                                    fillMenu(sub.submenu, ai.subActions)
                                                } else {
                                                    gridMenuItemComp.createObject(m, {"actionItem": ai})
                                                }
                                            })
                                        }
                                        Component { id: gridMenuComp; PlasmaExtras.Menu { visualParent: gridActionMenu.visualParent } }
                                        Component {
                                            id: gridSubMenuComp
                                            PlasmaExtras.MenuItem {
                                                property variant actionItem
                                                text: actionItem.text ? actionItem.text : ""
                                                icon: actionItem.icon ? actionItem.icon : null
                                                property PlasmaExtras.Menu submenu: PlasmaExtras.Menu { visualParent: parent.action }
                                            }
                                        }
                                        Component {
                                            id: gridMenuItemComp
                                            PlasmaExtras.MenuItem {
                                                property variant actionItem
                                                text:      actionItem.text      ? actionItem.text      : ""
                                                enabled:   actionItem.type !== "title" && ("enabled" in actionItem ? actionItem.enabled : true)
                                                separator: actionItem.type === "separator"
                                                section:   actionItem.type === "title"
                                                icon:      actionItem.icon      ? actionItem.icon      : null
                                                checkable: actionItem.checkable ? actionItem.checkable : false
                                                checked:   actionItem.checked   ? actionItem.checked   : false
                                                onClicked: {
                                                    var close = Tools.triggerAction(
                                                        gridDelegate.triggerModel, gridDelegate.index,
                                                        actionItem.actionId, actionItem.actionArgument)
                                                    if (close) launcherDialog.closeAnimated()
                                                }
                                            }
                                        }
                                    }
                                }

                                Keys.onPressed: event => {
                                    const cols = launcherDialog.columns
                                    const atLeft   = currentIndex % cols === 0
                                    const atTop    = currentIndex < cols
                                    const atRight  = currentIndex % cols === cols - 1
                                    const atBottom = currentIndex >= count - cols
                                    if (count > 1) {
                                        switch (event.key) {
                                        case Qt.Key_Left:  if (!atLeft  && !searchTextField.activeFocus) { moveCurrentIndexLeft();  event.accepted = true } break
                                        case Qt.Key_Right: if (!atRight && !searchTextField.activeFocus) { moveCurrentIndexRight(); event.accepted = true } break
                                        case Qt.Key_Up:    if (!atTop)    { moveCurrentIndexUp();   event.accepted = true } break
                                        case Qt.Key_Down:  if (!atBottom) { moveCurrentIndexDown(); event.accepted = true } break
                                        case Qt.Key_Return:
                                        case Qt.Key_Enter: if (currentItem) { currentItem.trigger(); event.accepted = true } break
                                        }
                                    }
                                    if (event.accepted) { appsGrid.movedWithKeyboard = true; gridKbTimer.restart() }
                                }
                                Timer { id: gridKbTimer; interval: 200; onTriggered: appsGrid.movedWithKeyboard = false }

                                Kirigami.WheelHandler {
                                    target: appsGrid
                                    filterMouseEvents: true
                                    verticalStepSize: 20 * Qt.styleHints.wheelScrollLines
                                    onWheel: { appsGrid.movedWithWheel = true; gridWheelTimer.restart() }
                                }
                                Timer { id: gridWheelTimer; interval: 200; onTriggered: appsGrid.movedWithWheel = false }

                                ScrollBar.vertical: ScrollBar {
                                    id: gridScrollBar
                                    z: 2; parent: appsGrid
                                    height: appsGrid.height
                                    anchors.right: appsGrid.right
                                    active: appsGrid.movedWithWheel
                                    contentItem: Rectangle {
                                        implicitWidth: 10; implicitHeight: 100
                                        radius: width / 2
                                        color: launcherMain.dimmedTextColor
                                        opacity: (gridScrollBar.active && gridScrollBar.size < 1.0) ? 0.75 : 0
                                        Behavior on opacity { NumberAnimation {} }
                                    }
                                    background: Rectangle {
                                        color: gridScrollBar.active ? launcherMain.contrastBgColor : "transparent"
                                        radius: width / 2
                                        opacity: (gridScrollBar.active && gridScrollBar.size < 1.0) ? 1 : 0
                                        Behavior on opacity { NumberAnimation {} }
                                    }
                                }
                            }
                            // end appsGrid

                            // ── Favorites ListView (when !showAllApps, like original AppListView) ──
                            ListView {
                                id: favsList

                                Layout.fillHeight: true
                                Layout.fillWidth:  true
                                Layout.rightMargin: launcherFs.innerPadding

                                visible: !launcherMain.showAllApps

                                property real availableWidth: width
                                property bool movedWithWheel: false
                                property bool movedWithKeyboard: false

                                focus: true
                                clip:  true
                                currentIndex: count > 0 ? 0 : -1
                                interactive: height < contentHeight
                                boundsBehavior: Flickable.StopAtBounds
                                keyNavigationEnabled: false
                                keyNavigationWraps:   false
                                highlightResizeDuration: 0
                                highlightMoveDuration:   50

                                model: launcherAppList.currentModel

                                highlight: Rectangle {
                                    width: favsList.availableWidth
                                    radius: 10; z: -1
                                    color: launcherMain.contrastBgColor
                                }

                                Connections {
                                    target: launcherDialog
                                    function onVisibleChanged() {
                                        if (!launcherDialog.visible) {
                                            favsList.currentIndex = 0
                                            favsList.positionViewAtBeginning()
                                        }
                                    }
                                }

                                delegate: Item {
                                    id: listDelegate
                                    required property var model
                                    required property int index

                                    property var triggerModel: favsList.model
                                    property bool isDraging: false
                                    property bool hasActionList: (
                                        (model.favoriteId !== null) ||
                                        (("hasActionList" in model) && model.hasActionList !== null)
                                    )

                                    width:  favsList.availableWidth
                                    height: Kirigami.Units.iconSizes.medium + Kirigami.Units.largeSpacing * 2

                                    function trigger() {
                                        triggerModel.trigger(index, "", null)
                                        launcherDialog.toggle()
                                    }
                                    function openActionMenu(x, y) {
                                        var al = model.actionList || []
                                        Tools.fillActionMenu(i18n, listActionMenu, al,
                                            launcherIntegration.globalFavorites, model.favoriteId)
                                        listActionMenu.visualParent = listDelegate
                                        listActionMenu.open(x, y)
                                    }

                                    RowLayout {
                                        anchors { fill: parent; leftMargin: 8; rightMargin: 8 }
                                        spacing: 8
                                        Kirigami.Icon {
                                            implicitWidth:  Kirigami.Units.iconSizes.medium
                                            implicitHeight: Kirigami.Units.iconSizes.medium
                                            Layout.alignment: Qt.AlignVCenter
                                            source: model.decoration
                                        }
                                        PlasmaComponents.Label {
                                            Layout.fillWidth: true
                                            text: ("name" in model ? model.name : model.display)
                                            color: launcherMain.textColor
                                            elide: Text.ElideRight
                                            verticalAlignment: Text.AlignVCenter
                                        }
                                    }

                                    MouseArea {
                                        anchors.fill: parent
                                        z: parent.z + 1
                                        acceptedButtons: Qt.LeftButton | Qt.RightButton
                                        cursorShape: Qt.PointingHandCursor
                                        hoverEnabled: !favsList.movedWithWheel
                                        onClicked: mouse => {
                                            if (mouse.button === Qt.RightButton) {
                                                if (listDelegate.hasActionList) openActionMenu(mouse.x, mouse.y)
                                            } else {
                                                listDelegate.trigger()
                                            }
                                        }
                                        onEntered: {
                                            if (!favsList.activeFocus) favsList.forceActiveFocus(Qt.MouseFocusReason)
                                            favsList.currentIndex = index
                                        }
                                        onPositionChanged: mouse => {
                                            isDraging = pressed
                                            if (pressed) {
                                                if ("pluginName" in model)
                                                    launcherDragHelper.startDrag(launcherIntegration.tasksRoot,
                                                        model.url, model.decoration,
                                                        "text/x-plasmoidservicename", model.pluginName)
                                                else
                                                    launcherDragHelper.startDrag(launcherIntegration.tasksRoot,
                                                        model.url, model.decoration)
                                            }
                                        }
                                    }

                                    Item {
                                        id: listActionMenu
                                        property QtObject menu: null
                                        property Item visualParent: listDelegate
                                        property variant actionList
                                        onActionListChanged: refreshMenu()
                                        function open(x, y) {
                                            if (!menu) return
                                            if (x !== undefined && y !== undefined) menu.open(x, y)
                                            else menu.open()
                                        }
                                        function refreshMenu() {
                                            if (menu) menu.destroy()
                                            if (!actionList) return
                                            menu = listMenuComp.createObject(listActionMenu)
                                            fillMenu(menu, actionList)
                                        }
                                        function fillMenu(m, items) {
                                            if (!items) return
                                            items.forEach(function(ai) {
                                                if (ai.subActions) {
                                                    var sub = listSubMenuComp.createObject(m, {"actionItem": ai})
                                                    fillMenu(sub.submenu, ai.subActions)
                                                } else {
                                                    listMenuItemComp.createObject(m, {"actionItem": ai})
                                                }
                                            })
                                        }
                                        Component { id: listMenuComp; PlasmaExtras.Menu { visualParent: listActionMenu.visualParent } }
                                        Component {
                                            id: listSubMenuComp
                                            PlasmaExtras.MenuItem {
                                                property variant actionItem
                                                text: actionItem.text ? actionItem.text : ""
                                                icon: actionItem.icon ? actionItem.icon : null
                                                property PlasmaExtras.Menu submenu: PlasmaExtras.Menu { visualParent: parent.action }
                                            }
                                        }
                                        Component {
                                            id: listMenuItemComp
                                            PlasmaExtras.MenuItem {
                                                property variant actionItem
                                                text:      actionItem.text      ? actionItem.text      : ""
                                                enabled:   actionItem.type !== "title" && ("enabled" in actionItem ? actionItem.enabled : true)
                                                separator: actionItem.type === "separator"
                                                section:   actionItem.type === "title"
                                                icon:      actionItem.icon      ? actionItem.icon      : null
                                                checkable: actionItem.checkable ? actionItem.checkable : false
                                                checked:   actionItem.checked   ? actionItem.checked   : false
                                                onClicked: {
                                                    var close = Tools.triggerAction(
                                                        listDelegate.triggerModel, listDelegate.index,
                                                        actionItem.actionId, actionItem.actionArgument)
                                                    if (close) launcherDialog.closeAnimated()
                                                }
                                            }
                                        }
                                    }
                                }

                                Keys.onPressed: event => {
                                    if (count >= 1) {
                                        switch (event.key) {
                                        case Qt.Key_Up:
                                            if (currentIndex > 0) { decrementCurrentIndex(); event.accepted = true } break
                                        case Qt.Key_Down:
                                            if (currentIndex < count - 1) { incrementCurrentIndex(); event.accepted = true } break
                                        case Qt.Key_Return:
                                        case Qt.Key_Enter:
                                            if (currentItem) { currentItem.trigger(); event.accepted = true } break
                                        }
                                    }
                                    if (event.accepted) { favsList.movedWithKeyboard = true; favsKbTimer.restart() }
                                }
                                Timer { id: favsKbTimer; interval: 200; onTriggered: favsList.movedWithKeyboard = false }

                                Kirigami.WheelHandler {
                                    target: favsList
                                    filterMouseEvents: true
                                    verticalStepSize: 20 * Qt.styleHints.wheelScrollLines
                                    onWheel: { favsList.movedWithWheel = true; favsWheelTimer.restart() }
                                }
                                Timer { id: favsWheelTimer; interval: 200; onTriggered: favsList.movedWithWheel = false }

                                ScrollBar.vertical: ScrollBar {
                                    id: favsScrollBar
                                    z: 2; parent: favsList
                                    height: favsList.height
                                    anchors.right: favsList.right
                                    active: favsList.movedWithWheel
                                    contentItem: Rectangle {
                                        implicitWidth: 10; implicitHeight: 100
                                        radius: width / 2
                                        color: launcherMain.dimmedTextColor
                                        opacity: (favsScrollBar.active && favsScrollBar.size < 1.0) ? 0.75 : 0
                                        Behavior on opacity { NumberAnimation {} }
                                    }
                                }
                            }
                            // end favsList
                        }
                        // end launcherAppList

                        // ── SearchList ─────────────────────────────────────────
                        ListView {
                            id: launcherSearchList
                            Layout.fillWidth:  true
                            Layout.fillHeight: true

                            visible: opacity > 0
                            opacity: launcherMain.searching ? 1.0 : 0.0
                            Behavior on opacity {
                                NumberAnimation { duration: 100; easing.type: Easing.OutQuart }
                            }

                            property real availableWidth: width - launcherFs.innerPadding
                            property bool movedWithWheel: false
                            property bool movedWithKeyboard: false

                            focus: true
                            clip:  true
                            currentIndex: count > 0 ? 0 : -1
                            interactive: height < contentHeight
                            boundsBehavior: Flickable.StopAtBounds
                            keyNavigationEnabled: false
                            keyNavigationWraps:   false
                            highlightResizeDuration: 0
                            highlightMoveDuration:   50

                            highlight: Rectangle {
                                width: launcherSearchList.availableWidth
                                radius: 10
                                z: -20
                                color: launcherMain.contrastBgColor
                            }

                            // ── AppListViewDelegate (inline) ──────────────────
                            delegate: Item {
                                id: listDelegate
                                required property var model
                                required property int index

                                property var triggerModel: launcherSearchList.model
                                property bool isSeparator: false

                                property bool hasActionList: (
                                    (model.favoriteId !== null) ||
                                    (("hasActionList" in model) && model.hasActionList !== null)
                                )

                                implicitWidth:  launcherSearchList.availableWidth
                                implicitHeight: 48

                                function trigger() {
                                    triggerModel.trigger(index, "", null)
                                    launcherDialog.toggle()
                                }

                                RowLayout {
                                    anchors.fill: parent
                                    anchors.leftMargin: 8; anchors.rightMargin: 8
                                    spacing: 8
                                    Kirigami.Icon {
                                        implicitWidth:  Kirigami.Units.iconSizes.medium
                                        implicitHeight: Kirigami.Units.iconSizes.medium
                                        Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                                        source: model.decoration
                                    }
                                    PlasmaComponents.Label {
                                        Layout.fillWidth: true
                                        text: ("name" in model ? model.name : model.display)
                                        textFormat: Text.PlainText
                                        elide: Text.ElideRight
                                        color: launcherMain.textColor
                                    }
                                }

                                MouseArea {
                                    anchors.fill: parent
                                    z: parent.z + 1
                                    acceptedButtons: Qt.LeftButton | Qt.RightButton
                                    cursorShape: Qt.PointingHandCursor
                                    hoverEnabled: !launcherSearchList.movedWithWheel
                                    onClicked: mouse => {
                                        if (mouse.button === Qt.RightButton) {
                                            if (listDelegate.hasActionList) {
                                                var al = model.actionList || []
                                                Tools.fillActionMenu(i18n, listActionMenu, al,
                                                    launcherIntegration.globalFavorites, model.favoriteId)
                                                listActionMenu.visualParent = listDelegate
                                                listActionMenu.open(mouse.x, mouse.y)
                                            }
                                        } else {
                                            listDelegate.trigger()
                                        }
                                    }
                                    onEntered: {
                                        if (!launcherSearchList.activeFocus)
                                            launcherSearchList.forceActiveFocus(Qt.MouseFocusReason)
                                        launcherSearchList.currentIndex = index
                                    }
                                }

                                Item {
                                    id: listActionMenu
                                    property QtObject menu: null
                                    property Item visualParent: listDelegate
                                    property variant actionList
                                    onActionListChanged: refreshListMenu()
                                    function open(x, y) { if (menu) { if (x !== undefined) menu.open(x,y); else menu.open() } }
                                    function refreshListMenu() {
                                        if (menu) menu.destroy()
                                        if (!actionList) return
                                        menu = listMenuComp.createObject(listActionMenu)
                                        fillListMenu(menu, actionList)
                                    }
                                    function fillListMenu(m, items) {
                                        if (!items) return
                                        items.forEach(function(ai) {
                                            if (ai.subActions) {
                                                var sub = listSubMenuComp.createObject(m, {"actionItem": ai})
                                                fillListMenu(sub.submenu, ai.subActions)
                                            } else {
                                                listMenuItemComp.createObject(m, {"actionItem": ai})
                                            }
                                        })
                                    }
                                    Component {
                                        id: listMenuComp
                                        PlasmaExtras.Menu { visualParent: listActionMenu.visualParent }
                                    }
                                    Component {
                                        id: listSubMenuComp
                                        PlasmaExtras.MenuItem {
                                            property variant actionItem
                                            text: actionItem.text ? actionItem.text : ""
                                            icon: actionItem.icon ? actionItem.icon : null
                                            property PlasmaExtras.Menu submenu: PlasmaExtras.Menu {
                                                visualParent: parent.action
                                            }
                                        }
                                    }
                                    Component {
                                        id: listMenuItemComp
                                        PlasmaExtras.MenuItem {
                                            property variant actionItem
                                            text:      actionItem.text      ? actionItem.text      : ""
                                            enabled:   actionItem.type !== "title" && ("enabled" in actionItem ? actionItem.enabled : true)
                                            separator: actionItem.type === "separator"
                                            section:   actionItem.type === "title"
                                            icon:      actionItem.icon      ? actionItem.icon      : null
                                            checkable: actionItem.checkable ? actionItem.checkable : false
                                            checked:   actionItem.checked   ? actionItem.checked   : false
                                            onClicked: {
                                                var close = Tools.triggerAction(
                                                    listDelegate.triggerModel,
                                                    listDelegate.index,
                                                    actionItem.actionId,
                                                    actionItem.actionArgument)
                                                if (close) launcherDialog.closeAnimated()
                                            }
                                        }
                                    }
                                }
                            }
                            // end delegate

                            Connections {
                                target: runnerModel
                                function onQueryChanged() {
                                    launcherSearchList.model = runnerModel.modelForRow(0)
                                    launcherSearchList.currentIndex = 0
                                }
                            }

                            Loader {
                                anchors.centerIn: parent
                                width: launcherSearchList.width - Kirigami.Units.gridUnit * 4
                                active:  launcherSearchList.count === 0
                                visible: active
                                asynchronous: true
                                sourceComponent: PlasmaExtras.PlaceholderMessage {
                                    id: emptyHint
                                    iconName: "edit-none"
                                    opacity: 0
                                    text: i18nc("@info:status", "No matches")
                                    Connections {
                                        target: runnerModel
                                        function onQueryFinished() { emptyAnim.restart() }
                                    }
                                    NumberAnimation {
                                        id: emptyAnim
                                        duration: Kirigami.Units.longDuration
                                        easing.type: Easing.OutCubic
                                        property: "opacity"; target: emptyHint; to: 1
                                    }
                                }
                            }

                            Keys.onPressed: event => {
                                if (count < 1) return
                                const atFirst = currentIndex === 0
                                const atLast  = currentIndex === count - 1
                                switch (event.key) {
                                case Qt.Key_Up:
                                    if (!atFirst) { decrementCurrentIndex(); event.accepted = true }; break
                                case Qt.Key_Down:
                                    if (!atLast)  { incrementCurrentIndex(); event.accepted = true }; break
                                case Qt.Key_Return:
                                case Qt.Key_Enter:
                                    if (currentItem) { currentItem.trigger(); event.accepted = true }; break
                                }
                                if (event.accepted) { movedWithKeyboard = true; searchKbTimer.restart() }
                            }
                            Timer { id: searchKbTimer; interval: 200; onTriggered: launcherSearchList.movedWithKeyboard = false }

                            Kirigami.WheelHandler {
                                target: launcherSearchList
                                filterMouseEvents: true
                                onWheel: { launcherSearchList.movedWithWheel = true; searchWheelTimer.restart() }
                            }
                            Timer { id: searchWheelTimer; interval: 200; onTriggered: launcherSearchList.movedWithWheel = false }

                            ScrollBar.vertical: ScrollBar {
                                id: searchScrollBar
                                z: 2
                                parent: launcherSearchList
                                height: launcherSearchList.height
                                anchors.right: launcherSearchList.right
                                active: launcherSearchList.movedWithWheel
                                contentItem: Rectangle {
                                    implicitWidth: 10; implicitHeight: 100
                                    radius: width / 2
                                    color: launcherMain.dimmedTextColor
                                    opacity: (searchScrollBar.active && searchScrollBar.size < 1.0) ? 0.75 : 0
                                    Behavior on opacity { NumberAnimation {} }
                                }
                            }
                        }
                        // end SearchList
                    }
                    // end ColumnLayout inside launcherMain
                }
                // end launcherMain
            }
            // end svgOffsetItem
        }
        // end FocusScope (mainItem)
    }
    // end PlasmaCore.Dialog

    Component.onCompleted: {
        rootModel.refreshed.connect(launcherDialog.reset)
        launcherDragHelper.dropped.connect(function() { launcherIntegration.dragSourceItem = null })
        rootModel.refresh()
    }
}
