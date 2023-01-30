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
import QtQuick 6.2
import QtQuick.Controls 6.2
import QtQuick.Layouts 6.2
import QtQuick.Controls.Material 6.2
import QtQuick.Dialogs 6.2


Item {
    id: globalConfContent
    anchors.fill: parent

    signal cancel()
    signal done()

    property var config: ({})

    Component.onCompleted: function() {
        config = uiModel.getDefaultConf() 
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
        contentWidth: parent.width
        contentHeight: confColumn.height
        flickableDirection: Flickable.VerticalFlick

		Column {
            id: confColumn
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.leftMargin: 32
            anchors.rightMargin: 32
            spacing: 4

            Item {
                width: 1
                height: 32
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
                            text: qsTr("Notify me about Snapshots")
                            checked: config ? config.snapshotNotify : false
                            onCheckedChanged: function() {
                                 config.snapshotNotify = checked
                            }
                        }
                    }
					Row {
					    leftPadding: 12
                        Row {
					        spacing: 16
                            Label {
                                topPadding: 8
                                id: apiKeyLabel
                                font.bold: true
                                text: qsTr("SteamGridDB-API-Key")
                            }
                            FluentTextInput {
                                width: 128
                                id: apiKeyInput
                                placeholderText: qsTr("...")
                                text: config.steamgridApiKey
                                onTextChanged: config.steamgridApiKey = text
                            }
                        }
                    }
				}
            }

            Item {
                width: 1
                height: 4
            }
			
			AdvancedTargetSettings {
                id: advancedTargetSettings
                shortcutInfo: config
                title: qsTr("Advanced default target settings ⚙️")
                subTitle: qsTr(
                    "Default settings when creating new shortcuts\n"
                    + "as well as settings applied when launching GlosSITarget without config")
            }

            Item {
                width: 1
                height: 4
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
					Label {
                        font.bold: true
                        font.pixelSize: 24
						text: qsTr("Experimental 🧪")
                    }
                    Row {
                        Row {
                            CheckBox {
                                id: globalModeUseGamepadUI
                                text: qsTr("Use BPM for global-/desktop-mode")
                                checked: config.globalModeUseGamepadUI
                                onCheckedChanged: config.globalModeUseGamepadUI = checked
                            }
                        }
                    }
                    Row {
                    	leftPadding: 12
                        Row {
                            spacing: 16
                            Label {
                                topPadding: 8
                                id: GlobalModeGameIdLabel
                                text: qsTr("GlobalMode GameId")
                            }
                            FluentTextInput {
                                width: 128
                                id: GlobalModeGameId
								enabled: false
                                text: config.globalModeGameId
                                onTextChanged: config.globalModeGameId = text
                            }
							Button {
                                id: GlobalModeGameIdButton
								text: qsTr("Create global-/desktop-mode shortcut")
								onClicked: {
									const globalModeConf = uiModel.getDefaultConf();
                                    globalModeConf.name = "GlosSI GlobalMode/Desktop";
									globalModeConf.launch.launch = false;
									uiModel.addTarget(globalModeConf);
                                    if (uiModel.addToSteam(globalModeConf, "")) {
									    steamChangedDialog.open();
                                    }
									const globalModeGID = uiModel.globalModeShortcutGameId();
									GlobalModeGameId.text = globalModeGID;
                                    setTimeout(() => {
                                        uiModel.saveDefaultConf(config);
                                        done();
                                    }, 10);
								}
								highlighted: true
                                visible: !uiModel.globalModeShortcutExists()
                            }
							Button {
                                id: GlobalModeGameIdConfigButton
								text: qsTr("Open global-/desktop-mode controller config")
								onClicked: {
                                    Qt.openUrlExternally("steam://currentcontrollerconfig/" + uiModel.globalModeShortcutAppId() + "/");
								}
                                visible: uiModel.globalModeShortcutExists()
                            }
                        }
                    }
				}
			}

            Item {
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
            text: qsTr("💾 Save")
            highlighted: true
            onClicked: function() {
			    uiModel.saveDefaultConf(config)
                done()
            }
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