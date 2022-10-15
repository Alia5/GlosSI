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
import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Dialogs
import Qt5Compat.GraphicalEffects

Window {
    id: window
    visible: true
    width: 1049
    height: 700
    Material.theme: Material.Dark
    Material.accent: Material.color(Material.Blue, Material.Shade900)

    property bool itemSelected: false;
    property var manualInfo: null

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
        onConfirmedExtra: function () {
		    window.close()
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
		text: uiModel.newVersionName + "\n\n" + qsTr("Would you like to visit the download page now?")
		onConfirmed: function (callback) {
		    Qt.openUrlExternally(`https://glossi.flatspot.pictures/#downloads${ uiModel.newVersionName.includes('snapshot') ? '-snapshots' : '' }-${uiModel.newVersionName}`);
            callback();
		}
        buttonText: qsTr("Yes")
		extraButton: true
		extraButtonText: qsTr("Remind me later")
		visible: !!uiModel.newVersionName
    }
	
	Dialog {
        id: manualAddDialog
        anchors.centerIn: parent
        visible: false
	    modal: true
	    dim: true
	    parent: Overlay.overlay
	    Overlay.modal: Rectangle {
		    color: Qt.rgba(0,0,0,0.4)
		    opacity: backdropOpacity
		    Behavior on opacity {
			    NumberAnimation {
				    duration: 300
			    }
		    }
	    }
	    property real backdropOpacity: 1.0
        enter: Transition {
		    NumberAnimation{target: madcontent; property: "y"; from: parent.height; to: 16; duration: 300; easing.type: Easing.OutQuad }
		    NumberAnimation{target: madbackground; property: "y"; from: parent.height; to: 0; duration: 300; easing.type: Easing.OutQuad }
		    NumberAnimation{target: manualAddDialog; property: "backdropOpacity"; from: 0; to: 1; duration: 300; easing.type: Easing.OutQuad }
	    }

	    exit: Transition {
		    NumberAnimation{target: madcontent; property: "y"; from: 16; to: parent.height; duration: 300; easing.type: Easing.InQuad }
		    NumberAnimation{target: madbackground; property: "y"; from: 0; to: parent.height; duration: 300; easing.type: Easing.InQuad }
		    NumberAnimation{target: manualAddDialog; property: "backdropOpacity"; from: 1; to: 0; duration: 300; easing.type: Easing.InQuad }
	    }

	    background: RPane {
		    id: madbackground
		    radius: 4
		    Material.elevation: 64
            bgOpacity: 0.97
	    }
	    contentItem: Item {
            id: madcontent
            implicitWidth: steamscreener.width
		    implicitHeight: madtext.height + 16 + steamscreener.height + 16 + madrow.height

            Label {
                id: madtext
                text: qsTr("Add \"GlosSITarget\" as game to Steam and change it's properties (in Steam) to this:")
            }

            Image {
                    anchors.top: madtext.bottom
                    anchors.left: madtext.left
                    anchors.topMargin: 16
                    id: steamscreener
                    source: "qrc:/steamscreener.png"
            }

            FluentTextInput {
                id: madnameinput
                text: manualInfo ? manualInfo.name : ""
                anchors.top: steamscreener.top
                anchors.left: steamscreener.left
                anchors.topMargin: 72
                anchors.leftMargin: 92
                readOnly: true
                background: Item {}
                width: 550
            }

            FluentTextInput {
                id: glossiPathInput
                text: manualInfo ? manualInfo.launch : ""
                anchors.top: steamscreener.top
                anchors.left: steamscreener.left
                anchors.topMargin: 192
                anchors.leftMargin: 24
                readOnly: true
                background: Item {}
                width: 550
            }

            FluentTextInput {
                id: startDirInput
                text: manualInfo ? manualInfo.launchDir : ""
                anchors.top: steamscreener.top
                anchors.left: steamscreener.left
                anchors.topMargin: 266
                anchors.leftMargin: 24
                readOnly: true
                background: Item {}
                width: 550
            }

             FluentTextInput {
                id: launchOptsInput
                text: manualInfo ? manualInfo.config : ""
                anchors.top: steamscreener.top
                anchors.left: steamscreener.left
                anchors.topMargin: 432
                anchors.leftMargin: 24
                readOnly: true
                background: Item {}
                width: 550
            }

        	Row {
			    id: madrow
			    anchors.top: steamscreener.bottom
			    anchors.topMargin: 16
			    spacing: 16

			    Button {
				    text: qsTr("OK")
				    onClicked: function(){
					    manualAddDialog.close()
				    }
			    }
			    anchors.right: parent.right
		    }
        }
    }

    InfoDialog {
        id: writeErrorDialog
        titleText: qsTr("Error")
        text: qsTr("Error writing shortcuts.vdf...\nPlease make sure Steam is running")
        extraButton: true
        extraButtonText: qsTr("Manual instructions")
        onConfirmedExtra: function(data) {
           manualAddDialog.open();
        }
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
                        if (uiModel.updateTarget(windowContent.editedIndex, shortcut)) {
						    if (steamShortcutsChanged == false) {
                                steamChangedDialog.open();
                            }
                        } else {
						    manualInfo = uiModel.manualProps(shortcut);
                            writeErrorDialog.open();
                        }
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
                if (param == "egs") {
                    props.egsSelectDialog.open();
                }
            }
        }
    }
}
