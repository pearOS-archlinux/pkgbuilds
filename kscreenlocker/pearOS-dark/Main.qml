
import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import Qt5Compat.GraphicalEffects
//import QtQuick.Shapes 1.17
import org.kde.breeze.components
import "components"


Rectangle {
    width: 640
    height: 480
    LayoutMirroring.enabled: Qt.locale().textDirection === Qt.RightToLeft
    LayoutMirroring.childrenInherit: true
    property int sizeAvatar: 80
    property int longitudMasLarga: 0

    property int lastIndexUser: user.currentIndex
    property string lastNameUser: user.currentText
    property string displayNameUser: ""
    property string currentUserHomeDir: ""
    property int implicitCustomWidth: 0
    property bool firtInteraction: true
    property bool startAnimationNames: false
    TextConstants {
        id: textConstants
    }

    Component.onCompleted: {
        try {
            var last = userModel.lastIndex
            displayNameUser = userModel.get(last).realName || userModel.get(last).name || ""
            currentUserHomeDir = userModel.get(last).homeDir || ""
        } catch (e) {
            displayNameUser = lastNameUser
        }
        sectionLogin.createModel()
        if (listuser.visible)
            baseOfUserDialog.forceActiveFocus()
    }

    // hack for disable autostart QtQuick.VirtualKeyboard

    function determinateNewIndex() {
        console.log(lastNameUser, lastIndexUser)
        for (var j = 0; j < qmlUserModel.count; j++) {
            if (qmlUserModel.get(j).name === lastNameUser) {
                return j
                break
            }
        }
    }

    FontLoader {
        id: fontbold
        source: "fonts/SFUIText-Semibold.otf"
    }

    Loader {
        id: inputPanel
        property bool keyboardActive: false
        source: "components/VirtualKeyboard.qml"
    }
    Connections {
        target: sddm
        function onLoginSucceeded() {
        }
        function onLoginFailed() {
            password.placeholderText = textConstants.loginFailed
            password.placeholderTextColor = "white"
            password.text = ""
            password.focus = false
            errorMsgContainer.visible = true
        }
    }

    Image {
        id: wallpaper
        anchors.fill: parent
        fillMode: Image.PreserveAspectCrop
        visible: true
        source: "file:///usr/share/extras/background.jpg"
        onStatusChanged: {
            if (status === Image.Error && source === "file:///usr/share/extras/background.jpg")
                source = "file:///usr/share/extras/background.png"
        }
    }

    Row {
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.rightMargin: 40
        anchors.topMargin: 15

        Item {

            Image {
                id: shutdown
                height: 22
                width: 22
                source: "images/system-shutdown.svg"
                fillMode: Image.PreserveAspectFit

                MouseArea {
                    anchors.fill: parent
                    hoverEnabled: true
                    onEntered: {
                        shutdown.source = "images/system-shutdown-hover.svg"
                        var component = Qt.createComponent(
                                    "components/ShutdownToolTip.qml")
                        if (component.status === Component.Ready) {
                            var tooltip = component.createObject(shutdown)
                            tooltip.x = -100
                            tooltip.y = 40
                            tooltip.destroy(600)
                        }
                    }
                    onExited: {
                        shutdown.source = "images/system-shutdown.svg"
                    }
                    onClicked: {
                        shutdown.source = "images/system-shutdown-pressed.svg"
                        sddm.powerOff()
                    }
                }
            }
        }
    }

    Row {
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.rightMargin: 70
        anchors.topMargin: 15

        Item {

            Image {
                id: reboot
                height: 22
                width: 22
                source: "images/system-reboot.svg"
                fillMode: Image.PreserveAspectFit

                MouseArea {
                    anchors.fill: parent
                    hoverEnabled: true
                    onEntered: {
                        reboot.source = "images/system-reboot-hover.svg"
                        var component = Qt.createComponent(
                                    "components/RebootToolTip.qml")
                        if (component.status === Component.Ready) {
                            var tooltip = component.createObject(reboot)
                            tooltip.x = -100
                            tooltip.y = 40
                            tooltip.destroy(600)
                        }
                    }
                    onExited: {
                        reboot.source = "images/system-reboot.svg"
                    }
                    onClicked: {
                        reboot.source = "images/system-reboot-pressed.svg"
                        sddm.reboot()
                    }
                }
            }
        }
    }
Row {
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.rightMargin: 88
        anchors.topMargin: 15

    }
        Row {
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.rightMargin: 110
        anchors.topMargin: 15

         ComboBox {
                    id: session
                    height: 22
                    width: 150
                    model: sessionModel
                    textRole: "name"
                    displayText: ""
                    currentIndex: sessionModel.lastIndex
                    background: Rectangle {
                    implicitWidth: parent.width
                    implicitHeight: parent.height
                    color: "transparent"
                }


                    delegate: MenuItem {
                        id: menuitems
                        width: slistview.width * 4
                        text: session.textRole ? (Array.isArray(session.model) ? modelData[session.textRole] : model[session.textRole]) : modelData
                        highlighted: session.highlightedIndex === index
                        hoverEnabled: session.hoverEnabled
                        onClicked: {
                            ava.source = "file:///usr/share/extras/.face.icon"
                            session.currentIndex = index
                            slistview.currentIndex = index
                            session.popup.close()
                        }
                    }
                    indicator: Rectangle{
                        anchors.right: parent.right
                        anchors.rightMargin: 9
                        height: parent.height
                        width: 22
                        color: "transparent"
                        Image{
                            anchors.verticalCenter: parent.verticalCenter
                            width: parent.width
                            height: width
                            fillMode: Image.PreserveAspectFit
                            source: "images/conf.svg"
                        }
                    }
                    popup: Popup {
                        width: parent.width
                        height: parent.height * (session.model ? session.model.count : 1)
                        implicitHeight: slistview.contentHeight
                        margins: 0
                        contentItem: ListView {
                            id: slistview
                            clip: true
                            anchors.fill: parent
                            model: session.model
                            spacing: 0
                            highlightFollowsCurrentItem: true
                            currentIndex: session.highlightedIndex
                            delegate: session.delegate
                        }
                    }

                }
    }

        BrightnessContrast {
        anchors.fill: parent
        source: wallpaper
        brightness: 0
        contrast: 0.3
        layer.enabled: true
                               layer.effect: OpacityMask {
                               maskSource: identclock
                                }
    }
    FastBlur {
        anchors.fill: parent
        source: wallpaper
        radius: 32
        visible: listuser.visible ? false : true
        layer.enabled: true
           layer.effect: OpacityMask {
           maskSource: maskOfBlur
        }
    }
    Rectangle {
        id: maskOfBlur
        anchors.fill: parent
        color: "transparent"
        visible: false
        Rectangle {
            implicitWidth: 250
            implicitHeight: 32
            color: "#000"
            radius: 15
            anchors.bottom:  parent.bottom
            anchors.bottomMargin: baseOfUserDialog.anchors.bottomMargin + password.anchors.bottomMargin
            anchors.horizontalCenter: parent.horizontalCenter
        }
    }
    Item {
    id: identclock
    width: parent.width
    height: parent.height
    opacity: 0.8
   Rectangle {
       height: 300
       width: 400
       anchors.horizontalCenter: parent.horizontalCenter
       color: "transparent"
            Clock {
            id: clock
            visible: true
            anchors.topMargin: 100
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.bottom: parent.bottom
        }
}
}


Rectangle {
    id: baseOfUserDialog
    property real contentAreaHeight: Math.max(listuser.height, sizeAvatar*.9 + 15 + 32)
    width: Math.max(listuser.width, 150)
    height: contentAreaHeight + 15 + 32 + 15 + greetingLabel.height
    anchors.horizontalCenter: parent.horizontalCenter
    anchors.bottom: parent.bottom
    anchors.bottomMargin: 40
    color: "transparent"
    focus: true
Item {
    id: sectionLogin
    height: parent.height
    width: parent.width

    ListModel {
        id: qmlUserModel

    }
    function createModel() {
        // Agregar todos los usuarios al modelo qmlUserModel (name = username, realName = full name for display)
        qmlUserModel.clear()
        for (var y = 0; y < userModel.count; y++) {
            user.currentIndex = y
            var realName = ""
            var homeDir = ""
            try { realName = userModel.get(y).realName || "" } catch (e) {}
            try { homeDir = userModel.get(y).homeDir || "" } catch (e) {}
            qmlUserModel.append({
                name: user.currentText,
                realName: realName || user.currentText,
                icon: "file:///usr/share/extras/.face.icon",
                homeDir: homeDir
            });
        }
        lastIndexUser = determinateNewIndex()
        // Mover lastIndexUser al final
        var tempName = ""
        var tempRealName = ""
        var tempIcon = ""
        var tempHomeDir = ""
        tempName = qmlUserModel.get(lastIndexUser).name
        tempRealName = qmlUserModel.get(lastIndexUser).realName || tempName
        tempIcon = qmlUserModel.get(lastIndexUser).icon
        tempHomeDir = qmlUserModel.get(lastIndexUser).homeDir || ""

        // Eliminar el usuario en lastIndexUser
        qmlUserModel.remove(lastIndexUser)

        qmlUserModel.append({
            name: tempName,
            realName: tempRealName,
            icon: tempIcon,
            homeDir: tempHomeDir
        });
        displayNameUser = tempRealName
        currentUserHomeDir = tempHomeDir
    }



    ListView {
        id: listuser
        width: implicitCustomWidth + sizeAvatar*.9
        height: ((sizeAvatar*.9) + 37)*userModel.count
        model: qmlUserModel
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: parent.top
        visible: true
        currentIndex: userModel.lastIndex

        delegate: Item {
            height: sizeAvatar*.9 + 37
            width: listuser.width
            Column {
                id: contentFullUser
                spacing: 15
                anchors.top: parent.top
                anchors.horizontalCenter: parent.horizontalCenter
                Rectangle {
                    id: maskByList
                    width: sizeAvatar*.9
                    height: width
                    color: "black"
                    visible: false
                    radius: height/2
                }
                Image {
                    id: avaList
                    source: model.icon
                    height: sizeAvatar*.9
                    width: sizeAvatar*.9
                    fillMode: Image.PreserveAspectFit
                    layer.enabled: true
                    layer.effect: OpacityMask {
                        maskSource: maskByList
                    }
                }
                Text {
                    id: nameList
                    text: model.realName || model.name
                    color: "white"
                    font.bold: true
                    visible: !startAnimationNames
                    font.pixelSize: 14
                    anchors.horizontalCenter: parent.horizontalCenter
                }
            }
            Rectangle {
                id: resalt
                color: "#ff991c"
                width: parent.width/3.5
                height: width
                radius: width/2
                border.color: "white"
                border.width: width/14
                visible: model.name === userModel.currentText
                anchors.bottom: parent.bottom
                anchors.right: parent.right
                Image {
                    id: palomita
                    anchors.horizontalCenter: parent.horizontalCenter
                    width: parent.width*.6
                    height: width
                    source: "images/palomita.svg"
                    sourceSize: Qt.size(width, width)
                    fillMode: Image.PreserveAspectFit
                    anchors.verticalCenter: parent.verticalCenter
                }
            }
            MouseArea {
                anchors.fill: contentFullUser
                onClicked: {
                    lastNameUser = model.name
                    displayNameUser = model.realName || model.name
                    currentUserHomeDir = model.homeDir || ""
                    lastIndexUser = determinateNewIndex()
                    listuser.visible = false
                }
            }
            Component.onCompleted: {
                implicitCustomWidth = nameList.implicitWidth > implicitCustomWidth ? nameList.implicitWidth : implicitCustomWidth
            }
        }
        states: [
            State {
                name: "visible"
                when: listuser.visible
                PropertyChanges {
                    target: listuser
                    opacity: 1
                }
            },
            State {
                name: "hidden"
                when: !listuser.visible
                PropertyChanges {
                    target: listuser
                    opacity: 0
                }
            }
        ]

        transitions: []


    }

    Text {
        id: touchIdHint
        text: textConstants.touchIdOrPassword
        visible: listuser.visible
        anchors.top: parent.top
        anchors.topMargin: sizeAvatar*.9 + 15 + 32 + 15
        anchors.horizontalCenter: parent.horizontalCenter
        color: "white"
        font.pointSize: 8
    }

                        Rectangle {
                            id: mask
                            width: sizeAvatar*.9
                            height: sizeAvatar*.9
                            radius: (sizeAvatar*.9)/2
                            visible: false
                            anchors.top: parent.top
                        }

                    Image {
                        id: ava
                        width: sizeAvatar*.9
                        height: sizeAvatar*.9
                        visible: !listuser.visible
                        fillMode: Image.PreserveAspectCrop
                        anchors.top: parent.top
                        anchors.horizontalCenter: parent.horizontalCenter

                        layer.enabled: true
                        layer.effect: OpacityMask {
                            maskSource: mask
                        }
                        source: "file:///usr/share/extras/.face.icon"
                         MouseArea {
                           anchors.fill: ava
                           onClicked: {
                              sectionLogin.createModel()
                              listuser.visible = true
                           }
                       }

                    }

                // Custom ComboBox for hack colors on DropDownMenu
                ComboBox {
                    id: user
                    height: 40
                    width: 226
                    textRole: "name"
                    currentIndex: userModel.lastIndex
                    anchors.left: parent.left
                    anchors.leftMargin: 50
                    model: userModel
                    visible: false

                }

                Text {
                    id: usernametext
                    text: displayNameUser || lastNameUser
                    anchors.top: parent.top
                    anchors.topMargin: listuser.visible ? listuser.height + 6 : sizeAvatar*.9 + 6
                    anchors.horizontalCenter: parent.horizontalCenter
                    font.pixelSize: 14
                    font.family: fontbold.name
                    font.capitalization: Font.MixedCase
                    font.bold: true
                    visible: false
                    color: "white"
                    layer.enabled: true
                       layer.effect: DropShadow {
                       horizontalOffset: 1
                       verticalOffset: 1
                       radius: 10
                samples: 25
                color: "#26000000"
            }
                }
                Text  {
                    id: demo
                    //font.pixelSize:
                    text: textConstants.password
                    font.weight: Font.DemiBold
                    visible: false
                }
                TextField {
                    id: password

                    property var vtext: TextInput.Password

                    anchors.top: parent.top
                    anchors.topMargin: sizeAvatar*.9 + 15
                    anchors.horizontalCenter: parent.horizontalCenter
                    height: 32
                    width: 150
                    color: "#fff"
                    placeholderTextColor: "#66FFFFFF"
                    echoMode: TextInput.Password
                    focus: true
                    font.weight: Font.DemiBold
                    placeholderText: textConstants.enterPassword
                    horizontalAlignment: TextInput.AlignLeft
                    verticalAlignment: TextInput.AlignVCenter
                    // leftPadding: (width - demo.implicitWidth)/2
                    visible: listuser.visible ? false : true


                    onAccepted: sddm.login(lastNameUser, password.text,
                                           session.currentIndex)

                    background: Rectangle {

                    implicitWidth: parent.width
                    implicitHeight: parent.height
                    color: "#fff"
                    opacity: 0.2
                    radius: 15
                    }

                    Image {
                        id: caps
                        width: 24
                        height: 24
                        opacity: 0
                        state: keyboard.capsLock ? "activated" : ""
                        anchors.right: password.right
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.rightMargin: 10
                        fillMode: Image.PreserveAspectFit
                        source: "images/capslock.svg"
                        sourceSize.width: 24
                        sourceSize.height: 24

                        states: [
                            State {
                                name: "activated"
                                PropertyChanges {
                                    target: caps
                                    opacity: 1
                                }
                            },
                            State {
                                name: ""
                                PropertyChanges {
                                    target: caps
                                    opacity: 0
                                }
                            }
                        ]

                        transitions: [
                            Transition {
                                to: "activated"
                                NumberAnimation {
                                    target: caps
                                    property: "opacity"
                                    from: 0
                                    to: 1
                                    duration: imageFadeIn
                                }
                            },

                            Transition {
                                to: ""
                                NumberAnimation {
                                    target: caps
                                    property: "opacity"
                                    from: 1
                                    to: 0
                                    duration: imageFadeOut
                                }
                            }
                        ]
                    }
                }
                Text {
                id: greetingLabel
                text: textConstants.touchIdOrPassword
                color: "#fff"
                visible: !listuser.visible
                font.pointSize: 8
                anchors.top: parent.top
                anchors.topMargin: sizeAvatar*.9 + 15 + 32 + 15
                anchors.horizontalCenter: parent.horizontalCenter
            }
}



                Keys.onPressed: function(event) {
                    if (listuser.visible) {
                        listuser.visible = false
                        password.forceActiveFocus()
                        if (event.text) {
                            password.insert(password.cursorPosition, event.text)
                        }
                        event.accepted = true
                        return
                    }
                    if (event.key === Qt.Key_Return
                            || event.key === Qt.Key_Enter) {
                        sddm.login(user.currentText, password.text,
                                   session.currentIndex)
                        event.accepted = true
                    }
                }

                // Custom ComboBox for hack colors on DropDownMenu

            }
    }

