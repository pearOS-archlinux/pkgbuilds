import QtQuick
import QtQuick.Controls

Rectangle {
    id: root
    property string icon: ""
    property string label: ""
    property bool selected: false
    signal clicked()

    height: 38
    radius: 8
    color: selected ? "#0071e3" : hovered ? "#d8d8de" : "transparent"

    property bool hovered: false

    MouseArea {
        anchors.fill: parent
        hoverEnabled: true
        onEntered: root.hovered = true
        onExited:  root.hovered = false
        onClicked: root.clicked()
    }

    Row {
        anchors.left: parent.left; anchors.leftMargin: 12; anchors.verticalCenter: parent.verticalCenter
        spacing: 10

        Image {
            source: "image://icon/" + root.icon
            width: 18; height: 18
            visible: root.icon.length > 0
        }

        Text {
            text: root.label
            color: root.selected ? "white" : "#1a1a1a"
            font.pixelSize: 13
            font.weight: root.selected ? Font.Medium : Font.Normal
        }
    }
}
