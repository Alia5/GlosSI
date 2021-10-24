import QtQuick 2.9
import QtQuick.Controls 2.9
import QtQuick.Controls.Material 2.9
import QtQuick.Controls.Material.impl 2.9

Pane {
    id: control
    property int radius: 0
    property color color: control.Material.backgroundColor
    property real bgOpacity: 1
    background: Rectangle {
        color: parent.color
        opacity: parent.bgOpacity
        radius: control.Material.elevation > 0 ? control.radius : 0

        layer.enabled: control.enabled && control.Material.elevation > 0
        layer.effect: ElevationEffect {
            elevation: control.Material.elevation
        }
    }
}