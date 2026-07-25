import QtQuick
import QtQuick.Controls
import "../components"

PageBase {
    title: "Spotlight"
    Component.onCompleted: Spotlight.refresh()

    property string newFolder: ""

    component Toggle: LiquidToggle {
        property bool on: false
        property var onChange: null
        checked: on
        onToggled: function(v) { if (onChange) onChange(v) }
    }

    component Row2: Item {
        property string label: ""
        property string sublabel: ""
        property bool toggled: false
        property var onToggle: null
        width: parent.width; height: sublabel.length > 0 ? 52 : 44
        Column {
            anchors.left: parent.left; anchors.verticalCenter: parent.verticalCenter; spacing: 2
            Text { text: parent.parent.label; font.pixelSize: 12; font.weight: Font.DemiBold; color: Theme.textPrimary }
            Text { visible: parent.parent.sublabel.length > 0; text: parent.parent.sublabel; font.pixelSize: 11; color: Theme.textSecondary }
        }
        Toggle {
            anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter
            on: parent.toggled
            onChange: parent.onToggle
        }
    }

    SettingsCard {
        Column {
            width: parent.width; spacing: 0
            Row2 {
                label: "Search Indexing"
                sublabel: Spotlight.indexedFileCount.length > 0
                          ? Spotlight.indexedFileCount + " files indexed" + (Spotlight.indexSize.length > 0 ? " · " + Spotlight.indexSize : "")
                          : "Lets Spotlight and KRunner search your files instantly"
                toggled: Spotlight.indexingEnabled
                onToggle: function(v) { Spotlight.setIndexingEnabled(v) }
            }
            Rectangle { width: parent.width; height: 1; color: Theme.divider }
            Row2 {
                label: "Index File Contents"
                sublabel: "Search inside documents, not just file names"
                toggled: Spotlight.indexContents
                onToggle: function(v) { Spotlight.setIndexContents(v) }
            }
            Rectangle { width: parent.width; height: 1; color: Theme.divider }
            Row2 {
                label: "Index Hidden Files"
                toggled: Spotlight.indexHidden
                onToggle: function(v) { Spotlight.setIndexHidden(v) }
            }
        }
    }
    Spacer {}

    SectionTitle { text: "Prevent Spotlight from Searching These Locations" }
    SettingsCard {
        Column {
            width: parent.width; spacing: 8; topPadding: 8; bottomPadding: 8

            Item {
                width: parent.width; height: 32
                visible: Spotlight.excludeFolders.length === 0
                Text { anchors.verticalCenter: parent.verticalCenter; text: "No excluded folders"; font.pixelSize: 12; color: Theme.textSecondary }
            }
            Repeater {
                model: Spotlight.excludeFolders
                delegate: Item {
                    width: parent.width; height: 32
                    Text {
                        anchors.left: parent.left; anchors.verticalCenter: parent.verticalCenter
                        text: modelData.path; font.pixelSize: 12; color: Theme.textPrimary
                        elide: Text.ElideMiddle; width: parent.width - 60
                    }
                    Rectangle {
                        anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter
                        height: 22; width: 50; radius: 5; color: Qt.rgba(239,68,68,0.15)
                        Text { anchors.centerIn: parent; text: "Remove"; font.pixelSize: 10; color: "#EF4444" }
                        MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor
                            onClicked: Spotlight.removeExcludeFolder(modelData.path) }
                    }
                }
            }

            Row {
                width: parent.width; spacing: 8; topPadding: 4
                TextField {
                    width: parent.width - 90; height: 32; font.pixelSize: 12
                    placeholderText: "/path/to/folder"
                    onTextChanged: newFolder = text
                }
                Rectangle {
                    height: 32; width: 82; radius: 6
                    color: newFolder.length > 0 ? Theme.accent : Qt.rgba(0,0,0,0.08)
                    Text { anchors.centerIn: parent; text: "Exclude"; font.pixelSize: 12; color: newFolder.length > 0 ? "white" : Theme.textSecondary }
                    MouseArea {
                        anchors.fill: parent; cursorShape: Qt.PointingHandCursor
                        onClicked: if (newFolder.length > 0) { Spotlight.addExcludeFolder(newFolder); newFolder = "" }
                    }
                }
            }
        }
    }
    Spacer {}
}
