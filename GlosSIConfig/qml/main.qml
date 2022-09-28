/*
Copyright 2021-2022 Peter Repukat - FlatspotSoftware

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/
import QtQuick 6.2
import QtQuick.Layouts 6.2
import QtQuick.Controls.Material 6.2

Window {
    id: window
    visible: true
    width: 1049
    height: 700
    Material.theme: Material.Dark
    Material.accent: Material.color(Material.Blue, Material.Shade900)

    property bool itemSelected: false;

    title: qsTr("GlosSI - Config")

    color: uiModel.hasAcrlyicEffect ? colorAlpha(Qt.darker(Material.background, 2), 0.65) : colorAlpha(Qt.darker(Material.background, 1.5), 0.98)

    function toggleMaximized() {
        if (window.visibility === Window.Maximized || window.visibility === Window.FullScreen) {
           window.visibility = Window.Windowed
        } else {
            window.visibility = Window.Maximized
        }
    }

    function colorAlpha(color, alpha) {
        return Qt.rgba(color.r, color.g, color.b, alpha);
    }

    property bool steamShortcutsChanged: false

    Component.onCompleted: function() {
        if (!uiModel.foundSteam) {
            steamNotFoundDialog.open();
            return;
        }
        if (!uiModel.steamInputXboxSupportEnabled) {
            steamXboxDisabledDialog.open();
        }
    }

    Image {
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        source: "qrc:/noise.png"
        fillMode: Image.Tile
        opacity: 0.033
    }

    SteamNotFoundDialog {
        id: steamNotFoundDialog
    }
    SteamInputXboxDisabledDialog {
        id: steamXboxDisabledDialog
    }


    InfoDialog {
        id: steamChangedDialog
        titleText: qsTr("Steam shortcuts changed!")
        text: qsTr("You have to restart Steam before your changes become visible")
        onConfirmed: function (callback) {
            callback();
        }
    }

    InfoDialog {
        id: steamChangedOnCloseDialog
        titleText: qsTr("Steam shortcuts changed!")
        text: qsTr("Please restart Steam to reload your changes\nRestart Steam now?")
        onConfirmed: function (closeWindow) {
			if (uiModel.restartSteam()) {
                closeWindow();            
            } else {
                // meh I really should write a dialogUtil or some shit...
                failedRestartingSteamDialog.confirmedParam = closeWindow;
                failedRestartingSteamDialog.open();
            }
        }
		buttonText: qsTr("Yes")
        extraButton: true
		extraButtonText: qsTr("No")
    }

    InfoDialog {
        id: failedRestartingSteamDialog
        titleText: qsTr("Failed restarting Steam!")
        text: qsTr("You have to restart Steam before your changes become visible")
        onConfirmed: function (callback) {
            callback();
        }
    }

	InfoDialog {
	    id: newVersionDialog
		titleText: qsTr("New version available!")
		text: qsTr("Would you like to visit the download page now?")
		onConfirmed: function (callback) {
            callback();
		    Qt.openUrlExternally(`https://glossi.flatspot.pictures/#downloads-${uiModel.newVersionName}`);
		}
        buttonText: qsTr("Yes")
		extraButton: true
		extraButtonText: qsTr("Remind me later")
		visible: !!uiModel.newVersionName
    }

    Rectangle {
        id: titleBar
        visible: uiModel.isWindows
        color: colorAlpha(Qt.darker(Material.background, 1.1), 0.90)
        height: visible ? 24 : 0
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        TapHandler {
            onTapped: if (tapCount === 2) toggleMaximized()
            gesturePolicy: TapHandler.DragThreshold
        }
        DragHandler {
            grabPermissions: TapHandler.CanTakeOverFromAnything
            onActiveChanged: if (active) { window.startSystemMove(); }
        }

        Label {
            text: window.title
            font.bold: true
            anchors.leftMargin: 16
            anchors.left: parent.left
            anchors.verticalCenter: parent.verticalCenter
        }

        RowLayout {
            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter
            spacing: 0
            ToolButton {
                text: "🗕"
                onClicked: window.showMinimized();
            }
            ToolButton {
                text: window.visibility === Window.Maximized || window.visibility === Window.FullScreen ? "🗗" : "🗖" 
                onClicked: window.toggleMaximized()
            }
            ToolButton {
                id: closbtn
                text: "🗙"
                onClicked: steamShortcutsChanged
                    ? (function(){
                        steamChangedOnCloseDialog.confirmedParam = () => {
                            window.close()
                        }
                        steamChangedOnCloseDialog.open()
                    })()
                    : window.close()
                background: Rectangle {
                    implicitWidth: 32
                    implicitHeight: 32
                    radius: 16
                    color: Qt.darker("red", closbtn.enabled && (closbtn.checked || closbtn.highlighted) ? 1.6 : 1.2)
                    opacity: closbtn.hovered ? 0.5 : 0
                    Behavior on opacity {
                        NumberAnimation {
                            duration: 350
                            easing.type: Easing.InOutQuad
                        }
                    }
                }
            }
        }
    }

    Item {
        id: windowContent
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: titleBar.bottom
        anchors.bottom: parent.bottom
        clip: true

        property int editedIndex: -1

        Item {
            id: homeContent
            anchors.fill: parent
            opacity: 1
            visible: opacity === 0 ? false : true
            Behavior on opacity {
                NumberAnimation {
                    duration: 300
                    easing.type: opacity === 0 ? Easing.OutQuad : Easing.InOutQuad
                }
            }
            Label {
                anchors.centerIn: parent
                text: qsTr("No shortcuts yet.\nClick \"+\" to get started")
                font.bold: true
                font.pixelSize: 24
                horizontalAlignment: Text.AlignHCenter
                visible: shortcutgrid.model.length == 0
            }

            ShortcutCards {
                id: shortcutgrid
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.topMargin: margins
                visible: model.length > 0
                margins: 16
                model: uiModel.targetList
                onEditClicked: function(index, shortcutInfo){
                    shortcutProps.opacity = 1;
                    homeContent.opacity = 0;
                    props.shortcutInfo = shortcutInfo
                    windowContent.editedIndex = index;
                }
            }
			Column {
                spacing: 8
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                anchors.margins: 24
				RoundButton {
                    id: optionsBtn
                    width: 64
                    height: 64
                    text: ""
                    contentItem: Item {
                        Image {
                            anchors.centerIn: parent
                            source: "qrc:/svg/settings_fill_white_24dp.svg"
                            width: 24
                            height: 24
                        }
					}
                    highlighted: true
                    onClicked: function() {
                        globalConf.opacity = 1;
                        homeContent.opacity = 0;
                    }
                }
                RoundButton {
                    id: addBtn
                    width: 64
                    height: 64
                    text: "+"
                    contentItem: Label {
                        anchors.centerIn: parent
                        text: addBtn.text
                        font.pixelSize: 32
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        elide: Text.ElideRight
                    }
                    highlighted: true
                    onClicked: selectTypeDialog.open()
                }
            }
        }

        Item {
            id: shortcutProps
            height: parent.height
            width: parent.width
            opacity: 0
            property real animMarg: opacity == 0 ? parent.height : 0 
            y: animMarg
            visible: opacity === 0 ? false : true
            Behavior on opacity {
                ParallelAnimation {
                    NumberAnimation {
                        duration: 300
                        property: "opacity"
                        easing.type: opacity === 0 ? Easing.OutQuad : Easing.InOutQuad
                    }
                    PropertyAnimation {
                        duration: 300
                        target: shortcutProps
                        property: "animMarg";
                        from: shortcutProps.animMarg
                        to: shortcutProps.animMarg > 0 ? 0 : shortcutProps.parent.height;
                        easing.type: opacity === 0 ? Easing.OutQuad : Easing.InOutQuad
                    }
                }
            }
            ShortcutProps {
                id: props
                anchors.fill: parent
                onCancel: function() {
                    shortcutProps.opacity = 0;
                    homeContent.opacity = 1;
                }
                onDone: function(shortcut) {
                    shortcutProps.opacity = 0;
                    homeContent.opacity = 1;
                    if (windowContent.editedIndex < 0) {
                        uiModel.addTarget(shortcut)
                    } else {
                        uiModel.updateTarget(windowContent.editedIndex, shortcut)
                    }
                }
            }
        }

		Item {
            id: globalConf
            height: parent.height
            width: parent.width
            opacity: 0
            property real animMarg: opacity == 0 ? parent.height : 0 
            y: animMarg
            visible: opacity === 0 ? false : true
            Behavior on opacity {
                ParallelAnimation {
                    NumberAnimation {
                        duration: 300
                        property: "opacity"
                        easing.type: opacity === 0 ? Easing.OutQuad : Easing.InOutQuad
                    }
                    PropertyAnimation {
                        duration: 300
                        target: globalConf
                        property: "animMarg";
                        from: globalConf.animMarg
                        to: globalConf.animMarg > 0 ? 0 : globalConf.parent.height;
                        easing.type: opacity === 0 ? Easing.OutQuad : Easing.InOutQuad
                    }
                }
            }
            GlobalConf {
                id: glConf
                anchors.fill: parent
                onCancel: function() {
                    globalConf.opacity = 0;
                    homeContent.opacity = 1;
                }
                onDone: function() {
                    globalConf.opacity = 0;
                    homeContent.opacity = 1;
                }
            }
        }


		Label {
            id: versionInfo
			anchors.bottom: parent.bottom
			anchors.left: parent.left
			anchors.margins: 8
			opacity: 0.5
			text: uiModel.versionString
        }

        AddSelectTypeDialog {
            id: selectTypeDialog
            visible: false
            onConfirmed: function(param) {
                shortcutProps.opacity = 1;
                homeContent.opacity = 0;
                props.resetInfo()
                windowContent.editedIndex = -1
                if (param == "launch") {
                    props.fileDialog.open();
                }
                if (param == "uwp") {
                    props.uwpSelectDialog.open();
                }
            }
        }
    }
}
