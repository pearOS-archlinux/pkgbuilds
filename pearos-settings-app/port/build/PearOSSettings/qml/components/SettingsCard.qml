import QtQuick

// Matches .settings-card: bg #F7F7F7, radius 12
// Inner content padded 12px all sides
// Bottom margin 20px via Column spacing in PageBase
Rectangle {
    id: root
    default property alias cardContent: col.children

    color: Theme.bgCard
    radius: 12
    width: parent ? parent.width : 400
    implicitHeight: col.implicitHeight + 24
    // 20px gap after each card — replicated as bottomPadding on the Column parent
    // Electron uses margin-bottom: 20px. We use a Spacer item after each card.

    Column {
        id: col
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.leftMargin: 12
        anchors.rightMargin: 12
        anchors.topMargin: 12
        spacing: 0
    }
}
