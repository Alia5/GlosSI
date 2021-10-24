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
import QtQuick.Controls 6.2
import QtQuick.Layouts 6.2
import QtQuick.Controls.Material 6.2
import QtQuick.Dialogs 6.2


Item {
    id: propsContent
    anchors.fill: parent

    property alias fileDialog: fileDialog
    signal cancel()
    signal done(var shortcut)

    property var shortcutInfo: ({
            version:  1,
            name: null,
            launch: false,
            launchPath: null,
            launchAppArgs: null,
            closeOnExit: true,
            hideDevices: true,
            windowMode: false,
            maxFps: null,
            scale: null
        })

    function resetInfo() {
        shortcutInfo = ({
            version:  1,
            name: null,
            launch: false,
            launchPath: null,
            launchAppArgs: null,
            closeOnExit: true,
            hideDevices: true,
            windowMode: false,
            maxFps: null,
            scale: null
        })
    }

    onShortcutInfoChanged: function() {
        nameInput.text = shortcutInfo.name || ""
        launchApp.checked = shortcutInfo.launch || false
        pathInput.text = shortcutInfo.launchPath || ""
        argsInput.text = shortcutInfo.launchAppArgs || ""
        closeOnExit.checked = shortcutInfo.closeOnExit || false
        hideDevices.checked = shortcutInfo.hideDevices || false
        windowMode.checked = shortcutInfo.windowMode || false
    }


    Column {
        anchors.margins: 64
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        spacing: 4
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
            height: 28
        }
        Row {
            spacing: 32
            width: parent.width
            height: closeOnExitCol.height
            CheckBox {
                id: launchApp
                text: qsTr("Launch app")
                checked: shortcutInfo.launch
                onCheckedChanged: shortcutInfo.launch = checked
            }
            Column {
                id: closeOnExitCol
                spacing: 2
                CheckBox {
                    id: closeOnExit
                    text: qsTr("Close when launched app quits")
                    checked: shortcutInfo.closeOnExit
                    onCheckedChanged: shortcutInfo.closeOnExit = checked
                }
                Label {
                    text: qsTr("Recommended to disable for launcher-games")
                    wrapMode: Text.WordWrap
                    width: parent.width
                }
            }
        }
        RowLayout {
            id: launchlayout
            spacing: 4
            width: parent.width
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
        Item {
            width: 1
            height: 28
        }
        Row {
            spacing: 64
            width: parent.width
            Column {
                spacing: 2
                width: parent.width/2 - 64
                CheckBox {
                    id: hideDevices
                    text: qsTr("Hide (Real) Controllers")
                    checked: shortcutInfo.hideDevices
                    onCheckedChanged: shortcutInfo.hideDevices = checked
                }
                Label {
                    text: qsTr("Hides real game controllers from the system\nThis may prevent doubled inputs")
                    wrapMode: Text.WordWrap
                    width: parent.width
                }
                Label {
                    text: qsTr("You can change this setting and which devices are hidden in the GlosSI overlay")
                    wrapMode: Text.WordWrap
                    width: parent.width
                }
            }
            Column {
                spacing: 2
                width: parent.width/2 - 64
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
                }
                Label {
                    text: qsTr("Use if blackscreen-issues are encountered.")
                    wrapMode: Text.WordWrap
                    width: parent.width
                }
            }
        }
    }

    Row {
        spacing: 8
        anchors.bottom: parent.bottom
        anchors.right: parent.right
        anchors.margins: 32
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
                }
                launchApp.checked = true
            }
        }
        onRejected: {
           
        }
    }

    // UWPSelectDialog {}

}
