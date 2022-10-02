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

CollapsiblePane {
    radius: 4
    Material.elevation: 32
    bgOpacity: 0.97
    title: qsTr("Advanced ⚙️")

	property string subTitle: ""

    property var shortcutInfo: ({})

    content: 
    Column {
        spacing: 16

		Label {
            id: subTitleLabel
			width: parent.width
			font.pixelSize: 16
			text: subTitle
			height: text == "" ? 0 : 24
        }

        RPane {
            width: parent.width
            radius: 4
            Material.elevation: 32
            bgOpacity: 0.97
            height: advancedLaunchCol.height + 24
            Column {
                id: advancedLaunchCol
                spacing: 4
                height: advancedLaunchedRow.height

                Row {
                    id: advancedLaunchedRow
                    spacing: 32
                    width: parent.width
                    height: closeOnExitCol.height
                    Column {
                        id: closeOnExitCol
                        spacing: 2
                        CheckBox {
                            id: closeOnExit
                            text: qsTr("Close when launched app quits")
                            checked: shortcutInfo.launch.closeOnExit
                            onCheckedChanged: function() {
                                shortcutInfo.launch.closeOnExit = checked
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
                            checked: shortcutInfo.launch.waitForChildProcs
                            onCheckedChanged: function(){
                                shortcutInfo.launch.waitForChildProcs = checked
                            }
                        }
                    }
                    Column {
                        spacing: 2
                        CheckBox {
                            id: allowDesktopConfig
                            text: qsTr("Allow desktop-config")
                            checked: shortcutInfo.controller.allowDesktopConfig
                            onCheckedChanged: function(){
                                shortcutInfo.controller.allowDesktopConfig = checked
                            }
                        }
                        Label {
                            text: qsTr("Allow desktop-config if launched application is not focused")
                            leftPadding: 32
                            topPadding: -8
                        }
                    }
                }
            }
        }

        Row {
            spacing: 16
            width: parent.width

            RPane {
                width: parent.width / 2 - 8
                height: 248
                radius: 4
                Material.elevation: 32
                bgOpacity: 0.97

                Column {
                    spacing: 0
                    width: parent.width
                    Row {
                        CheckBox {
                            id: hideDevices
                            text: qsTr("Hide (Real) Controllers")
                            checked: shortcutInfo.devices.hideDevices
                            onCheckedChanged: shortcutInfo.devices.hideDevices = checked
                        }
                        RoundButton {
                            onClicked: () => {
                                helpInfoDialog.titleText = qsTr("Hide (Real) Controllers")
                                helpInfoDialog.text = 
                                    qsTr("Hides real game controllers from the system\nThis may prevent doubled inputs")
                                    + "\n"
                                    + qsTr("You can change this setting and which devices are hidden in the GlosSI overlay")
                                    + "\n"
									+ qsTr("If ONLY using a SteamController or SteamDeck this settings is not required, but won't hurt")
                        
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
                            checked: shortcutInfo.devices.realDeviceIds
                            onCheckedChanged: shortcutInfo.devices.realDeviceIds = checked
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
                            checked: shortcutInfo.controller.emulateDS4 || false
                            onCheckedChanged: shortcutInfo.controller.emulateDS4 = checked
                        }
                        RoundButton {
                            onClicked: () => {
                                helpInfoDialog.titleText = qsTr("Emulate DS4")
                                helpInfoDialog.text = 
                                    qsTr("Emulates a DS4 instead of X360 Pad")
                                    + "\n"
                                    qsTr("for usage with, for example, PSNow")
                                    + "\n"
                                    + qsTr("If enabled you have to disable \"Playstation Configuration support\" in Steam")
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
                            editable: true
                            value: shortcutInfo.controller.maxControllers
                            from: 0
                            to: 4
                            onValueChanged: shortcutInfo.controller.maxControllers = value
                        }
                        RoundButton {
                            onClicked: () => {
                                helpInfoDialog.titleText = qsTr("Max. emulated controllers")
                                helpInfoDialog.text = 
                                    qsTr("GlosSI will only provide [NUMBER] of controllers")
                                    + "\n"
                                    + qsTr("Required to set to actually connected controller count when using \"real device IDs\" ")
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
                }
            }
            RPane {
                width: parent.width / 2 - 8
                height: 248
                radius: 4
                Material.elevation: 32
                bgOpacity: 0.97
                Column {
                    spacing: 0
                    width: parent.width
                    Row {
                        CheckBox {
                            id: windowMode
                            text: qsTr("Steam/GlosSI overlay as separate window")
                            checked: shortcutInfo.window.windowMode
                            onCheckedChanged: shortcutInfo.window.windowMode = checked
                        }
                        RoundButton {
                            onClicked: () => {
                                helpInfoDialog.titleText = qsTr("Steam/GlosSI overlay as separate window")
                                helpInfoDialog.text = 
                                    qsTr("Doesn't show overlay on top, but as separate window")
                                    + "\n"
                                    + qsTr("Use if blackscreen-issues are encountered.")

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
                            id: disableOverlayCheckbox
                            text: qsTr("Disable Steam/GlosSI overlay")
                            checked: shortcutInfo.window.disableOverlay
                            onCheckedChanged: shortcutInfo.window.disableOverlay = checked
                        }
                        RoundButton {
                            onClicked: () => {
                                helpInfoDialog.titleText = qsTr("Disable Steam/GlosSI overlay")
                                helpInfoDialog.text = 
                                    qsTr("Only controller emulation - No extra window")
                                    + "\n"
                                    + qsTr("Might help with Steam remote play.")

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
                            text: qsTr("GlosSI-Overlay scale")
                            topPadding: 16
                        }
                        SpinBox {
                            id: scaleSpinBox
                            width: 172
                            from: -100
                            value: shortcutInfo.window.scale * 100 || 0
                            to: 350
                            stepSize: 10
                            editable: true

                            property int decimals: 2
                            property real realValue: value / 100

                            validator: DoubleValidator {
                                bottom: Math.min(scaleSpinBox.from, scaleSpinBox.to)
                                top:  Math.max(scaleSpinBox.from, scaleSpinBox.to)
                            }

                            textFromValue: function(value, locale) {
                                return Number(value / 100).toLocaleString(locale, 'f', scaleSpinBox.decimals)
                            }

                            valueFromText: function(text, locale) {
                                return Number.fromLocaleString(locale, text) * 100
                            }
                            onValueChanged: function() {
                                if (value <= 0) {
                                    shortcutInfo.window.scale = null
                                    return
                                }
                                shortcutInfo.window.scale = value / 100
                            }
                        }
                        RoundButton {
                            onClicked: () => {
                                helpInfoDialog.titleText = qsTr("GloSI-Overlay scaling")
                                helpInfoDialog.text = 
                                    qsTr("Scales the elements of the GlosSI-Overlay (not Steam Overlay)")
                                    + "\n"
                                    + qsTr(" <= 0.0 to use auto-detection")

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
                            text: qsTr("Max. Overlay FPS")
                            topPadding: 16
                        }
                        SpinBox {
                            id: maxFPSSpinBox
                            width: 172
                            from: -1
                            value: shortcutInfo.window.maxFps || 0
                            to: 244
                            stepSize: 5
                            editable: true

                            onValueChanged: function() {
                                if (value <= 0) {
                                    shortcutInfo.window.maxFps = null
                                    return
                                }
                                shortcutInfo.window.maxFps = value
                            }
                        }
                        RoundButton {
                            onClicked: () => {
                                helpInfoDialog.titleText = qsTr("Max. Overlay FPS")
                                helpInfoDialog.text = 
                                    qsTr("Restricts the FPS of the overlay to the given value")
                                    + "\n"
                                    + qsTr(" <= 0.0 to use screen refresh rate")

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
                }
            }
        }

        RPane {
            width: parent.width
            radius: 4
            Material.elevation: 32
            bgOpacity: 0.97
            Column {
                spacing: 4
                Row {
                    Row {
                        CheckBox {
                            id: extendedLogging
                            text: qsTr("Extended Logging")
                            checked: shortcutInfo.extendedLogging
                            onCheckedChanged: shortcutInfo.extendedLogging = checked
                        }
                    }
                }
            }
        }
    }
}