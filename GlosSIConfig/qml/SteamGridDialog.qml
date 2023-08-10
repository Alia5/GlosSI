﻿/*
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
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Controls.Material

Dialog {
	id: gridDialog
	anchors.centerIn: parent

	signal confirmed(var param)

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

	property bool loading: true
	property bool hasTokenError: false


	onOpened: function() {
		loading = true;
		uiModel.loadSteamGridImages();
	}

	onClosed: function() {
	}

	enter: Transition {
		NumberAnimation{target: content; property: "y"; from: parent.height; to: 16; duration: 300; easing.type: Easing.OutQuad }
		NumberAnimation{target: background; property: "y"; from: parent.height; to: 0; duration: 300; easing.type: Easing.OutQuad }
		NumberAnimation{target: gridDialog; property: "backdropOpacity"; from: 0; to: 1; duration: 300; easing.type: Easing.OutQuad }
	}

	exit: Transition {
		NumberAnimation{target: content; property: "y"; from: 16; to: parent.height; duration: 300; easing.type: Easing.InQuad }
		NumberAnimation{target: background; property: "y"; from: 0; to: parent.height; duration: 300; easing.type: Easing.InQuad }
		NumberAnimation{target: gridDialog; property: "backdropOpacity"; from: 1; to: 0; duration: 300; easing.type: Easing.InQuad }
	}

	background: RPane {
		id: background
		radius: 4
		Material.elevation: 64
        bgOpacity: 0.97
	}
	contentItem: Item {
		id: content
		implicitWidth: listview.width
		implicitHeight: listview.height + titlelabel.height + 16 + 64
		clip: true
		Label {
			id: titlelabel
			text: qsTr("Loading Grid images...")
			font.pixelSize: 24
			font.bold: true
		}

		Item {
			id: topContentContainer
			anchors.top: titlelabel.bottom
			anchors.topMargin: 8
			anchors.horizontalCenter: parent.horizontalCenter
			anchors.left: parent.left
			anchors.right: parent.right
			height: loading || hasTokenError ? 72 : 0

			BusyIndicator {
				id: busyIndicator
				running: visible
				anchors.horizontalCenter: parent.horizontalCenter
				opacity: loading ? 1 : 0
				height: loading ? 72 : 0
				Behavior on opacity {
					NumberAnimation {
						duration: 350
						easing.type: Easing.InOutQuad
					}
				}
				visible: loading
			}

			Label {
				id: tokenErrorInfo
				anchors.horizontalCenter: parent.horizontalCenter
				opacity: hasTokenError ? 1 : 0
				anchors.left: parent.left
				anchors.right: parent.right
				wrapMode: Text.Wrap
				font.bold: true
				color: "yellow"
				textFormat: TextEdit.RichText 
				onLinkActivated: Qt.openUrlExternally(link)
				Behavior on opacity {
					NumberAnimation {
						duration: 350
						easing.type: Easing.InOutQuad
					}
				}
				visible: hasTokenError
				text: qsTr("Please go to <a href=\"https://www.steamgriddb.com/profile/preferences/api\">SteamGridDB</a> and gerenate a new API token. Then paste it into the settings dialog.")
			}
		
		}

		ListView {
			anchors.top: topContentContainer.bottom
			anchors.topMargin: 16
			anchors.bottom: parent.bottom
			anchors.bottomMargin: 16
			id: listview
			width: window.width * 0.45
			height: window.height * 0.66
			spacing: 0
			clip: true
			model: uiModel.steamgridOutput
			ScrollBar.vertical: ScrollBar {
			}
			onCountChanged: function() {
				listview.positionViewAtIndex(listview.count - 1, ListView.Visible);
				hasTokenError = listview.model.some((l) => l.includes("missing or invalid"));
				loading = !(listview.model.some((l) => l.includes("Press enter")) || hasTokenError);
			}

            Behavior on opacity {
				ParallelAnimation {
				    NumberAnimation {
						duration: 350
						easing.type: Easing.InOutQuad
					}
					PropertyAnimation {
						target: listview
						property: "anchors.topMargin"
						from: window.height * 0.75
						to: 16
						duration: 350
						easing.type: Easing.InOutQuad
					}
				}
            }

			
			delegate: /* Item {
				width: listview.width
				height: outputLabel.implicitHeight */

				Label {
					id: outputLabel
					text: modelData
				}
			// }
		}
		Button {
			anchors.right: parent.right
			anchors.bottom: parent.bottom
			anchors.bottomMargin: 2
			anchors.rightMargin: 2
			text: qsTr("Ok")
			onClicked: function() {
				gridDialog.close();
				confirmed(undefined);
			}
		}

	}
}