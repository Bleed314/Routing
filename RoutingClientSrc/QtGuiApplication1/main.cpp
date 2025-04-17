#include "QtGuiApplication1.h"
#include <QtWidgets/QApplication>
#include <QDir>

#include "OsrmDateResolveLib/OsrmDateResolveLib.h"
#include "OsrmDateResolveLib/RouteStrut.h"

#pragma comment(lib,"OsrmDateResolveLib")

//地图
#include <QVariantList>
#include <QQuickView>
#include <QQmlContext>
#include <QWebEngineView>
#include <QDebug>

//void drawRouteOnMap(const QVector<QPointF>& points) {
//	QVariantList path;
//	for (const auto& point : points) {
//		QVariantMap coordinate;
//		coordinate["latitude"] = point.x();
//		coordinate["longitude"] = point.y();
//		path.append(coordinate);
//	}
//
//	// 加载 QML 文件并传递数据
//	QString A = QDir::currentPath();
//
//	QQuickView* view=new QQuickView(nullptr);
//	view->rootContext()->setContextProperty("routePath", path);
//	view->setSource(QUrl(QStringLiteral("Resources/main.qml")));
//	view->rootContext()->setContextProperty("greetingMessage", "Helloooooooo");  // 向 QML 暴露变量
//	view->show();
//}
//
//
int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	QList<QPair<double, double>> routePoint;
	routePoint.push_back(QPair<double, double>(117.2272, 31.8206));
	routePoint.push_back(QPair<double, double>(118.4329, 31.3525));
	OsrmDateResolveLib osrm;
	QList<RouteInfo>  info = osrm.computerRouteFromServer(routePoint);
	qDebug()<< info.size();
	//QVector<QPointF> points;
	//for (RouteInfo route: info)
	//{
	//	for (QList<QPair<double, double>> line: route.routePoint)
	//	{
	//		for (QPair<double, double> point: line)
	//		{
	//			QPointF pos(point.second,point.first);
	//			points.push_back(pos);
	//		}
	//	}
	//	continue;
	//}

   //	drawRouteOnMap(points);

	return a.exec();
}



//#include <QGuiApplication>
//#include <QQmlApplicationEngine>
//
//int main(int argc, char *argv[]) {
//	QGuiApplication app(argc, argv);
//
//	QQmlApplicationEngine engine;
//	engine.load(QUrl(QStringLiteral("Resources/main.qml")));
//
//	if (engine.rootObjects().isEmpty())
//		return -1;
//
//	return app.exec();
//}

