import QtQuick
import Qt5Compat.GraphicalEffects

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

    function clamp(v, lo, hi) { return Math.max(lo, Math.min(hi, v)) }

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

    // ── Touch target + track + thumb ────────────────────────────────
    Item {
        anchors.left: parent.left; anchors.right: parent.right
        anchors.bottom: parent.bottom; anchors.bottomMargin: 6
        height: 24

        // Track background
        Rectangle {
            id: track
            anchors.left: parent.left; anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter
            height: 5; radius: 2.5
            color: Theme.dark ? "#3a3a3c" : "#D6D6DA"

            // Filled portion — accent gradient
            Rectangle {
                anchors.left: parent.left
                anchors.top: parent.top; anchors.bottom: parent.bottom
                width: root.clamp((root.value - root.from) / (root.to - root.from), 0, 1) * parent.width
                radius: parent.radius
                gradient: Gradient {
                    orientation: Gradient.Horizontal
                    GradientStop { position: 0.0; color: Qt.lighter(Theme.accent, 1.18) }
                    GradientStop { position: 1.0; color: Theme.accent }
                }
            }
        }

        // Thumb — 22×18 pill, glass style
        Item {
            id: thumbContainer
            width: 22; height: 22
            x: root.clamp((root.value - root.from) / (root.to - root.from), 0, 1) * (track.width - 22)
            y: (parent.height - height) / 2
            Behavior on x { NumberAnimation { duration: 60; easing.type: Easing.OutQuad } }

            Rectangle {
                id: thumbRect
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.verticalCenter: parent.verticalCenter
                width: 22
                height: thumbMA.pressed ? 16 : 18
                Behavior on height { NumberAnimation { duration: 120; easing.type: Easing.OutQuad } }
                radius: height / 2
                color: "white"

                layer.enabled: true
                layer.effect: DropShadow {
                    radius: 5; samples: 11
                    color: Theme.dark ? Qt.rgba(0, 0, 0, 0.35) : Qt.rgba(0, 0.12, 0.25, 0.18)
                    verticalOffset: 1; horizontalOffset: 0
                }

                // Specular — top edge highlight
                Rectangle {
                    anchors.left: parent.left; anchors.right: parent.right; anchors.top: parent.top
                    anchors.margins: 1
                    height: Math.round(parent.height * 0.45)
                    radius: parent.radius
                    gradient: Gradient {
                        GradientStop { position: 0.0; color: Qt.rgba(1, 1, 1, 0.75) }
                        GradientStop { position: 1.0; color: Qt.rgba(1, 1, 1, 0.0) }
                    }
                }

                // Inner border glow
                Rectangle {
                    anchors.fill: parent; radius: parent.radius
                    color: "transparent"
                    border.color: Qt.rgba(1, 1, 1, 0.6); border.width: 1
                }
            }
        }

        MouseArea {
            id: thumbMA
            anchors.fill: parent
            cursorShape: Qt.PointingHandCursor
            preventStealing: true

            function valueAt(mx) {
                var pct = root.clamp((mx - 11) / (track.width - 22), 0, 1)
                var raw = root.from + pct * (root.to - root.from)
                if (root.stepSize > 0)
                    raw = Math.round(raw / root.stepSize) * root.stepSize
                return root.clamp(raw, root.from, root.to)
            }

            onPressed:        { root.value = valueAt(mouseX); root.moved(root.value) }
            onPositionChanged: if (pressed) { root.value = valueAt(mouseX); root.moved(root.value) }
        }
    }
}
