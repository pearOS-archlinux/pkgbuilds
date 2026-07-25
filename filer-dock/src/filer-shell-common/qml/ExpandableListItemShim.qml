import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami

// Stand-in for org.kde.plasma.extras' PlasmaExtras.ExpandableListItem -- no
// 1:1 QtQuick.Controls equivalent (icon+title+subtitle row, a "default
// action" button, an overflow menu of contextual actions, and an
// expand/collapse toggle revealing an arbitrary `customExpandedViewContent`
// component below). Reimplemented from scratch, covering exactly the API
// surface this project's two consumers (ConnectionItem.qml, DeviceItem.qml)
// actually use.
QQC2.ItemDelegate {
    id: root

    // Not named "icon" -- QQC2.ItemDelegate (via AbstractButton) already
    // declares a FINAL `icon` grouped property (QQuickIcon) in C++; QML
    // can't override a FINAL property, even to widen its type.
    property var itemIcon: undefined
    property string title: ""
    property string subtitle: ""
    property bool isBusy: false
    property bool isDefault: false
    property QtObject defaultActionButtonAction: null
    property bool showDefaultActionButtonWhenBusy: false
    property list<QtObject> contextualActions: []
    property Component customExpandedViewContent: null
    readonly property Item customExpandedViewContentItem: expandedLoader.item
    property bool expanded: false

    signal itemCollapsed()

    function expand() { root.expanded = true }
    function collapse() { root.expanded = false; root.itemCollapsed(); }

    hoverEnabled: true
    highlighted: root.isDefault

    contentItem: ColumnLayout {
        spacing: 0

        RowLayout {
            Layout.fillWidth: true
            spacing: Kirigami.Units.smallSpacing

            Kirigami.Icon {
                source: root.itemIcon ?? ""
                visible: !!root.itemIcon
                Layout.preferredWidth: Kirigami.Units.iconSizes.medium
                Layout.preferredHeight: Kirigami.Units.iconSizes.medium
            }

            ColumnLayout {
                Layout.fillWidth: true
                spacing: 0

                QQC2.Label {
                    Layout.fillWidth: true
                    text: root.title
                    elide: Text.ElideRight
                    font.bold: root.isDefault
                }
                QQC2.Label {
                    Layout.fillWidth: true
                    text: root.subtitle
                    visible: root.subtitle.length > 0
                    elide: Text.ElideRight
                    opacity: 0.7
                    font: Kirigami.Theme.smallFont
                }
            }

            QQC2.BusyIndicator {
                visible: root.isBusy
                running: visible
                Layout.preferredWidth: Kirigami.Units.iconSizes.small
                Layout.preferredHeight: Kirigami.Units.iconSizes.small
            }

            QQC2.ToolButton {
                visible: root.defaultActionButtonAction !== null && (!root.isBusy || root.showDefaultActionButtonWhenBusy)
                action: root.defaultActionButtonAction
                display: QQC2.AbstractButton.IconOnly
            }

            QQC2.ToolButton {
                visible: root.contextualActions.length > 0
                icon.name: "overflow-menu-symbolic"
                onClicked: contextMenu.popup()

                QQC2.Menu {
                    id: contextMenu
                    Repeater {
                        model: root.contextualActions
                        QQC2.MenuItem {
                            required property QtObject modelData
                            action: modelData
                        }
                    }
                }
            }

            QQC2.ToolButton {
                visible: root.customExpandedViewContent !== null
                icon.name: root.expanded ? "arrow-up-symbolic" : "arrow-down-symbolic"
                onClicked: root.expanded ? root.collapse() : root.expand()
            }
        }

        Loader {
            id: expandedLoader
            Layout.fillWidth: true
            active: root.expanded && root.customExpandedViewContent !== null
            sourceComponent: root.customExpandedViewContent
        }
    }
}
