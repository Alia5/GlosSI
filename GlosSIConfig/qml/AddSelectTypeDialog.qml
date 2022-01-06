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
Dialog {
	id: dlg
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

	enter: Transition {
		NumberAnimation{target: content; property: "y"; from: parent.height; to: 16; duration: 300; easing.type: Easing.OutQuad }
		NumberAnimation{target: background; property: "y"; from: parent.height; to: 0; duration: 300; easing.type: Easing.OutQuad }
		NumberAnimation{target: dlg; property: "backdropOpacity"; from: 0; to: 1; duration: 300; easing.type: Easing.OutQuad }
	}

	exit: Transition {
		NumberAnimation{target: content; property: "y"; from: 16; to: parent.height; duration: 300; easing.type: Easing.InQuad }
		NumberAnimation{target: background; property: "y"; from: 0; to: parent.height; duration: 300; easing.type: Easing.InQuad }
		NumberAnimation{target: dlg; property: "backdropOpacity"; from: 1; to: 0; duration: 300; easing.type: Easing.InQuad }
	}

	background: RPane {
		id: background
		radius: 4
		Material.elevation: 64
        bgOpacity: 0.97
	}
	contentItem: Item {
		id: content
		readonly property real spacing: 16
		implicitWidth: row.width
		implicitHeight: title.height + row.height + spacing
		Label {
			id: title
            anchors.top: parent.top
			anchors.left: parent.left
            text: qsTr("Shortcut type")
            font.bold: true
            font.pixelSize: 24
        }
		Row {
			id: row
			anchors.top: title.bottom
			anchors.topMargin: parent.spacing
			spacing: 16
			Button {
				text: qsTr("Add manually")
				highlighted: true
				onClicked: function(){
					close()
					confirmed("man")
				}
			}
			Button {
				text: uiModel.isWindows ? qsTr("Win32 Program") : qsTr("Launch Program")
				highlighted: true
				onClicked: function(){
					close()
					confirmed("launch")
				}
			}
			Button {
			visible: uiModel.isWindows
				text: qsTr("UWP App")
				highlighted: true
				onClicked: function(){
					close()
					confirmed("uwp")
				}
			}
		}
	}

}
