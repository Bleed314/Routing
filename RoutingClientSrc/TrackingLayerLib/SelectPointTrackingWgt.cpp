#include "SelectPointTrackingWgt.h"

#include "ui_SelectPointTrackingWgt.h"

#include <QMessageBox>
#include <QComboBox>
#include <QLineEdit>
#include <QBuffer>
#include <QByteArray>

#include "GlobeMapCore/GlobeMapManager.h"
#include "mapcontrol/mapcontrol.h"
#include "GlobeMapCore/CMapWidget.h"

#include "UtilityLibrary/EntityInfoManager.h"
#include "LibSimDataTrans/EVRTIReceiver.h"
#include "LibSimDataTrans/LocalSimDataTrans.h"

#include "WritClientLibrary/databaseoperatorsingleton.h"
#include "GroupTrainData/GroupTrainDataInstance.h"

#include "SelectRouteWgt.h"
#include "RouteComputerAndPaint.h"
#include "CooTrans.h"

using namespace EarthView::World::Spatial::Atlas;
using namespace EarthView::World::Spatial3D::Controls;
using namespace EarthView::World::Spatial2D::Controls;

#pragma comment(lib, "LibSimDataTrans.lib")
#pragma comment(lib, "GlobeMapCore.lib")
#pragma comment(lib, "WritClientLibrary.lib")
#pragma comment(lib, "GroupTrainData.lib")

SelectPointTrackingWgt::SelectPointTrackingWgt(QWidget *parent, bool usePtn, bool useMenu ,bool isUseSpeed, bool isHaveStartPos)
	: BaseDialogEx(parent),ui(new Ui::SelectPointTrackingWgt), _selectRouteWgt(NULL)
{
	QWidget* widget = new QWidget;
	ui->setupUi(widget);
	setCentralWidget(widget);
	setWindowTitle(QString::fromLocal8Bit("路径规划"));

	connect(ui->pBtn_track, SIGNAL(clicked()), this, SLOT(Tracking_Clicked()));
	connect(ui->pBtn_add, SIGNAL(clicked()), this, SLOT(AddPoint_Clicked()));
	connect(ui->pushButton_delete, SIGNAL(clicked()), this, SLOT(DeletePoint_Clicked()));
	connect(ui->pushButton_start, SIGNAL(clicked()), this, SLOT(StartSelectPoint_Clicked()));
	connect(ui->pushButton_clear, SIGNAL(clicked()), this, SLOT(ClearSelectPoint_Clicked()));
	connect(ui->tableWidget, &QTableWidget::itemClicked, this, &SelectPointTrackingWgt::currentItemSlot);
	connect(ui->pushButton_select, SIGNAL(clicked()), this, SLOT(SelectRoute_Clicked()));

	_isUseSpeed = isUseSpeed;
	_isHaveStartPos = isHaveStartPos;

	if (!usePtn)
	{
		ui->pushButton_clear->hide();
		ui->pBtn_add->hide();
		ui->pushButton_delete->hide();
	}

	if (!useMenu)
	{
		ui->tableWidget->setContextMenuPolicy(Qt::CustomContextMenu);
		_menu = new QMenu(this);
		connect(_menu, &QMenu::triggered, this, &SelectPointTrackingWgt::menuSlot);
		_deleteAction = new QAction(QStringLiteral("删除途径点"));
		_deleteAction->setIcon(QPixmap(QString::fromLocal8Bit("%1%2").arg(QApplication::applicationDirPath()).arg(QStringLiteral("/../userData/data/widgetImage/widget/routePlanning/delete.png"))));
		_addAction = new QAction(QStringLiteral("添加途径点"));
		_addAction->setIcon(QPixmap(QString::fromLocal8Bit("%1%2").arg(QApplication::applicationDirPath()).arg(QStringLiteral("/../userData/data/widgetImage/widget/routePlanning/add.png"))));
		_clearAction = new QAction(QStringLiteral("清空途径点"));
		_clearAction->setIcon(QPixmap(QString::fromLocal8Bit("%1%2").arg(QApplication::applicationDirPath()).arg(QStringLiteral("/../userData/data/widgetImage/widget/routePlanning/clear.png"))));
		_menu->addAction(_deleteAction);
		_menu->addAction(_addAction);
		_menu->addAction(_clearAction);
		connect(ui->tableWidget, &QTableWidget::customContextMenuRequested, this, &SelectPointTrackingWgt::customContextMenuSlot);
	}

	_parent = parent;
	_allRoute.clear();

	qApp->installEventFilter(this);
	qApp->installEventFilter(GlobeMapManager::GetInstance()->GetMapWidget());

	_mode = MapPointSelect;
	initialModeSelectCom();
	initialTable();
	initial_PointLibWgt();
	initialTable_GaoSi();

	//获取跟踪层对象指针
	CGlobeControl *mpGlobeCtr = GlobeMapManager::GetInstance()->GetGlobeWidget()->getGlobeControl();
	CMapControl *mpMapCtr = GlobeMapManager::GetInstance()->GetMapWidget()->getMapControl();
	mpGlobeCtr->addGlobeControlListener(this);
	mpMapCtr->addMapControlListener(this);
	_routeComputerAndPaint = new RouteComputerAndPaint(mpGlobeCtr, mpMapCtr);
	connect(_routeComputerAndPaint, &RouteComputerAndPaint::sigServerNetError, this, &SelectPointTrackingWgt::slotServerNetError);
}

SelectPointTrackingWgt::~SelectPointTrackingWgt()
{
	_routeComputerAndPaint->ClearRoute();
}

void SelectPointTrackingWgt::setStartPos(QPair<double, double> startPos)
{
	_startPos = startPos;
}

QPair<double, double> SelectPointTrackingWgt::getStartPos()
{
	QPair<double, double> pos;
	if (_startPointSpinBoxXy.first)
	{
		pos.first = _startPointSpinBoxXy.first->value();
	}
	if (_startPointSpinBoxXy.second)
	{
		pos.second = _startPointSpinBoxXy.second->value();
	}
	return pos;
}

QPair<double, double> SelectPointTrackingWgt::getEndPos()
{
	return _endpos;
}

void SelectPointTrackingWgt::initialModeSelectCom()
{
	ui->comboBox_mode->addItems(QStringList()<<QString::fromLocal8Bit("地图点选") << QString::fromLocal8Bit("点位库选取")
		<< QString::fromLocal8Bit("高斯坐标输入"));
	connect(ui->comboBox_mode, &QComboBox::currentTextChanged,[&](const QString &text)
	{
		if (text== QString::fromLocal8Bit("地图点选"))
		{
			_mode = MapPointSelect;
			ui->pBtn_add->setVisible(true);
			ui->pushButton_delete->setVisible(true);
			ui->pushButton_start->setVisible(true);
			ui->pushButton_clear->setVisible(true);
		}
		else if (text == QString::fromLocal8Bit("点位库选取"))
		{
			_mode = PointLibrary;
			ui->pBtn_add->setVisible(false);
			ui->pushButton_delete->setVisible(false);
			ui->pushButton_start->setVisible(false);
			ui->pushButton_clear->setVisible(false);
		}
		else if (text == QString::fromLocal8Bit("高斯坐标输入"))
		{
			_mode = GaoSiInput;
			ui->pBtn_add->setVisible(true);
			ui->pushButton_delete->setVisible(true);
			ui->pushButton_clear->setVisible(true);
		}
		ClearSelectPoint_Clicked();
		ui->stackedWidget->setCurrentIndex(_mode);
	});
}

void SelectPointTrackingWgt::initialTable()
{
	ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	ui->tableWidget->setColumnCount(3);
	ui->tableWidget->setRowCount(2);
	ui->tableWidget->setHorizontalHeaderLabels(QStringList() << QStringLiteral("名称") << QStringLiteral("经度") << QStringLiteral("纬度"));
	ui->tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
	ui->tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui->tableWidget->setCurrentItem(ui->tableWidget->item(0, 0));

	QDoubleSpinBox* startX = new QDoubleSpinBox();
	connect(startX, &QDoubleSpinBox::customContextMenuRequested, this, &SelectPointTrackingWgt::customContextMenuSlot);

	startX->setDecimals(7);
	//startX->installEventFilter(this);
	startX->setMaximum(999999.9999999);
	QDoubleSpinBox* startY = new QDoubleSpinBox();
	connect(startY, &QDoubleSpinBox::customContextMenuRequested, this, &SelectPointTrackingWgt::customContextMenuSlot);
	startY->setDecimals(7);
	startY->installEventFilter(this);
	startY->setMaximum(999999.9999999);
	QDoubleSpinBox* endX = new QDoubleSpinBox();
	connect(endX, &QDoubleSpinBox::customContextMenuRequested, this, &SelectPointTrackingWgt::customContextMenuSlot);
	endX->setDecimals(7);
	endX->installEventFilter(this);
	endX->setMaximum(999999.9999999);
	QDoubleSpinBox* endY = new QDoubleSpinBox();
	connect(startY, &QDoubleSpinBox::customContextMenuRequested, this, &SelectPointTrackingWgt::customContextMenuSlot);
	endY->setDecimals(7);
	endY->installEventFilter(this);
	endY->setMaximum(999999.9999999);
	_startPointSpinBoxXy.first = startX;
	_startPointSpinBoxXy.second = startY;
	_endPointSpinBoxXy.first = endX;
	_endPointSpinBoxXy.second = endY;

	if (!_isHaveStartPos)
	{
		QTableWidgetItem* item1 = new QTableWidgetItem(QStringLiteral("起点"));
		item1->setTextAlignment(Qt::AlignHCenter);
		ui->tableWidget->setItem(0, 0, item1);
		ui->tableWidget->setCellWidget(0, 1, startX);
		ui->tableWidget->setCellWidget(0, 2, startY);

		QTableWidgetItem* item2 = new QTableWidgetItem(QStringLiteral("终点"));
		item2->setTextAlignment(Qt::AlignHCenter);
		ui->tableWidget->setItem(1, 0, item2);
		QDoubleSpinBox* ex = endX;
		QDoubleSpinBox* ey = endY;
		ui->tableWidget->setCellWidget(1, 1, ex);
		ui->tableWidget->setCellWidget(1, 2, ey);
	}
	else
	{
		QTableWidgetItem* item1 = new QTableWidgetItem(QStringLiteral("途经点"));
		item1->setTextAlignment(Qt::AlignHCenter);
		ui->tableWidget->setItem(0, 0, item1);
		ui->tableWidget->setCellWidget(0, 1, startX);
		ui->tableWidget->setCellWidget(0, 2, startY);
	}

}

void SelectPointTrackingWgt::initialTable_GaoSi()
{
	ui->tableWidget_2->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	ui->tableWidget_2->setColumnCount(3);
	ui->tableWidget_2->setRowCount(2);
	ui->tableWidget_2->setHorizontalHeaderLabels(QStringList() << QStringLiteral("名称") << QStringLiteral("经度") << QStringLiteral("纬度"));
	ui->tableWidget_2->setSelectionMode(QAbstractItemView::SingleSelection);
	ui->tableWidget_2->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui->tableWidget_2->setCurrentItem(ui->tableWidget->item(0, 0));

	QLineEdit* startX = new QLineEdit();
	connect(startX, &QLineEdit::customContextMenuRequested, this, &SelectPointTrackingWgt::customContextMenuSlot);

	QLineEdit* startY = new QLineEdit();
	connect(startY, &QLineEdit::customContextMenuRequested, this, &SelectPointTrackingWgt::customContextMenuSlot);

	QLineEdit* endX = new QLineEdit();
	connect(endX, &QLineEdit::customContextMenuRequested, this, &SelectPointTrackingWgt::customContextMenuSlot);

	QLineEdit* endY = new QLineEdit();
	connect(startY, &QLineEdit::customContextMenuRequested, this, &SelectPointTrackingWgt::customContextMenuSlot);

	_startPointLineEditXy.first = startX;
	_startPointLineEditXy.second = startY;
	_endPointLineEditXy.first = endX;
	_endPointLineEditXy.second = endY;

	QTableWidgetItem* item1 = new QTableWidgetItem(QStringLiteral("起点"));
	item1->setTextAlignment(Qt::AlignHCenter);
	ui->tableWidget_2->setItem(0, 0, item1);
	ui->tableWidget_2->setCellWidget(0, 1, startX);
	ui->tableWidget_2->setCellWidget(0, 2, startY);

	QTableWidgetItem* item2 = new QTableWidgetItem(QStringLiteral("终点"));
	item2->setTextAlignment(Qt::AlignHCenter);
	ui->tableWidget_2->setItem(1, 0, item2);
	ui->tableWidget_2->setCellWidget(1, 1, endX);
	ui->tableWidget_2->setCellWidget(1, 2, endY);
}

void SelectPointTrackingWgt::updateTable_GaoSi()
{
	//起点
	QTableWidgetItem* item1 = new QTableWidgetItem(QStringLiteral("起点"));
	item1->setTextAlignment(Qt::AlignHCenter);
	ui->tableWidget_2->setItem(0, 0, item1);

	//已有途径点
	if (_passPointCount_GaoSi > 0)
	{
		for (int it = 0; it != _allPassPointLineEditXy.size(); it++)
		{
			QString pointName = QString::fromLocal8Bit("途径点%1").arg(QString::number(it + 1));
			QTableWidgetItem* passItem = new QTableWidgetItem(pointName);
			passItem->setTextAlignment(Qt::AlignHCenter);
			ui->tableWidget_2->setItem(it + 1, 0, passItem);
		}
	}

	//终点
	QTableWidgetItem* item2 = new QTableWidgetItem(QStringLiteral("终点"));
	item2->setTextAlignment(Qt::AlignHCenter);
	ui->tableWidget_2->setItem(1 + _passPointCount_GaoSi, 0, item2);
}

void SelectPointTrackingWgt::updateTable()
{
	if (!_isHaveStartPos)
	{
		//起点
		QTableWidgetItem* item1 = new QTableWidgetItem(QStringLiteral("起点"));
		item1->setTextAlignment(Qt::AlignHCenter);
		ui->tableWidget->setItem(0, 0, item1);

		//已有途径点
		if (_passPointCount > 0)
		{
			for (int it = 0; it != _allPassPointSpinBoxXy.size(); it++)
			{
				QString pointName = QString::fromLocal8Bit("途径点%1").arg(QString::number(it + 1));
				QTableWidgetItem* passItem = new QTableWidgetItem(pointName);
				passItem->setTextAlignment(Qt::AlignHCenter);
				ui->tableWidget->setItem(it + 1, 0, passItem);
			}
		}

		//终点
		QTableWidgetItem* item2 = new QTableWidgetItem(QStringLiteral("终点"));
		item2->setTextAlignment(Qt::AlignHCenter);
		ui->tableWidget->setItem(1 + _passPointCount, 0, item2);
	}
	else
	{
		if (_passPointCount > 0)
		{
			for (int it = 0; it != _allPassPointSpinBoxXy.size(); it++)
			{
				QString pointName = QString::fromLocal8Bit("途径点%1").arg(QString::number(it + 1));
				QTableWidgetItem* passItem = new QTableWidgetItem(pointName);
				passItem->setTextAlignment(Qt::AlignHCenter);
				ui->tableWidget->setItem(it + 1, 0, passItem);
			}
		}
	}

}

void SelectPointTrackingWgt::initial_PointLibWgt()
{
	QList<Location> allLocation = DataBaseOperatorSingleton::getInstance()->GetAllLocaltions();
	for (Location location : allLocation)
	{
		ui->comboBox->addItem(location.locationName, QString::number(location.lon, 'f', 10) + "#" + QString::number(location.lat, 'f', 10));
	}

	connect(ui->pushButton_add, &QPushButton::clicked, this, &SelectPointTrackingWgt::slotAddPoint);
	ui->listWidget->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(ui->listWidget, &QListWidget::customContextMenuRequested, this, &SelectPointTrackingWgt::slotCustomContextMenu);
}

void SelectPointTrackingWgt::Tracking_Clicked()
{
	if (_mode==PointLibrary && ui->listWidget->count()<2&&!_isHaveStartPos)
	{
		QMessageBox::information(this, QStringLiteral("提示"), QStringLiteral("请添加至少两个点位！"));
		return;
	}
	if (_mode==GaoSiInput)
	{
		if (_allPassPointLineEditXy.size()<2)
		{
			QMessageBox::information(this, QStringLiteral("提示"), QStringLiteral("请输入至少两个高斯坐标！"));
			return;
		}
		QList <QPair<double, double>> allPoint;
		allPoint.push_back((GaussXYtoLL(_startPointLineEditXy.first->text().toDouble(), _startPointLineEditXy.second->text().toDouble())));
		for (int i = 0; i != _allPassPointSpinBoxXy.size(); i++)
		{
			allPoint.push_back((GaussXYtoLL(_allPassPointLineEditXy[i].first->text().toDouble(), _allPassPointLineEditXy[i].second->text().toDouble())));
		}
		allPoint.push_back(GaussXYtoLL(_endPointLineEditXy.first->text().toDouble(), _endPointLineEditXy.second->text().toDouble()));
		_routeComputerAndPaint->setRoutePoint(allPoint);
	}
	_routeComputerAndPaint->computerRouteFromServer();
	_routeComputerAndPaint->DrawRoute();
	_allRoute = _routeComputerAndPaint->getTrackingRouteInfo();
}

void SelectPointTrackingWgt::AddPoint_Clicked()
{
	if (_mode==MapPointSelect)
	{
		//本次添加途径点
		QDoubleSpinBox* BoxJ_ = new QDoubleSpinBox;
		connect(BoxJ_, &QDoubleSpinBox::customContextMenuRequested, this, &SelectPointTrackingWgt::customContextMenuSlot);
		BoxJ_->setDecimals(7);
		QDoubleSpinBox* BoxW_ = new QDoubleSpinBox;
		connect(BoxW_, &QDoubleSpinBox::customContextMenuRequested, this, &SelectPointTrackingWgt::customContextMenuSlot);
		BoxW_->setDecimals(7);
		BoxJ_->setMaximum(999999.9999999);
		BoxW_->setMaximum(999999.9999999);
		BoxJ_->installEventFilter(this);
		BoxW_->installEventFilter(this);
		QPair<QDoubleSpinBox*, QDoubleSpinBox*> pair;
		pair.first = BoxJ_;
		pair.second = BoxW_;
		_allPassPointSpinBoxXy.push_back(pair);

		_passPointCount++;

		ui->tableWidget->insertRow(_passPointCount);
		ui->tableWidget->setCellWidget(_passPointCount, 1, BoxJ_);
		ui->tableWidget->setCellWidget(_passPointCount, 2, BoxW_);

		updateTable();
	}
	else if (_mode == GaoSiInput)
	{
		//本次添加途径点
		QLineEdit* BoxJ_ = new QLineEdit;
		connect(BoxJ_, &QLineEdit::customContextMenuRequested, this, &SelectPointTrackingWgt::customContextMenuSlot);
		QLineEdit* BoxW_ = new QLineEdit;
		connect(BoxW_, &QLineEdit::customContextMenuRequested, this, &SelectPointTrackingWgt::customContextMenuSlot);
		QPair<QLineEdit*, QLineEdit*> pair;
		pair.first = BoxJ_;
		pair.second = BoxW_;
		_allPassPointLineEditXy.push_back(pair);

		_passPointCount_GaoSi++;

		ui->tableWidget_2->insertRow(_passPointCount_GaoSi);
		ui->tableWidget_2->setCellWidget(_passPointCount_GaoSi, 1, BoxJ_);
		ui->tableWidget_2->setCellWidget(_passPointCount_GaoSi, 2, BoxW_);

		updateTable_GaoSi();
	}

}

void SelectPointTrackingWgt::DeletePoint_Clicked()
{
	if (_mode==MapPointSelect)
	{
		int currentRow = ui->tableWidget->currentRow();
		//当前表格未选中行
		if (currentRow < 0)
		{
			QMessageBox::information(this, QStringLiteral("提示"), QStringLiteral("删除失败,请先选择要删除的点！"));
			return;
		}
		if (!_isHaveStartPos)
		{
			//只有起点终点时，不允许删除
			if (_passPointCount == 0)
			{
				QMessageBox::information(this, QStringLiteral("提示"), QStringLiteral("删除失败,最少保留起点和终点！"));
				return;
			}

			//删除的是起点
			if (currentRow == 0)
			{
				//把途径点1当作新的起点
				_startPointSpinBoxXy.first = static_cast<QDoubleSpinBox*>(ui->tableWidget->cellWidget(1, 1));
				_startPointSpinBoxXy.second = static_cast<QDoubleSpinBox*>(ui->tableWidget->cellWidget(1, 2));

				_allPassPointSpinBoxXy.removeAt(0);
				_passPointCount--;

				ui->tableWidget->removeRow(currentRow);
				updateTable();
			}
			//删除的是终点
			if (currentRow == ui->tableWidget->rowCount() - 1)
			{
				_endPointSpinBoxXy.first = static_cast<QDoubleSpinBox*>(ui->tableWidget->cellWidget(_passPointCount, 1));
				_endPointSpinBoxXy.second = static_cast<QDoubleSpinBox*>(ui->tableWidget->cellWidget(_passPointCount, 2));

				_allPassPointSpinBoxXy.removeAt(_allPassPointSpinBoxXy.size() - 1);
				_passPointCount--;

				ui->tableWidget->removeRow(currentRow);
				updateTable();
			}

			//删除得是途径点
			if (currentRow > 0 && ui->tableWidget->currentRow() <= _passPointCount)
			{
				_allPassPointSpinBoxXy.removeAt(currentRow - 1);
				_passPointCount--;

				ui->tableWidget->removeRow(currentRow);
				updateTable();
			}
		}
		else
		{
			if (currentRow == 0)
			{
				QMessageBox::information(this, QStringLiteral("提示"), QStringLiteral("最少保留一个途经点！"));
				return;
			}
			_allPassPointSpinBoxXy.removeAt(currentRow - 1);
			_passPointCount--;

			ui->tableWidget->removeRow(currentRow);
			updateTable();
		}
	
	}
	else if (_mode == GaoSiInput)
	{
		int currentRow = ui->tableWidget_2->currentRow();
		//当前表格未选中行
		if (currentRow < 0)
		{
			QMessageBox::information(this, QStringLiteral("提示"), QStringLiteral("删除失败,请先选择要删除的点！"));
			return;
		}

		//只有起点终点时，不允许删除
		if (_passPointCount_GaoSi == 0)
		{
			QMessageBox::information(this, QStringLiteral("提示"), QStringLiteral("删除失败,最少保留起点和终点！"));
			return;
		}

		//删除的是起点
		if (currentRow == 0)
		{
			//把途径点1当作新的起点
			_startPointLineEditXy.first = static_cast<QLineEdit*>(ui->tableWidget_2->cellWidget(1, 1));
			_startPointLineEditXy.second = static_cast<QLineEdit*>(ui->tableWidget_2->cellWidget(1, 2));

			_allPassPointLineEditXy.removeAt(0);
			_passPointCount_GaoSi--;

			ui->tableWidget_2->removeRow(currentRow);
			updateTable();
		}
		//删除的是终点
		if (currentRow == ui->tableWidget_2->rowCount() - 1)
		{
			_endPointLineEditXy.first = static_cast<QLineEdit*>(ui->tableWidget_2->cellWidget(_passPointCount_GaoSi, 1));
			_endPointLineEditXy.second = static_cast<QLineEdit*>(ui->tableWidget_2->cellWidget(_passPointCount_GaoSi, 2));

			_allPassPointLineEditXy.removeAt(_allPassPointSpinBoxXy.size() - 1);
			_passPointCount_GaoSi--;

			ui->tableWidget_2->removeRow(currentRow);
			updateTable_GaoSi();
		}

		//删除得是途径点
		if (currentRow > 0 && ui->tableWidget_2->currentRow() <= _passPointCount_GaoSi)
		{
			_allPassPointSpinBoxXy.removeAt(currentRow - 1);
			_passPointCount_GaoSi--;

			ui->tableWidget_2->removeRow(currentRow);
			updateTable_GaoSi();
		}
	}

	//再重新绘点
	reSetAndDrawPoint();
}

void SelectPointTrackingWgt::StartSelectPoint_Clicked()
{
	if (ui->pushButton_start->text() == QStringLiteral("开启拾取"))
	{
		ui->pushButton_start->setText(QStringLiteral("关闭拾取"));
		_parent->setCursor(Qt::CrossCursor);
		_isSelectPoint = true;
		return;
	}

	if (ui->pushButton_start->text() == QStringLiteral("关闭拾取"))
	{
		ui->pushButton_start->setText(QStringLiteral("开启拾取"));
		_parent->setCursor(Qt::ArrowCursor);
		_isSelectPoint = false;
		return;
	}
}

void SelectPointTrackingWgt::ClearSelectPoint_Clicked()
{
	if (_mode==MapPointSelect)
	{
		if (_passPointCount == 0)
		{
			return;
		}

		//清空内存和界面
		_allPassPointSpinBoxXy.clear();
		for (int i = 1; i <= _passPointCount;i++)
		{
			ui->tableWidget->removeRow(1);
		}

		_passPointCount = 0;

		//重新绘点
		reSetAndDrawPoint();

		_allRoute.clear();
	}
	else if (_mode == GaoSiInput)
	{
		if (_passPointCount_GaoSi == 0)
		{
			return;
		}

		//清空内存和界面
		_allPassPointLineEditXy.clear();
		for (int i = 1; i <= _passPointCount_GaoSi;i++)
		{
			ui->tableWidget_2->removeRow(1);
		}

		_passPointCount_GaoSi = 0;
		//重新绘点
		reSetAndDrawPoint();
		_allRoute.clear();
	}
	else if (_mode == PointLibrary)
	{
		ui->listWidget->clear();
	}

}

void SelectPointTrackingWgt::SelectRoute_Clicked()
{
	if (_allRoute.size() == 0)
	{
		QMessageBox::information(this, QStringLiteral("提示"), QStringLiteral("选择路线前请先规划出路线！"));
		return;
	}
	if (!_selectRouteWgt)
	{
		_selectRouteWgt = new SelectRouteWgt(this, _isUseSpeed);
		connect(_selectRouteWgt, &SelectRouteWgt::selectRoute, [&](RouteInfo routeInfo)
		{
			emit selectedRoute(routeInfo);
			_endpos.first = _endPointSpinBoxXy.first->value();
			_endpos.second = _endPointSpinBoxXy.second->value();

			GroupTrainDataInstance::getInstance()->set2dDrawRoute(routeInfo.routePoint);
			QString usrname = DataBaseOperatorSingleton::getInstance()->getCurrentUser();
			//通知绘制插件在二维上绘制路线
			QString interName = "baseInter.FederationInteract.TwoDRoute";
			QHash<QString, QVariant> params;
			params.insert("otherinfo", "clear_my"+ usrname);
			bool isSuccess = EVRTIReceiver::GetInstance()->SendInterToEngine(interName, params);
		});
	}

	_selectRouteWgt->setInfo(_allRoute);
	_selectRouteWgt->show();
}

void SelectPointTrackingWgt::currentItemSlot(QTableWidgetItem * item)
{
	int currentRow = item->row();
	ui->tableWidget->cellWidget(currentRow, 1)->setFocus();

	ui->pushButton_start->setText(QStringLiteral("关闭拾取"));
	_parent->setCursor(Qt::CrossCursor);
	_isSelectPoint = true;
}

void SelectPointTrackingWgt::customContextMenuSlot(const QPoint & p)
{
	if (_mode==MapPointSelect)
	{
		if (ui->tableWidget->rowCount() > 2)
		{
			_deleteAction->setEnabled(true);
			_clearAction->setEnabled(true);
		}
		if (ui->tableWidget->rowCount() == 2)
		{
			_deleteAction->setEnabled(false);
			_clearAction->setEnabled(false);
		}
	}
	else if (_mode == GaoSiInput)
	{
		if (ui->tableWidget_2->rowCount() > 2)
		{
			_deleteAction->setEnabled(true);
			_clearAction->setEnabled(true);
		}
		if (ui->tableWidget_2->rowCount() == 2)
		{
			_deleteAction->setEnabled(false);
			_clearAction->setEnabled(false);
		}
	}

	_menu->exec(QCursor::pos());
}

void SelectPointTrackingWgt::menuSlot(QAction * action)
{
	if (action == _clearAction)
	{
		ClearSelectPoint_Clicked();
	}
	if (action == _addAction)
	{
		AddPoint_Clicked();
	}
	if (action == _deleteAction)
	{
		DeletePoint_Clicked();
	}
}

void SelectPointTrackingWgt::slotServerNetError()
{
	QMessageBox::information(this, QStringLiteral("错误"), QStringLiteral("访问路径规划服务失败！"));
	return;
}

void SelectPointTrackingWgt::slotAddPoint()
{
	QString posText = ui->comboBox->currentText();
	if (ui->listWidget->count() > 0)
	{
		QString lastPoint = ui->listWidget->item(ui->listWidget->count() - 1)->text();
		if (posText == lastPoint)
		{
			QMessageBox::information(nullptr, QStringLiteral("提示"), QStringLiteral("连续途经点不可以设置成相同点位！"));
			return;
		}
	}


	QString x = ui->comboBox->currentData().toString().split("#").at(0);
	QString y = ui->comboBox->currentData().toString().split("#").at(1);
	//double z = MathUtility::GetInstance()->GetClampAltitude(x.toDouble(), y.toDouble());
	QString pos = x + "#" + y;/*+ QString::number(z, 'f', 10);*/

	QListWidgetItem* item = new QListWidgetItem();
	item->setText(posText);
	item->setData(Qt::UserRole, pos);
	ui->listWidget->addItem(item);

	if (ui->listWidget->count()>1)
	{
		reSetAndDrawPoint();
	}
}

void SelectPointTrackingWgt::slotCustomContextMenu()
{
	QListWidgetItem* currentItem = ui->listWidget->currentItem();
	if (!currentItem)
	{
		QMessageBox::information(nullptr, QStringLiteral("提示"), QStringLiteral("请先选中节点"));
		return;
	}

	QMenu* menu = new QMenu(this);
	QAction* deletePointAction = new QAction(QString::fromLocal8Bit("删除该点"), menu);
	connect(deletePointAction, &QAction::triggered, [&](bool trig)
	{
		ui->listWidget->takeItem(ui->listWidget->row(currentItem));
	});
	menu->addAction(deletePointAction);
	menu->exec(QCursor::pos());
}

void SelectPointTrackingWgt::reSetAndDrawPoint()
{
	QList <QPair<double, double>> allPoint;
	if (_mode == MapPointSelect&&!_isHaveStartPos)
	{
		allPoint.push_back(QPair<double, double>(_startPointSpinBoxXy.first->value(), _startPointSpinBoxXy.second->value()));
		for (int i = 0; i != _allPassPointSpinBoxXy.size(); i++)
		{
			allPoint.push_back(QPair<double, double>(_allPassPointSpinBoxXy.at(i).first->value(), _allPassPointSpinBoxXy.at(i).second->value()));
		}
		allPoint.push_back(QPair<double, double>(_endPointSpinBoxXy.first->value(), _endPointSpinBoxXy.second->value()));
	}
	else if (_mode == MapPointSelect&&_isHaveStartPos)
	{
		allPoint.push_back(_startPos);
		allPoint.push_back(QPair<double, double>(_startPointSpinBoxXy.first->value(), _startPointSpinBoxXy.second->value()));
		for (int i = 0; i != _allPassPointSpinBoxXy.size(); i++)
		{
			allPoint.push_back(QPair<double, double>(_allPassPointSpinBoxXy.at(i).first->value(), _allPassPointSpinBoxXy.at(i).second->value()));
		}
	}
	else if(_mode == PointLibrary)
	{
		for (int i = 0;i < ui->listWidget->count();i++)
		{
			QStringList posLst = ui->listWidget->item(i)->data(Qt::UserRole).toString().split("#");
			if (posLst.size()!=2)
			{
				continue;
			}
			double x = posLst[0].toDouble();
			double y = posLst[1].toDouble();
			allPoint.push_back(QPair<double,double>(x,y));
		}
	}
	else if(_mode == GaoSiInput)
	{
		return;
		//allPoint.push_back(QPair<double, double>(_startPointLineEditXy.first->text().toDouble, _startPointSpinBoxXy.second->value()));
		//for (int i = 0; i != _allPassPointSpinBoxXy.size(); i++)
		//{
		//	allPoint.push_back(QPair<double, double>(_allPassPointSpinBoxXy.at(i).first->value(), _allPassPointSpinBoxXy.at(i).second->value()));
		//}
		//allPoint.push_back(QPair<double, double>(_endPointSpinBoxXy.first->value(), _endPointSpinBoxXy.second->value()));
	}

	//通知绘制插件在二维上绘制路线
	QString usrname = DataBaseOperatorSingleton::getInstance()->getCurrentUser();
	QString interName = "baseInter.FederationInteract.TwoDRoute";
	QHash<QString, QVariant> params;
	QString start = QString::number(_startPointSpinBoxXy.first->value(),'f',10)+","+ QString::number(_startPointSpinBoxXy.second->value(), 'f',10);
	QString end = QString::number(_endPointSpinBoxXy.first->value(), 'f', 10) + "," + QString::number(_endPointSpinBoxXy.second->value(), 'f', 10);
	QString pos = QString("point%1#"+start+"#"+end).arg(usrname);
	params.insert("otherinfo", pos);
	bool isSuccess = EVRTIReceiver::GetInstance()->SendInterToEngine(interName, params);

	_routeComputerAndPaint->setRoutePoint(allPoint);
	_routeComputerAndPaint->drawPoint();
	_routeComputerAndPaint->drawPoint_3D();
}


QList<QList<QPair<double, double>>> SelectPointTrackingWgt::arrryToList(QByteArray arrry)
{
	QList<QList<QPair<double, double>>>  pointLst;
	if (arrry.isEmpty()) return QList<QList<QPair<double, double>>>();

	QBuffer deserial_buffer(&arrry);
	deserial_buffer.open(QIODevice::ReadOnly);
	QDataStream in_stream(&deserial_buffer);
	in_stream.setVersion(QDataStream::Qt_DefaultCompiledVersion);
	in_stream >> pointLst;
	deserial_buffer.close();
	return pointLst;
}

QByteArray SelectPointTrackingWgt::listToArrry(QList<QList<QPair<double, double>>> routePoint)
{
	QByteArray serial_array;
	QBuffer serial_buffer(&serial_array);
	serial_buffer.open(QIODevice::WriteOnly);
	QDataStream out_stream(&serial_buffer);
	out_stream.setVersion(QDataStream::Qt_DefaultCompiledVersion);

	out_stream << routePoint;
	serial_buffer.close();
	return serial_array;
}

ev_void SelectPointTrackingWgt::onMousePosition(Real lat, Real lon, Real alt, ev_bool isValid)
{
	if (!isValid)
	{
		return;
	}
	if (!_isSelectPoint)
	{
		return;
	}
	if (GlobeMapManager::GetInstance()->GetGlobeWidget()->getGlobeControl() && 
		GlobeMapManager::GetInstance()->GetGlobeWidget()->getGlobeControl()->getCamera() 
		&& (CGlobeCamera*)GlobeMapManager::GetInstance()->GetGlobeWidget()->getGlobeControl()->getCamera())
	{
		onMousePosit(lon, lat);
	}
}

void SelectPointTrackingWgt::onMousePosit(double lon, double lat)
{
	_currentPoint.first = lon;
	_currentPoint.second = lat;
}

bool SelectPointTrackingWgt::eventFilter(QObject * obj, QEvent * event)
{
	if (event->type() == QEvent::MouseButtonDblClick)
	{
		//双击结束拾取，在地图上画出该点
		if (_isSelectPoint)
		{
			//不是在地图上点击则返回
			QWidget *currentDblClickWgt = qobject_cast<QWidget*>(obj);
			//if (currentDblClickWgt != _parent)
			//{
			//	return QObject::eventFilter(obj, event);
			//}

			int currentRow = ui->tableWidget->currentRow();
			if (currentRow==-1)
			{
				currentRow = 0;
			}
			JWG currentGes = GroupTrainDataInstance::getInstance()->getCurrentGes();
			qobject_cast<QDoubleSpinBox*>(ui->tableWidget->cellWidget(currentRow, 1))->setValue(currentGes.X);
			qobject_cast<QDoubleSpinBox*>(ui->tableWidget->cellWidget(currentRow, 2))->setValue(currentGes.Y);

			//重新画点
			reSetAndDrawPoint();
		}
	}

	if (event->type() == QEvent::FocusIn)
	{
		QDoubleSpinBox *currentSpinBox = qobject_cast<QDoubleSpinBox*>(obj);
		if (!currentSpinBox)
		{
			return QObject::eventFilter(obj, event);
		}

		for (int row = 0; row < ui->tableWidget->rowCount(); row++)
		{
			if (currentSpinBox == ui->tableWidget->cellWidget(row, 1) || currentSpinBox == ui->tableWidget->cellWidget(row, 2))
			{
				ui->tableWidget->setCurrentItem(ui->tableWidget->item(row, 0));
				break;
			}
		}

		if (this->isVisible())
		{
			ui->pushButton_start->setText(QStringLiteral("关闭拾取"));
			_parent->setCursor(Qt::CrossCursor);
			_isSelectPoint = true;
		}

	}


	if (event->type() == QEvent::ContextMenu)
	{
		QDoubleSpinBox *currentSpinBox = qobject_cast<QDoubleSpinBox*>(obj);
		if (currentSpinBox)
		{
			customContextMenuSlot(QCursor::pos());
			return true;
		}
	}

	return QObject::eventFilter(obj, event);
}

void SelectPointTrackingWgt::closeEvent(QCloseEvent * event)
{
	//通知绘制插件在二维上绘制路线
	QString usrname = DataBaseOperatorSingleton::getInstance()->getCurrentUser();
	QString interName = "baseInter.FederationInteract.TwoDRoute";
	QHash<QString, QVariant> params;
	params.insert("otherinfo", "clear_my"+ usrname);
	bool isSuccess = EVRTIReceiver::GetInstance()->SendInterToEngine(interName, params);

	ui->pushButton_start->setText(QStringLiteral("开启拾取"));
	_parent->setCursor(Qt::ArrowCursor);
	_isSelectPoint = false;

	_routeComputerAndPaint->ClearRoute();

	hide();
}
