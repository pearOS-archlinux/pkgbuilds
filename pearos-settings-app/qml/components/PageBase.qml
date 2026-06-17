import QtQuick
import QtQuick.Layouts

// Matches .content-area: padding-left 7px, right 16px, top 20px, bottom 20px
Item {
    id: root
    property string title: ""
    default property alias pageContent: col.children
    property alias overlay: overlayItem.children

    Layout.fillWidth: true
    Layout.fillHeight: true

    Flickable {
        width: parent.width
        height: parent.height
        contentWidth: width
        contentHeight: col.implicitHeight + 40
        clip: true

        Column {
            id: col
            x: 7
            y: 20
            width: parent.width - 23  // 7 left + 16 right
            spacing: 0
        }
    }

    Item {
        id: overlayItem
        anchors.fill: parent
        z: 100
    }
}
