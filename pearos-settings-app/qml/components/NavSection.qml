import QtQuick
import QtQuick.Controls

// A group of sidebar nav items (like .nav-section in Electron CSS)
Column {
    id: root
    property var model: []
    property int currentIdx: -1
    signal itemClicked(int idx)

    width: parent ? parent.width : 215
    spacing: 0
    bottomPadding: 11   // matches .nav-section { padding-bottom: 11px }

    Repeater {
        model: root.model
        delegate: NavItem {
            width: root.width
            icon:     modelData.icon
            label:    modelData.label
            pageIdx:  modelData.idx
            selected: root.currentIdx === modelData.idx
            onClicked: root.itemClicked(modelData.idx)
        }
    }
}
