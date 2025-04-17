iimport QtQuick 2.15
import QtQuick.Controls 2.15
import QtLocation 5.15

ApplicationWindow {
    visible: true
    width: 640
    height: 480
    title: "Qt Location Example"

    // 地图视图
    Map {
        anchors.fill: parent
        plugin: MapPlugin {
            // 使用 OpenStreetMap
            url: "http://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png"
        }

        // 默认位置（经纬度）
        center: QtPositioning.coordinate(39.9042, 116.4074)  // 北京的经纬度
        zoomLevel: 10  // 缩放级别

        // 添加标记
        MapItem {
            coordinate: QtPositioning.coordinate(39.9042, 116.4074)  // 标记在北京
            anchorPoint.x: 16
            anchorPoint.y: 32

            Image {
                source: "qrc:/icons/marker.png"
                width: 32
                height: 32
            }
        }
    }
}

