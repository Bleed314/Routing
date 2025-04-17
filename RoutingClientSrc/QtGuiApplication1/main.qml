import QtQuick 2.7
import QtQuick.Controls 2.0
import QtLocation 5.7
import QtPositioning 5.7

ApplicationWindow {
    visible: true
    width: 800
    height: 600

    WebEngineView {
        anchors.fill: parent
        url: "https://www.openstreetmap.org"  // 或自定义 HTML 地图页面
    }
}




