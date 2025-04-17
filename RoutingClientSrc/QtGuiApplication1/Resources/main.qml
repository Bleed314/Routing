import QtQuick 2.7
import QtQuick.Controls 2.0
import QtLocation 5.7
import QtPositioning 5.7
import QtLocation 5.3

ApplicationWindow {
    visible: true
    width: 2400
    height: 1800
	title: "Local Map Example"

	    // ���� Map ��ͼ
    Map 
	{
        anchors.fill: parent
        plugin: Plugin 
		{
            name: "osm"  // ʹ�� OpenStreetMap ���
        }
        // ���õ�ͼ��������
        center: QtPositioning.coordinate(51.5074, -0.1278)  // �׶�����
        zoomLevel: 12


        // ���һ�����
        MapQuickItem 
		{
            coordinate: QtPositioning.coordinate(51.5074, -0.1278)
            anchorPoint.x: 16
            anchorPoint.y: 16
            sourceItem: Rectangle 
			{
                width: 32
                height: 32
                color: "red"
                radius: 16
            }
        }
    }
}








