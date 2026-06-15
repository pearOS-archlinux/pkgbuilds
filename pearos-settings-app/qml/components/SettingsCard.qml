import QtQuick

// .settings-card — border-radius 12, padding 12px left/right, 5px top/bottom (card-section-vertical)
// Use padV: 12 for single-row cards (.card-section style)
Rectangle {
    id: root
    default property alias cardContent: col.children
    property int padV: 5   // vertical padding (top + bottom); 5px matches card-section-vertical
    property int padH: 12  // horizontal padding

    color: Theme.bgCard
    radius: 12
    width: parent ? parent.width : 400
    implicitHeight: col.implicitHeight + padV * 2

    Column {
        id: col
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.leftMargin: padH
        anchors.rightMargin: padH
        anchors.topMargin: padV
        spacing: 0
    }
}
