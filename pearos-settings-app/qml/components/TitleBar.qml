import QtQuick
import QtQuick.Controls

Rectangle {
    id: root
    height: 44
    color: "transparent"

    property alias title: titleText.text
    signal closeClicked()
    signal minimizeClicked()

    // drag area for frameless window
    MouseArea {
        anchors.fill: parent
        property point startPos
        onPressed:  (mouse) => { startPos = Qt.point(mouse.screenX - window.x, mouse.screenY - window.y) }
        onPositionChanged: (mouse) => { if (pressed) { window.x = mouse.screenX - startPos.x; window.y = mouse.screenY - startPos.y } }
    }

    Row {
        anchors.left: parent.left; anchors.leftMargin: 16; anchors.verticalCenter: parent.verticalCenter
        spacing: 8

        // Close
        Rectangle {
            width: 14; height: 14; radius: 7
            color: closeBtn.containsMouse ? "#ff5f57" : "#ff5f57"
            border.color: "#e0443e"; border.width: 0.5
            MouseArea { id: closeBtn; anchors.fill: parent; hoverEnabled: true; onClicked: root.closeClicked() }
        }
        // Minimize
        Rectangle {
            width: 14; height: 14; radius: 7
            color: minBtn.containsMouse ? "#febc2e" : "#febc2e"
            border.color: "#d9a020"; border.width: 0.5
            MouseArea { id: minBtn; anchors.fill: parent; hoverEnabled: true; onClicked: root.minimizeClicked() }
        }
        // Disabled maximize (greyed out)
        Rectangle {
            width: 14; height: 14; radius: 7
            color: "#29c940"; opacity: 0.4
            border.color: "#1aab29"; border.width: 0.5
        }
    }

    Text {
        id: titleText
        anchors.centerIn: parent
        font.pixelSize: 14
        font.weight: Font.Medium
        color: "#1a1a1a"
    }
}
