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
import QtQuick 2.9
import QtQuick.Controls 2.9
import QtQuick.Controls.Material 2.9
import QtQuick.Controls.Material.impl 2.9

Pane {
    id: control
    property int radius: 0
    property color color: control.Material.backgroundColor
    property real bgOpacity: 1
	property string bgImgSource: null
	property real bgImgOpacity: -1
    background: Rectangle {
        color: parent.color
        opacity: parent.bgOpacity
        radius: control.Material.elevation > 0 ? control.radius : 0

        layer.enabled: control.enabled && control.Material.elevation > 0
        layer.effect: ElevationEffect {
            elevation: control.Material.elevation
        }
        Image {
            id: bgImage
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            source: bgImgSource ? bgImgSource : "qrc:/noise.png"
            fillMode: bgImgSource ? Image.PreserveAspectCrop : Image.Tile
            opacity: bgImgOpacity < 0 ? 0.035 : bgImgOpacity
        }
    }
}