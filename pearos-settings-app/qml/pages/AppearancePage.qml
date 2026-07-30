import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "../components"

PageBase {
    title: "Appearance"
    readonly property string ap: "file:///usr/share/extras/system-settings/assets/"
    property bool devPanelVisible: false
    Component.onCompleted: { Appearance.refresh(); Scrollbars.refresh() }

    Item {
        width: 0; height: 0
        Shortcut {
            sequence: "Meta+Alt+C"
            onActivated: {
                devPanelVisible = !devPanelVisible
                if (devPanelVisible) FilerConfig.refresh()
            }
        }
    }

    // Theme selection card
    SettingsCard {
        Item {
            width: parent.width; height: 76

            Text {
                anchors.left: parent.left; anchors.verticalCenter: parent.verticalCenter
                text: "Appearance"; font.pixelSize: 13; font.weight: Font.DemiBold; color: Theme.textPrimary
            }

            Row {
                anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter
                spacing: 12
                Repeater {
                    model: [
                        { name: "auto",  label: "Auto",  img: ap + "auto.png" },
                        { name: "light", label: "Light", img: ap + "light.png" },
                        { name: "dark",  label: "Dark",  img: ap + "dark.png" },
                    ]
                    delegate: Column {
                        spacing: 5
                        Rectangle {
                            width: 72; height: 48; radius: 8
                            border.color: Appearance.colorScheme === modelData.name ? Theme.accent : Qt.rgba(0,0,0,0.12)
                            border.width: Appearance.colorScheme === modelData.name ? 2 : 1
                            color: "transparent"
                            clip: true
                            Image {
                                anchors.fill: parent; anchors.margins: 2
                                source: modelData.img; fillMode: Image.PreserveAspectFit
                            }
                            MouseArea {
                                anchors.fill: parent
                                cursorShape: Qt.PointingHandCursor
                                onClicked: {
                                    Appearance.setColorScheme(modelData.name)
                                    if (modelData.name === "dark") Dock.set("skinName", "Tahoe Dark")
                                    else if (modelData.name === "light") Dock.set("skinName", "Tahoe")
                                }
                            }
                        }
                        Text { text: modelData.label; font.pixelSize: 10; color: Theme.textPrimary; anchors.horizontalCenter: parent.horizontalCenter }
                    }
                }
            }
        }
    }
    Spacer {}

    SectionTitle { text: "Theme" }
    SettingsCard {
        Column {
            width: parent.width; spacing: 0

            Item {
                width: parent.width; height: 44
                Text { anchors.left: parent.left; anchors.verticalCenter: parent.verticalCenter; text: "Color"; font.pixelSize: 12; font.weight: Font.DemiBold; color: Theme.textPrimary }
                Row {
                    anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter; spacing: 6
                    Repeater {
                        model: [
                            { name: "purple",      hex: "#8B5CF6" },
                            { name: "magenta",     hex: "#EC4899" },
                            { name: "orange",      hex: "#F97316" },
                            { name: "yellow",      hex: "#EAB308" },
                            { name: "green",       hex: "#22C55E" },
                            { name: "azul",        hex: "#06B6D4" },
                            { name: "blue",        hex: "#3B82F6" },
                            { name: "lila",        hex: "#A855F7" },
                            { name: "dark-purple", hex: "#6B21A8" },
                            { name: "grey",        hex: "#6B7280" },
                        ]
                        delegate: Rectangle {
                            width: 20; height: 20; radius: 10; color: modelData.hex
                            border.color: "white"
                            border.width: Appearance.accent === modelData.name ? 2 : 0
                            scale: Appearance.accent === modelData.name ? 1.15 : 1.0
                            Behavior on scale { NumberAnimation { duration: 120 } }
                            MouseArea {
                                anchors.fill: parent
                                cursorShape: Qt.PointingHandCursor
                                onClicked: Appearance.setAccent(modelData.name)
                            }
                        }
                    }
                }
            }
            Rectangle { width: parent.width; height: 1; color: Theme.divider }

            Item {
                width: parent.width; height: 44
                Text { anchors.left: parent.left; anchors.verticalCenter: parent.verticalCenter; text: "Text highlight color"; font.pixelSize: 12; font.weight: Font.DemiBold; color: Theme.textPrimary }
                Text { anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter; text: "Automatic"; font.pixelSize: 12; color: Theme.textSecondary }
            }
        }
    }
    Spacer {}

    SettingsCard {
        Column {
            width: parent.width; spacing: 0

            Item {
                width: parent.width; height: 44
                Text { anchors.left: parent.left; anchors.verticalCenter: parent.verticalCenter; text: "Icon & widget style"; font.pixelSize: 12; font.weight: Font.DemiBold; color: Theme.textPrimary }
                Row {
                    anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter; spacing: 6
                    property int selected: 3
                    Repeater {
                        model: ["Default", "Dark", "Clear", "Tinted"]
                        delegate: Rectangle {
                            height: 28; width: sLbl.implicitWidth + 16; radius: 6
                            color: parent.selected === index ? Theme.accent : Qt.rgba(0,0,0,0.08)
                            Text {
                                id: sLbl; anchors.centerIn: parent; text: modelData; font.pixelSize: 12
                                color: parent.parent.selected === index ? "white" : Theme.textPrimary
                            }
                            MouseArea { anchors.fill: parent; onClicked: parent.parent.selected = index }
                        }
                    }
                }
            }
            Rectangle { width: parent.width; height: 1; color: Theme.divider }

            Item {
                width: parent.width; height: 44
                Text { anchors.left: parent.left; anchors.verticalCenter: parent.verticalCenter; text: "Folder color"; font.pixelSize: 12; font.weight: Font.DemiBold; color: Theme.textPrimary }
                Text { anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter; text: "Automatic"; font.pixelSize: 12; color: Theme.textSecondary }
            }
        }
    }
    Spacer {}

    SectionTitle { text: "Windows" }
    SettingsCard {
        Column {
            width: parent.width; spacing: 0

            Item {
                width: parent.width; height: 44
                Text { anchors.left: parent.left; anchors.verticalCenter: parent.verticalCenter; text: "Sidebar icon size"; font.pixelSize: 12; font.weight: Font.DemiBold; color: Theme.textPrimary }
                SettingsComboBox {
                    anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter
                    model: ["Small", "Medium", "Large"]
                    currentIndex: 1; width: 100; height: 28; font.pixelSize: 12
                }
            }
            Rectangle { width: parent.width; height: 1; color: Theme.divider }

            Item {
                width: parent.width; height: 44
                Text { anchors.left: parent.left; anchors.verticalCenter: parent.verticalCenter; text: "Tint window with wallpaper color"; font.pixelSize: 12; font.weight: Font.DemiBold; color: Theme.textPrimary }
                LiquidToggle {
                    anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter
                    checked: Appearance.tintEnabled
                    onToggled: function(v) { Appearance.setTintEnabled(v) }
                }
            }
            Rectangle { width: parent.width; height: 1; color: Theme.divider }

            Item {
                width: parent.width; height: 44
                Text { anchors.left: parent.left; anchors.verticalCenter: parent.verticalCenter; text: "Animation speed"; font.pixelSize: 12; font.weight: Font.DemiBold; color: Theme.textPrimary }
                SettingsComboBox {
                    anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter
                    readonly property var factors: [0, 0.5, 1, 2]
                    model: ["Off", "Fast", "Normal", "Slow"]
                    currentIndex: {
                        var f = Appearance.animationSpeed
                        if (f <= 0) return 0
                        if (f <= 0.5) return 1
                        if (f <= 1) return 2
                        return 3
                    }
                    width: 100; height: 28; font.pixelSize: 12
                    onActivated: Appearance.setAnimationSpeed(factors[currentIndex])
                }
            }
        }
    }
    Spacer {}

    SettingsCard {
        Column {
            width: parent.width; spacing: 0

            component ScrollRadio: Item {
                property bool selected: false
                property string label: ""
                property var onSelect: null
                width: parent.width; height: 20
                Row {
                    spacing: 8; anchors.verticalCenter: parent.verticalCenter
                    Rectangle {
                        width: 16; height: 16; radius: 8; border.color: Theme.accent; border.width: 2
                        color: selected ? Theme.accent : "transparent"
                        anchors.verticalCenter: parent.verticalCenter
                    }
                    Text { text: label; font.pixelSize: 12; color: Theme.textPrimary; anchors.verticalCenter: parent.verticalCenter }
                }
                MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor; onClicked: if (onSelect) onSelect() }
            }

            Column {
                width: parent.width; spacing: 8; topPadding: 8; bottomPadding: 8
                Text { text: "Show scroll bars"; font.pixelSize: 12; font.weight: Font.DemiBold; color: Theme.textPrimary }
                ScrollRadio {
                    label: "Automatically based on mouse or trackpad"
                    selected: !Scrollbars.alwaysVisible
                    onSelect: function() { Scrollbars.setAlwaysVisible(false) }
                }
                ScrollRadio {
                    label: "Always"
                    selected: Scrollbars.alwaysVisible
                    onSelect: function() { Scrollbars.setAlwaysVisible(true) }
                }
            }
            Rectangle { width: parent.width; height: 1; color: Theme.divider }

            Column {
                width: parent.width; spacing: 8; topPadding: 8; bottomPadding: 8
                Text { text: "Click in the scroll bar to"; font.pixelSize: 12; font.weight: Font.DemiBold; color: Theme.textPrimary }
                ScrollRadio {
                    label: "Jump to the spot that's clicked"
                    selected: Scrollbars.clickToJump
                    onSelect: function() { Scrollbars.setClickToJump(true) }
                }
                ScrollRadio {
                    label: "Jump to the next page"
                    selected: !Scrollbars.clickToJump
                    onSelect: function() { Scrollbars.setClickToJump(false) }
                }
            }
            Text {
                width: parent.width; wrapMode: Text.WordWrap; topPadding: 4; bottomPadding: 4
                text: "Applies to GTK applications (Files, browsers, and other non-Qt apps). Qt/Plasma apps don't support this."
                font.pixelSize: 11; color: Theme.textSecondary
            }
        }
    }
    Spacer {}

    SectionTitle { text: "Liquid Gel" }
    SettingsCard {
        Column {
            width: parent.width; spacing: 0

            // Enable toggle
            Item {
                width: parent.width; height: 44
                Text { anchors.left: parent.left; anchors.verticalCenter: parent.verticalCenter; text: "Enable Liquid Gel"; font.pixelSize: 12; font.weight: Font.DemiBold; color: Theme.textPrimary }
                LiquidToggle {
                    anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter
                    checked: Appearance.lgEnabled
                    onToggled: function(v) { Appearance.setLgEnabled(v) }
                }
            }
            Rectangle { width: parent.width; height: 1; color: Theme.divider }

            // Blur Strength
            Item {
                width: parent.width; height: 44
                Text { anchors.left: parent.left; anchors.verticalCenter: parent.verticalCenter; text: "Blur Strength"; font.pixelSize: 12; font.weight: Font.DemiBold; color: Theme.textPrimary }
                Row {
                    anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter; spacing: 8
                    Slider {
                        width: 140; height: 28; from: 1; to: 15; stepSize: 1
                        value: Appearance.lgBlurStrength
                        onMoved: Appearance.setLgBlurStrength(Math.round(value))
                    }
                    Text { text: Appearance.lgBlurStrength; font.pixelSize: 12; color: Theme.textSecondary; width: 22; horizontalAlignment: Text.AlignRight }
                }
            }
            Rectangle { width: parent.width; height: 1; color: Theme.divider }

            // Noise Strength
            Item {
                width: parent.width; height: 44
                Text { anchors.left: parent.left; anchors.verticalCenter: parent.verticalCenter; text: "Noise Strength"; font.pixelSize: 12; font.weight: Font.DemiBold; color: Theme.textPrimary }
                Row {
                    anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter; spacing: 8
                    Slider {
                        width: 140; height: 28; from: 0; to: 14; stepSize: 1
                        value: Appearance.lgNoiseStrength
                        onMoved: Appearance.setLgNoiseStrength(Math.round(value))
                    }
                    Text { text: Appearance.lgNoiseStrength; font.pixelSize: 12; color: Theme.textSecondary; width: 22; horizontalAlignment: Text.AlignRight }
                }
            }
            Rectangle { width: parent.width; height: 1; color: Theme.divider }

            // Refraction Strength
            Item {
                width: parent.width; height: 44
                Text { anchors.left: parent.left; anchors.verticalCenter: parent.verticalCenter; text: "Refraction Strength"; font.pixelSize: 12; font.weight: Font.DemiBold; color: Theme.textPrimary }
                Row {
                    anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter; spacing: 8
                    Slider {
                        width: 140; height: 28; from: 0; to: 20; stepSize: 1
                        value: Appearance.lgRefractionStrength
                        onMoved: Appearance.setLgRefractionStrength(Math.round(value))
                    }
                    Text { text: Appearance.lgRefractionStrength; font.pixelSize: 12; color: Theme.textSecondary; width: 22; horizontalAlignment: Text.AlignRight }
                }
            }
            Rectangle { width: parent.width; height: 1; color: Theme.divider }

            // Refraction Edge Size
            Item {
                width: parent.width; height: 44
                Text { anchors.left: parent.left; anchors.verticalCenter: parent.verticalCenter; text: "Refraction Edge Size"; font.pixelSize: 12; font.weight: Font.DemiBold; color: Theme.textPrimary }
                Row {
                    anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter; spacing: 8
                    Slider {
                        width: 140; height: 28; from: 0; to: 20; stepSize: 1
                        value: Appearance.lgRefractionEdgeSize
                        onMoved: Appearance.setLgRefractionEdgeSize(Math.round(value))
                    }
                    Text { text: Appearance.lgRefractionEdgeSize; font.pixelSize: 12; color: Theme.textSecondary; width: 22; horizontalAlignment: Text.AlignRight }
                }
            }
            Rectangle { width: parent.width; height: 1; color: Theme.divider }

            // RGB Fringing
            Item {
                width: parent.width; height: 44
                Text { anchors.left: parent.left; anchors.verticalCenter: parent.verticalCenter; text: "RGB Fringing"; font.pixelSize: 12; font.weight: Font.DemiBold; color: Theme.textPrimary }
                Row {
                    anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter; spacing: 8
                    Slider {
                        width: 140; height: 28; from: 0; to: 20; stepSize: 1
                        value: Appearance.lgRGBFringing
                        onMoved: Appearance.setLgRGBFringing(Math.round(value))
                    }
                    Text { text: Appearance.lgRGBFringing; font.pixelSize: 12; color: Theme.textSecondary; width: 22; horizontalAlignment: Text.AlignRight }
                }
            }
        }
    }
    Spacer {}

    // ── Filer live config editor (Meta+Alt+C) ───────────────────────────
    Column {
        width: parent.width; spacing: 0
        visible: devPanelVisible

        SectionTitle { text: "Filer — Live Settings (Meta+Alt+C)" }

        Repeater {
            model: FilerConfig.groups
            delegate: Column {
                id: groupDelegate
                width: parent.width
                property var groupData: modelData

                SectionTitle { text: groupDelegate.groupData.name }
                SettingsCard {
                    Column {
                        width: parent.width; spacing: 0
                        Repeater {
                            model: groupDelegate.groupData.keys
                            delegate: Column {
                                id: keyDelegate
                                width: parent.width
                                property var keyData: modelData
                                property string group: groupDelegate.groupData.name

                                // bool → toggle
                                Item {
                                    visible: keyDelegate.keyData.type === "bool"
                                    width: parent.width; height: visible ? 40 : 0
                                    Text {
                                        anchors.left: parent.left; anchors.verticalCenter: parent.verticalCenter
                                        text: keyDelegate.keyData.key; font.pixelSize: 12; color: Theme.textPrimary
                                    }
                                    LiquidToggle {
                                        anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter
                                        checked: keyDelegate.keyData.value === "true"
                                        onToggled: function(v) { FilerConfig.setValue(keyDelegate.group, keyDelegate.keyData.key, v ? "true" : "false") }
                                    }
                                }

                                // int → plain text field (numeric)
                                Item {
                                    visible: keyDelegate.keyData.type === "int"
                                    width: parent.width; height: visible ? 40 : 0
                                    Text {
                                        anchors.left: parent.left; anchors.verticalCenter: parent.verticalCenter
                                        text: keyDelegate.keyData.key; font.pixelSize: 12; color: Theme.textPrimary
                                        width: parent.width * 0.35; elide: Text.ElideRight
                                    }
                                    TextField {
                                        anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter
                                        width: parent.width * 0.6; height: 28; font.pixelSize: 11
                                        validator: IntValidator {}
                                        text: keyDelegate.keyData.value
                                        onEditingFinished: FilerConfig.setValue(keyDelegate.group, keyDelegate.keyData.key, text)
                                    }
                                }

                                // list (comma-separated) → chips + add row
                                Column {
                                    id: listBlock
                                    visible: keyDelegate.keyData.type === "list"
                                    width: parent.width; spacing: 6; topPadding: 8; bottomPadding: 8
                                    property string newItem: ""
                                    readonly property var items: keyDelegate.keyData.value
                                        .split(',').map(function(s) { return s.trim() }).filter(function(s) { return s.length > 0 })

                                    function commit(arr) {
                                        FilerConfig.setValue(keyDelegate.group, keyDelegate.keyData.key, arr.join(', '))
                                    }

                                    Text { text: keyDelegate.keyData.key + " (" + listBlock.items.length + ")"; font.pixelSize: 12; font.weight: Font.DemiBold; color: Theme.textPrimary }

                                    property bool dragging: false
                                    property int dragIndex: -1
                                    property real ghostX: 0
                                    property real ghostY: 0
                                    property string ghostText: ""

                                    Item {
                                        width: parent.width; height: chipFlow.height

                                        Flow {
                                            id: chipFlow
                                            width: parent.width; spacing: 6
                                            Repeater {
                                                id: chipRep
                                                model: listBlock.items
                                                delegate: Rectangle {
                                                    id: chipRoot
                                                    height: 24; radius: 6
                                                    color: Qt.rgba(0,0,0,0.08)
                                                    opacity: (listBlock.dragging && listBlock.dragIndex === index) ? 0.25 : 1.0
                                                    width: chipRow.implicitWidth + 20

                                                    Row {
                                                        id: chipRow; anchors.centerIn: parent; spacing: 6

                                                        // Drag handle — the MouseArea lives inside its own wrapper Item
                                                        // (sized with generous padding) so it never overlaps the ✕ button
                                                        // that sits later in this same Row.
                                                        Item {
                                                            width: dragHandle.implicitWidth + 12; height: chipRoot.height
                                                            Text {
                                                                id: dragHandle
                                                                anchors.centerIn: parent
                                                                text: "⠿"; font.pixelSize: 11; color: Theme.textSecondary
                                                            }
                                                            MouseArea {
                                                                id: dragMA
                                                                anchors.fill: parent
                                                                cursorShape: Qt.SizeAllCursor
                                                                preventStealing: true
                                                                property real pressDX: 0
                                                                property real pressDY: 0

                                                                onPressed: function(mouse) {
                                                                    pressDX = mouse.x
                                                                    pressDY = mouse.y
                                                                    listBlock.dragIndex = index
                                                                }
                                                                onPositionChanged: function(mouse) {
                                                                    if (!pressed) return
                                                                    listBlock.dragging = true
                                                                    var g = dragMA.mapToItem(chipFlow, mouse.x - pressDX, mouse.y - pressDY)
                                                                    listBlock.ghostX = g.x
                                                                    listBlock.ghostY = g.y
                                                                    listBlock.ghostText = modelData

                                                                    var p = dragMA.mapToItem(chipFlow, mouse.x, mouse.y)
                                                                    for (var i = 0; i < chipRep.count; i++) {
                                                                        var it = chipRep.itemAt(i)
                                                                        if (!it) continue
                                                                        if (p.x >= it.x && p.x <= it.x + it.width &&
                                                                            p.y >= it.y && p.y <= it.y + it.height) {
                                                                            if (i !== listBlock.dragIndex) {
                                                                                var arr = listBlock.items.slice()
                                                                                var moved = arr.splice(listBlock.dragIndex, 1)[0]
                                                                                arr.splice(i, 0, moved)
                                                                                listBlock.dragIndex = i
                                                                                listBlock.commit(arr)
                                                                            }
                                                                            break
                                                                        }
                                                                    }
                                                                }
                                                                onReleased: { listBlock.dragging = false; listBlock.dragIndex = -1 }
                                                                onCanceled: { listBlock.dragging = false; listBlock.dragIndex = -1 }
                                                            }
                                                        }

                                                        Text { text: modelData; font.pixelSize: 11; color: Theme.textPrimary; anchors.verticalCenter: parent.verticalCenter }
                                                        Text {
                                                            text: "✕"; font.pixelSize: 11; color: Theme.textSecondary; anchors.verticalCenter: parent.verticalCenter
                                                            MouseArea {
                                                                anchors.fill: parent; anchors.margins: -4; cursorShape: Qt.PointingHandCursor
                                                                onClicked: {
                                                                    var arr = listBlock.items.slice()
                                                                    arr.splice(index, 1)
                                                                    listBlock.commit(arr)
                                                                }
                                                            }
                                                        }
                                                    }
                                                }
                                            }
                                        }

                                        Rectangle {
                                            visible: listBlock.dragging
                                            x: listBlock.ghostX; y: listBlock.ghostY
                                            z: 1000; opacity: 0.9
                                            height: 24; radius: 6; color: Theme.accent
                                            width: ghostLbl.implicitWidth + 20
                                            Text { id: ghostLbl; anchors.centerIn: parent; text: listBlock.ghostText; font.pixelSize: 11; color: "white" }
                                        }
                                    }

                                    Row {
                                        width: parent.width; spacing: 6
                                        TextField {
                                            width: parent.width - 70; height: 28; font.pixelSize: 11
                                            placeholderText: "Add item…"
                                            onTextChanged: listBlock.newItem = text
                                            Keys.onReturnPressed: addBtn.clicked()
                                        }
                                        Rectangle {
                                            id: addBtn
                                            height: 28; width: 64; radius: 6
                                            color: listBlock.newItem.trim().length > 0 ? Theme.accent : Qt.rgba(0,0,0,0.08)
                                            Text { anchors.centerIn: parent; text: "Add"; font.pixelSize: 12; color: listBlock.newItem.trim().length > 0 ? "white" : Theme.textSecondary }
                                            signal clicked()
                                            onClicked: {
                                                var v = listBlock.newItem.trim()
                                                if (v.length === 0) return
                                                var arr = listBlock.items.slice()
                                                arr.push(v)
                                                listBlock.commit(arr)
                                                listBlock.newItem = ""
                                            }
                                            MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor; onClicked: addBtn.clicked() }
                                        }
                                    }
                                }

                                // color → swatch + hex field
                                Item {
                                    visible: keyDelegate.keyData.type === "color"
                                    width: parent.width; height: visible ? 40 : 0
                                    Text {
                                        anchors.left: parent.left; anchors.verticalCenter: parent.verticalCenter
                                        text: keyDelegate.keyData.key; font.pixelSize: 12; color: Theme.textPrimary
                                    }
                                    Row {
                                        anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter; spacing: 6
                                        Rectangle {
                                            width: 20; height: 20; radius: 4; anchors.verticalCenter: parent.verticalCenter
                                            color: keyDelegate.keyData.value; border.color: Theme.divider; border.width: 1
                                        }
                                        TextField {
                                            width: 90; height: 28; font.pixelSize: 11
                                            text: keyDelegate.keyData.value
                                            onEditingFinished: FilerConfig.setValue(keyDelegate.group, keyDelegate.keyData.key, text)
                                        }
                                    }
                                }

                                // string → plain text field
                                Item {
                                    visible: keyDelegate.keyData.type === "string"
                                    width: parent.width; height: visible ? 40 : 0
                                    Text {
                                        anchors.left: parent.left; anchors.verticalCenter: parent.verticalCenter
                                        text: keyDelegate.keyData.key; font.pixelSize: 12; color: Theme.textPrimary
                                        width: parent.width * 0.35; elide: Text.ElideRight
                                    }
                                    TextField {
                                        anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter
                                        width: parent.width * 0.6; height: 28; font.pixelSize: 11
                                        text: keyDelegate.keyData.value
                                        onEditingFinished: FilerConfig.setValue(keyDelegate.group, keyDelegate.keyData.key, text)
                                    }
                                }

                                Rectangle {
                                    width: parent.width; height: 1; color: Theme.divider
                                    visible: index < groupDelegate.groupData.keys.length - 1
                                }
                            }
                        }
                    }
                }
                Spacer {}
            }
        }
    }
}
