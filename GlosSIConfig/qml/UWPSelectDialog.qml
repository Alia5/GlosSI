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

	property var unfilteredModel: null;
	property var filteredModel: [];


	onOpened: function() {
		unfilteredModel = null;
		unfilteredModel = uiModel.uwpList;
		listview.model = null;
		filteredModel = [];
		for(let i = 0; i < unfilteredModel.length; i++)
		{
			filteredModel.push(unfilteredModel[i])					
		}
		listview.model = filteredModel
	}

	onClosed: function() {
		listview.model = null;
		unfilteredModel = null;
		filteredModel = null;
	}

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
		implicitWidth: listview.width
		implicitHeight: listview.height + titlelabel.height + 16 + 64
		clip: true
		Label {
			id: titlelabel
			text: qsTr("Select UWP App...")
			font.pixelSize: 24
			font.bold: true
		}

		FluentTextInput {
            width: listview.width - 2
			x: 1
            anchors.top: titlelabel.bottom
            anchors.topMargin: 8
            id: searchBar
            enabled: true
			placeholderText: qsTr("Search...")
            text: ""
            onTextChanged: function() {
				listview.model = null;
				filteredModel = [];
				for(let i = 0; i < unfilteredModel.length; i++)
				{
					if(unfilteredModel[i].AppName.toLowerCase().includes(searchBar.text.toLowerCase())) {
						filteredModel.push(unfilteredModel[i])					
					}
				}
				listview.model = filteredModel
			}
        }

		BusyIndicator {
			running: visible
			anchors.centerIn: parent
			opacity: (!unfilteredModel || unfilteredModel.length == 0) ? 1 : 0
            Behavior on opacity {
                NumberAnimation {
                    duration: 350
                    easing.type: Easing.InOutQuad
                }
            }
			visible: opacity == 0 ? false : true
		}

		Button {
			anchors.right: parent.right
			anchors.top: listview.bottom
			anchors.topMargin: 16
			anchors.rightMargin: 2
			text: qsTr("Cancel")
			onClicked: dlg.close()
		}

		ListView {
			anchors.top: searchBar.bottom
			anchors.topMargin: 16
			id: listview
			width: window.width * 0.45
			height: window.height * 0.66
			spacing: 0
			clip: true
			model: filteredModel
			ScrollBar.vertical: ScrollBar {
			}

			opacity: (!unfilteredModel || unfilteredModel.length == 0) ? 0 : 1
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

			
			delegate: Item {
				width: listview.width
				height: 72

				Image {
					id: maybeIcon
					width: 56
					height: 56
					anchors.left: parent.left
					anchors.verticalCenter: parent.verticalCenter
					source: "file:///" + modelData.IconPath
					mipmap: true
					smooth: true
				}

				Column {
					anchors.left: maybeIcon.right
					anchors.right: parent.right
					anchors.leftMargin: 16
					anchors.verticalCenter: parent.verticalCenter
					spacing: 2
					Label {
						text: modelData.AppName
						font.pixelSize: 18
						font.bold: true
					}
					Label {
						text: modelData.AppUMId
						font.pixelSize: 12
					}
				}

				Rectangle {
					anchors.bottom: parent.bottom
					height: 1
					width: parent.width
					color: Qt.rgba(1,1,1,0.25)
				}

				MouseArea {
					anchors.fill: parent
					onClicked: function(){
						confirmed(modelData)
						dlg.close();
					}
				}
			}
		}
	}
}