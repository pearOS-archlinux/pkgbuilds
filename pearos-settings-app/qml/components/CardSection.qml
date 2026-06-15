import QtQuick
import QtQuick.Layouts

// Matches .card-section: flex row, space-between, padding 10px 0
// Usage: CardSection { labelText: "…"; content: SomeControl {} }
Item {
    id: root
    property alias labelText: lbl.text
    default property alias sectionContent: rightSlot.children

    implicitWidth: parent ? parent.width : 400
    implicitHeight: Math.max(lbl.implicitHeight, rightSlot.implicitHeight) + 20

    // Label (color-title style: 12px, 600)
    Text {
        id: lbl
        anchors.left: parent.left; anchors.verticalCenter: parent.verticalCenter
        font.pixelSize: 12
        font.weight: Font.DemiBold
        color: Theme.textPrimary
    }

    // Right-side control
    Item {
        id: rightSlot
        anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter
        implicitWidth: childrenRect.width
        implicitHeight: childrenRect.height
    }
}
