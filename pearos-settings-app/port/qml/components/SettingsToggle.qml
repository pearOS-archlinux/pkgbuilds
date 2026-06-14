import QtQuick

// Matches the Electron CSS .switch: 36×18px pill toggle
// accent color #3B82F6
Item {
    id: root
    property alias labelText: lbl.text
    property bool checked: false
    signal toggled(bool value)

    implicitWidth: parent ? parent.width : 400
    implicitHeight: Math.max(lbl.implicitHeight, toggle.height) + 20

    Text {
        id: lbl
        anchors.left: parent.left; anchors.verticalCenter: parent.verticalCenter
        font.pixelSize: 12
        font.weight: Font.DemiBold
        color: Theme.textPrimary
    }

    // The toggle itself — matches .switch width:36 height:18
    Rectangle {
        id: toggle
        anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter
        width: 36; height: 18
        radius: 9
        color: root.checked ? "#3B82F6" : "#cccccc"

        Behavior on color { ColorAnimation { duration: 200 } }

        // Thumb — matches .slider:before height:14 width:14 left:2 bottom:2
        Rectangle {
            id: thumb
            width: 14; height: 14
            radius: 7
            color: "white"
            y: 2
            x: root.checked ? 20 : 2
            Behavior on x { NumberAnimation { duration: 200; easing.type: Easing.InOutQuad } }
        }

        MouseArea {
            anchors.fill: parent
            cursorShape: Qt.PointingHandCursor
            onClicked: { root.checked = !root.checked; root.toggled(root.checked) }
        }
    }
}
