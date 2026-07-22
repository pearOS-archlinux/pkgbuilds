/*
    SPDX-FileCopyrightText: 2013 Heena Mahour <heena393@gmail.com>
    SPDX-FileCopyrightText: 2013 Sebastian Kügler <sebas@kde.org>
    SPDX-FileCopyrightText: 2016 Kai Uwe Broulik <kde@privat.broulik.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import QtQuick.Window
import QtQml

import org.kde.plasma.plasmoid
import org.kde.plasma.core as PlasmaCore
import org.kde.plasma.private.keyboardindicator as KeyboardIndicator
import org.kde.plasma.components as PlasmaComponents3
import org.kde.kirigami as Kirigami
import org.kde.plasma.plasma5support as Plasma5Support
import org.kde.taskmanager as TaskManager
import plasma.applet.org.kde.plasma.appmenu

PlasmoidItem {
    id: root

    readonly property bool vertical: Plasmoid.formFactor === PlasmaCore.Types.Vertical
    readonly property bool view: Plasmoid.configuration.compactView

    onViewChanged: {
        Plasmoid.view = view;
    }

    Plasmoid.constraintHints: Plasmoid.CanFillArea
    preferredRepresentation: Plasmoid.configuration.compactView ? compactRepresentation : fullRepresentation

    compactRepresentation: PlasmaComponents3.ToolButton {
        readonly property int fakeIndex: 0
        Layout.fillWidth: false
        Layout.fillHeight: false
        Layout.minimumWidth: implicitWidth
        Layout.maximumWidth: implicitWidth
        enabled: (appMenuModel.menuAvailable && appMenuModel.visible) || shouldShowBasicMenu
        checkable: ((appMenuModel.menuAvailable && appMenuModel.visible) || shouldShowBasicMenu) && Plasmoid.currentIndex === fakeIndex
        checked: checkable
        icon.name: "application-menu"

        display: PlasmaComponents3.AbstractButton.IconOnly
        text: Plasmoid.title
        Accessible.description: toolTipSubText

        onClicked: Plasmoid.trigger(this, 0);
    }

    fullRepresentation: GridLayout {
        id: buttonGrid

        function checkAndUpdateBasicMenu() {
            if (!appMenuModel.menuAvailable || !appMenuModel.visible) {
                root.shouldShowBasicMenu = true;
                return;
            }
            root.shouldShowBasicMenu = (buttonRepeater.count === 0);
        }

        Connections {
            target: appMenuModel
            function onMenuAvailableChanged() { buttonGrid.checkAndUpdateBasicMenu(); }
            function onVisibleChanged() { buttonGrid.checkAndUpdateBasicMenu(); }
        }

        Component.onCompleted: {
            checkAndUpdateBasicMenu();
        }

        Plasmoid.status: {
            if (appMenuModel.menuAvailable && Plasmoid.currentIndex > -1 && buttonRepeater.count > 0) {
                return PlasmaCore.Types.NeedsAttentionStatus;
            } else {
                return buttonRepeater.count > 0 || basicMenuRepeater.count > 0 || Plasmoid.configuration.compactView ? PlasmaCore.Types.ActiveStatus : PlasmaCore.Types.HiddenStatus;
            }
        }

        LayoutMirroring.enabled: Application.layoutDirection === Qt.RightToLeft
        Layout.minimumWidth: implicitWidth
        Layout.minimumHeight: implicitHeight

        flow: root.vertical ? GridLayout.TopToBottom : GridLayout.LeftToRight
        rowSpacing: 0
        columnSpacing: 0

        Binding {
            target: Plasmoid
            property: "buttonGrid"
            value: buttonGrid
            restoreMode: Binding.RestoreNone
        }

        Connections {
            target: Plasmoid
            function onRequestActivateIndex(index: int) {
                const button = buttonRepeater.itemAt(index);
                if (button) {
                    button.activated();
                }
            }
        }

        Connections {
            target: Plasmoid
            function onActivated() {
                const button = buttonRepeater.itemAt(0);
                if (button) {
                    button.activated();
                }
            }
        }

        PlasmaComponents3.ToolButton {
            id: noMenuPlaceholder
            visible: buttonRepeater.count === 0 && basicMenuRepeater.count === 0
            text: Plasmoid.title
            Layout.fillWidth: root.vertical
            Layout.fillHeight: !root.vertical
        }

        Repeater {
            id: buttonRepeater
            model: (appMenuModel.menuAvailable && appMenuModel.visible) ? appMenuModel : null

            onCountChanged: {
                buttonGrid.checkAndUpdateBasicMenu();
            }

            MenuDelegate {
                readonly property int buttonIndex: index

                Layout.fillWidth: root.vertical
                Layout.fillHeight: !root.vertical
                text: activeMenu
                Kirigami.MnemonicData.active: altState.pressed

                down: pressed || Plasmoid.currentIndex === index
                visible: text !== "" && (model.activeActions?.visible ?? false)

                menuIsOpen: Plasmoid.currentIndex !== -1
                onActivated: Plasmoid.trigger(this, index)

                KeyboardIndicator.KeyState {
                    id: altState
                    key: Qt.Key_Alt
                }
            }
        }

        Repeater {
            id: basicMenuRepeater
            model: root.shouldShowBasicMenu ? basicMenuModel : null

            MenuDelegate {
                id: menuButton
                required property string menuName
                required property int index
                readonly property int buttonIndex: index

                Layout.fillWidth: root.vertical
                Layout.fillHeight: !root.vertical
                text: menuName
                Kirigami.MnemonicData.active: altState.pressed

                down: pressed
                visible: text !== ""

                menuIsOpen: root.openBasicMenuId === menuName

                onActivated: {
                    var windowState = {}
                    if (menuName === "Window") {
                        windowState = {
                            "hasWindow": tasksModel.activeTask.valid,
                            "isMin":     tasksModel.activeTask.valid && (tasksModel.data(tasksModel.activeTask, TaskManager.AbstractTasksModel.IsMinimizable) || false),
                            "isMax":     tasksModel.activeTask.valid && (tasksModel.data(tasksModel.activeTask, TaskManager.AbstractTasksModel.IsMaximizable) || false),
                            "isMaximized": tasksModel.activeTask.valid && (tasksModel.data(tasksModel.activeTask, TaskManager.AbstractTasksModel.IsMaximized) || false),
                            "isClose":   tasksModel.activeTask.valid && (tasksModel.data(tasksModel.activeTask, TaskManager.AbstractTasksModel.IsClosable) || false)
                        }
                    }
                    Plasmoid.showBasicMenu(menuName, menuButton, windowState)
                }

                KeyboardIndicator.KeyState {
                    id: altState
                    key: Qt.Key_Alt
                }
            }
        }

        Item {
            Layout.preferredWidth: 0
            Layout.preferredHeight: 0
            Layout.fillWidth: true
            Layout.fillHeight: true
        }
    }

    AppMenuModel {
        id: appMenuModel
        containmentStatus: Plasmoid.containment.status
        screenGeometry: root.screenGeometry
        allScreens: Plasmoid.configuration.allScreens
        onRequestActivateIndex: Plasmoid.requestActivateIndex(index)
        Component.onCompleted: {
            Plasmoid.model = appMenuModel;
        }
    }

    property bool shouldShowBasicMenu: true
    property string openBasicMenuId: ""

    Connections {
        target: Plasmoid
        function onBasicMenuShown(menuId) { root.openBasicMenuId = menuId }
        function onBasicMenuHidden()      { root.openBasicMenuId = "" }
        function onBasicMenuAction(actionId) {
            if      (actionId === "file.newWindow")   root.openNautilus()
            else if (actionId === "file.newFolder")   root.createNewFolder()
            else if (actionId === "edit.undo")        root.undo()
            else if (actionId === "edit.redo")        root.redo()
            else if (actionId === "edit.cut")         root.cut()
            else if (actionId === "edit.copy")        root.copy()
            else if (actionId === "edit.paste")       root.paste()
            else if (actionId === "edit.selectAll")   root.selectAll()
            else if (actionId === "view.zoomIn")      root.sendKeyShortcut("super+equal")
            else if (actionId === "view.zoomOut")     root.sendKeyShortcut("super+minus")
            else if (actionId === "view.actualSize")  root.sendKeyShortcut("super+0")
            else if (actionId === "view.refresh")     root.sendKeyShortcut("super+r")
            else if (actionId === "go.back")          root.goBack()
            else if (actionId === "go.forward")       root.goForward()
            else if (actionId === "go.recents")       root.openRecents()
            else if (actionId === "go.documents")     root.openDocuments()
            else if (actionId === "go.desktop")       root.openDesktop()
            else if (actionId === "go.downloads")     root.openDownloads()
            else if (actionId === "go.home")          root.openHome()
            else if (actionId === "go.computer")      root.openComputer()
            else if (actionId === "go.applications")  root.openApplications()
            else if (actionId === "go.gotoFolder")    root.openGoToFolder()
            else if (actionId.startsWith("go.recentFolder:"))
                root.openRecentFolder(actionId.substring("go.recentFolder:".length))
            else if (actionId === "window.minimize") {
                if (tasksModel.activeTask.valid)
                    tasksModel.requestToggleMinimized(tasksModel.activeTask)
            }
            else if (actionId === "window.maximize") {
                if (tasksModel.activeTask.valid)
                    tasksModel.requestToggleMaximized(tasksModel.activeTask)
            }
            else if (actionId === "window.close") {
                if (tasksModel.activeTask.valid)
                    tasksModel.requestClose(tasksModel.activeTask)
            }
            else if (actionId === "window.bringAll") {
                for (var i = 0; i < tasksModel.count; i++) {
                    var taskIndex = tasksModel.index(i, 0)
                    if (taskIndex.valid && tasksModel.data(taskIndex, TaskManager.AbstractTasksModel.IsWindow))
                        tasksModel.requestActivate(taskIndex)
                }
            }
            else if (actionId === "help.donate")    root.openUrl("https://pearos.xyz/donate")
            else if (actionId === "help.discord")   root.openUrl("https://discord.gg/7jfh8MPDMR")
            else if (actionId === "help.nordvpn")   root.openUrl("https://go.nordvpn.net/aff_c?offer_id=15&aff_id=136731&url_id=902")
            else if (actionId === "help.nordpass")  root.openUrl("https://go.nordpass.io/aff_c?offer_id=488&aff_id=136731&url_id=9356")
            else if (actionId === "help.reddit")    root.openUrl("https://www.reddit.com/r/pearos/")
            else if (actionId === "help.youtube")   root.openUrl("https://www.youtube.com/pearOS")
            else if (actionId === "help.instagram") root.openUrl("https://www.instagram.com/pear_os/")
            else if (actionId === "help.website")   root.openUrl("https://pearos.xyz")
            else if (actionId === "help.pearid")    root.openUrl("https://account.pearos.xyz")
            else if (actionId === "help.github")    root.openUrl("https://github.com/pearOS-archlinux/")
        }
    }

    TaskManager.TasksModel {
        id: tasksModel
        screenGeometry: root.screenGeometry
    }

    Plasma5Support.DataSource {
        id: executable
        engine: "executable"
        connectedSources: []
        onNewData: function(sourceName, data) {
            disconnectSource(sourceName)
        }
    }

    function executeCommand(cmd) {
        executable.connectSource(cmd)
    }

    function openNautilus() {
        executeCommand("nautilus --new-window")
    }

    function createNewFolder() {
        executeCommand("cd ~/Desktop && name='New Folder' && counter=1 && while [ -d \"$name\" ]; do name=\"New Folder $counter\"; counter=$((counter+1)); done && mkdir -p \"$name\"")
    }

    function sendKeyShortcut(keys) {
        executeCommand("xdotool key " + keys)
    }

    function undo() { sendKeyShortcut("ctrl+z") }
    function redo() { sendKeyShortcut("ctrl+shift+z") }
    function cut() { sendKeyShortcut("ctrl+x") }
    function copy() { sendKeyShortcut("ctrl+c") }
    function paste() { sendKeyShortcut("ctrl+v") }
    function selectAll() { sendKeyShortcut("ctrl+a") }
    function goBack() { sendKeyShortcut("XF86Back") }
    function goForward() { sendKeyShortcut("XF86Forward") }

    function openRecents() {
        executeCommand("nautilus --new-window \"$(xdg-user-dir RECENT)\"")
    }
    function openDocuments() {
        executeCommand("nautilus --new-window \"$(xdg-user-dir DOCUMENTS)\"")
    }
    function openDesktop() {
        executeCommand("nautilus --new-window \"$(xdg-user-dir DESKTOP)\"")
    }
    function openDownloads() {
        executeCommand("nautilus --new-window \"$(xdg-user-dir DOWNLOAD)\"")
    }
    function openHome() {
        executeCommand("nautilus --new-window \"$HOME\"")
    }
    function openComputer() {
        executeCommand("nautilus --new-window computer://")
    }
    function openApplications() {
        executeCommand("nautilus --new-window applications://")
    }

    function openFolderPath(path) {
        if (path && path.trim() !== "") {
            var normalizedPath = path.trim()
            if (normalizedPath.startsWith("~")) {
                normalizedPath = normalizedPath.replace("~", "$HOME")
            }
            executeCommand("bash -c 'nautilus \"" + normalizedPath.replace(/"/g, '\\"') + "\"'")
        }
    }

    function openRecentFolder(path) {
        openFolderPath(path)
    }

    function openUrl(url) {
        Qt.openUrlExternally(url)
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

    property var goToFolderDialogInstance: null

    ListModel {
        id: basicMenuModel
        ListElement { menuName: "File" }
        ListElement { menuName: "Edit" }
        ListElement { menuName: "View" }
        ListElement { menuName: "Go" }
        ListElement { menuName: "Window" }
        ListElement { menuName: "Help" }
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

            color: Kirigami.Theme.backgroundColor

            Component.onCompleted: {
                var screen = Qt.application.screens[0]
                dialog.x = (screen.width - dialog.width) / 2
                dialog.y = (screen.height - dialog.height) / 2
            }

            Rectangle {
                anchors.fill: parent
                color: Kirigami.Theme.backgroundColor

                Column {
                    anchors.fill: parent
                    anchors.margins: Kirigami.Units.largeSpacing
                    spacing: Kirigami.Units.smallSpacing

                    QQC2.Label {
                        text: i18n("Enter folder path:")
                        width: parent.width
                    }

                    QQC2.TextField {
                        id: folderPathInput
                        width: parent.width
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

                    Row {
                        layoutDirection: Qt.RightToLeft
                        width: parent.width
                        spacing: Kirigami.Units.smallSpacing

                        QQC2.Button {
                            text: i18n("OK")
                            onClicked: {
                                root.openFolderPath(folderPathInput.text)
                                dialog.close()
                            }
                        }
                        QQC2.Button {
                            text: i18n("Cancel")
                            onClicked: dialog.close()
                        }
                    }
                }
            }

            onClosing: dialog.destroy()
        }
    }
}
