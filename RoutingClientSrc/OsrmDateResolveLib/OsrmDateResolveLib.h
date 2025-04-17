#pragma once

#include <QPair>
#include <QList>

struct RouteInfo;

#include "osrmdateresolvelib_global.h"

class OSRMDATERESOLVELIB_EXPORT OsrmDateResolveLib
{
public:
	OsrmDateResolveLib();

	QList<RouteInfo> computerRouteFromServer(QList<QPair<double, double>> routePoint);
};
