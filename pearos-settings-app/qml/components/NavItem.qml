import QtQuick

Rectangle {
    id: root
    property string icon: ""
    property string label: ""
    property int    pageIdx: -1
    property bool   selected: false
    signal clicked()

    width: parent ? parent.width : 215
    height: 30
    radius: 5
    color: "transparent"

    // Background rect inset 5px each side — matches Electron sidenav { padding: 0 5px }
    Rectangle {
        x: 5; y: 0
        width: parent.width - 10
        height: parent.height
        radius: 5
        color: root.selected ? Theme.activeBg : "transparent"
    }

    MouseArea {
        anchors.fill: parent
        onClicked: root.clicked()
        cursorShape: Qt.PointingHandCursor
    }

    Row {
        anchors.left: parent.left; anchors.leftMargin: 15; anchors.verticalCenter: parent.verticalCenter
        spacing: 4

        Image {
            source: root.icon
            width: 20; height: 21
            fillMode: Image.PreserveAspectFit
            smooth: true
            visible: root.icon.length > 0
        }

        Text {
            text: root.label
            font.pixelSize: 14
            color: Theme.textPrimary
            anchors.verticalCenter: parent.verticalCenter
        }
    }
}
