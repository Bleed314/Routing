import QtQuick 2.5
import QtQuick.Controls 1.4
import QtLocation 5.6
import QtPositioning 5.5

ApplicationWindow {
    visible: true
    width: 800
    height: 600

    Plugin {
        id: mapPlugin
        name: "osm" // ʹ�� OpenStreetMap ���
		parameters: [
        PluginParameter { name: "osm.mapping.host"; value: "https://tile.openstreetmap.org/10/845/380.png" },
        PluginParameter { name: "osm.cache.disk.path"; value: "D/map_cache" }, // ����·��
        PluginParameter { name: "osm.cache.disk.size"; value: 1024 } // �����С����λΪ MB
    }

    Map {
        id: map
        anchors.fill: parent
        plugin: mapPlugin
        center: QtPositioning.coordinate(30.67, 104.06) // ���õ�ͼ���ĵ� (γ��, ����)
        zoomLevel: 12

        // ����·��
        MapPolyline {
            line.width: 5
            line.color: "blue"
            path: [
                QtPositioning.coordinate(30.67, 104.06), // ���
                QtPositioning.coordinate(30.68, 104.07), // ��;��
                QtPositioning.coordinate(30.69, 104.08)  // �յ�
            ]
        }
    }
}

