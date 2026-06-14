import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "../components"

PageBase {
    title: "Keyboard"
    Component.onCompleted: Keyboard.refresh()

    SettingsCard {
        Item {
            width: parent.width; height: 60
            Column {
                anchors.left: parent.left; anchors.verticalCenter: parent.verticalCenter; spacing: 3
                Text { text: "Key Repeat Delay"; font.pixelSize: 13; font.weight: Font.Medium; color: Theme.textPrimary }
                Text { text: "Time before a key starts repeating"; font.pixelSize: 12; color: Theme.textSecondary }
            }
            ComboBox {
                anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter
                model: ["Short", "Medium", "Long", "Very Long"]
                currentIndex: {
                    var d = Keyboard.repeatDelay
                    if (d <= 200) return 0
                    if (d <= 400) return 1
                    if (d <= 600) return 2
                    return 3
                }
                width: 120; height: 28; font.pixelSize: 12
                onActivated: {
                    var vals = [200, 400, 600, 800]
                    Keyboard.setRepeat(vals[currentIndex], Keyboard.repeatRate)
                }
            }
        }
    }
    Spacer {}

    SettingsCard {
        Item {
            width: parent.width; height: 60
            Column {
                anchors.left: parent.left; anchors.verticalCenter: parent.verticalCenter; spacing: 3
                Text { text: "Key Repeat Rate"; font.pixelSize: 13; font.weight: Font.Medium; color: Theme.textPrimary }
                Text { text: "How fast keys repeat when held down"; font.pixelSize: 12; color: Theme.textSecondary }
            }
            ComboBox {
                anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter
                model: ["Slow", "Medium", "Fast", "Very Fast"]
                currentIndex: {
                    var r = Keyboard.repeatRate
                    if (r <= 10) return 0
                    if (r <= 20) return 1
                    if (r <= 25) return 2
                    return 3
                }
                width: 120; height: 28; font.pixelSize: 12
                onActivated: {
                    var vals = [10, 20, 25, 30]
                    Keyboard.setRepeat(Keyboard.repeatDelay, vals[currentIndex])
                }
            }
        }
    }
    Spacer {}

    SettingsCard {
        Item {
            width: parent.width; height: 60
            Column {
                anchors.left: parent.left; anchors.verticalCenter: parent.verticalCenter; spacing: 3
                Text { text: "Keyboard Layout"; font.pixelSize: 13; font.weight: Font.Medium; color: Theme.textPrimary }
                Text { text: "Select your keyboard layout"; font.pixelSize: 12; color: Theme.textSecondary }
            }
            ComboBox {
                anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter
                model: Keyboard.layouts.length > 0 ? Keyboard.layouts : ["US", "UK", "German", "French", "Spanish", "Italian", "Romanian", "Russian"]
                currentIndex: Math.max(0, model.indexOf(Keyboard.layout))
                width: 150; height: 28; font.pixelSize: 12
                onActivated: Keyboard.setLayout(currentText)
            }
        }
    }
    Spacer {}
}
