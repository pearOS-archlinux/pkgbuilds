import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "components"
import "pages"

ApplicationWindow {
    id: window
    width: 860
    height: 560
    minimumWidth: 820
    minimumHeight: 520
    visible: true
    title: "System Settings"
    flags: Qt.Window | Qt.FramelessWindowHint
    color: "transparent"

    readonly property string ap: "file:///home/alxb421/Desktop/pkgbuilds/pearos-settings-app/port/assets/"

    // Page index map — must match StackLayout order below
    // 0  Wi-Fi             1  Bluetooth          2  Network         3  Battery
    // 4  General           5  Accessibility       6  Appearance      7  Menu Bar
    // 8  Pear Intelligence 9  Desktop and Dock   10  Displays        11 Wallpaper
    // 12 Spotlight         13 Notifications       14 Sound           15 Focus
    // 16 Screen Time       17 Lock Screen         18 Privacy          19 Touch ID
    // 20 Users & Groups    21 Internet Accounts   22 Game Center      23 pCloud
    // 24 Wallet & Pear Pay 25 Keyboard            26 Trackpad         27 Printers
    // 28 About             29 Software Update     30 Storage          31 Date & Time
    // 32 Pear ID

    property string searchQuery: ""

    readonly property var allNavItems: [
        { label: "Wi-Fi",                    idx: 0  },
        { label: "Bluetooth",                idx: 1  },
        { label: "Network",                  idx: 2  },
        { label: "Battery",                  idx: 3  },
        { label: "General",                  idx: 4  },
        { label: "Accessibility",            idx: 5  },
        { label: "Appearance",               idx: 6  },
        { label: "Menu Bar",                 idx: 7  },
        { label: "Pear Intelligence & Piri", idx: 8  },
        { label: "Desktop and Dock",         idx: 9  },
        { label: "Displays",                 idx: 10 },
        { label: "Wallpaper",                idx: 11 },
        { label: "Spotlight",                idx: 12 },
        { label: "Notifications",            idx: 13 },
        { label: "Sound",                    idx: 14 },
        { label: "Focus",                    idx: 15 },
        { label: "Screen Time",              idx: 16 },
        { label: "Lock Screen",              idx: 17 },
        { label: "Privacy & Security",       idx: 18 },
        { label: "Touch ID & Password",      idx: 19 },
        { label: "Users & Groups",           idx: 20 },
        { label: "Internet Accounts",        idx: 21 },
        { label: "Game Center",              idx: 22 },
        { label: "pCloud",                   idx: 23 },
        { label: "Wallet & Pear Pay",        idx: 24 },
        { label: "Keyboard",                 idx: 25 },
        { label: "Trackpad",                 idx: 26 },
        { label: "Printers & Scanners",      idx: 27 },
        { label: "About",                    idx: 28 },
        { label: "Software Update",          idx: 29 },
        { label: "Storage",                  idx: 30 },
        { label: "Date & Time",              idx: 31 },
        { label: "Pear ID",                  idx: 32 },
    ]

    readonly property var pageTitles: [
        "Wi-Fi", "Bluetooth", "Network", "Battery",
        "General", "Accessibility", "Appearance", "Menu Bar",
        "Pear Intelligence & Piri", "Desktop and Dock", "Displays", "Wallpaper",
        "Spotlight", "Notifications", "Sound", "Focus", "Screen Time",
        "Lock Screen", "Privacy & Security", "Touch ID & Password", "Users & Groups",
        "Internet Accounts", "Game Center", "pCloud", "Wallet & Pear Pay",
        "Keyboard", "Trackpad", "Printers & Scanners",
        "About", "Software Update", "Storage", "Date & Time",
        "Pear ID"
    ]

    Rectangle {
        id: root
        anchors.fill: parent
        radius: 25
        color: Theme.bg
        border.color: Qt.styleHints.colorScheme === Qt.ColorScheme.Dark ? "#35404C" : Qt.rgba(0,0,0,0.12)
        border.width: 1
        clip: true

        // Window tint overlay — picks up wallpaper average color like original CSS --window-tint
        Rectangle {
            anchors.fill: parent
            color: {
                var hex = Wallpaper.tintColor
                if (!hex || hex === "transparent" || hex.length < 7) return "transparent"
                var c = Qt.color(hex)
                return Qt.rgba(c.r, c.g, c.b, 0.08)
            }
            radius: 25
            z: 100
            enabled: false
        }

        // ── SIDENAV ──────────────────────────────────────────────────
        Rectangle {
            id: sidenav
            x: 6; y: 6
            width: 218
            height: parent.height - 12
            radius: 10
            color: Theme.bgSidenav
            border.color: Theme.border
            border.width: 1
            layer.enabled: true
            clip: true

            // Traffic lights (topnav area of sidenav)
            Row {
                id: trafficLights
                x: 14; y: 14
                spacing: 6
                z: 10

                Rectangle {
                    width: 13; height: 13; radius: 7
                    color: "#fe5b51"
                    MouseArea {
                        anchors.fill: parent
                        onClicked: Qt.quit()
                        cursorShape: Qt.PointingHandCursor
                    }
                }
                Rectangle {
                    width: 13; height: 13; radius: 7
                    color: "#e6c02a"
                    MouseArea {
                        anchors.fill: parent
                        onClicked: window.showMinimized()
                        cursorShape: Qt.PointingHandCursor
                    }
                }
                Rectangle {
                    width: 13; height: 13; radius: 7
                    color: "#51c329"
                    opacity: 0.5
                }
            }

            // Search bar
            Rectangle {
                x: 8; y: 40
                width: parent.width - 16
                height: 30
                radius: 15
                color: Theme.bgSearch

                Row {
                    anchors.left: parent.left; anchors.leftMargin: 10
                    anchors.verticalCenter: parent.verticalCenter
                    spacing: 6
                    Text {
                        text: "⌕"; font.pixelSize: 15; opacity: 0.5
                        color: Theme.textPrimary
                        anchors.verticalCenter: parent.verticalCenter
                    }
                    TextInput {
                        id: searchInput
                        width: 150; height: 18
                        font.pixelSize: 13
                        color: Theme.textPrimary
                        clip: true
                        verticalAlignment: TextInput.AlignVCenter
                        onTextChanged: window.searchQuery = text.toLowerCase()
                        Text {
                            anchors.fill: parent
                            text: "Search"
                            color: Theme.textTertiary
                            font.pixelSize: 13
                            visible: parent.text.length === 0
                            verticalAlignment: Text.AlignVCenter
                        }
                    }
                }
            }

            // Search results dropdown
            Rectangle {
                x: 8; y: 78
                width: parent.width - 16
                visible: window.searchQuery.length > 0
                radius: 8
                color: Theme.bgCard
                border.color: Theme.divider; border.width: 1
                z: 10
                clip: true

                property var filtered: {
                    var q = window.searchQuery
                    return window.allNavItems.filter(function(n) {
                        return n.label.toLowerCase().indexOf(q) !== -1
                    })
                }

                height: filtered.length > 0 ? filtered.length * 34 : 0

                Column {
                    width: parent.width; spacing: 0
                    Repeater {
                        model: parent.parent.filtered
                        delegate: Item {
                            width: parent.width; height: 34
                            Text {
                                anchors.left: parent.left; anchors.leftMargin: 12
                                anchors.verticalCenter: parent.verticalCenter
                                text: modelData.label; font.pixelSize: 13; color: Theme.textPrimary
                            }
                            MouseArea {
                                anchors.fill: parent; cursorShape: Qt.PointingHandCursor
                                onClicked: { Navigator.currentIdx = modelData.idx; searchInput.text = "" }
                            }
                            Rectangle {
                                anchors.bottom: parent.bottom; width: parent.width; height: 1
                                color: Theme.divider
                                visible: index < parent.parent.count - 1
                            }
                        }
                    }
                }
            }

            // User / Pear ID section
            Item {
                id: userSection
                x: 8; y: 78
                width: parent.width - 16
                height: 56
                visible: window.searchQuery.length === 0

                Rectangle {
                    anchors.fill: parent; radius: 8
                    color: userSectionHov.containsMouse ? Theme.hoverBg : "transparent"
                    MouseArea {
                        id: userSectionHov
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: Navigator.currentIdx = 32
                    }
                }

                Row {
                    anchors.left: parent.left; anchors.leftMargin: 6
                    anchors.verticalCenter: parent.verticalCenter
                    spacing: 8

                    Rectangle {
                        width: 40; height: 40; radius: 20; color: Theme.bgSearch
                        clip: true
                        Image {
                            anchors.fill: parent
                            source: window.ap + "user.png"
                            fillMode: Image.PreserveAspectCrop
                        }
                    }

                    Column {
                        anchors.verticalCenter: parent.verticalCenter
                        spacing: 1
                        Text {
                            text: PearID.state === "loggedin" && PearID.userName.length > 0
                                  ? PearID.userName : "Sign in"
                            font.pixelSize: 13; font.weight: Font.DemiBold
                            color: Theme.textPrimary
                        }
                        Text {
                            text: PearID.state === "loggedin" && PearID.userEmail.length > 0
                                  ? PearID.userEmail : "with your pear account"
                            font.pixelSize: 11
                            color: Theme.textSecondary
                        }
                    }
                }
            }

            // Nav items list
            ScrollView {
                x: 0; y: 140
                width: sidenav.width
                height: sidenav.height - 140
                clip: true
                ScrollBar.vertical.policy: ScrollBar.AlwaysOff

                Flickable {
                    anchors.fill: parent
                    contentWidth: width
                    contentHeight: navColumn.implicitHeight + 20
                    boundsMovement: Flickable.FollowBoundsBehavior
                    boundsBehavior: Flickable.DragAndOvershootBounds
                    flickableDirection: Flickable.VerticalFlick

                    Column {
                        id: navColumn
                        width: parent.width
                        spacing: 0

                        // Section 1: Connectivity
                        NavSection {
                            model: [
                                { icon: window.ap + "cs-network.svg",   label: "Wi-Fi",    idx: 0 },
                                { icon: window.ap + "cs-bluetooth.svg", label: "Bluetooth", idx: 1 },
                                { icon: window.ap + "cs-network.svg",   label: "Network",  idx: 2 },
                                { icon: window.ap + "cs-power.svg",     label: "Battery",  idx: 3 },
                            ]
                            currentIdx: Navigator.currentIdx
                            onItemClicked: (idx) => Navigator.currentIdx = idx
                        }

                        // Section 2: System
                        NavSection {
                            model: [
                                { icon: window.ap + "application-default-icon.svg",               label: "General",                  idx: 4  },
                                { icon: window.ap + "accessib.png",                               label: "Accessibility",            idx: 5  },
                                { icon: window.ap + "preferences-desktop-theme-global.svg",       label: "Appearance",               idx: 6  },
                                { icon: window.ap + "preferences-ubuntu-panel.svg",               label: "Menu Bar",                 idx: 7  },
                                { icon: window.ap + "pear_intelligence.png",                      label: "Pear Intelligence & Piri", idx: 8  },
                                { icon: window.ap + "preferences-desktop-workspaces.svg",         label: "Desktop and Dock",         idx: 9  },
                                { icon: window.ap + "cinnamon-preferences-desktop-display.svg",   label: "Displays",                 idx: 10 },
                                { icon: window.ap + "preferences-desktop-wallpaper.svg",          label: "Wallpaper",                idx: 11 },
                                { icon: window.ap + "preferences-other.svg",                      label: "Spotlight",                idx: 12 },
                            ]
                            currentIdx: Navigator.currentIdx
                            onItemClicked: (idx) => Navigator.currentIdx = idx
                        }

                        // Section 3: Media & Focus
                        NavSection {
                            model: [
                                { icon: window.ap + "cs-notifications.svg", label: "Notifications", idx: 13 },
                                { icon: window.ap + "cs-sound.svg",         label: "Sound",         idx: 14 },
                                { icon: window.ap + "preferences-other.svg", label: "Focus",        idx: 15 },
                                { icon: window.ap + "preferences-other.svg", label: "Screen Time",  idx: 16 },
                            ]
                            currentIdx: Navigator.currentIdx
                            onItemClicked: (idx) => Navigator.currentIdx = idx
                        }

                        // Section 4: Security
                        NavSection {
                            model: [
                                { icon: window.ap + "preferences-desktop-user-password.svg", label: "Lock Screen",        idx: 17 },
                                { icon: window.ap + "cs-privacy.svg",                        label: "Privacy & Security", idx: 18 },
                                { icon: window.ap + "preferences-desktop-user-password.svg", label: "Touch ID & Password",idx: 19 },
                                { icon: window.ap + "cs-user-accounts.svg",                  label: "Users & Groups",     idx: 20 },
                            ]
                            currentIdx: Navigator.currentIdx
                            onItemClicked: (idx) => Navigator.currentIdx = idx
                        }

                        // Section 5: Accounts
                        NavSection {
                            model: [
                                { icon: window.ap + "cs-online-accounts.svg",    label: "Internet Accounts", idx: 21 },
                                { icon: window.ap + "preferences-other.svg",     label: "Game Center",       idx: 22 },
                                { icon: window.ap + "preferences-other.svg",     label: "pCloud",            idx: 23 },
                                { icon: window.ap + "preferences-other.svg",     label: "Wallet & Pear Pay", idx: 24 },
                            ]
                            currentIdx: Navigator.currentIdx
                            onItemClicked: (idx) => Navigator.currentIdx = idx
                        }

                        // Section 6: Hardware
                        NavSection {
                            model: [
                                { icon: window.ap + "cs-keyboard.svg",                    label: "Keyboard",           idx: 25 },
                                { icon: window.ap + "preferences-desktop-touchpad.svg",   label: "Trackpad",           idx: 26 },
                                { icon: window.ap + "cs-printer.svg",                     label: "Printers & Scanners",idx: 27 },
                            ]
                            currentIdx: Navigator.currentIdx
                            onItemClicked: (idx) => Navigator.currentIdx = idx
                        }

                        // Section 7: Info
                        NavSection {
                            model: [
                                { icon: window.ap + "system-help.svg",              label: "About",           idx: 28 },
                                { icon: window.ap + "system-software-update.svg",   label: "Software Update", idx: 29 },
                                { icon: window.ap + "preferences-system-disks.svg", label: "Storage",         idx: 30 },
                                { icon: window.ap + "preferences-system-time.svg",  label: "Date & Time",     idx: 31 },
                            ]
                            currentIdx: Navigator.currentIdx
                            onItemClicked: (idx) => Navigator.currentIdx = idx
                        }
                    }
                }
            }
        }

        // ── RIGHT AREA ──────────────────────────────────────────────
        Item {
            anchors.left: sidenav.right
            anchors.leftMargin: 0
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.bottom: parent.bottom

            // Navigation bar
            Item {
                id: navBar
                height: 50
                anchors.left: parent.left; anchors.right: parent.right; anchors.top: parent.top

                MouseArea {
                    anchors.fill: parent
                    onPressed: window.startSystemMove()
                }

                // Back/Forward pill
                Rectangle {
                    x: 16; y: 10
                    width: 76; height: 30
                    radius: 15
                    color: Qt.rgba(1,1,1,0.12)
                    border.color: Qt.rgba(1,1,1,0.2); border.width: 1

                    Row {
                        anchors.centerIn: parent; spacing: 0

                        Rectangle {
                            width: 36; height: 28; radius: 14
                            color: backArea.containsMouse ? Qt.rgba(0,0,0,0.08) : "transparent"
                            MouseArea {
                                id: backArea; anchors.fill: parent; hoverEnabled: true
                                onClicked: if (Navigator.currentIdx > 0) Navigator.currentIdx--
                                cursorShape: Qt.PointingHandCursor
                            }
                            Text {
                                anchors.centerIn: parent; text: "‹"; font.pixelSize: 20
                                color: Navigator.currentIdx > 0 ? Theme.textPrimary : Theme.textTertiary
                            }
                        }
                        Text {
                            text: "|"; color: Theme.divider; font.pixelSize: 18
                            anchors.verticalCenter: parent.verticalCenter
                        }
                        Rectangle {
                            width: 36; height: 28; radius: 14
                            color: fwdArea.containsMouse ? Qt.rgba(0,0,0,0.08) : "transparent"
                            MouseArea {
                                id: fwdArea; anchors.fill: parent; hoverEnabled: true
                                onClicked: if (Navigator.currentIdx < 32) Navigator.currentIdx++
                                cursorShape: Qt.PointingHandCursor
                            }
                            Text {
                                anchors.centerIn: parent; text: "›"; font.pixelSize: 20
                                color: Navigator.currentIdx < 32 ? Theme.textPrimary : Theme.textTertiary
                            }
                        }
                    }
                }

                Text {
                    anchors.centerIn: parent
                    text: pageTitles[Navigator.currentIdx] || "Settings"
                    font.pixelSize: 17; font.weight: Font.DemiBold
                    color: Theme.textPrimary
                }
            }

            // Content stack — pages are lazily loaded via Loader so only the
            // active page fires Component.onCompleted, eliminating the startup blackscreen.
            StackLayout {
                id: stack
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: navBar.bottom
                anchors.bottom: parent.bottom
                currentIndex: Navigator.currentIdx

                Loader { Layout.fillWidth:true; Layout.fillHeight:true; active: Navigator.currentIdx===0;  sourceComponent: Component { WiFiPage {} } }
                Loader { Layout.fillWidth:true; Layout.fillHeight:true; active: Navigator.currentIdx===1;  sourceComponent: Component { BluetoothPage {} } }
                Loader { Layout.fillWidth:true; Layout.fillHeight:true; active: Navigator.currentIdx===2;  sourceComponent: Component { NetworkPage {} } }
                Loader { Layout.fillWidth:true; Layout.fillHeight:true; active: Navigator.currentIdx===3;  sourceComponent: Component { BatteryPage {} } }
                Loader { Layout.fillWidth:true; Layout.fillHeight:true; active: Navigator.currentIdx===4;  sourceComponent: Component { GeneralPage {} } }
                StubPage { stubTitle: "Accessibility" }
                Loader { Layout.fillWidth:true; Layout.fillHeight:true; active: Navigator.currentIdx===6;  sourceComponent: Component { AppearancePage {} } }
                StubPage { stubTitle: "Menu Bar" }
                Loader { Layout.fillWidth:true; Layout.fillHeight:true; active: Navigator.currentIdx===8;  sourceComponent: Component { PearIntelligencePage {} } }
                Loader { Layout.fillWidth:true; Layout.fillHeight:true; active: Navigator.currentIdx===9;  sourceComponent: Component { DesktopDockPage {} } }
                Loader { Layout.fillWidth:true; Layout.fillHeight:true; active: Navigator.currentIdx===10; sourceComponent: Component { DisplaysPage {} } }
                Loader { Layout.fillWidth:true; Layout.fillHeight:true; active: Navigator.currentIdx===11; sourceComponent: Component { WallpaperPage {} } }
                StubPage { stubTitle: "Spotlight" }
                Loader { Layout.fillWidth:true; Layout.fillHeight:true; active: Navigator.currentIdx===13; sourceComponent: Component { NotificationsPage {} } }
                Loader { Layout.fillWidth:true; Layout.fillHeight:true; active: Navigator.currentIdx===14; sourceComponent: Component { SoundPage {} } }
                StubPage { stubTitle: "Focus" }
                StubPage { stubTitle: "Screen Time" }
                Loader { Layout.fillWidth:true; Layout.fillHeight:true; active: Navigator.currentIdx===17; sourceComponent: Component { LockScreenPage {} } }
                Loader { Layout.fillWidth:true; Layout.fillHeight:true; active: Navigator.currentIdx===18; sourceComponent: Component { PrivacyPage {} } }
                StubPage { stubTitle: "Touch ID & Password" }
                Loader { Layout.fillWidth:true; Layout.fillHeight:true; active: Navigator.currentIdx===20; sourceComponent: Component { UsersPage {} } }
                StubPage { stubTitle: "Internet Accounts" }
                StubPage { stubTitle: "Game Center" }
                StubPage { stubTitle: "pCloud" }
                StubPage { stubTitle: "Wallet & Pear Pay" }
                Loader { Layout.fillWidth:true; Layout.fillHeight:true; active: Navigator.currentIdx===25; sourceComponent: Component { KeyboardPage {} } }
                Loader { Layout.fillWidth:true; Layout.fillHeight:true; active: Navigator.currentIdx===26; sourceComponent: Component { TrackpadPage {} } }
                StubPage { stubTitle: "Printers & Scanners" }
                Loader { Layout.fillWidth:true; Layout.fillHeight:true; active: Navigator.currentIdx===28; sourceComponent: Component { AboutPage {} } }
                Loader { Layout.fillWidth:true; Layout.fillHeight:true; active: Navigator.currentIdx===29; sourceComponent: Component { SoftwareUpdatePage {} } }
                Loader { Layout.fillWidth:true; Layout.fillHeight:true; active: Navigator.currentIdx===30; sourceComponent: Component { StoragePage {} } }
                Loader { Layout.fillWidth:true; Layout.fillHeight:true; active: Navigator.currentIdx===31; sourceComponent: Component { DateTimePage {} } }
                Loader { Layout.fillWidth:true; Layout.fillHeight:true; active: Navigator.currentIdx===32; sourceComponent: Component { PearIDPage {} } }
            }
        }

        // Window resize drag
        MouseArea {
            width: 16; height: 16
            anchors.right: parent.right; anchors.bottom: parent.bottom
            cursorShape: Qt.SizeFDiagCursor
            onPressed: window.startSystemResize(Qt.BottomEdge | Qt.RightEdge)
        }
    }
}
