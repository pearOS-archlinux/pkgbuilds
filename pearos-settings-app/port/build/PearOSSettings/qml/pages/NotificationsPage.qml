import QtQuick
import QtQuick.Layouts
import "../components"

PageBase {
    title: "Notifications"
    readonly property string ap: "file:///home/alxb421/Desktop/pkgbuilds/pearos-settings-app/port/assets/"

    SettingsCard {
        Column {
            width: parent.width; spacing: 20; topPadding: 40; bottomPadding: 40
            anchors.horizontalCenter: parent.horizontalCenter

            Image {
                source: ap + "cs-notifications.svg"
                width: 80; height: 80; fillMode: Image.PreserveAspectFit; opacity: 0.8
                anchors.horizontalCenter: parent.horizontalCenter
            }
            Text {
                text: "Feature not available"
                font.pixelSize: 22; font.weight: Font.DemiBold; color: Theme.textPrimary
                anchors.horizontalCenter: parent.horizontalCenter
            }
            Text {
                text: "This feature is currently not available. Please check back later for updates."
                font.pixelSize: 14; color: Theme.textSecondary; wrapMode: Text.WordWrap
                width: parent.width; horizontalAlignment: Text.AlignHCenter
            }
        }
    }
    Spacer {}
}
