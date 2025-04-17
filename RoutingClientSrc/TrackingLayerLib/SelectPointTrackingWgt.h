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
	//��ʼ��ģʽѡ��������
	void initialModeSelectCom();
	//��ͼʰȡ��ʽ�����ʼ���͸���
	void  initialTable();
	void  updateTable();
	//��˹���귽ʽ�����ʼ��
	void  initialTable_GaoSi();
	void updateTable_GaoSi();
	//��λ�ⷽʽ�����ʼ��
	void initial_PointLibWgt();


signals:
	void selectedRoute(RouteInfo routeInfo);

private slots:
	void Tracking_Clicked();//�����ȡ·��
	void AddPoint_Clicked();//���;����
	void DeletePoint_Clicked();//ɾ��;����
	void StartSelectPoint_Clicked();// ��ʼ/����ʰȡ
	void ClearSelectPoint_Clicked();//���;����
	void SelectRoute_Clicked();//ѡ��滮��·��
	void currentItemSlot(QTableWidgetItem* item);
	void customContextMenuSlot(const QPoint& p);
	void menuSlot(QAction* action);
	void slotServerNetError();

	//��λ��ѡ��ģʽ�²�
	void slotAddPoint();
	void slotCustomContextMenu();

private:
	void reSetAndDrawPoint();

private:
	QList<QList<QPair<double, double>>> arrryToList(QByteArray arrry);
	QByteArray listToArrry(QList<QList<QPair<double, double>>> routePoint);
	//��׽������ھ�γ��
	virtual ev_void onMousePosition(Real lat, Real lon, Real alt, ev_bool isValid) override;
	//virtual ev_bool onMouseMove(_in ev_int32 button, _in ev_int32 shift, _in ev_int32 x, _in ev_int32 y, _in ev_real64 geoX, _in ev_real64 geoY);

	virtual void onMousePosit(double lon, double lat) ;

protected:
	virtual bool eventFilter(QObject *obj, QEvent *event) override;
	virtual void closeEvent(QCloseEvent* event) override;

private:
	QWidget* _parent;
	//;������
	int _passPointCount = 0;
	//��㣬�յ�
	QPair<QDoubleSpinBox*, QDoubleSpinBox*> _startPointSpinBoxXy;
	QPair<QDoubleSpinBox*, QDoubleSpinBox*> _endPointSpinBoxXy;
	//��¼����;�����ӦSpinBox
	QList<QPair<QDoubleSpinBox*, QDoubleSpinBox*>> _allPassPointSpinBoxXy;
	//��ǰ������ڵ�
	QPair<double, double> _currentPoint;
	//�Ƿ�ʼʰȡ
	bool _isSelectPoint = false;
	//�滮����·��
	QList<RouteInfo>_allRoute;
	//ѡ��·�߽���
	SelectRouteWgt* _selectRouteWgt;
	//·������ͻ���
	RouteComputerAndPaint* _routeComputerAndPaint;
	bool _isUseSpeed;
	//�Ѿ������
	bool _isHaveStartPos;
	QPair<double, double> _startPos;
	//�������棬�˵�ȡ����ť
	QMenu* _menu;
	QAction* _deleteAction;
	QAction* _addAction;
	QAction* _clearAction;

	//ѡ��ģʽ
	WgtMode _mode;

	//��˹����ѡ��ģʽ�½���
	//��㣬�յ�
	QPair<QLineEdit*, QLineEdit*> _startPointLineEditXy;
	QPair<QLineEdit*, QLineEdit*> _endPointLineEditXy;
	//��¼����;�����ӦSpinBox
	QList<QPair<QLineEdit*, QLineEdit*>> _allPassPointLineEditXy;
	int _passPointCount_GaoSi=0;

	QPair<double, double> _endpos;

private:
	Ui::SelectPointTrackingWgt* ui;
};
