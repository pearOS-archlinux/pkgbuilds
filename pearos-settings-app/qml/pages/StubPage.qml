import QtQuick
import QtQuick.Layouts
import "../components"

PageBase {
    property string stubTitle: ""

    SettingsCard {
        Column {
            width: parent.width; spacing: 16; topPadding: 32; bottomPadding: 32

            Text {
                text: "Feature not available"
                font.pixelSize: 18; font.weight: Font.DemiBold; color: Theme.textPrimary
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
