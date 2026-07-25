import QtQuick

// Stand-in for org.kde.plasma.extras' PlasmaExtras.Highlight -- a plain
// selection/hover rectangle for ListView.highlight or a Loader's
// sourceComponent, no 1:1 QtQuick.Controls equivalent exists for it.
Rectangle {
    property bool hovered: false

    radius: 4
    color: Qt.rgba(1, 1, 1, hovered ? 0.08 : 0.15)
}
