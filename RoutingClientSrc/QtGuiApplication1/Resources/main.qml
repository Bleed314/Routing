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

	    // 创建 Map 视图
    Map 
	{
        anchors.fill: parent
        plugin: Plugin 
		{
            name: "osm"  // 使用 OpenStreetMap 插件
        }
        // 设置地图中心坐标
        center: QtPositioning.coordinate(51.5074, -0.1278)  // 伦敦坐标
        zoomLevel: 12


        // 添加一个标记
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








