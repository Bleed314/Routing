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
		loadName = QStringLiteral("�������");
	}
	QUuid id;
	QString loadName;//·��
	QString image;//��������ͼ��
	double drivingDistance;//��·��Ҫ�н�����
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