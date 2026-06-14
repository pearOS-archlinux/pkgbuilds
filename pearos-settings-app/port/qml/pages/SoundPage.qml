import QtQuick
import QtQuick.Layouts
import "../components"

PageBase {
    title: "Sound"
    Component.onCompleted: { Audio.refreshVolumes(); Audio.refreshOutputs(); Audio.refreshInputs() }

    SectionTitle { text: "Output" }
    SettingsCard {
        SettingsSlider { labelText: "Volume"; value: Audio.outputVolume; unit: "%"; onMoved: v => Audio.setOutputVolume(Math.round(v)) }
        Rectangle { width: parent.width; height: 1; color: Theme.divider }
        Column {
            width: parent.width; spacing: 0
            Item { width: parent.width; height: 4 }
            Text { text: "Output Device"; font.pixelSize: 12; color: Theme.textSecondary; bottomPadding: 8 }
            Repeater {
                id: outRep; model: Audio.outputDevices
                delegate: Item {
                    width: parent.width; height: 36
                    Row {
                        anchors.left: parent.left; anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter
                        Text { text: modelData.description || modelData.name; font.pixelSize: 13; color: Theme.textPrimary; width: parent.width - 24; elide: Text.ElideRight }
                        Rectangle {
                            width: 16; height: 16; radius: 8
                            border.color: "#3B82F6"; border.width: 2
                            color: modelData.name === Audio.currentOutputName ? "#3B82F6" : "transparent"
                            anchors.verticalCenter: parent.verticalCenter
                            MouseArea { anchors.fill: parent; onClicked: Audio.setOutputDevice(modelData.index) }
                        }
                    }
                    Rectangle { anchors.bottom: parent.bottom; width: parent.width; height: 1; color: Theme.divider; visible: index < outRep.count - 1 }
                }
            }
        }
    }
    Spacer {}

    SectionTitle { text: "Input" }
    SettingsCard {
        SettingsSlider { labelText: "Input Volume"; value: Audio.inputVolume; unit: "%"; onMoved: v => Audio.setInputVolume(Math.round(v)) }
        Rectangle { width: parent.width; height: 1; color: Theme.divider }
        Column {
            width: parent.width; spacing: 0
            Text { text: "Input Device"; font.pixelSize: 12; color: Theme.textSecondary; bottomPadding: 8 }
            Repeater {
                id: inRep; model: Audio.inputDevices
                delegate: Item {
                    width: parent.width; height: 36
                    Row {
                        anchors.left: parent.left; anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter
                        Text { text: modelData.description || modelData.name; font.pixelSize: 13; color: Theme.textPrimary; width: parent.width - 24; elide: Text.ElideRight }
                        Rectangle {
                            width: 16; height: 16; radius: 8
                            border.color: "#3B82F6"; border.width: 2
                            color: modelData.name === Audio.currentInputName ? "#3B82F6" : "transparent"
                            anchors.verticalCenter: parent.verticalCenter
                            MouseArea { anchors.fill: parent; onClicked: Audio.setInputDevice(modelData.index) }
                        }
                    }
                    Rectangle { anchors.bottom: parent.bottom; width: parent.width; height: 1; color: Theme.divider; visible: index < inRep.count - 1 }
                }
            }
        }
    }
    Spacer {}
}
