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
	author:		������
	comment:    ·���滮��֧�ֶ���ά·���滮����Ⱦ
*********************************************************************/

class TRACKINGLAYERLIB_EXPORT RouteComputerAndPaint:public QObject
{
	Q_OBJECT
public:
	//����ʱ�������ά��Чָ�뼴�ɻ���
	RouteComputerAndPaint(CGlobeControl *GlobeCtr=nullptr,CMapControl *MapCtr=nullptr);

	~RouteComputerAndPaint();

public:
	//����·���㣬���ٰ��������յ㣬Ĭ�ϵ�һ�������(��γ��)���һ�����յ�
	//���ø��������ӿ�֮ǰ���ȵ��øýӿ�����·����
	void setRoutePoint(QList<QPair<double, double>> routePoint);

	//ͨ���������·���滮,���ض��������,����Ҫ����ʱͨ���ýӿڼ����õ��滮��·��
	QList<QList<QList<QPair<double, double>>>> computerRouteFromServer();

	//��ȡ���й滮·���ĳ���
	QList<qint64> getComputeredRouteLength();

	//��ȡ�滮��·��
	QList<RouteInfo> getTrackingRouteInfo();

	//���ƹ滮��·�ߣ���������һ�λ��Ƶ�·��
	void DrawRoute(bool isOnlyPaintMainRoute=false);

	//������ѡ·�����߱�ѡ·��
	void DrawRoute(int route);
	
	void setRrawCount(int flag);

	void setRouteColor(int r,int g,int b,int a);

	//�������õ�·���㣨��ά����ά��
	void drawPoint();
	void drawPoint_3D();

	//��ջ���
	void ClearRoute();

	//����·������
	void setRouteName(const QString& name);


signals:
	void sigServerNetError();

private:
	/////////////////////////////////�������/////////////////////////////////////////

	//��ʼ�����ٲ�ķ���
	void initGeometryItem();
	//������ٲ�ķ���
	void clearGeomeTryItem();

	//���ơ�����㡢�ߺ�����
	void clearPoint();
	void clearPoint_3D();
	void clearLine();
	void clearLine_3D();
	void drawText(double X, double Y, QString text);
	void drawText_3D(double X, double Y, QString text);
	void clear2D3DText();
	//���Ƴ�����ͽ����㵽��·�������
	void drawPointLineRoute();
	//����һ��·��(������ά�Ͷ�ά)
	void DrawRoute(QList<QList<QPair<double, double>>> route);
	//���ƶ���·��
	void DrawMuiltRoute(QList<QList<QList<QPair<double, double>>>> muiltRoute);
	
private:
	//·����
	QList<qint64> _computeredRouteLength;
	//·����
	QList<QPair<double, double>> _routePoint;
	//��������������·����Ϣ
	QList<RouteInfo> _allRoute;
	//���񷵻ؽ�������·��
	QList<QList<QList<QPair<double, double>>>> _muiltRoute;
	//����·������
	int _drawRouteCount;
	//�滮·�߹�·������
	QPair<double, double> _startPos;
	//�滮·�߹�·������
	QPair<double, double> _endPos;
	//·�����ƣ�������·���м�
	QString _routeName="";
	//·����ɫ���ⲿ����
	QString _lineColor;
	//��ɫ����
	static int _colorCount;


private:
	//ͼ�㻭ͼӦ��
	CTrackingLayer *m_pTrackingLayer;

	CTrackingGeomsItem* _pointItem;//��ɫ-������·����
	CTrackingGeomsItem* _blueLineItem;//��ɫ-�������滮·����·��
	CTrackingGeomsItem* _grayLineItem;//��ɫ-�������滮·�߸�·��
	CTrackingGeomsItem* _blueDotLineItem;//��ɫ-���������ߣ������㵽��·��
	CTrackingGeomsItem* _blueDotLineItem_2;//��ɫ-���������ߣ��յ㵽��·��

	QList<CTrackingTextItem* >  _allTextItem;//���Ƶ������ı�
	QList<CMultiGeometry3DExtension*> _3dPointList;//��ά����
	QList<CMultiGeometry3DExtension*> _3dLineList;//��ά����
	CGlobeControl *mpGlobeCtr;//��ά��ͼ
	CMapControl	*mpMapCtr;//��ά��ͼ
	
};
