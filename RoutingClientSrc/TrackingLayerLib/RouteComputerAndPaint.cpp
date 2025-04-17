#include "RouteComputerAndPaint.h"

#include <QApplication>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QTextStream>
#include <QFile>

#include "graphic/camera.h"
#include "mapcontrol/mapcontrol.h"
#include "spatialinterface/ifeatureclass.h"
#include "spatialdatabase/queryfilter.h"
#include "spatialinterface/ifeatureiterator.h"
#include "spatialinterface/ifeature.h"
#include "symbol/simplemarkersymbol.h"
#include "symbol/simplelinesymbol.h"
#include "symbol/simplefillsymbol.h"
#include "symbol/simpletextsymbol.h"
#include "color/rgbcolor.h"
#include "spatialobject/coordinatesystem/coordinatefactory.h"
#include "spatialobject/geometry/linestring.h"
#include "spatialobject/geometry/point.h"
#include "qtplugins/mapwidget.h"
//三维
#include "core/datastream.h"
#include "symbol/billboard3dsymbol.h"
#include "spatialobject/geoobject.h"
#include "geometry3d/geometry3dextension/multipoint3dextension.h"
#include "symbol/simpleline3dsymbol.h"
#include "geometry3d/geometry3dextension/multipolyline3dextension.h"
#include "geometry3d/geometry3d/wideline.h"

#include "./JQNet.h"
#include "./PolylineDecoder.h"

#include "WritClientLibrary/databaseoperatorsingleton.h"
#include "GroupTrainData/GroupTrainDataInstance.h"
#include "UtilityLibrary/EntityInfoManager.h"
#include "LibSimDataTrans/EVRTIReceiver.h"
#include "LibSimDataTrans/LocalSimDataTrans.h"

using namespace EarthView::World::Spatial::Geometry;
using namespace EarthView::World::Spatial2D::Controls;
using namespace EarthView::World::Spatial::GeoDataset;
using namespace EarthView::World::Spatial::Utility;
using namespace EarthView::World::Spatial::Display;

using namespace EarthView::World::Graphic;
using namespace EarthView::World::Spatial;
using namespace EarthView::World::Spatial::Math;
using namespace EarthView::World::Spatial::Display;
using namespace EarthView::World::Geometry3D;
using namespace EarthView::World::Spatial::Utility;

int RouteComputerAndPaint::_colorCount;

RouteComputerAndPaint::RouteComputerAndPaint(CGlobeControl *GlobeCtr, CMapControl *MapCtr)
{
	_drawRouteCount = 0;
	mpGlobeCtr = GlobeCtr;
	mpMapCtr = MapCtr;
	if (mpMapCtr)
	{
		m_pTrackingLayer = mpMapCtr->getTrackingLayer();
	}
	initGeometryItem();
	_lineColor = "255,0,0,255";
}


RouteComputerAndPaint::~RouteComputerAndPaint()
{
	/*ClearRoute();*/
}

void RouteComputerAndPaint::setRoutePoint(QList<QPair<double, double>> routePoint)
{
	_routePoint = routePoint;
}

QList<qint64> RouteComputerAndPaint::getComputeredRouteLength()
{
	return _computeredRouteLength;
}

QList<RouteInfo> RouteComputerAndPaint::getTrackingRouteInfo()
{
	return _allRoute;
}

QList<QList<QList<QPair<double, double>>>> RouteComputerAndPaint::computerRouteFromServer()
{
	_computeredRouteLength.clear();
	QString dataString;
	for (int i = 0;i < _routePoint.size();i++)
	{
		dataString += QString::number(_routePoint.at(i).first, 'f', 7) + "," + QString::number(_routePoint.at(i).second, 'f', 7) + ";";
	}
	dataString.chop(1);
	QHash<QString, QString> list;
	QString strIniPath = QString::fromLocal8Bit("%1%2").arg(QApplication::applicationDirPath()).arg("/../userData/data/config/routeServer.ini");

	QFile file(strIniPath);
	QString severUrl, serverParam;
	if (file.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		QTextStream outStream(&file);
		severUrl = outStream.readLine();
		serverParam = outStream.readLine();
	}
	QString url = severUrl + dataString + serverParam;
	const auto &&reply = JQNet::HTTP::get(url,3*1000);

	qDebug() << "HTTP post reply:" << reply.first << reply.second.size() << reply.second;

	if (!reply.first)
	{
		emit sigServerNetError();
		return QList<QList<QList<QPair<double, double>>>>();
	}

	// 将 JSON 字符串转换为 QJsonDocument
	QJsonDocument jsonDoc = QJsonDocument::fromJson(reply.second);

	QList<QList<QList<QPair<double, double>>>> muiltRoute;//存储多条组成线
														  // 检查 JSON 是否有效
	if (!jsonDoc.isNull() && jsonDoc.isObject())
	{
		QJsonObject jsonObject = jsonDoc.object();

		// 获取 code 字段
		QString message = jsonObject["message"].toString();
		qDebug() << "message:" << message;

		// 获取 code 字段
		QString code = jsonObject["code"].toString();
		qDebug() << "Code:" << code;

		// 获取 waypoints 字段
		QJsonArray waypointsArray = jsonObject["waypoints"].toArray();
		foreach(const QJsonValue &waypointValue, waypointsArray) {
			QJsonObject waypointObject = waypointValue.toObject();
			QString name = waypointObject["name"].toString();
			double latitude = waypointObject["location"].toArray().at(1).toDouble();
			double longitude = waypointObject["location"].toArray().at(0).toDouble();
			qDebug() << "Waypoint Name:" << name << "Latitude:" << latitude << "Longitude:" << longitude;
		}

		_allRoute.clear();

		// 获取 routes 字段
		QJsonArray routesArray = jsonObject["routes"].toArray();
		int routeCount = 0;//用于计数路，最多两条
		foreach(const QJsonValue &routeValue, routesArray)
		{
			QList<DrivingInfo> drivingList;
			QList<QList<QPair<double, double>>> route;//存储多条组成线
			QJsonObject routeObject = routeValue.toObject();

			// 获取 geometry 字段
			QString geometry = routeObject["geometry"].toString();
			qint64 routeAllDistance = routeObject["distance"].toDouble();
			_computeredRouteLength.push_back(routeAllDistance);
			qDebug() << "Geometry:" << geometry;

			std::vector<std::pair<double, double>> decodedPoints;
			polyline::PolylineDecoder::decodePoly(geometry.toStdString(), decodedPoints, 5);

			for (int i = 0; i < decodedPoints.size(); ++i)
			{
				std::pair<double, double> pair = decodedPoints[i];
				qDebug() << "Geometry lat lon" << pair.first << pair.second;
			}
			// 获取 legs 字段
			QJsonArray legsArray = routeObject["legs"].toArray();
			foreach(const QJsonValue &legValue, legsArray) {
				QJsonObject legObject = legValue.toObject();

				// 获取 steps 字段
				QJsonArray stepsArray = legObject["steps"].toArray();
				foreach(const QJsonValue &stepValue, stepsArray) {
					QJsonObject stepObject = stepValue.toObject();

					QString geometry = stepObject["geometry"].toString();
					//qDebug() << "Geometry:" << geometry;

					std::vector<std::pair<double, double>> decodedPoints;
					polyline::PolylineDecoder::decodePoly(geometry.toStdString(), decodedPoints, 5);

					QList<QPair<double, double>> track;
					for (int i = 0; i < decodedPoints.size(); ++i)
					{
						std::pair<double, double> pair = decodedPoints[i];
						qDebug() << "steps" << i << pair.first << pair.second;


						track.append(qMakePair(pair.first, pair.second));
					}

					route.append(track);
					QString instruction = stepObject["maneuver"].toObject()["instruction"].toString();
					double duration = stepObject["duration"].toDouble();

					QString loadName = stepObject["name"].toString();
					QString direction = stepObject["maneuver"].toObject()["modifier"].toString();
					double distance = stepObject["distance"].toDouble();
					DrivingInfo driving;
					driving.loadName = loadName;
					driving.image = direction;
					driving.drivingDistance = distance;
					drivingList.push_back(driving);

					qDebug() << "Instruction:" << instruction << "Distance:" << distance << "Duration:" << duration;
				}


				// 获取 distance 和 duration 字段
				double legDistance = legObject["distance"].toDouble();
				double legDuration = legObject["duration"].toDouble();
				qDebug() << "Leg Distance:" << legDistance << "Leg Duration:" << legDuration;
			}

			// 获取 distance 和 duration 字段
			double routeDistance = routeObject["distance"].toDouble();
			double routeDuration = routeObject["duration"].toDouble();
			qDebug() << "Route Distance:" << routeDistance << "Route Duration:" << routeDuration;

			muiltRoute.append(route);
			RouteInfo routeInfo;
			if (routeCount == 0)
			{
				routeInfo.name = QStringLiteral("主选道路");
			}
			if (routeCount == 1)
			{
				routeInfo.name = QStringLiteral("备选道路");
			}
			routeInfo.length = routeDistance / 1000;//km
			routeInfo.spendTime = routeDuration;//s
			routeInfo.routePoint = route;
			routeInfo.drivingInfoLst = drivingList;
			_allRoute.push_back(routeInfo);

			routeCount++;
			if (routeCount==2)
			{
				break;
			}
		}
	}

	_muiltRoute = muiltRoute;
	return muiltRoute;
}

void RouteComputerAndPaint::DrawRoute(bool isOnlyPaintMainRoute)
{
	if (isOnlyPaintMainRoute)
	{
		if (_muiltRoute.size()>0)
		{
			DrawRoute(_muiltRoute.at(0));
		}
	}
	else
	{
		DrawMuiltRoute(_muiltRoute);
	}
}

void RouteComputerAndPaint::DrawRoute(int route)
{
	if (route==1)
	{
		DrawRoute(_muiltRoute.at(0));
	}
	else
	{
		if (_muiltRoute.size()>1)
		{
			DrawRoute(_muiltRoute.at(1));
		}
	}
	drawPointLineRoute();
}

void RouteComputerAndPaint::setRrawCount(int flag)
{
	_drawRouteCount = flag;
}

void RouteComputerAndPaint::setRouteColor(int r, int g, int b, int a)
{
	_lineColor = QString::number(r)+ "," +QString::number(g) + "," + QString::number(b) + "," + QString::number(a);
}

void RouteComputerAndPaint::ClearRoute()
{
	clearPoint();
	clearPoint_3D();
	clear2D3DText();
	clearLine();
	clearLine_3D();
}

void RouteComputerAndPaint::setRouteName(const QString & name)
{
	_routeName = name;
}
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                      
void RouteComputerAndPaint::initGeometryItem()
{
	//点
	//_pointItem = new CTrackingGeomsItem;
	//CRgbColor rgb(255, 0, 0);
	//CSimpleMarkerSymbol markSymbol;

	//markSymbol.setSize(4.5);
	//markSymbol.setColor(&rgb);
	//_pointItem->setSymbol(&markSymbol);
	//实线
	//_blueLineItem = new CTrackingGeomsItem;
	//CSimpleLineSymbol lineSymbol;
	CRgbColor rgb1(255, 0, 0);
	//lineSymbol.setWidth(100);
	//lineSymbol.setColor(&rgb1);
	//lineSymbol.setSimpleLineType(SLT_SimpleLineType_DashDotDot);
	//_blueLineItem->setSymbol(&lineSymbol);

	//_grayLineItem = new CTrackingGeomsItem;
	//CSimpleLineSymbol lineSymbol2;
	////lineSymbol2.setSimpleLineType(SLT_SimpleLineType_Dot);
	//CRgbColor rgb2(128, 128, 128);
	//lineSymbol2.setWidth(100);
	//lineSymbol2.setColor(&rgb2);
	//lineSymbol2.setSimpleLineType(SLT_SimpleLineType_DashDotDot);
	//_grayLineItem->setSymbol(&lineSymbol2);

	////虚线
	//_blueDotLineItem = new CTrackingGeomsItem;
	//CSimpleLineSymbol lineSymbol_3;
	//lineSymbol_3.setSimpleLineType(SLT_SimpleLineType_DashDotDot);
	//lineSymbol_3.setWidth(100);
	//lineSymbol_3.setColor(&rgb1);
	//_blueDotLineItem->setSymbol(&lineSymbol_3);

	//_blueDotLineItem_2 = new CTrackingGeomsItem;
	//CSimpleLineSymbol lineSymbol_4;
	//lineSymbol_4.setSimpleLineType(SLT_SimpleLineType_DashDotDot);
	//lineSymbol_4.setWidth(100);
	//lineSymbol_4.setColor(&rgb1);
	//_blueDotLineItem_2->setSymbol(&lineSymbol_4);

	//资源加载
	CResourceGroupManager* resourceGM = CResourceGroupManager::getSingletonPtr();
	EVString folder;
	CDirectory::getCurrentDirectory(folder);
	folder += "/data/samplesMedia/geometry3d";
	if (!resourceGM->getResourceGroup("geometry3d"))
	{
		resourceGM->createResourceGroup("geometry3d");
		resourceGM->addResourceLocation(folder, "FileSystem", "geometry3d");
		resourceGM->initialiseResourceGroup("geometry3d");
	}

}

void RouteComputerAndPaint::clearGeomeTryItem()
{
	////清除跟踪层数据
	//if (_pointItem != NULL)
	//{
	//	_pointItem->clear();
	//}
}

void RouteComputerAndPaint::drawPoint()
{
	//if (_routePoint.size() == 0)
	//{
	//	return;
	//}
	//
	////因为点变化了，每次清空所有点，重新绘制
	//clearPoint();
	////清空文字
	//clear2D3DText();

	//EarthView::World::Spatial::Geometry::CLineString* line1 = EV_NEW EarthView::World::Spatial::Geometry::CLineString();
	////添加起点
	//EarthView::World::Spatial::Geometry::CCoordinate p0 = EarthView::World::Spatial::Geometry::CCoordinate(_routePoint.at(0).first, _routePoint.at(0).second, 0);
	//line1->add(p0, 0);
	////drawText_3D(_routePoint.at(0).first, _routePoint.at(0).second, "起点");
	////添加途径点
	//int index = 1;
	//int pointSize = _routePoint.size();
	//for (int i = 1; i < pointSize -1;i++)
	//{
	//	EarthView::World::Spatial::Geometry::CCoordinate p1 = EarthView::World::Spatial::Geometry::CCoordinate(_routePoint.at(i).first, _routePoint.at(i).second, 0);
	//	line1->add(p1, index);
	//	//文字
	//	drawText(_routePoint.at(i).first, _routePoint.at(i).second, QString::number(index));
	//	index++;
	//}
	////添加终点
	//EarthView::World::Spatial::Geometry::CCoordinate p2 = EarthView::World::Spatial::Geometry::CCoordinate(_routePoint.at(pointSize-1).first, _routePoint.at(pointSize-1).second, 0);
	//line1->add(p2, index);
	////drawText_3D(_routePoint.at(pointSize - 1).first, _routePoint.at(pointSize - 1).second, "终点");

	//line1->setSpatialReferenceRef(CCoordinateSystemFactory::createCoordSys(GEO_WGS84));
	//line1->update();
}

void RouteComputerAndPaint::drawPoint_3D()
{
	if (_routePoint.size() == 0)
	{
		return;
	}
	if (!mpGlobeCtr)
	{
		return;
	}
	clearPoint_3D();//清空所有点

	int pointSize = _routePoint.size();
	QList<QPair<double, double>> allPoint;
	QPair<double, double> startPos;
	startPos.first = _routePoint.at(0).first;
	startPos.second = _routePoint.at(0).second;
	QPair<double, double> endPos;
	endPos.first = _routePoint.at(pointSize-1).first;
	endPos.second = _routePoint.at(pointSize - 1).second;

	if (startPos.first == 0 && startPos.second == 0 && endPos.first == 0 && endPos.second == 0)
	{
		return;
	}
	allPoint.push_back(startPos);
	for (int i = 0; i < _routePoint.size();i++)
	{
		QPair<double, double> Pos;
		Pos.first = _routePoint.at(i).first;
		Pos.second = _routePoint.at(i).second;
		allPoint.push_back(Pos);
	}
	allPoint.push_back(endPos);

	for (int i = 0; i < allPoint.size();i++)
	{

		//创建广告版点风格,
		CBillboard3DSymbol* symbol = EV_NEW CBillboard3DSymbol();
		symbol->setName("PointExtensionSymbol");
		symbol->setAltitudeMode(AM_Absolute);
		symbol->setHightenValue(0);
		symbol->setShowPicture(true);
		symbol->setPictureScale(1.0);
		symbol->setBillboardWidth(25.0);
		symbol->setBillboardHeight(25.0);

		CImage image;
		image.load("A.png", "geometry3d");

		DataStreamPtr streamptr = image.encode("png");
		MemoryDataStreamPtr memoryStreamPtr(streamptr);
		symbol->setImage(0, "A.png", memoryStreamPtr);

		symbol->setShowText(true);
		symbol->setPropertyName("name");
		symbol->setFontName("宋体");
		symbol->setFontSize(20);
		symbol->setColor((255, 0, 0));

		EarthView::World::Spatial::Geometry::CPoint* p = EV_NEW EarthView::World::Spatial::Geometry::CPoint(allPoint.at(i).first, allPoint.at(i).second, 0);
		p->setSpatialReferenceRef(CCoordinateSystemFactory::createCoordSys(GEO_WGS84));
		p->update();

		CGeoObject* obj1 = EV_NEW CGeoObject();
		obj1->setGeometry(p, false, true);//第三个参数为true，说明setGeometry时clone了一下,外面可以释放p了;
		obj1->setSymbol(symbol, false, true);//第三个参数为true，说明setSymbol时clone了一下，外面可以释放symbol了;
		EV_DELETE symbol;
		EV_DELETE p;
		if (i == 0)
		{
			obj1->addProperty("name", "起点");
		}
		else if (i == allPoint.size() - 1)
		{
			obj1->addProperty("name", "终点");
		}
		else
		{
			obj1->addProperty("name", QString::number(i).toStdString().c_str());
		}

		CMultiGeometry3DExtension* extensions = EV_NEW CMultiPoint3DExtension(mpGlobeCtr->getSceneManager(), "extensionTest");
		extensions = EV_NEW CMultiPoint3DExtension(mpGlobeCtr->getSceneManager(), "extensionTest");
		extensions->appendGeoObject(obj1);
		extensions->build();
		extensions->render();
		_3dPointList.push_back(extensions);
	}

	int fontCount = _routeName.count();
	if (_routeName != "" && _muiltRoute.size()>0)
	{
		//取中间点
		int pos = _muiltRoute[0].size() / 2;
		////路名沿路
		//if (_muiltRoute[0][pos].size() >fontCount)
		//{
		//	for (size_t i = 0; i < fontCount; i++)
		//	{
		//		drawText_3D(_muiltRoute[0][pos][i].second, _muiltRoute[0][pos][i].first, _routeName.data()[i]);
		//	}
		//}
		//简单将路名绘制在路中间
		if (_muiltRoute[0].size() > 0/*&& _muiltRoute[0][pos].size()<fontCount*/)
		{
			//取中间点
			int pos = _muiltRoute[0].size() / 2;
			if (pos != 0)
			{
				int pos2 = _muiltRoute[0][pos].size() / 2;
				QPair<double, double> middlePoint = QPair<double, double>(_muiltRoute[0][pos][pos2].second, _muiltRoute[0][pos][pos2].first);
				drawText(middlePoint.first, middlePoint.second, _routeName);
				drawText_3D(middlePoint.first, middlePoint.second, _routeName);
			}
		}
	}
}

void RouteComputerAndPaint::clearPoint_3D()
{
	if (_3dPointList.size() == 0)
	{
		return;
	}
	for (int i = 0; i < _3dPointList.size(); i++)
	{
		if (_3dPointList[i])
		{
			for (int j = 0; j < _3dPointList[i]->getGeoObjectCount(); j++)
			{
				CGeoObject* go = _3dPointList[i]->getGeoObject(j);
				EV_DELETE go;
				go = NULL;
			}
			EV_DELETE _3dPointList[i];
			_3dPointList[i] = NULL;
		}
	}
	_3dPointList.clear();

	if (mpGlobeCtr)
	{
		mpGlobeCtr->getSceneManager()->updateQuadImage(false, false, false);
	}
}

void RouteComputerAndPaint::clearLine_3D()
{
	if (_3dLineList.size() == 0)
	{
		return;
	}
	for (int i = 0; i < _3dLineList.size(); i++)
	{
		if (_3dLineList[i])
		{
			for (int j = 0; j < _3dLineList[i]->getGeoObjectCount(); j++)
			{
				CGeoObject* go = _3dLineList[i]->getGeoObject(j);
				EV_DELETE go;
				go = NULL;
			}
			EV_DELETE _3dLineList[i];
			_3dLineList[i] = NULL;
		}
	}
	_3dLineList.clear();
	if (mpGlobeCtr)
	{
		mpGlobeCtr->getSceneManager()->updateQuadImage(false, false, false);
	}
}

void RouteComputerAndPaint::clearLine()
{
	//if (_blueLineItem != NULL)
	//{
	//	_blueLineItem->clear();
	//}
	//if (_grayLineItem != NULL)
	//{
	//	_grayLineItem->clear();
	//}
	//if (_blueDotLineItem != NULL)
	//{
	//	_blueDotLineItem->clear();
	//}
	//if (_blueDotLineItem_2 != NULL)
	//{
	//	_blueDotLineItem_2->clear();
	//}
}

void RouteComputerAndPaint::drawText(double X, double Y, QString text)
{
	//if (!m_pTrackingLayer)
	//{
	//	return;
	//}
	//CTrackingTextItem* TextItem = new CTrackingTextItem;
	//_allTextItem.append(TextItem);
	//CRgbColor textColor(255, 0, 0);
	//CSimpleTextSymbol labelSymbol;
	//labelSymbol.setColor(&textColor);
	//labelSymbol.setSize(10.0);
	//labelSymbol.setBold(true);
	//TextItem->setPositionType(TIP_LeftBottom);
	//TextItem->setSymbol(&labelSymbol);
	//TextItem->setVisible(true);
	//m_pTrackingLayer->addItem(TextItem);
	//TextItem->setPos(X + 0.001, Y + 0.001);
	//TextItem->setText(text.toStdString().c_str());
}

void RouteComputerAndPaint::drawText_3D(double X, double Y, QString text)
{
	//创建广告版点风格,
	CBillboard3DSymbol* symbol = EV_NEW CBillboard3DSymbol();
	symbol->setName("PointExtensionSymbol");
	symbol->setAltitudeMode(AM_Absolute);
	symbol->setHightenValue(2);
	symbol->setShowPicture(true);
	symbol->setPictureScale(1.0);
	symbol->setBillboardWidth(25.0);
	symbol->setBillboardHeight(25.0);

	//CImage image;
	//image.load("A.png", "geometry3d");

	//DataStreamPtr streamptr = image.encode("png");
	//MemoryDataStreamPtr memoryStreamPtr(streamptr);
	//symbol->setImage(0, "A.png", memoryStreamPtr);

	symbol->setShowText(true);
	symbol->setPropertyName("name");
	symbol->setFontName("宋体");
	symbol->setFontSize(10);
	symbol->setColor((255, 0, 0));

	EarthView::World::Spatial::Geometry::CPoint* p = EV_NEW EarthView::World::Spatial::Geometry::CPoint(X, Y, 0);
	p->setSpatialReferenceRef(CCoordinateSystemFactory::createCoordSys(GEO_WGS84));
	p->update();

	CGeoObject* obj1 = EV_NEW CGeoObject();
	obj1->setGeometry(p, false, true);//第三个参数为true，说明setGeometry时clone了一下,外面可以释放p了;
	obj1->setSymbol(symbol, false, true);//第三个参数为true，说明setSymbol时clone了一下，外面可以释放symbol了;
	EV_DELETE symbol;
	EV_DELETE p;
	obj1->addProperty("name", text.toLocal8Bit().data());


	CMultiGeometry3DExtension* extensions = EV_NEW CMultiPoint3DExtension(mpGlobeCtr->getSceneManager(), "extensionTest");
	extensions = EV_NEW CMultiPoint3DExtension(mpGlobeCtr->getSceneManager(), "extensionTest");
	extensions->appendGeoObject(obj1);
	extensions->build();
	extensions->render();
	_3dPointList.push_back(extensions);
	//if (!m_pTrackingLayer)
	//{
	//	return;
	//}
	//CTrackingTextItem* TextItem = new CTrackingTextItem;
	//_allTextItem.append(TextItem);
	//CRgbColor textColor(255, 0, 0);
	//CSimpleTextSymbol labelSymbol;
	//labelSymbol.setColor(&textColor);
	//labelSymbol.setSize(10.0);
	//labelSymbol.setBold(true);
	//TextItem->setPositionType(TIP_LeftBottom);
	//TextItem->setSymbol(&labelSymbol);
	//TextItem->setVisible(true);
	//m_pTrackingLayer->addItem(TextItem);
	//TextItem->setPos(X + 0.01, Y + 0.01);
	//TextItem->setText(text);
}

void RouteComputerAndPaint::drawPointLineRoute()
{
	{
		EarthView::World::Spatial::Geometry::CLineString* line1 = EV_NEW EarthView::World::Spatial::Geometry::CLineString();
		EarthView::World::Spatial::Geometry::CCoordinate p1 = EarthView::World::Spatial::Geometry::CCoordinate(_routePoint.at(0).first, _routePoint.at(0).second, 0);
		EarthView::World::Spatial::Geometry::CCoordinate p2 = EarthView::World::Spatial::Geometry::CCoordinate(_startPos.first, _startPos.second, 0);
		//起点到公路
		if (m_pTrackingLayer)
		{
			line1->add(p1, 0);
			line1->add(p2, 1);
			line1->setSpatialReferenceRef(CCoordinateSystemFactory::createCoordSys(GEO_WGS84));
			line1->update();
			//m_pTrackingLayer->addItem(_blueDotLineItem);
			//_blueDotLineItem->addGeometry(line1);
		}
		if (mpGlobeCtr)
		{
			CSimpleLine3DSymbol * lineSymbol = EV_NEW CSimpleLine3DSymbol();
			CRgbColor color;
			color.setRGB(255, 255, 255);
			lineSymbol->setLineColor(&color);
			lineSymbol->setAltitudeMode(AM_Absolute);
			lineSymbol->setTransectShapeType(TST_Point);
			CGeoObject* obj2 = EV_NEW CGeoObject();
			obj2->setGeometry(line1, false, true);//第三个参数为true，说明setGeometry时clone了一下,外面可以释放line1了;
			obj2->setSymbol(lineSymbol, false, true);//第三个参数为true，说明setSymbol时clone了一下,外面可以释放lineSymbol了;
			EV_DELETE line1;
			EV_DELETE lineSymbol;
			CMultiGeometry3DExtension* extensions = EV_NEW CMultiPolyline3DExtension(mpGlobeCtr->getSceneManager(), "extensionTest");
			extensions->appendGeoObject(obj2);
			extensions->build();
			extensions->render();
			_3dLineList.push_back(extensions);
		}
	}

	EarthView::World::Spatial::Geometry::CLineString* line2 = EV_NEW EarthView::World::Spatial::Geometry::CLineString();
	EarthView::World::Spatial::Geometry::CCoordinate p3 = EarthView::World::Spatial::Geometry::CCoordinate(_routePoint.at(_routePoint.size() - 1).first, _routePoint.at(_routePoint.size() - 1).second, 0);
	EarthView::World::Spatial::Geometry::CCoordinate p4 = EarthView::World::Spatial::Geometry::CCoordinate(_endPos.first, _endPos.second, 0);
	//终点到公路
	if (m_pTrackingLayer)
	{
		line2->add(p3, 0);
		line2->add(p4, 1);
		line2->setSpatialReferenceRef(CCoordinateSystemFactory::createCoordSys(GEO_WGS84));
		line2->update();
		//m_pTrackingLayer->addItem(_blueDotLineItem_2);
		//_blueDotLineItem_2->addGeometry(line2);
	}
	if (mpGlobeCtr)
	{
		CSimpleLine3DSymbol * lineSymbol = EV_NEW CSimpleLine3DSymbol();
		CRgbColor color;
		color.setRGB(255, 255, 255);
		lineSymbol->setLineColor(&color);
		lineSymbol->setAltitudeMode(AM_Absolute);
		lineSymbol->setTransectShapeType(TST_Point);
		CGeoObject* obj2 = EV_NEW CGeoObject();
		obj2->setGeometry(line2, false, true);//第三个参数为true，说明setGeometry时clone了一下,外面可以释放line1了;
		obj2->setSymbol(lineSymbol, false, true);//第三个参数为true，说明setSymbol时clone了一下,外面可以释放lineSymbol了;
		EV_DELETE line2;
		EV_DELETE lineSymbol;
		CMultiGeometry3DExtension* extensions = EV_NEW CMultiPolyline3DExtension(mpGlobeCtr->getSceneManager(), "extensionTest");
		extensions->appendGeoObject(obj2);
		extensions->build();
		extensions->render();
		_3dLineList.push_back(extensions);
	}
}


void RouteComputerAndPaint::DrawRoute(QList<QList<QPair<double, double>>> route)
{
	if (!mpGlobeCtr)
	{
		return;
	}
	if (route.size()==0)
	{
		return;
	}

	EarthView::World::Spatial::Geometry::CLineString* line1 = EV_NEW EarthView::World::Spatial::Geometry::CLineString();
	int index = 0;
	for (int i = 0; i < route.size(); i++)
	{
		for (int j = 0; j < route.at(i).size(); j++)
		{
			//记住第一个公路点
			if (i == 0 && j == 0)
			{
				_startPos.first = route.at(i).at(j).second;
				_startPos.second = route.at(i).at(j).first;
			}
			//记住最后一个公路点
			if (i == route.size() - 1)
			{
				if (j == route.at(i).size() - 1)
				{
					_endPos.first = route.at(i).at(j).second;
					_endPos.second = route.at(i).at(j).first;
				}
			}
			
			double height = mpGlobeCtr->getSceneManager()->getHeightAt(route.at(i).at(j).first,route.at(i).at(j).second ,-1);
			EarthView::World::Spatial::Geometry::CCoordinate p1 = EarthView::World::Spatial::Geometry::CCoordinate(route.at(i).at(j).second, route.at(i).at(j).first, height);
			line1->add(p1, index++);
		}
	}
	line1->setSpatialReferenceRef(CCoordinateSystemFactory::createCoordSys(GEO_WGS84));
	//line1->update();

	//深蓝色
	if (_drawRouteCount == 1)
	{
		//m_pTrackingLayer->addItem(_blueLineItem);
		//_blueLineItem->addGeometry(line1);

		//3d绘制
		CSimpleLine3DSymbol * lineSymbol = EV_NEW CSimpleLine3DSymbol();
		QStringList colorLst = _lineColor.split(",");
		if (colorLst.size()==4)
		{
			CRgbColor color(colorLst.at(0).toInt(), colorLst.at(1).toInt(), colorLst.at(2).toInt(), colorLst.at(3).toInt());
			lineSymbol->setLineColor(&color);
		}
		else
		{
			CRgbColor color(255, 0, 0, 150);
			lineSymbol->setLineColor(&color);
		}
		//if (_colorCount%2==0)
		//{
		//	CRgbColor color(255, 0, 0,150);
		//	lineSymbol->setLineColor(&color);
		//}
		//else if (_colorCount % 3 == 0)
		//{
		//	CRgbColor color(255, 0, 0,100);
		//	lineSymbol->setLineColor(&color);
		//}
		//else if (_colorCount % 4 == 0)
		//{
		//	CRgbColor color(255, 0, 0, 50);
		//	lineSymbol->setLineColor(&color);
		//}
		//else
		//{
		//	CRgbColor color(255, 0, 0);
		//	lineSymbol->setLineColor(&color);
		//}
		//

		lineSymbol->setAltitudeMode(AM_Absolute);
		lineSymbol->setTransectShapeType(TST_WideLine);
		lineSymbol->setPropertyValue(0, "5");
		//lineSymbol->setSimpleLineType(SLT_SimpleLineType_DashDotDot);
		_colorCount++;
		//lineSymbol->setInterpolateDistance(50);

		//CWideLine* lineSymbol = EV_NEW CWideLine(2, 20);
		//CRgbColor color;
		//color.setRGB(255, 0, 0);
	

		CGeoObject* obj2 = EV_NEW CGeoObject();
		obj2->setGeometry(line1, false, true);//第三个参数为true，说明setGeometry时clone了一下,外面可以释放line1了;
		obj2->setSymbol(lineSymbol, false, true);//第三个参数为true，说明setSymbol时clone了一下,外面可以释放lineSymbol了;
		EV_DELETE line1;
		EV_DELETE lineSymbol;

		CMultiGeometry3DExtension* extensions = EV_NEW CMultiPolyline3DExtension(mpGlobeCtr->getSceneManager(), "extensionTest");
		extensions->appendGeoObject(obj2);
		extensions->build();
		extensions->render();
		_3dLineList.push_back(extensions);

	}
	//淡蓝色
	if (_drawRouteCount == 0)
	{
		//m_pTrackingLayer->addItem(_grayLineItem);
		//_grayLineItem->addGeometry(line1);

		//3d绘制
		CSimpleLine3DSymbol * lineSymbol = EV_NEW CSimpleLine3DSymbol();
		CRgbColor color;
		color.setRGB(125, 125, 125);
		lineSymbol->setLineColor(&color);
		lineSymbol->setAltitudeMode(AM_Absolute);
		lineSymbol->setTransectShapeType(TST_Point);
		CGeoObject* obj2 = EV_NEW CGeoObject();
		obj2->setGeometry(line1, false, true);//第三个参数为true，说明setGeometry时clone了一下,外面可以释放line1了;
		obj2->setSymbol(lineSymbol, false, true);//第三个参数为true，说明setSymbol时clone了一下,外面可以释放lineSymbol了;
		EV_DELETE line1;
		EV_DELETE lineSymbol;

		CMultiGeometry3DExtension* extensions = EV_NEW CMultiPolyline3DExtension(mpGlobeCtr->getSceneManager(), "extensionTest");
		extensions->appendGeoObject(obj2);
		extensions->build();
		extensions->render();
		_3dLineList.push_back(extensions);
	}


	QString usrname = DataBaseOperatorSingleton::getInstance()->getCurrentUser();
	GroupTrainDataInstance::getInstance()->set2dDrawRoute(route);
	//通知绘制插件在二维上绘制路线
	QString interName = "baseInter.FederationInteract.TwoDRoute";
	QHash<QString, QVariant> params;
	params.insert("otherinfo", "route" + usrname);
	bool isSuccess = EVRTIReceiver::GetInstance()->SendInterToEngine(interName, params);

	_drawRouteCount++;
}

void RouteComputerAndPaint::DrawMuiltRoute(QList<QList<QList<QPair<double, double>>>> muiltRoute)
{	//再重新绘点
	drawPoint();

	clearLine();
	clearLine_3D();

	//没有路线
	if (muiltRoute.size() == 0)
	{
		return;
	}

	//如果有两条路线，先画辅路
	if (muiltRoute.size() == 2)
	{
		_drawRouteCount = 0;
		DrawRoute(muiltRoute.at(1));
		DrawRoute(muiltRoute.at(0));
	}

	//如果一条路
	if (muiltRoute.size() == 1)
	{
		_drawRouteCount = 1;
		DrawRoute(muiltRoute.at(0));
	}

	//绘制出发点和结束点到公路的虚线
	drawPointLineRoute();

}

void RouteComputerAndPaint::clearPoint()
{
	//if (_pointItem != NULL)
	//{
	//	_pointItem->clear();
	//}
}

void RouteComputerAndPaint::clear2D3DText()
{
	//清空文字
	for (int i = 0; i < _allTextItem.size();i++)
	{
		_allTextItem.at(i)->setText("");
		_allTextItem.at(i)->setVisible(false);
	}
	_allTextItem.clear();
}
