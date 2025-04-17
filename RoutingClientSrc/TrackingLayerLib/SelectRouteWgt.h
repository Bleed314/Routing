#pragma once

#include <QWidget>
#include <QList>
#include <QListWidgetItem>
#include <QCloseEvent>

#include "UILibrary/BaseDialogEx.h"

#include "core/stringdefines.h"
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

class QByteArray;

using namespace EarthView::World::Spatial::Atlas;
using namespace EarthView::World::Spatial3D::Controls;
using namespace EarthView::World::Spatial2D::Controls;
using namespace EarthView::World::Graphic;
using namespace EarthView::World::Spatial3D;

using namespace EarthView::World::Core;
using namespace EarthView::World::Geometry3D;
using namespace EarthView::World::Spatial3D::Controls;
using namespace EarthView::World::Spatial::Geometry;

#include "trackinglayerlib_global.h"

#include "RouteStrut.h"
#include "DrivingInfoWgt.h"

namespace Ui { class	SelectRouteWgt; };

class  TRACKINGLAYERLIB_EXPORT SelectRouteWgt : public BaseDialogEx, public CGlobeControlListener, public IMapControlListener
{
	Q_OBJECT

public:
	SelectRouteWgt(QWidget *parent = Q_NULLPTR,bool isNeedSetSpeed=false);
	~SelectRouteWgt();

	void setSaveToDBShow(bool isShow);
	void hideSomeT();

protected:
	void closeEvent(QCloseEvent* event);

signals:
	void selectRoute(RouteInfo routeInfo);
	void preViewRoute(RouteInfo routeInfo);
	void signalCancle();

public:
	void setInfo(QList<RouteInfo> routeInfo);

private:
	void initialTable(QList<RouteInfo> routeInfo);
	QString secondToTime(const double second);

	QByteArray listToArrry(QList<QList<QPair<double, double>>> routePoint);
	QList<QList<QPair<double, double>>> arrryToList(QByteArray arrry);

private slots: 
	void okSlot();
	void cancelSlot();
	void lstItemWgtSlot(QListWidgetItem *item);
	void hideSlot();
	void itemClickedSlot(QTableWidgetItem* item);
	void itemDoubleClickedSlot(QTableWidgetItem* item);
	//保存路线到数据库
	void saveRouteSlot();

//图层相关
private:
	void initGeometryItem();
	void drawLine(QList<QPair<double, double>> line);
	void drawText(double X, double Y, QString text);
	void drawText_3D(double X, double Y, EVString text);
	void clearLine();
	void clearText();

private:
	QList<RouteInfo> _routeInfoLst;
	int _currentRow = -1;

private:
	//图层画图应用
	CTrackingLayer *m_pTrackingLayer;
	//用来高亮
	CTrackingGeomsItem* _lineItem;
	QList<CMultiGeometry3DExtension*> _3dLineList;
	QList<CMultiGeometry3DExtension*> _3dPointList;
	//绘制点名称文本
	QList<CTrackingTextItem* >  _allTextItem;
	//三维地图
	CGlobeControl *mpGlobeCtr;
	//二维地图
	CMapControl	*mpMapCtr;

private:
	Ui::SelectRouteWgt* ui;
};
