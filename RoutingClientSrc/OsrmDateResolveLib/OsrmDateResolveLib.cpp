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
	//1.·��������
	QString dataString;
	for (int i = 0; i < routePoint.size(); i++)
	{
		dataString += QString::number(routePoint.at(i).first, 'f', 7) + "," + QString::number(routePoint.at(i).second, 'f', 7) + ";";
	}
	dataString.chop(1);
	QHash<QString, QString> list;
	//QString strIniPath = QString::fromLocal8Bit("%1%2").arg(QApplication::applicationDirPath()).arg("/../userData/data/config/routeServer.ini");
	QString strIniPath = "./routeServer.ini";

	//2.����·���滮����
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

	//3.���񷵻����ݽ���
	if (reply.first)
	{
		QJsonDocument jsonDoc = QJsonDocument::fromJson(reply.second);

		QList<QList<QList<QPair<double, double>>>> muiltRoute;//�洢���������
															  // ��� JSON �Ƿ���Ч
		if (!jsonDoc.isNull() && jsonDoc.isObject())
		{
			QJsonObject jsonObject = jsonDoc.object();

			// ��ȡ code �ֶ�
			QString message = jsonObject["message"].toString();
			qDebug() << "message:" << message;

			// ��ȡ code �ֶ�
			QString code = jsonObject["code"].toString();
			qDebug() << "code:" << code;

			// ��ȡ waypoints �ֶ�
			QJsonArray waypointsArray = jsonObject["waypoints"].toArray();
			foreach(const QJsonValue &waypointValue, waypointsArray) 
			{
				QJsonObject waypointObject = waypointValue.toObject();
				QString name = waypointObject["name"].toString();
				double latitude = waypointObject["location"].toArray().at(1).toDouble();
				double longitude = waypointObject["location"].toArray().at(0).toDouble();
				qDebug() << "Waypoint Name:" << name << "Latitude:" << latitude << "Longitude:" << longitude;
			}

			// ��ȡ routes �ֶ�
			QJsonArray routesArray = jsonObject["routes"].toArray();
			int routeCount = 0;//���ڼ���·���������
			foreach(const QJsonValue &routeValue, routesArray)
			{
				QList<DrivingInfo> drivingList;
				QList<QList<QPair<double, double>>> route;//�洢���������
				QJsonObject routeObject = routeValue.toObject();

				// ��ȡ geometry �ֶ�
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
				// ��ȡ legs �ֶ�
				QJsonArray legsArray = routeObject["legs"].toArray();
				foreach(const QJsonValue &legValue, legsArray) 
				{
					QJsonObject legObject = legValue.toObject();

					// ��ȡ steps �ֶ�
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


					// ��ȡ distance �� duration �ֶ�
					double legDistance = legObject["distance"].toDouble();
					double legDuration = legObject["duration"].toDouble();
					qDebug() << "Leg Distance:" << legDistance << "Leg Duration:" << legDuration;
				}

				// ��ȡ distance �� duration �ֶ�
				double routeDistance = routeObject["distance"].toDouble();
				double routeDuration = routeObject["duration"].toDouble();
				qDebug() << "Route Distance:" << routeDistance << "Route Duration:" << routeDuration;

				muiltRoute.append(route);
				RouteInfo routeInfo;
				if (routeCount == 0)
				{
					routeInfo.name = QStringLiteral("��ѡ��·");
				}
				if (routeCount == 1)
				{
					routeInfo.name = QStringLiteral("��ѡ��·");
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
