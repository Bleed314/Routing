#include "OsrmDateResolveLib.h"

#include <QHash>
#include <QFile>
#include <QTextStream>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDir>

#include "JQNet.h"
#include "RouteStrut.h"
#include "CooTrans.h"
#include "PolylineDecoder.h"

OsrmDateResolveLib::OsrmDateResolveLib()
{
}

QList<RouteInfo> OsrmDateResolveLib::computerRouteFromServer(QList<QPair<double, double>> routePoint)
{

	QList<RouteInfo> allRoute;
	//1.路径点设置
	QString dataString;
	for (int i = 0; i < routePoint.size(); i++)
	{
		dataString += QString::number(routePoint.at(i).first, 'f', 7) + "," + QString::number(routePoint.at(i).second, 'f', 7) + ";";
	}
	dataString.chop(1);
	QHash<QString, QString> list;
	//QString strIniPath = QString::fromLocal8Bit("%1%2").arg(QApplication::applicationDirPath()).arg("/../userData/data/config/routeServer.ini");
	QString strIniPath = "./routeServer.ini";

	//2.访问路径规划服务
	QFile file(strIniPath);
	QString severUrl, serverParam;
	if (file.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		QTextStream outStream(&file);
		severUrl = outStream.readLine();
		serverParam = outStream.readLine();
	}
	QString url = severUrl + dataString + serverParam;
	const auto &&reply = JQNet::HTTP::get(url, 3 * 1000);
	qDebug() << "HTTP post reply:" << reply.first << reply.second.size() << reply.second;

	//3.服务返回数据解析
	if (reply.first)
	{
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
			qDebug() << "code:" << code;

			// 获取 waypoints 字段
			QJsonArray waypointsArray = jsonObject["waypoints"].toArray();
			foreach(const QJsonValue &waypointValue, waypointsArray) 
			{
				QJsonObject waypointObject = waypointValue.toObject();
				QString name = waypointObject["name"].toString();
				double latitude = waypointObject["location"].toArray().at(1).toDouble();
				double longitude = waypointObject["location"].toArray().at(0).toDouble();
				qDebug() << "Waypoint Name:" << name << "Latitude:" << latitude << "Longitude:" << longitude;
			}

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
				foreach(const QJsonValue &legValue, legsArray) 
				{
					QJsonObject legObject = legValue.toObject();

					// 获取 steps 字段
					QJsonArray stepsArray = legObject["steps"].toArray();
					foreach(const QJsonValue &stepValue, stepsArray)
					{
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
				allRoute.push_back(routeInfo);
			}
		}
	}

	return allRoute;
}
