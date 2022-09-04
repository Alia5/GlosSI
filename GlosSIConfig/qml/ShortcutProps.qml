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
import QtQuick.Controls 6.2
import QtQuick.Layouts 6.2
import QtQuick.Controls.Material 6.2
import QtQuick.Dialogs 6.2


Item {
    id: propsContent
    anchors.fill: parent

    property alias fileDialog: fileDialog
    property alias uwpSelectDialog: uwpSelectDialog
    signal cancel()
    signal done(var shortcut)

    property var shortcutInfo: ({
            version:  1,
            name: null,
            launch: false,
            launchPath: null,
            launchAppArgs: null,
            closeOnExit: true,
            waitForChildProcs: true,
            hideDevices: true,
            windowMode: false,
            maxFps: null,
            scale: null,
            icon: null,
            maxControllers: 4,
            disableOverlay: false,
            realDeviceIds: false,
            allowDesktopConfig: false,
            emulateDS4: false,
        })

    function resetInfo() {
        shortcutInfo = ({
            version:  1,
            name: null,
            launch: false,
            launchPath: null,
            launchAppArgs: null,
            closeOnExit: true,
            waitForChildProcs: true,
            hideDevices: true,
            windowMode: false,
            maxFps: null,
            scale: null,
            icon: null,
            maxControllers: 4,
            disableOverlay: false,
            realDeviceIds: false,
            allowDesktopConfig: false,
            emulateDS4: false,
        })
    }

    onShortcutInfoChanged: function() {
        nameInput.text = shortcutInfo.name || ""
        launchApp.checked = shortcutInfo.launch || false
        pathInput.text = shortcutInfo.launchPath || ""
        argsInput.text = shortcutInfo.launchAppArgs || ""
        closeOnExit.checked = shortcutInfo.closeOnExit || false
        waitForChildren.checked = shortcutInfo.waitForChildProcs
        hideDevices.checked = shortcutInfo.hideDevices || false
        windowMode.checked = shortcutInfo.windowMode || false
        maxControllersSpinBox.value = shortcutInfo.maxControllers
        disableOverlayCheckbox.checked = shortcutInfo.disableOverlay || false
        realDeviceIds.checked = shortcutInfo.realDeviceIds || false
        allowDesktopConfig.checked = shortcutInfo.allowDesktopConfig || false
        emulateDS4.checked = shortcutInfo.emulateDS4 || false
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
                height: 248
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
                        height: closeOnExitCol.height
                        CheckBox {
                            id: launchApp
                            text: qsTr("Launch app")
                            checked: shortcutInfo.launch
                            onCheckedChanged: function() {
                                shortcutInfo.launch = checked
                                if (checked) {
                                    closeOnExit.enabled = true;
                                    if (closeOnExit.checked) {
                                        waitForChildren.enabled = true;
                                    }
                                    allowDesktopConfig.enabled = true;
                                } else {
                                    waitForChildren.enabled = false;
                                    closeOnExit.enabled = false;
                                    allowDesktopConfig.enabled = false;
                                }
                            }
                        }
                        Column {
                            id: closeOnExitCol
                            spacing: 2
                            CheckBox {
                                id: closeOnExit
                                text: qsTr("Close when launched app quits")
                                checked: shortcutInfo.closeOnExit
                                onCheckedChanged: function() {
                                    shortcutInfo.closeOnExit = checked
                                    if (checked) {
                                        waitForChildren.enabled = true;
                                    } else {
                                        waitForChildren.enabled = false;
                                    }
                                }
                            }
                            Label {
                                text: qsTr("Recommended to disable for launcher-games")
                                wrapMode: Text.WordWrap
                                width: parent.width
                                leftPadding: 32
                                topPadding: -8
                            }
                            CheckBox {
                                id: waitForChildren
                                text: qsTr("Wait for child processes")
                                checked: shortcutInfo.waitForChildProcs
                                onCheckedChanged: function(){
                                    shortcutInfo.waitForChildProcs = checked
                                }
                            }
                        }
                        Column {
                            spacing: 2
                            CheckBox {
                                id: allowDesktopConfig
                                text: qsTr("Allow desktop-config")
                                checked: shortcutInfo.allowDesktopConfig
                                onCheckedChanged: function(){
                                    shortcutInfo.allowDesktopConfig = checked
                                }
                            }
                            Label {
                                text: qsTr("Use desktop-config if launched application is not focused")
                                leftPadding: 32
                                topPadding: -8
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
                        width: parent.width
                        Image {
                            id: maybeIcon
                            source: shortcutInfo.icon
                                ? shortcutInfo.icon.endsWith(".exe")
                                    ? "image://exe/" + shortcutInfo.icon
                                    : "file:///" + shortcutInfo.icon
                                : null
                            Layout.preferredWidth: 48
                            Layout.preferredHeight: 48
                            visible: shortcutInfo.icon
                            Layout.alignment: Qt.AlignVCenter
                        }
                        Item {
                            Layout.preferredWidth: 8
                            Layout.preferredHeight: 8
                            visible: shortcutInfo.icon
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
                                text: shortcutInfo.launchPath || ""
                                onTextChanged: shortcutInfo.launchPath = text
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
                                text: shortcutInfo.launchAppArgs
                                onTextChanged: shortcutInfo.launchAppArgs = text
                            }
                        }
                    }
                }
            }
            Item {
                width: 1
                height: 8
            }
            Row {
                spacing: 16
                width: parent.width

                RPane {
                    width: parent.width / 2 - 8
                    height: 264
                    radius: 4
		            Material.elevation: 32
                    bgOpacity: 0.97

                    Column {
                        spacing: 2
                        width: parent.width
                        Row {
                            CheckBox {
                                id: hideDevices
                                text: qsTr("Hide (Real) Controllers")
                                checked: shortcutInfo.hideDevices
                                onCheckedChanged: shortcutInfo.hideDevices = checked
                            }
                            RoundButton {
                                onClicked: () => {
                                    helpInfoDialog.titleText = qsTr("Hide (Real) Controllers")
                                    helpInfoDialog.text = 
                                        qsTr("Hides real game controllers from the system\nThis may prevent doubled inputs")
                                        + "\n"
                                        + qsTr("You can change this setting and which devices are hidden in the GlosSI overlay")
                            
                                    helpInfoDialog.open()
                                }
                                width: 48
                                height: 48
                                Material.elevation: 0
                                anchors.topMargin: 16
                                Image {
                                    anchors.centerIn: parent
                                    source: "qrc:/svg/help_outline_white_24dp.svg"
                                    width: 24
                                    height: 24
                                }
                            }
                        }
                        Item {
                            width: 1
                            height: 4
                        }
                        Row {
                            CheckBox {
                                id: realDeviceIds
                                text: qsTr("Use real device (USB)-IDs")
                                checked: shortcutInfo.realDeviceIds
                                onCheckedChanged: shortcutInfo.realDeviceIds = checked
                            }
                            RoundButton {
                                onClicked: () => {
                                    helpInfoDialog.titleText = qsTr("Use real device (USB)-IDs")
                                    helpInfoDialog.text = 
                                        qsTr("Only enable if input's are not recognized by the game")
                                        + "\n"
                                        + qsTr("If enabled, device-hiding won't work.\nUse the \"Max. Controller count\" setting!")
                            
                                    helpInfoDialog.open()
                                }
                                width: 48
                                height: 48
                                Material.elevation: 0
                                anchors.topMargin: 16
                                Image {
                                    anchors.centerIn: parent
                                    source: "qrc:/svg/help_outline_white_24dp.svg"
                                    width: 24
                                    height: 24
                                }
                            }
                        }
                        Item {
                            width: 1
                            height: 4
                        }
                        Row {
                            CheckBox {
                                id: emulateDS4
                                text: qsTr("Emulate DS4")
                                checked: shortcutInfo.emulateDS4
                                onCheckedChanged: shortcutInfo.emulateDS4 = checked
                            }
                             RoundButton {
                                onClicked: () => {
                                    helpInfoDialog.titleText = qsTr("Emulate DS4")
                                    helpInfoDialog.text = 
                                        qsTr("Instead of X360 Pad")
                                        + "\n"
                                        + qsTr("Disable \"Playstation Configuration support\" in Steam")
                                    helpInfoDialog.open()
                                }
                                width: 48
                                height: 48
                                Material.elevation: 0
                                anchors.topMargin: 16
                                Image {
                                    anchors.centerIn: parent
                                    source: "qrc:/svg/help_outline_white_24dp.svg"
                                    width: 24
                                    height: 24
                                }
                            }
                        }
                        Item {
                            width: 1
                            height: 4
                        }
                        Row {
                            leftPadding: 16
                            Label {
                                text: qsTr("Max. emulated controllers")
                                topPadding: 16
                            }
                            SpinBox {
                                id: maxControllersSpinBox
                                width: 128
                                value: 4
                                from: 0
                                to: 4
                                onValueChanged: shortcutInfo.maxControllers = value
                            }
                        }
                    }
                }
                RPane {
                    width: parent.width / 2 - 8
                    height: 264
                    radius: 4
		            Material.elevation: 32
                    bgOpacity: 0.97
                    Column {
                        spacing: 2
                        width: parent.width
                        CheckBox {
                            id: windowMode
                            text: qsTr("Steam/GlosSI overlay as separate window")
                            checked: shortcutInfo.windowMode
                            onCheckedChanged: shortcutInfo.windowMode = checked
                        }
                        Label {
                            text: qsTr("Doesn't show overlay on top, but as separate window")
                            wrapMode: Text.WordWrap
                            width: parent.width
                            leftPadding: 32
                            topPadding: -8
                        }
                        Label {
                            text: qsTr("Use if blackscreen-issues are encountered.")
                            wrapMode: Text.WordWrap
                            width: parent.width
                            leftPadding: 32
                        }
                        Item {
                            width: 1
                            height: 4
                        }
                        CheckBox {
                            id: disableOverlayCheckbox
                            text: qsTr("Disable Steam/GlosSI overlay")
                            checked: shortcutInfo.disableOverlay
                            onCheckedChanged: shortcutInfo.disableOverlay = checked
                        }
                        Label {
                            text: qsTr("Only controller emulation - No extra window")
                            wrapMode: Text.WordWrap
                            width: parent.width
                            leftPadding: 32
                            topPadding: -8
                        }
                    }
                }
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
                cancel()
            }
        }
        Button {
            text: qsTr("Done")
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
                    shortcutInfo.icon = nameInput.text
                }
                launchApp.checked = true
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
