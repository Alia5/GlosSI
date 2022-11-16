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
import QtQuick.Controls.Material 6.2

TextField {
    id: control
    selectByMouse: true
    padding: 16
    bottomInset: padding/2
    background: Rectangle {
        implicitWidth: control.width + control.padding*2
        radius: 4
        color: control.enabled ? Qt.rgba(0,0,0,0.3) : Qt.rgba(0.2,0.2,0.2,0.3)
        border.color: control.enabled ? Qt.rgba(0.5,0.5,0.5,0.5) :  Qt.rgba(0.3,0.3,0.3,0.5)
    }
}