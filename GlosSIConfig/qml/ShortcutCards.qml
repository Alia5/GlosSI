/*
Copyright 2021 Peter Repukat - FlatspotSoftware

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
import QtQuick.Controls 6.2
import QtQuick.Controls.Material 6.2
import QtQuick.Dialogs 6.2
import Qt5Compat.GraphicalEffects

GridView {
    id: shortcutgrid
    anchors.top: parent.top
    anchors.bottom: parent.bottom
    anchors.horizontalCenter: parent.horizontalCenter
    anchors.topMargin: margins
    visible: model.length > 0
    signal editClicked(var index, var shortcutInfo)
    ScrollBar.vertical: ScrollBar {
    }


    property real margins: 16
    cellWidth: 292 + 16
    cellHeight: 190 + 16
    readonly property real displayedItems: Math.floor((parent.width - margins*2) / cellWidth)
    width: displayedItems * cellWidth
    model: uiModel.targetList;
    GridView.delayRemove: true

    property var manualInfo: null

    // TODO: animations only properly work with abstractListModel... grrr...
    addDisplaced: Transition {
        NumberAnimation { properties: "x,y"; duration: 300 }
    }
    add: Transition {
        ParallelAnimation {
            NumberAnimation { property: "opacity"; from: 0; duration: 300; easing.type: Easing.OutQuad }
            NumberAnimation { properties: "x,y"; from: height; duration: 300; easing.type: Easing.OutQuad }
        }
    }

    populate: Transition {
        ParallelAnimation {
            NumberAnimation { property: "opacity"; from: 0; duration: 300; easing.type: Easing.OutQuad }
            NumberAnimation { properties: "x,y"; duration: 300; easing.type: Easing.OutQuad }
        }
    }

    remove: Transition {
        NumberAnimation { property: "opacity"; to: 0; duration: 300; easing.type: Easing.InQuad }
    }
    removeDisplaced: Transition {
        NumberAnimation { properties: "x,y"; duration: 300; easing.type: Easing.InQuad }
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
        text: qsTr("Error writing shortcuts.vdf...")
        extraButton: true
        extraButtonText: qsTr("Manual instructions")
        onConfirmedExtra: function(data) {
           manualAddDialog.open();
        }
    }

    delegate: RPane {
        id: delegateRoot
        color: Qt.lighter(Material.background, 1.6)
        bgOpacity: 0.3
        radius: 8
        width: 292
        height: 190
        Material.elevation: 4
        clip: true
        property bool isInSteam: uiModel.isInSteam(modelData);

        Image {
            anchors.top: parent.top
            anchors.left: parent.left
            id: maybeIcon
            source: !!modelData.icon
                ? modelData.icon.endsWith(".exe")
                    ? "image://exe/" + modelData.icon
                    : "file:///" + modelData.icon
                : null
            width: 48
            height: 48
            visible: !!modelData.icon
        }

        Label {
            id: label
            anchors.top: parent.top
            anchors.leftMargin: 8
            anchors.left: maybeIcon.right
            anchors.right: parent.right
            text: modelData.name
            font.bold: true
            font.pixelSize: 16
            elide: Text.ElideRight
        }

        Column {
            anchors.top: maybeIcon.bottom
            anchors.left: parent.left
            anchors.bottom: buttonrow.top
            anchors.margins: 12
            spacing: 8
            Row {
                spacing: 8
                visible: !!modelData.launchPath && modelData.launchPath.length > 0
                Label {
                    id: typeLabel
                    text: uiModel.isWindows && !!modelData.launchPath
                        ? modelData.launchPath.replace(/^.{1,3}:/, "").length < modelData.launchPath.length
                            ? "Win32"
                            : "UWP"
                        : qsTr("Launch")
                    font.bold: true
                }
                Label {
                    property string te: modelData.launchPath
                        ? modelData.launchPath.replace(/.*(\\|\/)/gm, "")
                        : ""
                    text: uiModel.isWindows ? te : te.replace(/\..{3}$/, "")
                    width: 292 - typeLabel.width - 72
                    elide: Text.ElideRight
                }
            }
            Row {
                visible: false // TODO: dunno about this...
                spacing: 4
                Label {
                    text: qsTr("Is in")
                    font.bold: true
                }
                Image {
                    anchors.verticalCenter: parent.verticalCenter
                    source: "qrc:/svg/steam.svg"
                    width: 16
                    height: 16
                    smooth: true
                    mipmap: true
                    ColorOverlay {
                        anchors.fill: parent
                        source: parent
                        color: "white"
                    }
                }
                Item {
                    width: 4
                    height: 1
                }
                Label {
                    anchors.verticalCenter: parent.verticalCenter
                    text: delegateRoot.isInSteam ? qsTr("Yes") : qsTr("No")
                    width: 292 - typeLabel.width - 72
                    elide: Text.ElideRight
                }
            }
        }

        Button {
            id: steambutton
            anchors.left: parent.left
            anchors.bottom: parent.bottom
            width: 72
            onClicked: function(){
                if (delegateRoot.isInSteam) {
                    if (!uiModel.removeFromSteam(modelData.name, "")) {
                        writeErrorDialog.open();
                        return;
                    }                
                } else {
                    if (!uiModel.addToSteam(modelData, "")) {
                        manualInfo = uiModel.manualProps(modelData);
                        writeErrorDialog.open();
                        return;
                    }
                }
                if (steamShortcutsChanged == false) {
                    steamChangedDialog.open();
                }
                delegateRoot.isInSteam = uiModel.isInSteam(modelData)
                steamShortcutsChanged = true
            }
            highlighted: delegateRoot.isInSteam
            Material.accent: Material.color(Material.Red, Material.Shade800)
            Row {
                anchors.centerIn: parent
                spacing: 8
                Label {
                    anchors.verticalCenter: parent.verticalCenter
                    text: delegateRoot.isInSteam ? "-" : "+"
                    font.bold: true
                    font.pixelSize: 24
                }
                Image {
                    anchors.verticalCenter: parent.verticalCenter
                    source: "qrc:/svg/steam.svg"
                    width: 22
                    height: 22
                    smooth: true
                    mipmap: true
                    ColorOverlay {
                        anchors.fill: parent
                        source: parent
                        color: "white"
                    }
                }
            }
        }

        Row {
            id: buttonrow
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            spacing: 4

            RoundButton {
                onClicked: uiModel.deleteTarget(index)
                highlighted: true
                Material.accent: Material.color(Material.Red, Material.Shade900)
                Image {
                    anchors.centerIn: parent
                    source: "qrc:/svg/delete_white_24dp.svg"
                    width: 16
                    height: 16
                }
            }

            RoundButton {
                onClicked: editClicked(index, modelData)
                highlighted: true
                Image {
                    anchors.centerIn: parent
                    source: "qrc:/svg/edit_white_24dp.svg"
                    width: 16
                    height: 16
                }
            }
        }

    }   
}