import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import QtQuick.Window
import QtCore

import org.kde.plasma.plasmoid
import org.kde.plasma.core as PlasmaCore
import org.kde.plasma.plasma5support as Plasma5Support
import org.kde.kirigami as Kirigami
import org.kde.taskmanager as TaskManager

PlasmoidItem {
    id: root

    Plasmoid.title: i18n("PearFinder")
    Plasmoid.icon: "org.gnome.files"
    Plasmoid.backgroundHints: PlasmaCore.Types.NoBackground
    toolTipSubText: i18n("Opens Files in the Home Folder")

    property bool nautilusRunning: false
    property var nautilusWindowIds: []
    property var nautilusTaskIndexes: []
    property int currentWindowIndex: 0

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
        // Folosim TasksModel pentru a verifica ferestrele Nautilus
        updateNautilusWindows()
    }

    function showNautilusWindows() {
        if (nautilusTaskIndexes.length > 0) {
            // Activează prima fereastră Nautilus folosind TasksModel (funcționează în Wayland)
            var index = nautilusTaskIndexes[0]
            // Folosim requestActivate care funcționează și în Wayland
            tasksModel.requestActivate(index)
        }
    }

    property var recentFiles: []

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
                    // Actualizăm meniul după ce s-au încărcat fișierele
                    Qt.callLater(root.updateContextualActions)
                }
            }
        }
    }

    function loadRecentFiles() {
        // Citește ultimele 10 fișiere recente din recently-used.xbel
        // Script Python simplu pentru a extrage și formata fișierele recente
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
            // Expand ~ to home directory folosind comanda shell
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
        // Activează toate ferestrele Nautilus secvențial (funcționează în Wayland)
        if (nautilusTaskIndexes.length > 0) {
            currentWindowIndex = 0
            activateWindowTimer.start()
        }
    }

    function openRecentFile(path) {
        executable.connectSource("nautilus \"" + path.replace(/"/g, '\\"') + "\"")
    }

    // Verifică periodic dacă Nautilus rulează
    Timer {
        id: checkTimer
        interval: 1000 // Verifică la fiecare secundă
        running: true
        repeat: true
        onTriggered: root.checkNautilus()
    }

    Plasmoid.onActivated: {
        if (nautilusRunning && nautilusWindowIds.length > 0) {
            showNautilusWindows()
        } else {
            openHome()
        }
    }

    Component {
        id: recentFileActionComponent
        PlasmaCore.Action {
            property string filePath
            text: model.name
            icon.name: "document-open"
            onTriggered: root.openRecentFile(filePath)
        }
    }
    
    function updateContextualActions() {
        // Construim lista de acțiuni dinamic
        var actions = []
        
        // New Files Window
        actions.push(newWindowAction)
        actions.push(separator1)
        
        // Go to Folder
        actions.push(goToFolderAction)
        actions.push(separatorForRecentFiles)
        
        // Fișiere recente
        for (var i = 0; i < recentFilesModel.count; i++) {
            var item = recentFilesModel.get(i)
            var action = Qt.createQmlObject(
                'import org.kde.plasma.core as PlasmaCore; PlasmaCore.Action { text: "' + 
                item.name.replace(/"/g, '\\"') + '"; icon.name: "document-open"; property string filePath: "' + 
                item.path.replace(/"/g, '\\"') + '"; onTriggered: root.openRecentFile(filePath) }',
                root
            )
            actions.push(action)
        }
        
        // Separator final
        if (recentFilesModel.count > 0) {
            actions.push(separatorBeforeShowAll)
        }
        
        // Show All Windows
        actions.push(showAllWindowsAction)
        
        Plasmoid.contextualActions = actions
    }
    
    // Acțiuni fixe pentru meniu
    PlasmaCore.Action {
        id: newWindowAction
        text: i18n("New Files Window")
        icon.name: "window-new"
        onTriggered: root.openNewWindow()
    }
    
    PlasmaCore.Action {
        id: separator1
        isSeparator: true
    }
    
    PlasmaCore.Action {
        id: goToFolderAction
        text: i18n("Go to Folder…") + "\t⇧⌘G"
        icon.name: "folder-open"
        shortcut: "Shift+Meta+G"
        onTriggered: root.openGoToFolder()
    }
    
    PlasmaCore.Action {
        id: separatorForRecentFiles
        isSeparator: true
    }
    
    PlasmaCore.Action {
        id: separatorBeforeShowAll
        isSeparator: true
    }
    
    PlasmaCore.Action {
        id: showAllWindowsAction
        text: i18n("Show All Windows")
        icon.name: "window"
        onTriggered: root.showAllWindows()
    }
    
    // Reîncărcăm fișierele recente periodic
    Timer {
        id: reloadRecentFilesTimer
        interval: 5000 // Reîncarcă la fiecare 5 secunde
        running: true
        repeat: true
        onTriggered: root.loadRecentFiles()
    }
    
    Component.onCompleted: {
        // Verificăm Nautilus și actualizăm lista de ferestre
        root.checkNautilus()
        
        // Actualizăm lista de ferestre periodic
        checkTimer.triggered.connect(function() {
            root.updateNautilusWindows()
        })
        
        // Încărcăm fișierele recente la startup
        root.loadRecentFiles()
        
        // Inițializăm meniul
        root.updateContextualActions()
        
        // Conectăm semnalul pentru reîncărcarea când este deschis meniul
        try {
            Plasmoid.contextualActionsAboutToShow.connect(function() {
                root.loadRecentFiles()
            })
        } catch(e) {
            // Semnalul nu există în această versiune
        }
    }
    
    // Actualizăm meniul când se schimbă lista de fișiere recente
    Connections {
        target: recentFilesModel
        function onCountChanged() {
            root.updateContextualActions()
        }
    }

    // Dialog pentru "Go to Folder"
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
                // Centrează fereastra pe ecran
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
    
    property var goToFolderDialogInstance: null

    preferredRepresentation: fullRepresentation

    readonly property bool inPanel: (Plasmoid.location === PlasmaCore.Types.TopEdge
        || Plasmoid.location === PlasmaCore.Types.RightEdge
        || Plasmoid.location === PlasmaCore.Types.BottomEdge
        || Plasmoid.location === PlasmaCore.Types.LeftEdge)

    fullRepresentation: MouseArea {
        id: mouseArea

        anchors.fill: parent

        activeFocusOnTab: true
        hoverEnabled: true

        onClicked: Plasmoid.activated()

        Item {
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.verticalCenter: parent.verticalCenter
            //anchors.horizontalCenterOffset: 15
            anchors.verticalCenterOffset: -2
            readonly property bool isHorizontal: Plasmoid.formFactor === PlasmaCore.Types.Horizontal
            readonly property real baseSize: isHorizontal ? parent.height : parent.width
            width: baseSize * (Plasmoid.configuration.iconSize / 100)
            height: width

            Kirigami.Icon {
                source: Plasmoid.icon
                anchors.fill: parent
                active: false
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
            visible: root.inPanel
        }

        PlasmaCore.ToolTipArea {
            anchors.fill: parent
            mainText: Plasmoid.title
            subText: root.toolTipSubText
        }
    }
}
