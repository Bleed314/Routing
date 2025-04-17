#pragma once

#include <QString>
#include <QList>
#include <QPair>
#include <QUuid>

#include "trackinglayerlib_global.h"

struct  TRACKINGLAYERLIB_EXPORT DrivingInfo
{
	DrivingInfo()
	{
		id = QUuid::createUuid();
		loadName = QStringLiteral("无名大道");
	}
	QUuid id;
	QString loadName;//路名
	QString image;//导航方向图标
	double drivingDistance;//道路需要行进距离
};

struct TRACKINGLAYERLIB_EXPORT RouteInfo
{
	RouteInfo()
	{

	}

	//RouteInfo operator=(const RouteInfo it)
	//{
	//	runSpeed = it.runSpeed;
	//	name = it.name;
	//	length = it.length;
	//	spendTime = it.spendTime;
	//	routePoint = it.routePoint;
	//	drivingInfoLst = it.drivingInfoLst;
	//	return *this;
	//}

	QString runSpeed;
	QString name;
	double length;//km
	double spendTime;//s
	QList<QList<QPair<double, double>>> routePoint;
	QList<DrivingInfo> drivingInfoLst;
};