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
import QtQuick.Controls.Material 6.2
import QtQuick.Dialogs

Window {
    id: window
    visible: true
    width: 1280
    height: 719
    Material.theme: Material.Dark
    Material.accent: Material.color(Material.Blue, Material.Shade900)

    property bool itemSelected: false;

    title: qsTr("GlosSI - Config")

    color: uiModel.hasAcrlyicEffect ? "transparent" : colorAlpha(Material.background, 0.9)

    function toggleMaximized() {
        if (window.visibility === Window.Maximized || window.visibility === Window.FullScreen) {
           window.visibility = Window.Windowed
        } else {
            window.visibility = Window.Maximized
        }
    }

    function colorAlpha(color, alpha) {
        return Qt.rgba(color.r, color.g, color.b, alpha);
    }

    Rectangle {
        id: titleBar
        visible: uiModel.isWindows
        color: colorAlpha(Qt.darker(Material.background, 1.1), 0.90)
        height: visible ? 24 : 0
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        TapHandler {
            onTapped: if (tapCount === 2) toggleMaximized()
            gesturePolicy: TapHandler.DragThreshold
        }
        DragHandler {
            grabPermissions: TapHandler.CanTakeOverFromAnything
            onActiveChanged: if (active) { window.startSystemMove(); }
        }

        Label {
            text: window.title
            font.bold: true
            anchors.leftMargin: 16
            anchors.left: parent.left
            anchors.verticalCenter: parent.verticalCenter
        }

        RowLayout {
            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter
            spacing: 0
            ToolButton {
                text: "🗕"
                onClicked: window.showMinimized();
            }
            ToolButton {
                text: window.visibility === Window.Maximized || window.visibility === Window.FullScreen ? "🗗" : "🗖" 
                onClicked: window.toggleMaximized()
            }
            ToolButton {
                id: closbtn
                text: "🗙"
                onClicked: window.close()
                background: Rectangle {
                    implicitWidth: 32
                    implicitHeight: 32
                    radius: 16
                    color: Qt.darker("red", closbtn.enabled && (closbtn.checked || closbtn.highlighted) ? 1.6 : 1.2)
                    opacity: closbtn.hovered ? 0.5 : 0
                    Behavior on opacity {
                        NumberAnimation {
                            duration: 350
                            easing.type: Easing.InOutQuad
                        }
                    }
                }
            }
        }

    }

    Item {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: titleBar.bottom
        anchors.bottom: parent.bottom

        RPane {
            id: existingTargetsPane
            anchors.left: parent.left
            width:window.width / 3.301 + 16
            Component.onCompleted: console.log(width)
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            Material.elevation: 6
            anchors.leftMargin: -16
            radius: 16
            color: Qt.lighter(Material.background, 1.6)
            bgOpacity: 0.3

            Item {
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.leftMargin: 16
                clip: true
                ListView {
                    anchors.fill: parent
                    spacing: 0
                    model: uiModel.getTargetList();
                    delegate: Item {
                        width: parent.width
                        height: lbl.height + lbl.anchors.topMargin + lbl.anchors.bottomMargin
                        // TODO: Left size App icon
                        Label {
                            id: lbl
                            anchors.left: parent.left
                            anchors.right: parent.right
                            anchors.rightMargin: 48
                            anchors.topMargin: 8
                            anchors.bottomMargin: 8
                            anchors.leftMargin: 4
                            anchors.verticalCenter: parent.verticalCenter
                            text: modelData
                            font.pixelSize: 16
                        }
                        // TODO: Right side icon if in steam
                        Rectangle {
                            width: parent.width
                            height: 1
                            anchors.bottom: parent.bottom
                            color: Qt.rgba(1,1,1,0.3)
                        }
                    }
                }
            }
        }

        RoundButton {
            id: addBtn
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            anchors.margins: 24
            width: 64
            height: 64
            text: "+"
            contentItem: Label {
                anchors.centerIn: parent
                text: addBtn.text
                font.pixelSize: 32
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                elide: Text.ElideRight
            }
            highlighted: true
            onClicked: fileDialog.open();
        }

        FileDialog {
            id: fileDialog
            title: qsTr("Please choose a Program to Launch")
            nameFilters: uiModel.isWindows ? ["Executable files (*.exe *.bat *.ps1)"] : []
            onAccepted: {
                console.log("You chose: " + fileDialog.selectedFile)
            }
            onRejected: {
                console.log("Canceled")
            }
        }

    }
}
