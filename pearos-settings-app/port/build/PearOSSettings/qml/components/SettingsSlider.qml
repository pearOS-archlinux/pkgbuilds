import QtQuick
import QtQuick.Controls

Item {
    id: root
    property alias labelText: lbl.text
    property real value: 50
    property real from: 0
    property real to: 100
    property real stepSize: 1
    property string unit: ""
    signal moved(real value)

    implicitWidth: parent ? parent.width : 400
    implicitHeight: 56

    Text {
        id: lbl
        anchors.left: parent.left; anchors.top: parent.top; anchors.topMargin: 4
        font.pixelSize: 12; font.weight: Font.DemiBold
        color: Theme.textPrimary
    }

    Text {
        anchors.right: parent.right; anchors.top: parent.top; anchors.topMargin: 4
        text: Math.round(root.value) + root.unit
        font.pixelSize: 12; color: Theme.textSecondary
    }

    Slider {
        anchors.left: parent.left; anchors.right: parent.right; anchors.bottom: parent.bottom
        from: root.from; to: root.to; stepSize: root.stepSize
        value: root.value
        palette.accent: "#3B82F6"
        onMoved: root.moved(value)
    }
}
