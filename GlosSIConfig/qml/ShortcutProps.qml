/*
Copyright 2021-2023 Peter Repukat - FlatspotSoftware

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


Item {
    id: propsContent
    anchors.fill: parent

    property alias fileDialog: fileDialog
    property alias uwpSelectDialog: uwpSelectDialog
    property alias egsSelectDialog: egsSelectDialog
    signal cancel()
    signal done(var shortcut)

    property var shortcutInfo: ({})

    function resetInfo() {
        shortcutInfo = uiModel.getDefaultConf() 
    }
	
    Component.onCompleted: function() {
        resetInfo()
    }

    onShortcutInfoChanged: function() {
	    if (!shortcutInfo) {
            return;
        }
	    if (nameInput) { // basic info (not in collapsible container)
            nameInput.text = shortcutInfo.name || ""
			launchApp.checked = shortcutInfo.launch.launch
            pathInput.text = shortcutInfo.launch.launchPath || ""
            argsInput.text = shortcutInfo.launch.launchAppArgs || ""
		}
		if (advancedTargetSettings) { // advanced settings (collapsible container)
            advancedTargetSettings.shortcutInfo = shortcutInfo;
        }
        if (maybeIcon) {
		    maybeIcon.source = shortcutInfo.icon
                    ? shortcutInfo.icon.endsWith(".exe")
                        ? "image://exe/" + shortcutInfo.icon
                        : "file:///" + shortcutInfo.icon
                    : 'qrc:/svg/add_photo_alternate_white_24dp.svg';
        }
    }
	
	Image {
        id: bgImage
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        fillMode: Image.PreserveAspectCrop
        source: "file:///" + uiModel.getGridImagePath(shortcutInfo)
        autoTransform: true
        opacity: 0
    }
	
	LinearGradient {
        id: mask
        anchors.fill: bgImage
        gradient: Gradient {
            GradientStop { position: 0.0; color: "#afFFFFFF"}
            GradientStop { position: 0.7; color: "transparent" }
            GradientStop { position: 1; color: "transparent" }
        }
    }
    OpacityMask {
        source: bgImage
        maskSource: mask
		anchors.fill: bgImage
    }

    Flickable {
        id: flickable
        anchors.margins: 0
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        clip: true
        ScrollBar.vertical: ScrollBar {

        }
        contentWidth: propscolumn.width
        contentHeight: propscolumn.height
        flickableDirection: Flickable.VerticalFlick


        Column {
            id: propscolumn
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.leftMargin: 32
            spacing: 4

            Item {
                id: topspacing
                width: 1
                height: 32
            }

            Item {
                id: namewrapper
                width: parent.width / 3
                height: 64
                Label {
                    anchors.left: parent.left
                    anchors.leftMargin: 4
                    id: nameLabel
                    font.bold: true
                    text: qsTr("Name")
                }
                FluentTextInput {
                    width: parent.width
                    anchors.top: nameLabel.bottom
                    anchors.topMargin: 4
                    id: nameInput
                    placeholderText: qsTr("...")
                    text: shortcutInfo.name
                    onTextChanged: shortcutInfo.name = text
                    validator: RegularExpressionValidator { regularExpression: /([0-z]|\s|.)+/gm }
                }
            }
            Item {
                width: 1
                height: 8
            }
            RPane {
                width: parent.width
                radius: 4
		        Material.elevation: 32
                bgOpacity: 0.97
                Column {
                    width: parent.width
                    height: parent.height
                    spacing: 4
                    Row {
                        spacing: 32
                        width: parent.width
                        CheckBox {
                            id: launchApp
                            text: qsTr("Launch app")
                            checked: shortcutInfo ? shortcutInfo.launch.launch : false
                            onCheckedChanged: function() {
                                shortcutInfo.launch.launch = checked
                                if (checked) {
                                    if (closeOnExit) {
                                        closeOnExit.enabled = true;
                                        if (closeOnExit.checked) {
                                            waitForChildren.enabled = true;
                                        }
                                    }
                                    allowDesktopConfig.enabled = true;
                                } else {
                                    waitForChildren.enabled = false;
                                    closeOnExit.enabled = false;
                                    allowDesktopConfig.enabled = false;
                                }
                            }
                        }
                    }
                    Item {
                        width: 1
                        height: 8
                    }
                    RowLayout {
                        id: launchlayout
                        spacing: 4
                        anchors.left: parent.left
						anchors.right: parent.right
						anchors.leftMargin: 8
						anchors.rightMargin: 16
						Button {
						    id: iconButton
                            Layout.preferredWidth: 56
                            Layout.preferredHeight: 64
                            Layout.alignment: Qt.AlignVCenter
							flat: true
							contentItem: Image {
                                id: maybeIcon
                                fillMode: Image.PreserveAspectFit
                                source: shortcutInfo.icon
                                    ? shortcutInfo.icon.endsWith(".exe")
                                        ? "image://exe/" + shortcutInfo.icon
                                        : "file:///" + shortcutInfo.icon
                                    : 'qrc:/svg/add_photo_alternate_white_24dp.svg'
                            }
							onClicked: iconFileDialog.open()
                        }
                        Item {
                            Layout.preferredWidth: 8
                            Layout.preferredHeight: 8
                        }
                        Item {
                            Layout.preferredWidth: parent.width / 2
                            Layout.fillWidth: true
                            height: 64
                            Label {
                                anchors.left: parent.left
                                anchors.leftMargin: 4
                                id: pathLabel
                                font.bold: true
                                text: qsTr("Path")
                            }
                            FluentTextInput {
                                width: parent.width
                                anchors.top: pathLabel.bottom
                                anchors.topMargin: 4
                                id: pathInput
                                placeholderText: qsTr("...")
                                enabled: launchApp.checked
                                text: shortcutInfo.launch.launchPath || ""
                                onTextChanged: function() {
                                    shortcutInfo.launch.launchPath = text
									shortcutInfo = shortcutInfo
								}
                            }
                        }
                        Button {
                            Layout.preferredWidth: 64
                            Layout.alignment: Qt.AlignBottom
                            text: qsTr("...")
                            onClicked: fileDialog.open();
                        }
                        Button {
                            Layout.preferredWidth: 64
                            Layout.alignment: Qt.AlignBottom
                            text: qsTr("UWP")
                            visible: uiModel.isWindows
                            onClicked: uwpSelectDialog.open();
                        }
                        Button {
                            Layout.preferredWidth: 64
                            Layout.alignment: Qt.AlignBottom
                            text: qsTr("EGS")
                            visible: uiModel.isWindows
                            onClicked: egsSelectDialog.open();
                        }
                        Item {
                            height: 1
                            Layout.preferredWidth: 12
                        }
                        Item {
                            Layout.preferredWidth: parent.width / 2.5
                            height: 64
                            Label {
                                anchors.left: parent.left
                                anchors.leftMargin: 4
                                id: argslabel
                                font.bold: true
                                text: qsTr("Launch Arguments")
                            }
                            FluentTextInput {
                                width: parent.width
                                anchors.top: argslabel.bottom
                                anchors.topMargin: 4
                                id: argsInput
                                enabled: launchApp.checked
                                text: shortcutInfo.launch.launchAppArgs
                                onTextChanged: shortcutInfo.launch.launchAppArgs = text
                            }
                        }
                    }
                }
            }
            Item {
                width: 1
                height: 8
            }

			AdvancedTargetSettings {
                id: advancedTargetSettings
            }

            Item {
                id: bottomspacing
                width: 1
                height: 32
            }
        }
    }

    Row {
        spacing: 8
        anchors.bottom: parent.bottom
        anchors.right: parent.right
        anchors.margins: 24
        anchors.bottomMargin: 16
        Button {
            text: qsTr("Cancel")
            onClicked: function() {
                resetInfo();
                cancel()
            }
        }
        Button {
            text: qsTr("💾 Save")
            highlighted: true
            enabled: nameInput.acceptableInput
            onClicked: function() {
                done(shortcutInfo)
            }
        }
    }

    FileDialog {
        id: fileDialog
        title: qsTr("Please choose a Program to Launch")
        nameFilters: uiModel.isWindows ? ["Executable files (*.exe *.bat *.ps1)"] : []
        onAccepted: {
            if (fileDialog.selectedFile != null) {
                pathInput.text = fileDialog.selectedFile.toString().replace("file:///", "")
                if (nameInput.text == "") {
                    nameInput.text = pathInput.text.replace(/.*(\\|\/)/,"").replace(/\.[0-z]*$/, "")
                }
				shortcutInfo.icon = pathInput.text
                launchApp.checked = true
            }
		    shortcutInfo = shortcutInfo;
        }
        onRejected: {
           
        }
    }
	
    FileDialog {
        id: iconFileDialog
        title: qsTr("Please choose an icon")
        nameFilters: uiModel.isWindows ? ["Image/Executable (*.exe *.png *.ico *.jpg)"] : ["Image (*.png *.ico *.jpg)"]
        onAccepted: {
            if (iconFileDialog.selectedFile != null) {
                shortcutInfo.icon = iconFileDialog.selectedFile.toString().replace("file:///", "")
				shortcutInfo = shortcutInfo;
            }
        }
        onRejected: {
           
        }
    }

    UWPSelectDialog {
        id: uwpSelectDialog
        onConfirmed: function(modelData) {
            if (nameInput.text == "") {
                    nameInput.text = modelData.AppName
            }
            if (modelData.IconPath) {
                shortcutInfo.icon = modelData.IconPath
            }
            pathInput.text = modelData.AppUMId
            launchApp.checked = true
        }
    }
    EGSSelectDialog {
        id: egsSelectDialog
        onConfirmed: function(modelData) {
            if (nameInput.text == "") {
                    nameInput.text = modelData.InstallLocation.split('/').pop().split('\\').pop().replace(/([a-z])([A-Z])/g, '$1 $2')
            }
            pathInput.text = "com.epicgames.launcher://apps/"
                + modelData.NamespaceId
                + "%3A"
                + modelData.ItemId
                + "%3A"
                + modelData.ArtifactId + "?action=launch&silent=true"
            launchApp.checked = true
        }
    }

    InfoDialog {
        id: helpInfoDialog
        titleText: qsTr("")
        text: qsTr("")
        extraButton: false
        extraButtonText: qsTr("")
        onConfirmedExtra: function(data) {
        }
    }

}
