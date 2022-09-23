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
import QtQuick 2.9
import QtQuick.Controls 2.9
import QtQuick.Controls.Material 2.9
import QtQuick.Controls.Material.impl 2.9


RPane {
    property alias title: paneTitle.text
	width: parent.width

    property alias content: ldr.sourceComponent
    clip: true
    height: paneTitle.height + collapseColumn.spacing
    property bool collapsed: true
	id: collapsePane

    Behavior on height {
        NumberAnimation {
            duration: 300
            easing.type: Easing.InOutQuad
        }
    }

    Column {
	    id: collapseColumn
	    width: parent.width
        spacing: 16
		Item {
        	width: parent.width
			height: paneTitle.height
        	Label {
		        id: paneTitle
                anchors.left: parent.left
                anchors.leftMargin: 4
                font.bold: true
                font.pixelSize: 24
				anchors.top: parent.top
				anchors.topMargin: -2
            }
            RoundButton {
			    width: 48
                height: 48
                Material.elevation: 0
                anchors.rightMargin: 0
				anchors.top: parent.top
				anchors.topMargin: -12
				onClicked: function(){
                    collapsed = !collapsed;
                    if (collapsed) {
                        collapsePane.height = paneTitle.height + collapseColumn.spacing
                    } else {
                        collapsePane.height = paneTitle.height + collapseColumn.spacing * 3 + ldr.item.height
                    }
				}
                Image {
				    id: arrowImg
                    anchors.centerIn: parent
                    source: "qrc:/svg/expand_more_white_24dp.svg"
                    width: 24
                    height: 24
					transform: Rotation{
                        angle: collapsed ? 0 : 180
                        origin.x: arrowImg.width/2
                        origin.y: arrowImg.height/2          
                        Behavior on angle {
                            NumberAnimation {
                                duration: 125
                                easing.type: Easing.InOutQuad
                            }
                        }
                    }
                }
			    anchors.right: parent.right
            }
        }
        Loader {
            id: ldr
            width: parent.width
        }
	}
}
