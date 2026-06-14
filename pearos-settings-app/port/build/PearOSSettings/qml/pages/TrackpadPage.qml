import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "../components"

PageBase {
    title: "Trackpad"
    property int activeTab: 0
    readonly property string ap: "file:///home/alxb421/Desktop/pkgbuilds/pearos-settings-app/port/assets/"
    Component.onCompleted: Trackpad.refresh()

    // GIFs section
    SettingsCard {
        Row {
            width: parent.width; height: 120; spacing: 16
            Image { source: ap + "touch.gif"; width: (parent.width - 16) / 2; height: 120; fillMode: Image.PreserveAspectFit }
            Image { source: ap + "touch1.gif"; width: (parent.width - 16) / 2; height: 120; fillMode: Image.PreserveAspectFit }
        }
    }
    Spacer {}

    // Tabs
    SettingsCard {
        Row {
            width: parent.width; height: 36; spacing: 0
            Repeater {
                model: ["Point & Click", "Scroll & Zoom", "More Gestures"]
                delegate: Rectangle {
                    width: parent.width / 3; height: 36; radius: 6
                    color: activeTab === index ? Qt.rgba(0,0,0,0.1) : "transparent"
                    Text { anchors.centerIn: parent; text: modelData; font.pixelSize: 12; font.weight: Font.Medium; color: Theme.textPrimary }
                    MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor; onClicked: activeTab = index }
                }
            }
        }
    }
    Spacer {}

    // Point & Click tab
    SettingsCard {
        visible: activeTab === 0
        Column {
            width: parent.width; spacing: 0

            Item {
                width: parent.width; height: 44
                Text { anchors.left: parent.left; anchors.verticalCenter: parent.verticalCenter; text: "Tap to Click"; font.pixelSize: 12; font.weight: Font.DemiBold; color: Theme.textPrimary }
                Rectangle {
                    id: tapTog; width: 36; height: 18; radius: 9
                    property bool localState: Trackpad.tapToClick
                    anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter
                    color: localState ? "#3B82F6" : "#cccccc"
                    Behavior on color { ColorAnimation { duration: 200 } }
                    Rectangle { width: 14; height: 14; radius: 7; color: "white"; y: 2; x: tapTog.localState ? 20 : 2; Behavior on x { NumberAnimation { duration: 200 } } }
                    MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor; onClicked: { tapTog.localState = !tapTog.localState; Trackpad.setTapToClick(tapTog.localState) } }
                }
            }

            Rectangle { width: parent.width; height: 1; color: Theme.divider }

            Column {
                width: parent.width; spacing: 8; topPadding: 10; bottomPadding: 10
                Text { text: "Pointer Acceleration"; font.pixelSize: 12; font.weight: Font.DemiBold; color: Theme.textPrimary }
                Row {
                    width: parent.width; spacing: 10
                    Text { text: "-1"; font.pixelSize: 11; color: Theme.textSecondary; anchors.verticalCenter: parent.verticalCenter }
                    Slider {
                        from: -1; to: 1; stepSize: 0.1; value: Trackpad.speed
                        palette.accent: "#3B82F6"; width: parent.width - 60
                        anchors.verticalCenter: parent.verticalCenter
                        onMoved: Trackpad.setSpeed(value)
                    }
                    Text { text: "+1"; font.pixelSize: 11; color: Theme.textSecondary; anchors.verticalCenter: parent.verticalCenter }
                }
            }
        }
    }

    // Scroll & Zoom tab
    SettingsCard {
        visible: activeTab === 1
        Column {
            width: parent.width; spacing: 0

            Item {
                width: parent.width; height: 44
                Text { anchors.left: parent.left; anchors.verticalCenter: parent.verticalCenter; text: "Natural Scrolling"; font.pixelSize: 12; font.weight: Font.DemiBold; color: Theme.textPrimary }
                Rectangle {
                    id: natTog; width: 36; height: 18; radius: 9
                    property bool localState: Trackpad.naturalScroll
                    anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter
                    color: localState ? "#3B82F6" : "#cccccc"
                    Behavior on color { ColorAnimation { duration: 200 } }
                    Rectangle { width: 14; height: 14; radius: 7; color: "white"; y: 2; x: natTog.localState ? 20 : 2; Behavior on x { NumberAnimation { duration: 200 } } }
                    MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor; onClicked: { natTog.localState = !natTog.localState; Trackpad.setNaturalScroll(natTog.localState) } }
                }
            }

            Rectangle { width: parent.width; height: 1; color: Theme.divider }

            Item {
                width: parent.width; height: 44
                Text { anchors.left: parent.left; anchors.verticalCenter: parent.verticalCenter; text: "Two Finger Scrolling"; font.pixelSize: 12; font.weight: Font.DemiBold; color: Theme.textPrimary }
                Rectangle {
                    id: twoTog; width: 36; height: 18; radius: 9
                    property bool localState: Trackpad.twoFingerScroll
                    anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter
                    color: localState ? "#3B82F6" : "#cccccc"
                    Behavior on color { ColorAnimation { duration: 200 } }
                    Rectangle { width: 14; height: 14; radius: 7; color: "white"; y: 2; x: twoTog.localState ? 20 : 2; Behavior on x { NumberAnimation { duration: 200 } } }
                    MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor; onClicked: { twoTog.localState = !twoTog.localState; Trackpad.setTwoFingerScroll(twoTog.localState) } }
                }
            }
        }
    }

    // More Gestures tab
    SettingsCard {
        visible: activeTab === 2
        Item {
            width: parent.width; height: 60
            Text { anchors.centerIn: parent; text: "More gestures coming soon"; font.pixelSize: 13; color: Theme.textSecondary }
        }
    }

    Spacer {}
}
