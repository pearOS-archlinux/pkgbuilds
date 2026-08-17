import QtQuick 2.15
import QtQuick.Controls 2.15

import org.kde.kirigami 2.3 as Kirigami
import org.kde.plasma.plasmoid 2.0
import org.kde.kcmutils as KCM

KCM.SimpleKCM {
    id: configGeneral

    property alias cfg_iconSize: iconSize.value

  Kirigami.FormLayout {

    anchors.left: parent.left
    anchors.right: parent.right

    SpinBox {
        id: iconSize
        Kirigami.FormData.label: i18n("Icon size:")
        from: 50
        to: 200
        value: Plasmoid.configuration.iconSize
        stepSize: 1
        editable: true
    }
  }
}

