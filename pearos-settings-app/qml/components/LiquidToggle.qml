import QtQuick
import Qt5Compat.GraphicalEffects

// Drop-in 36×18 liquid-glass toggle
// Usage:  LiquidToggle { checked: SomeBackend.flag; onToggled: SomeBackend.setFlag(v) }
Rectangle {
    id: root
    property bool checked: false
    property alias value: root.checked  // compat alias
    signal toggled(bool v)

    width: 36; height: 18; radius: 9
    color: checked ? Theme.accent : Theme.toggleOff
    Behavior on color { ColorAnimation { duration: 200 } }

    // Inner top shadow when OFF — depth
    Rectangle {
        visible: !root.checked
        anchors { left: parent.left; right: parent.right; top: parent.top }
        height: parent.height * 0.5; radius: parent.radius
        gradient: Gradient {
            GradientStop { position: 0.0; color: Qt.rgba(0, 0, 0, 0.10) }
            GradientStop { position: 1.0; color: Qt.rgba(0, 0, 0, 0.0)  }
        }
    }

    // Inner top highlight when ON — glass refraction
    Rectangle {
        visible: root.checked
        anchors { left: parent.left; right: parent.right; top: parent.top; margins: 1 }
        height: parent.height * 0.4; radius: parent.radius
        gradient: Gradient {
            GradientStop { position: 0.0; color: Qt.rgba(1, 1, 1, 0.28) }
            GradientStop { position: 1.0; color: Qt.rgba(1, 1, 1, 0.0)  }
        }
    }

    // Thumb
    Rectangle {
        id: thumb
        width: 14; height: 14; radius: 7; color: "white"
        y: 2
        x: root.checked ? 20 : 2
        Behavior on x { NumberAnimation { duration: 220; easing.type: Easing.InOutQuad } }

        layer.enabled: true
        layer.effect: DropShadow {
            radius: 4; samples: 9
            color: Qt.rgba(0, 0.10, 0.25, 0.22)
            verticalOffset: 1; horizontalOffset: 0
        }

        // Specular top highlight
        Rectangle {
            anchors { left: parent.left; right: parent.right; top: parent.top; margins: 1 }
            height: parent.height * 0.45; radius: parent.radius
            gradient: Gradient {
                GradientStop { position: 0.0; color: Qt.rgba(1, 1, 1, 0.80) }
                GradientStop { position: 1.0; color: Qt.rgba(1, 1, 1, 0.0)  }
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
        onClicked: root.toggled(!root.checked)
    }
}
