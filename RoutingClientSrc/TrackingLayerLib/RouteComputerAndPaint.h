#pragma once
#include <QList>
#include <QPair>
#include <QObject>
#include <QString>

#include "core/stringdefines.h"
#include "UILibrary/BaseDialogEx.h"
#include "qtplugins/globewidget.h"
#include "globecontrol/globecontrol.h"
#include "mapcontrol/mapcontrol.h"
#include "globecontrol/globecontrollistener.h"
#include "spatial3dengine/globecamera.h"
#include "spatialinterface/imap.h"
#include "spatialinterface/ilayer.h"
#include "spatialinterface/ispatialdisplay.h"
#include "mapcontrol/trackinggeomsitem.h"
#include "mapcontrol/trackinglayer.h"
#include "mapcontrol/trackingtextitem.h"
#include "geometry3d/geometry3dextension/multigeometry3dextension.h"
#include "geometry3d/geopolygonex.h"

#include "color/rgbcolor.h"

#include "trackinglayerlib_global.h"
#include "RouteStrut.h"


using namespace EarthView::World::Spatial::Atlas;
using namespace EarthView::World::Spatial3D::Controls;
using namespace EarthView::World::Spatial2D::Controls;
using namespace EarthView::World::Graphic;
using namespace EarthView::World::Spatial3D;

using namespace EarthView::World::Core;
using namespace EarthView::World::Geometry3D;
using namespace EarthView::World::Spatial3D::Controls;
using namespace EarthView::World::Spatial::Geometry;

using namespace EarthView::World::Spatial::Display;

/********************************************************************
	created:	2023/08/28
	created:	28:8:2023   17:34
	author:		李卫东
	comment:    路径规划，支持二三维路径规划后渲染
*********************************************************************/

class TRACKINGLAYERLIB_EXPORT RouteComputerAndPaint:public QObject
{
	Q_OBJECT
public:
	//构造时传入二三维有效指针即可绘制
	RouteComputerAndPaint(CGlobeControl *GlobeCtr=nullptr,CMapControl *MapCtr=nullptr);

	~RouteComputerAndPaint();

public:
	//设置路径点，至少包含起点和终点，默认第一个是起点(经纬度)最后一个是终点
	//调用该类其他接口之前需先调用该接口设置路径点
	void setRoutePoint(QList<QPair<double, double>> routePoint);

	//通过服务进行路径规划,返回多条组成线,不需要绘制时通过该接口即可拿到规划的路线
	QList<QList<QList<QPair<double, double>>>> computerRouteFromServer();

	//获取所有规划路径的长度
	QList<qint64> getComputeredRouteLength();

	//获取规划的路径
	QList<RouteInfo> getTrackingRouteInfo();

	//绘制规划的路线，会清理上一次绘制的路线
	void DrawRoute(bool isOnlyPaintMainRoute=false);

	//绘制主选路径或者备选路径
	void DrawRoute(int route);
	
	void setRrawCount(int flag);

	void setRouteColor(int r,int g,int b,int a);

	//绘制设置的路径点（二维、三维）
	void drawPoint();
	void drawPoint_3D();

	//清空绘制
	void ClearRoute();

	//设置路线名称
	void setRouteName(const QString& name);


signals:
	void sigServerNetError();

private:
	/////////////////////////////////绘制相关/////////////////////////////////////////

	//初始化跟踪层的符号
	void initGeometryItem();
	//清理跟踪层的符号
	void clearGeomeTryItem();

	//绘制、清理点、线和文字
	void clearPoint();
	void clearPoint_3D();
	void clearLine();
	void clearLine_3D();
	void drawText(double X, double Y, QString text);
	void drawText_3D(double X, double Y, QString text);
	void clear2D3DText();
	//绘制出发点和结束点到公路点的虚线
	void drawPointLineRoute();
	//绘制一条路径(包括三维和二维)
	void DrawRoute(QList<QList<QPair<double, double>>> route);
	//绘制多条路径
	void DrawMuiltRoute(QList<QList<QList<QPair<double, double>>>> muiltRoute);
	
private:
	//路径长
	QList<qint64> _computeredRouteLength;
	//路径点
	QList<QPair<double, double>> _routePoint;
	//服务计算解析出的路线信息
	QList<RouteInfo> _allRoute;
	//服务返回解析出的路线
	QList<QList<QList<QPair<double, double>>>> _muiltRoute;
	//绘制路线条数
	int _drawRouteCount;
	//规划路线公路出发点
	QPair<double, double> _startPos;
	//规划路线公路结束点
	QPair<double, double> _endPos;
	//路线名称，绘制在路线中间
	QString _routeName="";
	//路线颜色，外部设置
	QString _lineColor;
	//颜色区分
	static int _colorCount;


private:
	//图层画图应用
	CTrackingLayer *m_pTrackingLayer;

	CTrackingGeomsItem* _pointItem;//红色-用来画路径点
	CTrackingGeomsItem* _blueLineItem;//蓝色-用来画规划路线主路线
	CTrackingGeomsItem* _grayLineItem;//灰色-用来画规划路线辅路线
	CTrackingGeomsItem* _blueDotLineItem;//蓝色-用来画虚线（出发点到公路）
	CTrackingGeomsItem* _blueDotLineItem_2;//蓝色-用来画虚线（终点到公路）

	QList<CTrackingTextItem* >  _allTextItem;//绘制点名称文本
	QList<CMultiGeometry3DExtension*> _3dPointList;//三维画点
	QList<CMultiGeometry3DExtension*> _3dLineList;//三维画线
	CGlobeControl *mpGlobeCtr;//三维地图
	CMapControl	*mpMapCtr;//二维地图
	
};
