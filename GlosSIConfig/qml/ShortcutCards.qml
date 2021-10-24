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
import QtQuick.Layouts 6.2
import QtQuick.Controls 6.2
import QtQuick.Controls.Material 6.2

GridView {
    id: shortcutgrid
    anchors.top: parent.top
    anchors.bottom: parent.bottom
    anchors.horizontalCenter: parent.horizontalCenter
    anchors.topMargin: margins
    visible: model.length > 0
    signal editClicked(var index, var shortcutInfo)

    property real margins: 16
    cellWidth: 242 + 8
    cellHeight: 149 + 8
    readonly property real displayedItems: Math.floor((parent.width - margins*2) / cellWidth)
    width: displayedItems * cellWidth
    model: uiModel.targetList;
    delegate: RPane {
        color: Qt.lighter(Material.background, 1.6)
        bgOpacity: 0.3
        radius: 8
        width: 242
        height: 149
        Material.elevation: 4

        Label {
            anchors.top: parent.top
            anchors.left: parent.left
            text: modelData.name
            font.bold: true
            font.pixelSize: 16
        }

        Row {
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            spacing: 4

            Button {
                text: qsTr("Add to Steam") // TODO
                onClicked: console.log("TODO") // TODO
                highlighted: true
            }

            Button {
                text: qsTr("Edit")
                onClicked: editClicked(index, modelData)
            }
        }

    }   
}