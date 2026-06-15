import QtQuick

// Matches .card-divider: border-top 1px solid rgba(0,0,0,0.1), margin 15px 0
Rectangle {
    color: Qt.rgba(0, 0, 0, 0.1)
    height: 1
    width: parent ? parent.width : 400
    y: 0  // margins handled by the container spacing
}
