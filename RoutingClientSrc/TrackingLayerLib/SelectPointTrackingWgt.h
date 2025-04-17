#pragma once

#include <QWidget>
#include <QMenu>
#include <QDoubleSpinBox>
#include <QPair>

#include "UILibrary/BaseDialogEx.h"

#include "RouteStrut.h"

#include "trackinglayerlib_global.h"

#include "qtplugins/globewidget.h"
#include "globecontrol/globecontrol.h"
#include "mapcontrol/mapcontrol.h"
#include "globecontrol/globecontrollistener.h"
#include "spatial3dengine/globecamera.h"

class SelectRouteWgt;
class RouteComputerAndPaint;

using namespace EarthView::World::Spatial2D::Controls;
using namespace EarthView::World::Spatial3D::Controls;

enum WgtMode
{
	MapPointSelect=0,
	PointLibrary,
	GaoSiInput
};

namespace Ui { class SelectPointTrackingWgt; };

class TRACKINGLAYERLIB_EXPORT SelectPointTrackingWgt : public BaseDialogEx, public CGlobeControlListener, public IMapControlListener
{
	Q_OBJECT

public:
	SelectPointTrackingWgt(QWidget *parent = Q_NULLPTR, bool usePtn = true, bool useMenu = true, bool isUseSpeed = false, bool isHaveStartPos = false);
	~SelectPointTrackingWgt();

	void setStartPos(QPair<double, double> startPos);

	QPair<double, double> getStartPos();
	QPair<double, double> getEndPos();

private:
	//初始化模式选择下拉框
	void initialModeSelectCom();
	//地图拾取方式界面初始化和更新
	void  initialTable();
	void  updateTable();
	//高斯坐标方式界面初始化
	void  initialTable_GaoSi();
	void updateTable_GaoSi();
	//点位库方式界面初始化
	void initial_PointLibWgt();


signals:
	void selectedRoute(RouteInfo routeInfo);

private slots:
	void Tracking_Clicked();//请求获取路线
	void AddPoint_Clicked();//添加途径点
	void DeletePoint_Clicked();//删除途径点
	void StartSelectPoint_Clicked();// 开始/结束拾取
	void ClearSelectPoint_Clicked();//清空途径点
	void SelectRoute_Clicked();//选择规划的路线
	void currentItemSlot(QTableWidgetItem* item);
	void customContextMenuSlot(const QPoint& p);
	void menuSlot(QAction* action);
	void slotServerNetError();

	//点位库选择模式下槽
	void slotAddPoint();
	void slotCustomContextMenu();

private:
	void reSetAndDrawPoint();

private:
	QList<QList<QPair<double, double>>> arrryToList(QByteArray arrry);
	QByteArray listToArrry(QList<QList<QPair<double, double>>> routePoint);
	//捕捉鼠标所在经纬度
	virtual ev_void onMousePosition(Real lat, Real lon, Real alt, ev_bool isValid) override;
	//virtual ev_bool onMouseMove(_in ev_int32 button, _in ev_int32 shift, _in ev_int32 x, _in ev_int32 y, _in ev_real64 geoX, _in ev_real64 geoY);

	virtual void onMousePosit(double lon, double lat) ;

protected:
	virtual bool eventFilter(QObject *obj, QEvent *event) override;
	virtual void closeEvent(QCloseEvent* event) override;

private:
	QWidget* _parent;
	//途径点编号
	int _passPointCount = 0;
	//起点，终点
	QPair<QDoubleSpinBox*, QDoubleSpinBox*> _startPointSpinBoxXy;
	QPair<QDoubleSpinBox*, QDoubleSpinBox*> _endPointSpinBoxXy;
	//记录所有途径点对应SpinBox
	QList<QPair<QDoubleSpinBox*, QDoubleSpinBox*>> _allPassPointSpinBoxXy;
	//当前鼠标所在点
	QPair<double, double> _currentPoint;
	//是否开始拾取
	bool _isSelectPoint = false;
	//规划过的路径
	QList<RouteInfo>_allRoute;
	//选择路线界面
	SelectRouteWgt* _selectRouteWgt;
	//路径计算和绘制
	RouteComputerAndPaint* _routeComputerAndPaint;
	bool _isUseSpeed;
	//已经有起点
	bool _isHaveStartPos;
	QPair<double, double> _startPos;
	//美化界面，菜单取代按钮
	QMenu* _menu;
	QAction* _deleteAction;
	QAction* _addAction;
	QAction* _clearAction;

	//选点模式
	WgtMode _mode;

	//高斯坐标选点模式下界面
	//起点，终点
	QPair<QLineEdit*, QLineEdit*> _startPointLineEditXy;
	QPair<QLineEdit*, QLineEdit*> _endPointLineEditXy;
	//记录所有途径点对应SpinBox
	QList<QPair<QLineEdit*, QLineEdit*>> _allPassPointLineEditXy;
	int _passPointCount_GaoSi=0;

	QPair<double, double> _endpos;

private:
	Ui::SelectPointTrackingWgt* ui;
};
