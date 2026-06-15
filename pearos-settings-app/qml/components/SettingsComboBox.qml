import QtQuick
import QtQuick.Controls

Item {
    id: root

    property var model: []
    property int currentIndex: 0
    property string currentText: {
        if (!model) return ""
        if (Array.isArray(model)) return model[currentIndex] || ""
        return ""
    }
    property string displayText: currentText
    property font font

    signal activated(int index)

    implicitWidth: 120
    implicitHeight: 30

    // Button
    Rectangle {
        id: btn
        anchors.fill: parent
        radius: 6
        color: btnArea.pressed ? Qt.darker(Theme.bgCard, 1.08)
             : btnArea.containsMouse ? Qt.lighter(Theme.bgCard, 1.08)
             : Theme.bgCard
        border.color: Theme.divider
        border.width: 1

        Text {
            anchors.left: parent.left
            anchors.right: chevron.left
            anchors.leftMargin: 10
            anchors.rightMargin: 4
            anchors.verticalCenter: parent.verticalCenter
            text: root.displayText
            font: root.font
            color: Theme.textPrimary
            elide: Text.ElideRight
            verticalAlignment: Text.AlignVCenter
        }

        Text {
            id: chevron
            anchors.right: parent.right
            anchors.rightMargin: 9
            anchors.verticalCenter: parent.verticalCenter
            anchors.verticalCenterOffset: 1
            text: "⌄"
            font.pixelSize: 14
            color: Theme.textSecondary
        }

        MouseArea {
            id: btnArea
            anchors.fill: parent
            hoverEnabled: true
            cursorShape: Qt.PointingHandCursor
            onClicked: popup.open()
        }
    }

    Popup {
        id: popup
        y: root.height + 2
        width: root.width
        padding: 0
        topPadding: 4
        bottomPadding: 4
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

        background: Rectangle {
            color: Theme.bgCard
            radius: 8
            border.color: Theme.divider
            border.width: 1
        }

        contentItem: ListView {
            id: listView
            clip: true
            implicitHeight: Math.min(contentHeight, 220)
            model: root.model
            currentIndex: root.currentIndex

            ScrollBar.vertical: ScrollBar {
                policy: listView.contentHeight > 220 ? ScrollBar.AlwaysOn : ScrollBar.AlwaysOff
            }

            delegate: Item {
                width: popup.width
                height: 30

                Rectangle {
                    anchors.fill: parent
                    color: rowArea.containsMouse ? Theme.activeBg
                         : (index === root.currentIndex ? Qt.rgba(Theme.accent.r, Theme.accent.g, Theme.accent.b, 0.12)
                         : "transparent")
                }

                Text {
                    anchors.left: parent.left
                    anchors.leftMargin: 10
                    anchors.right: parent.right
                    anchors.rightMargin: 10
                    anchors.verticalCenter: parent.verticalCenter
                    text: modelData
                    font: root.font
                    color: Theme.textPrimary
                    elide: Text.ElideRight
                    verticalAlignment: Text.AlignVCenter
                }

                MouseArea {
                    id: rowArea
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: {
                        root.currentIndex = index
                        root.activated(index)
                        popup.close()
                    }
                }
            }
        }
    }
}
