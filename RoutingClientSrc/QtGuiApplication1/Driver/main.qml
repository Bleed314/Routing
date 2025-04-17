iimport QtQuick 2.15
import QtQuick.Controls 2.15
import QtLocation 5.15

ApplicationWindow {
    visible: true
    width: 640
    height: 480
    title: "Qt Location Example"

    // ��ͼ��ͼ
    Map {
        anchors.fill: parent
        plugin: MapPlugin {
            // ʹ�� OpenStreetMap
            url: "http://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png"
        }

        // Ĭ��λ�ã���γ�ȣ�
        center: QtPositioning.coordinate(39.9042, 116.4074)  // �����ľ�γ��
        zoomLevel: 10  // ���ż���

        // ��ӱ��
        MapItem {
            coordinate: QtPositioning.coordinate(39.9042, 116.4074)  // ����ڱ���
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

