import QtQuick
import Qt5Compat.GraphicalEffects

// 36×18 pill toggle — Liquid Glass style matching Electron
Item {
    id: root
    property alias labelText: lbl.text
    property bool checked: false
    signal toggled(bool value)

    implicitWidth: parent ? parent.width : 400
    implicitHeight: Math.max(lbl.implicitHeight, 18) + 20

    Text {
        id: lbl
        anchors.left: parent.left; anchors.verticalCenter: parent.verticalCenter
        font.pixelSize: 12; font.weight: Font.DemiBold
        color: Theme.textPrimary
    }

    // ── Track ──────────────────────────────────────────────────────
    Item {
        id: trackWrap
        anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter
        width: 36; height: 18

        // Track base
        Rectangle {
            id: track
            anchors.fill: parent; radius: 9
            color: root.checked ? Theme.accent : Theme.toggleOff
            Behavior on color { ColorAnimation { duration: 200 } }

            // Inner top shadow — depth when OFF
            Rectangle {
                visible: !root.checked
                anchors.left: parent.left; anchors.right: parent.right; anchors.top: parent.top
                height: parent.height * 0.5; radius: parent.radius
                gradient: Gradient {
                    GradientStop { position: 0.0; color: Qt.rgba(0, 0, 0, 0.08) }
                    GradientStop { position: 1.0; color: Qt.rgba(0, 0, 0, 0.0) }
                }
            }

            // Inner top highlight — glass refraction when ON
            Rectangle {
                visible: root.checked
                anchors.left: parent.left; anchors.right: parent.right; anchors.top: parent.top
                anchors.margins: 1
                height: parent.height * 0.4; radius: parent.radius
                gradient: Gradient {
                    GradientStop { position: 0.0; color: Qt.rgba(1, 1, 1, 0.25) }
                    GradientStop { position: 1.0; color: Qt.rgba(1, 1, 1, 0.0) }
                }
                Behavior on opacity { OpacityAnimator { duration: 200 } }
                opacity: root.checked ? 1 : 0
            }
        }

        // ── Thumb ─────────────────────────────────────────────────
        Rectangle {
            id: thumb
            width: 14; height: 14; radius: 7
            color: "white"
            y: 2
            x: root.checked ? 20 : 2
            Behavior on x { NumberAnimation { duration: 220; easing.type: Easing.InOutQuad } }

            layer.enabled: true
            layer.effect: DropShadow {
                radius: 4; samples: 9
                color: Qt.rgba(0, 0.1, 0.25, 0.22)
                verticalOffset: 1; horizontalOffset: 0
            }

            // Specular top highlight
            Rectangle {
                anchors.left: parent.left; anchors.right: parent.right; anchors.top: parent.top
                anchors.margins: 1
                height: parent.height * 0.45; radius: parent.radius
                gradient: Gradient {
                    GradientStop { position: 0.0; color: Qt.rgba(1, 1, 1, 0.8) }
                    GradientStop { position: 1.0; color: Qt.rgba(1, 1, 1, 0.0) }
                }
            }

            // Inner border glow
            Rectangle {
                anchors.fill: parent; radius: parent.radius
                color: "transparent"
                border.color: Qt.rgba(1, 1, 1, 0.55); border.width: 1
            }
        }

        MouseArea {
            anchors.fill: parent
            cursorShape: Qt.PointingHandCursor
            onClicked: { root.checked = !root.checked; root.toggled(root.checked) }
        }
    }
}
