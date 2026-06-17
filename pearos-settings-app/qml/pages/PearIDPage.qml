import QtQuick
import QtQuick.Controls
import Qt5Compat.GraphicalEffects
import "../components"

PageBase {
    title: "Pear ID"

    property string emailInput_: ""
    property string passwordInput_: ""
    property bool   busy: false
    property string errorMsg: ""

    property int subPage: 0

    property bool   editDialogOpen: false
    property string editDialogTitle: ""
    property string editDialogField: ""
    property string editDialogValue: ""
    property string editDialogField2: ""
    property string editDialogValue2: ""

    Component.onCompleted: {
        PearID.loginResult.connect(function(success, message) {
            busy = false
            if (success) { errorMsg = ""; subPage = 0 }
            else errorMsg = message
        })
        PearID.updateResult.connect(function(field, success, message) {
            editDialogOpen = false
        })
    }

    // ─── LOADING ──────────────────────────────────────────────────
    Item {
        width: parent.width; height: 80
        visible: PearID.state === "loading"
        BusyIndicator { anchors.centerIn: parent; running: true }
    }

    // ─── LOGGED OUT ───────────────────────────────────────────────
    SettingsCard {
        visible: PearID.state === "loggedout"
        Column {
            width: parent.width; spacing: 0; topPadding: 20; bottomPadding: 20

            Image {
                source: window.ap + "pearid.png"
                width: 64; height: 64; fillMode: Image.PreserveAspectFit
                anchors.horizontalCenter: parent.horizontalCenter
            }
            Item { width: 1; height: 10 }
            Text {
                text: "Sign in with Pear ID"
                font.pixelSize: 17; font.weight: Font.DemiBold; color: Theme.textPrimary
                anchors.horizontalCenter: parent.horizontalCenter
            }
            Item { width: 1; height: 4 }
            Text {
                text: "Use your Pear ID to sync apps, settings, and more."
                font.pixelSize: 12; color: Theme.textSecondary; wrapMode: Text.WordWrap
                width: parent.width; horizontalAlignment: Text.AlignHCenter
            }
            Item { width: 1; height: 16 }

            Rectangle {
                width: parent.width; height: 38; radius: 8; color: Theme.bgSearch
                TextInput {
                    anchors.fill: parent; anchors.leftMargin: 12; anchors.rightMargin: 12
                    verticalAlignment: TextInput.AlignVCenter
                    font.pixelSize: 14; color: Theme.textPrimary; clip: true
                    inputMethodHints: Qt.ImhEmailCharactersOnly
                    onTextChanged: emailInput_ = text
                    Text { anchors.fill: parent; text: "Email"; color: Theme.textTertiary; font.pixelSize: 14; visible: parent.text.length === 0; verticalAlignment: Text.AlignVCenter }
                }
            }
            Item { width: 1; height: 8 }
            Rectangle {
                width: parent.width; height: 38; radius: 8; color: Theme.bgSearch
                TextInput {
                    anchors.fill: parent; anchors.leftMargin: 12; anchors.rightMargin: 12
                    verticalAlignment: TextInput.AlignVCenter
                    font.pixelSize: 14; color: Theme.textPrimary; clip: true
                    echoMode: TextInput.Password
                    onTextChanged: passwordInput_ = text
                    Keys.onReturnPressed: if (!busy) { busy = true; PearID.login(emailInput_, passwordInput_) }
                    Text { anchors.fill: parent; text: "Password"; color: Theme.textTertiary; font.pixelSize: 14; visible: parent.text.length === 0; verticalAlignment: Text.AlignVCenter }
                }
            }
            Item { width: 1; height: 8 }
            Text { text: errorMsg; color: "#ff5f57"; font.pixelSize: 12; anchors.horizontalCenter: parent.horizontalCenter; visible: errorMsg.length > 0 }
            Item { width: 1; height: 8; visible: errorMsg.length > 0 }
            Rectangle {
                width: 140; height: 36; radius: 18
                color: busy ? Qt.rgba(Theme.accent.r, Theme.accent.g, Theme.accent.b, 0.5) : Theme.accent
                anchors.horizontalCenter: parent.horizontalCenter
                Text { anchors.centerIn: parent; text: busy ? "Signing in…" : "Sign In"; font.pixelSize: 14; font.weight: Font.Medium; color: "white" }
                MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor; onClicked: if (!busy) { busy = true; PearID.login(emailInput_, passwordInput_) } }
            }
        }
    }

    Spacer { visible: PearID.state === "loggedout" }

    // ─── LOGGED IN ────────────────────────────────────────────────

    // Back header for sub-pages
    Item {
        visible: PearID.state === "loggedin" && subPage > 0
        width: parent.width; height: 36
        MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor; onClicked: subPage = 0 }
        Row {
            anchors.verticalCenter: parent.verticalCenter; spacing: 4
            Text { text: "‹"; font.pixelSize: 20; color: Theme.accent }
            Text { text: "Pear ID"; font.pixelSize: 14; color: Theme.accent }
        }
    }

    // ══════════════════════════════════════
    // SUB-PAGE 0 — USER PROFILE
    // ══════════════════════════════════════
    Item { visible: PearID.state === "loggedin" && subPage === 0; width: 1; height: 20 }

    Image {
        visible: PearID.state === "loggedin" && subPage === 0
        width: 120; height: 120
        anchors.horizontalCenter: parent.horizontalCenter
        source: PearID.avatarPath.length > 0 ? "file://" + PearID.avatarPath : window.ap + "user.png"
        fillMode: Image.PreserveAspectCrop
        layer.enabled: true
        layer.effect: OpacityMask {
            maskSource: Rectangle { width: 120; height: 120; radius: 60 }
        }
    }

    Item { visible: PearID.state === "loggedin" && subPage === 0; width: 1; height: 5 }

    Text {
        visible: PearID.state === "loggedin" && subPage === 0
        text: PearID.userName || "Pear ID User"
        font.pixelSize: 24; font.weight: Font.DemiBold; color: Theme.textPrimary
        anchors.horizontalCenter: parent.horizontalCenter
    }
    Text {
        visible: PearID.state === "loggedin" && subPage === 0 && PearID.userEmail.length > 0
        text: PearID.userEmail
        font.pixelSize: 14; color: Theme.textSecondary
        anchors.horizontalCenter: parent.horizontalCenter
    }

    Spacer { visible: PearID.state === "loggedin" && subPage === 0; height: 10 }

    // Card 1: Personal Information, Sign In & Security, Payment & Shipping
    SettingsCard {
        visible: PearID.state === "loggedin" && subPage === 0
        Column {
            width: parent.width; spacing: 0
            Repeater {
                model: [
                    { label: "Personal Information", icon: "preferences-desktop-user.svg",          page: 1 },
                    { label: "Sign In & Security",   icon: "preferences-desktop-user-password.svg", page: 2 },
                    { label: "Payment & Shipping",   icon: "preferences-desktop-online-accounts.svg", page: 3 },
                ]
                delegate: Item {
                    width: parent.width; height: 44
                    Rectangle { visible: index > 0; width: parent.width; height: 1; color: Theme.border; opacity: 0.5 }
                    Row {
                        anchors.verticalCenter: parent.verticalCenter; spacing: 10
                        Image {
                            source: window.ap + modelData.icon
                            width: 22; height: 22; fillMode: Image.PreserveAspectFit
                            anchors.verticalCenter: parent.verticalCenter
                        }
                        Text { text: modelData.label; font.pixelSize: 14; color: Theme.textPrimary; anchors.verticalCenter: parent.verticalCenter }
                    }
                    Text { text: "›"; font.pixelSize: 18; color: Theme.textTertiary; anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter }
                    MouseArea {
                        anchors.fill: parent; cursorShape: Qt.PointingHandCursor
                        onClicked: { subPage = modelData.page; PearID.fetchExtendedInfo() }
                    }
                }
            }
        }
    }

    Spacer { visible: PearID.state === "loggedin" && subPage === 0; height: 10 }

    // Card 2: Pear Cloud, Media & Purchases, Sign in with Pear
    SettingsCard {
        visible: PearID.state === "loggedin" && subPage === 0
        Column {
            width: parent.width; spacing: 0
            Repeater {
                model: [
                    { label: "Pear Cloud",         icon: "preferences-desktop-online-accounts.svg", page: 4 },
                    { label: "Media & Purchases",  icon: "preferences-other.svg",                   page: 5 },
                    { label: "Sign in with Pear",  icon: "cs-user.svg",                             page: -1 },
                ]
                delegate: Item {
                    width: parent.width; height: 44
                    Rectangle { visible: index > 0; width: parent.width; height: 1; color: Theme.border; opacity: 0.5 }
                    Row {
                        anchors.verticalCenter: parent.verticalCenter; spacing: 10
                        Image {
                            source: window.ap + modelData.icon
                            width: 22; height: 22; fillMode: Image.PreserveAspectFit
                            anchors.verticalCenter: parent.verticalCenter
                        }
                        Text { text: modelData.label; font.pixelSize: 14; color: Theme.textPrimary; anchors.verticalCenter: parent.verticalCenter }
                    }
                    Text { text: "›"; font.pixelSize: 18; color: Theme.textTertiary; anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter }
                    MouseArea {
                        anchors.fill: parent; cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            if (modelData.page === 5) { subPage = 5; PearID.fetchApps() }
                            else if (modelData.page > 0) subPage = modelData.page
                        }
                    }
                }
            }
        }
    }

    // Devices section header
    Text {
        visible: PearID.state === "loggedin" && subPage === 0
        text: "Devices"
        font.pixelSize: 13; font.weight: Font.Medium; color: Theme.textSecondary
        topPadding: 20
    }

    SettingsCard {
        visible: PearID.state === "loggedin" && subPage === 0
        Component.onCompleted: if (PearID.state === "loggedin") PearID.fetchDevices()

        Column {
            width: parent.width; spacing: 0
            Repeater {
                model: PearID.devices
                delegate: Item {
                    width: parent.width; height: 44
                    Rectangle { visible: index > 0; width: parent.width; height: 1; color: Theme.border; opacity: 0.5 }
                    Row {
                        anchors.verticalCenter: parent.verticalCenter; spacing: 10
                        Image { source: window.ap + "cs-user-accounts.svg"; width: 22; height: 22; fillMode: Image.PreserveAspectFit; anchors.verticalCenter: parent.verticalCenter }
                        Text { text: modelData.name || "Unknown"; font.pixelSize: 14; color: Theme.textPrimary; anchors.verticalCenter: parent.verticalCenter }
                    }
                    Text { text: "›"; font.pixelSize: 18; color: Theme.textTertiary; anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter }
                }
            }
            Item {
                visible: PearID.devices.length === 0
                width: parent.width; height: 44
                Text { text: "Loading devices…"; font.pixelSize: 14; color: Theme.textTertiary; anchors.centerIn: parent }
            }
        }
    }

    Spacer { visible: PearID.state === "loggedin" && subPage === 0; height: 10 }

    SettingsCard {
        visible: PearID.state === "loggedin" && subPage === 0
        Item {
            width: parent.width; height: 44
            Text { text: "Sign Out…"; font.pixelSize: 14; color: "#ff5f57"; anchors.verticalCenter: parent.verticalCenter }
            MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor; onClicked: PearID.logout() }
        }
    }

    // ══════════════════════════════════════
    // SUB-PAGE 1 — PERSONAL INFORMATION
    // ══════════════════════════════════════
    Text { visible: PearID.state === "loggedin" && subPage === 1; text: "Personal Information"; font.pixelSize: 20; font.weight: Font.DemiBold; color: Theme.textPrimary; topPadding: 8 }
    Spacer { visible: PearID.state === "loggedin" && subPage === 1; height: 10 }
    SettingsCard {
        visible: PearID.state === "loggedin" && subPage === 1
        Column {
            width: parent.width; spacing: 0
            Item {
                width: parent.width; height: 44
                Text { text: "Name"; font.pixelSize: 14; color: Theme.textSecondary; anchors.verticalCenter: parent.verticalCenter }
                Row {
                    anchors.right: chevron_pi1.left; anchors.rightMargin: 8; anchors.verticalCenter: parent.verticalCenter; spacing: 4
                    Text { text: PearID.userName || "—"; font.pixelSize: 14; color: Theme.textPrimary }
                }
                Text { id: chevron_pi1; text: "›"; font.pixelSize: 18; color: Theme.textTertiary; anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter }
                MouseArea {
                    anchors.fill: parent; cursorShape: Qt.PointingHandCursor
                    onClicked: {
                        var parts = PearID.userName.split(" ")
                        editDialogTitle = "Edit Name"
                        editDialogField = "firstName";   editDialogValue  = parts[0] || ""
                        editDialogField2 = "lastName";   editDialogValue2 = parts.slice(1).join(" ")
                        editDialogOpen = true
                    }
                }
            }
            Rectangle { width: parent.width; height: 1; color: Theme.border; opacity: 0.5 }
            Item {
                width: parent.width; height: 44
                Text { text: "Birthday"; font.pixelSize: 14; color: Theme.textSecondary; anchors.verticalCenter: parent.verticalCenter }
                Text { text: PearID.birthdate || "—"; font.pixelSize: 14; color: Theme.textPrimary; anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter }
            }
        }
    }

    // ══════════════════════════════════════
    // SUB-PAGE 2 — SIGN IN & SECURITY
    // ══════════════════════════════════════
    Text { visible: PearID.state === "loggedin" && subPage === 2; text: "Sign In & Security"; font.pixelSize: 20; font.weight: Font.DemiBold; color: Theme.textPrimary; topPadding: 8 }
    Spacer { visible: PearID.state === "loggedin" && subPage === 2; height: 10 }
    SettingsCard {
        visible: PearID.state === "loggedin" && subPage === 2
        Column {
            width: parent.width; spacing: 0
            Item {
                width: parent.width; height: 44
                Text { text: "Email"; font.pixelSize: 14; color: Theme.textSecondary; anchors.verticalCenter: parent.verticalCenter }
                Text { text: PearID.userEmail || "—"; font.pixelSize: 14; color: Theme.textPrimary; anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter }
            }
            Rectangle { width: parent.width; height: 1; color: Theme.border; opacity: 0.5 }
            Item {
                width: parent.width; height: 44
                Text { text: "Phone"; font.pixelSize: 14; color: Theme.textSecondary; anchors.verticalCenter: parent.verticalCenter }
                Text {
                    text: PearID.phone || "Add phone number"
                    font.pixelSize: 14; color: PearID.phone.length > 0 ? Theme.textPrimary : Theme.accent
                    anchors.right: chevron_sis.left; anchors.rightMargin: 8; anchors.verticalCenter: parent.verticalCenter
                }
                Text { id: chevron_sis; text: "›"; font.pixelSize: 18; color: Theme.textTertiary; anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter }
                MouseArea {
                    anchors.fill: parent; cursorShape: Qt.PointingHandCursor
                    onClicked: { editDialogTitle = "Phone Number"; editDialogField = "phone"; editDialogValue = PearID.phone; editDialogField2 = ""; editDialogValue2 = ""; editDialogOpen = true }
                }
            }
            Rectangle { width: parent.width; height: 1; color: Theme.border; opacity: 0.5 }
            Item {
                width: parent.width; height: 44
                Text { text: "Change Password"; font.pixelSize: 14; color: Theme.accent; anchors.verticalCenter: parent.verticalCenter }
                MouseArea {
                    anchors.fill: parent; cursorShape: Qt.PointingHandCursor
                    onClicked: { editDialogTitle = "Change Password"; editDialogField = "oldPassword"; editDialogValue = ""; editDialogField2 = "newPassword"; editDialogValue2 = ""; editDialogOpen = true }
                }
            }
        }
    }

    // ══════════════════════════════════════
    // SUB-PAGE 3 — PAYMENT & SHIPPING
    // ══════════════════════════════════════
    Text { visible: PearID.state === "loggedin" && subPage === 3; text: "Payment & Shipping"; font.pixelSize: 20; font.weight: Font.DemiBold; color: Theme.textPrimary; topPadding: 8 }
    Spacer { visible: PearID.state === "loggedin" && subPage === 3; height: 10 }
    SettingsCard {
        visible: PearID.state === "loggedin" && subPage === 3
        Column {
            width: parent.width; spacing: 0
            Item {
                width: parent.width; height: 44
                Text { text: "Billing Address"; font.pixelSize: 14; color: Theme.textSecondary; anchors.verticalCenter: parent.verticalCenter }
                Text {
                    text: PearID.billingAddress || "Add address"
                    font.pixelSize: 14; color: PearID.billingAddress.length > 0 ? Theme.textPrimary : Theme.accent
                    anchors.right: chevron_ps.left; anchors.rightMargin: 8; anchors.verticalCenter: parent.verticalCenter
                    elide: Text.ElideRight; width: 180
                }
                Text { id: chevron_ps; text: "›"; font.pixelSize: 18; color: Theme.textTertiary; anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter }
                MouseArea {
                    anchors.fill: parent; cursorShape: Qt.PointingHandCursor
                    onClicked: { editDialogTitle = "Billing Address"; editDialogField = "billing"; editDialogValue = PearID.billingAddress; editDialogField2 = ""; editDialogValue2 = ""; editDialogOpen = true }
                }
            }
        }
    }

    // ══════════════════════════════════════
    // SUB-PAGE 4 — PEAR CLOUD
    // ══════════════════════════════════════
    Text { visible: PearID.state === "loggedin" && subPage === 4; text: "Pear Cloud"; font.pixelSize: 20; font.weight: Font.DemiBold; color: Theme.textPrimary; topPadding: 8 }
    Spacer { visible: PearID.state === "loggedin" && subPage === 4; height: 10 }
    SettingsCard {
        visible: PearID.state === "loggedin" && subPage === 4
        Item {
            width: parent.width; height: 100
            Column {
                anchors.centerIn: parent; spacing: 8
                Image { source: window.ap + "preferences-desktop-online-accounts.svg"; width: 40; height: 40; fillMode: Image.PreserveAspectFit; anchors.horizontalCenter: parent.horizontalCenter }
                Text {
                    text: "Pear Cloud syncs your data across all your devices."
                    font.pixelSize: 13; color: Theme.textSecondary; wrapMode: Text.WordWrap
                    width: 300; horizontalAlignment: Text.AlignHCenter; anchors.horizontalCenter: parent.horizontalCenter
                }
            }
        }
    }

    // ══════════════════════════════════════
    // SUB-PAGE 5 — MEDIA & PURCHASES
    // ══════════════════════════════════════
    Text { visible: PearID.state === "loggedin" && subPage === 5; text: "Media & Purchases"; font.pixelSize: 20; font.weight: Font.DemiBold; color: Theme.textPrimary; topPadding: 8 }
    Spacer { visible: PearID.state === "loggedin" && subPage === 5; height: 10 }
    SettingsCard {
        visible: PearID.state === "loggedin" && subPage === 5
        Column {
            width: parent.width; spacing: 0
            Item {
                width: parent.width; height: 44
                Text { text: "Account"; font.pixelSize: 14; color: Theme.textSecondary; anchors.verticalCenter: parent.verticalCenter }
                Text { text: PearID.userEmail || "—"; font.pixelSize: 14; color: Theme.textPrimary; anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter }
            }
            Repeater {
                model: PearID.apps
                delegate: Item {
                    width: parent.width; height: 44
                    Rectangle { width: parent.width; height: 1; color: Theme.border; opacity: 0.5 }
                    Text { text: modelData.name || "—"; font.pixelSize: 14; color: Theme.textPrimary; anchors.verticalCenter: parent.verticalCenter }
                }
            }
            Item {
                visible: PearID.apps.length === 0
                width: parent.width; height: 44
                Rectangle { width: parent.width; height: 1; color: Theme.border; opacity: 0.5 }
                Text { text: "No apps found"; font.pixelSize: 14; color: Theme.textTertiary; anchors.verticalCenter: parent.verticalCenter }
            }
        }
    }

    // ═══════════════════════════════════════════════════════════════
    // EDIT DIALOG
    // ═══════════════════════════════════════════════════════════════
    overlay: Rectangle {
        visible: editDialogOpen
        anchors.fill: parent
        color: Qt.rgba(0, 0, 0, 0.5)

        MouseArea { anchors.fill: parent; onClicked: editDialogOpen = false }

        Rectangle {
            width: Math.min(parent.width - 48, 360)
            height: dlgCol.implicitHeight + 40
            anchors.centerIn: parent
            color: Theme.bgSidenav
            radius: 12
            MouseArea { anchors.fill: parent }

            Column {
                id: dlgCol
                anchors { top: parent.top; left: parent.left; right: parent.right; topMargin: 20; leftMargin: 20; rightMargin: 20 }
                spacing: 12

                Text { text: editDialogTitle; font.pixelSize: 16; font.weight: Font.DemiBold; color: Theme.textPrimary; anchors.horizontalCenter: parent.horizontalCenter }

                Rectangle {
                    width: parent.width; height: 38; radius: 8; color: Theme.bgSearch
                    TextInput {
                        id: dlgInput1
                        anchors.fill: parent; anchors.leftMargin: 12; anchors.rightMargin: 12
                        verticalAlignment: TextInput.AlignVCenter
                        font.pixelSize: 14; color: Theme.textPrimary; clip: true
                        echoMode: editDialogField === "oldPassword" ? TextInput.Password : TextInput.Normal
                        text: editDialogValue
                        onTextChanged: editDialogValue = text
                        Text {
                            anchors.fill: parent; font.pixelSize: 14; color: Theme.textTertiary; verticalAlignment: Text.AlignVCenter
                            visible: parent.text.length === 0
                            text: {
                                if (editDialogField === "firstName")   return "First name"
                                if (editDialogField === "phone")       return "Phone number"
                                if (editDialogField === "billing")     return "Billing address"
                                if (editDialogField === "oldPassword") return "Current password"
                                return editDialogTitle
                            }
                        }
                    }
                }

                Rectangle {
                    visible: editDialogField2.length > 0
                    width: parent.width; height: 38; radius: 8; color: Theme.bgSearch
                    TextInput {
                        id: dlgInput2
                        anchors.fill: parent; anchors.leftMargin: 12; anchors.rightMargin: 12
                        verticalAlignment: TextInput.AlignVCenter
                        font.pixelSize: 14; color: Theme.textPrimary; clip: true
                        echoMode: editDialogField2 === "newPassword" ? TextInput.Password : TextInput.Normal
                        text: editDialogValue2
                        onTextChanged: editDialogValue2 = text
                        Text {
                            anchors.fill: parent; font.pixelSize: 14; color: Theme.textTertiary; verticalAlignment: Text.AlignVCenter
                            visible: parent.text.length === 0
                            text: {
                                if (editDialogField2 === "lastName")    return "Last name"
                                if (editDialogField2 === "newPassword") return "New password"
                                return ""
                            }
                        }
                    }
                }

                Row {
                    anchors.horizontalCenter: parent.horizontalCenter; spacing: 12
                    Rectangle {
                        width: 100; height: 34; radius: 17; color: Theme.bgSearch
                        Text { anchors.centerIn: parent; text: "Cancel"; font.pixelSize: 14; color: Theme.textSecondary }
                        MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor; onClicked: editDialogOpen = false }
                    }
                    Rectangle {
                        width: 100; height: 34; radius: 17; color: Theme.accent
                        Text { anchors.centerIn: parent; text: "Save"; font.pixelSize: 14; font.weight: Font.Medium; color: "white" }
                        MouseArea {
                            anchors.fill: parent; cursorShape: Qt.PointingHandCursor
                            onClicked: {
                                if (editDialogField === "firstName")        PearID.updateName(editDialogValue, editDialogValue2)
                                else if (editDialogField === "phone")       PearID.updatePhone(editDialogValue)
                                else if (editDialogField === "billing")     PearID.updateBillingAddress(editDialogValue)
                                else if (editDialogField === "oldPassword") PearID.changePassword(editDialogValue, editDialogValue2)
                            }
                        }
                    }
                }
                Item { width: 1; height: 4 }
            }
        }
    }
}
