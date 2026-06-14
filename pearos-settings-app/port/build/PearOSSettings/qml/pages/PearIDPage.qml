import QtQuick
import QtQuick.Controls
import "../components"

PageBase {
    title: "Pear ID"

    property string emailInput_: ""
    property string passwordInput_: ""
    property bool   busy: false
    property string errorMsg: ""

    Component.onCompleted: {
        PearID.checkState()
        PearID.loginResult.connect(function(success, message) {
            busy = false
            errorMsg = success ? "" : message
        })
    }

    // Loading indicator
    Item {
        width: parent.width; height: 80
        visible: PearID.state === "loading"
        BusyIndicator { anchors.centerIn: parent; running: true }
    }

    // --- LOGGED OUT: sign-in form ---
    SettingsCard {
        visible: PearID.state === "loggedout"
        Column {
            width: parent.width; spacing: 0; topPadding: 20; bottomPadding: 20

            Image {
                source: "file:///home/alxb421/Desktop/pkgbuilds/pearos-settings-app/port/assets/pearid.png"
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

            // Email field
            Rectangle {
                width: parent.width; height: 38; radius: 8
                color: Theme.bgSearch
                TextInput {
                    anchors.fill: parent; anchors.leftMargin: 12; anchors.rightMargin: 12
                    verticalAlignment: TextInput.AlignVCenter
                    font.pixelSize: 14; color: Theme.textPrimary; clip: true
                    inputMethodHints: Qt.ImhEmailCharactersOnly
                    onTextChanged: emailInput_ = text
                    Text {
                        anchors.fill: parent
                        text: "Email"; color: Theme.textTertiary; font.pixelSize: 14
                        visible: parent.text.length === 0
                        verticalAlignment: Text.AlignVCenter
                    }
                }
            }
            Item { width: 1; height: 8 }

            // Password field
            Rectangle {
                width: parent.width; height: 38; radius: 8
                color: Theme.bgSearch
                TextInput {
                    anchors.fill: parent; anchors.leftMargin: 12; anchors.rightMargin: 12
                    verticalAlignment: TextInput.AlignVCenter
                    font.pixelSize: 14; color: Theme.textPrimary; clip: true
                    echoMode: TextInput.Password
                    onTextChanged: passwordInput_ = text
                    Keys.onReturnPressed: if (!busy) { busy = true; PearID.login(emailInput_, passwordInput_) }
                    Text {
                        anchors.fill: parent
                        text: "Password"; color: Theme.textTertiary; font.pixelSize: 14
                        visible: parent.text.length === 0
                        verticalAlignment: Text.AlignVCenter
                    }
                }
            }
            Item { width: 1; height: 8 }

            Text {
                text: errorMsg; color: "#ff5f57"; font.pixelSize: 12
                anchors.horizontalCenter: parent.horizontalCenter
                visible: errorMsg.length > 0
            }
            Item { width: 1; height: 8; visible: errorMsg.length > 0 }

            // Sign in button
            Rectangle {
                width: 140; height: 36; radius: 18
                color: busy ? Qt.rgba(0.23, 0.51, 0.96, 0.5) : "#3B82F6"
                anchors.horizontalCenter: parent.horizontalCenter
                Text {
                    anchors.centerIn: parent
                    text: busy ? "Signing in…" : "Sign In"
                    font.pixelSize: 14; font.weight: Font.Medium; color: "white"
                }
                MouseArea {
                    anchors.fill: parent; cursorShape: Qt.PointingHandCursor
                    onClicked: if (!busy) { busy = true; PearID.login(emailInput_, passwordInput_) }
                }
            }
        }
    }

    Spacer {}

    // --- LOGGED IN: profile ---
    SettingsCard {
        visible: PearID.state === "loggedin"
        Column {
            width: parent.width; spacing: 0; topPadding: 20; bottomPadding: 20

            Rectangle {
                width: 64; height: 64; radius: 32
                color: Theme.bgSearch
                anchors.horizontalCenter: parent.horizontalCenter
                clip: true
                Image {
                    anchors.fill: parent
                    source: "file:///home/alxb421/Desktop/pkgbuilds/pearos-settings-app/port/assets/user.png"
                    fillMode: Image.PreserveAspectCrop
                }
            }
            Item { width: 1; height: 10 }
            Text {
                text: PearID.userName || "Pear ID User"
                font.pixelSize: 17; font.weight: Font.DemiBold; color: Theme.textPrimary
                anchors.horizontalCenter: parent.horizontalCenter
            }
            Text {
                text: PearID.userEmail
                font.pixelSize: 13; color: Theme.textSecondary
                anchors.horizontalCenter: parent.horizontalCenter
                visible: PearID.userEmail.length > 0
            }
        }
    }

    Spacer {}

    SettingsCard {
        visible: PearID.state === "loggedin"
        Column {
            width: parent.width; spacing: 0
            Item {
                width: parent.width; height: 44
                Text {
                    anchors.left: parent.left; anchors.verticalCenter: parent.verticalCenter
                    text: "Sign Out"; font.pixelSize: 14; color: "#ff5f57"
                }
                MouseArea {
                    anchors.fill: parent; cursorShape: Qt.PointingHandCursor
                    onClicked: PearID.logout()
                }
            }
        }
    }
}
