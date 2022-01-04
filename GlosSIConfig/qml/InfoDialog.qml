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
Dialog {
	id: dlg
	anchors.centerIn: parent

	property var confirmedParam: null
	signal confirmed(var param)
	signal confirmedExtra(var param)

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
	
	property alias titleText: title.text
	property alias text: text.text

	property bool extraButton: false
	property alias extraButtonText: extrabutton.text
	property var confirmedExtraParam: null

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
		implicitWidth: Math.max(row.width, col.width)
		implicitHeight: title.height + row.height + spacing + col.height
		Label {
			id: title
            anchors.top: parent.top
			anchors.left: parent.left
			text: dlg.title
            font.bold: true
        }
		Column {
			id: col
			anchors.top: title.bottom
			anchors.topMargin: parent.spacing
			spacing: 16
			Label {
				id: text
			}
		}
		Row {
			id: row
			anchors.top: col.bottom
			anchors.topMargin: parent.spacing
			spacing: 16

			Button {
				id: extrabutton
				visible: extraButton
				onClicked: function(){
					close()
					confirmedExtra(confirmedExtraParam)
				}
			}

			Button {
				text: qsTr("OK")
				onClicked: function(){
					close()
					confirmed(confirmedParam)
				}
			}
			anchors.right: parent.right
		}
	}

}
