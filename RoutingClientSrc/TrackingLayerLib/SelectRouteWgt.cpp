#include "SelectRouteWgt.h"

#include "ui_SelectRouteWgt.h"

#include <QPushButton>
#include <QMessageBox>
#include <QListWidget>
#include <QLabel>
#include <QBuffer>
#include <QByteArray>

#include "GlobeMapCore/GlobeMapManager.h"
#include "mapcontrol/mapcontrol.h"
#include "GlobeMapCore/CMapWidget.h"

#include "symbol/simplelinesymbol.h"
#include "symbol/simpleline3dsymbol.h"
#include "geometry3d/geometry3dextension/multipoint3dextension.h"
#include "geometry3d/geometry3dextension/multipolyline3dextension.h"
#include "symbol/simpletextsymbol.h"
#include "symbol/billboard3dsymbol.h"

#include "WritClientLibrary/databaseoperatorsingleton.h"

#include "UtilityLibrary/EntityInfoManager.h"
#include "LibSimDataTrans/EVRTIReceiver.h"
#include "LibSimDataTrans/LocalSimDataTrans.h"

#include "GroupTrainData/GroupTrainDataInstance.h"


#include "RouteStrut.h"
#include "NameRouteDlg.h"

#pragma comment(lib, "UILibrary.lib")
#pragma comment(lib, "WritClientLibrary.lib")

using namespace EarthView::World::Spatial::Atlas;
using namespace EarthView::World::Spatial3D::Controls;
using namespace EarthView::World::Spatial2D::Controls;
using namespace EarthView::World::Spatial::Display;
using namespace EarthView::World::Spatial::Utility;
using namespace EarthView::World::Spatial;
using namespace EarthView::World::Geometry3D;

SelectRouteWgt::SelectRouteWgt(QWidget *parent,bool isNeedSetSpeed)
	: BaseDialogEx(parent), ui(new Ui::SelectRouteWgt)
{
	QWidget* widget = new QWidget;
	ui->setupUi(widget);
	setCentralWidget(widget);
	setWindowTitle(QString::fromLocal8Bit("选择路线"));

	ui->listWidget->hide();
	if (isNeedSetSpeed)
	{

	}
	else
	{
		ui->label->hide();
		ui->doubleSpinBox->hide();
	}

	connect(ui->pushButton_ok, &QPushButton::clicked, this, &SelectRouteWgt::okSlot);
	connect(ui->pushButton_cancel, &QPushButton::clicked, this, &SelectRouteWgt::cancelSlot);
	connect(ui->listWidget,&QListWidget::itemClicked,this,&SelectRouteWgt::lstItemWgtSlot);
	connect(ui->pushButton_save, &QPushButton::clicked, this, &SelectRouteWgt::saveRouteSlot);
	//connect(ui->pushButton_hide, &QPushButton::clicked, this, &SelectRouteWgt::hideSlot);

	connect(ui->tableWidget, &QTableWidget::itemClicked, this, &SelectRouteWgt::itemClickedSlot);
	connect(ui->tableWidget, &QTableWidget::itemDoubleClicked, this, &SelectRouteWgt::itemDoubleClickedSlot);

	ui->splitter->setStretchFactor(0, 0.65);
	ui->splitter->setStretchFactor(1, 0.35);

	qApp->installEventFilter(this);

	//获取跟踪层对象指针
	mpGlobeCtr = GlobeMapManager::GetInstance()->GetGlobeWidget()->getGlobeControl();
	mpMapCtr = GlobeMapManager::GetInstance()->GetMapWidget()->getMapControl();
	mpGlobeCtr->addGlobeControlListener(this);
	mpMapCtr->addMapControlListener(this);
	if (mpMapCtr)
	{
		m_pTrackingLayer = mpMapCtr->getTrackingLayer();
	}

	initGeometryItem();
}

SelectRouteWgt::~SelectRouteWgt()
{
}

void SelectRouteWgt::setSaveToDBShow(bool isShow)
{
	ui->pushButton_save->setHidden(!isShow);
}

void SelectRouteWgt::hideSomeT()
{
	ui->label->hide();
	ui->doubleSpinBox->hide();
	ui->pushButton_save->hide();
}

void SelectRouteWgt::closeEvent(QCloseEvent * event)
{
	clearLine();
	clearText();
	emit signalCancle();
}

void SelectRouteWgt::setInfo(QList<RouteInfo> routeInfo)
{
	_routeInfoLst.clear();
	_routeInfoLst = routeInfo;
	initialTable(routeInfo);
	ui->listWidget->clear();
	ui->listWidget->hide();
}

void SelectRouteWgt::initialTable(QList<RouteInfo> routeInfo)
{
	ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	ui->tableWidget->setColumnCount(3);
	ui->tableWidget->setRowCount(routeInfo.size());
	ui->tableWidget->setHorizontalHeaderLabels(QStringList() << QStringLiteral("路径") << QStringLiteral("全长") << QStringLiteral("耗时"));
	ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	ui->tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
	ui->tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
	
	for (int i = 0; i < routeInfo.size();i++)
	{	
		QTableWidgetItem* item1 = new QTableWidgetItem(routeInfo.at(i).name);
		item1->setTextAlignment(Qt::AlignHCenter);
		ui->tableWidget->setItem(i, 0, item1);
		QTableWidgetItem* item2 = new QTableWidgetItem(QString::number(routeInfo.at(i).length) + "Km");
		item2->setTextAlignment(Qt::AlignHCenter);
		ui->tableWidget->setItem(i, 1, item2);
		QTableWidgetItem* item3 = new QTableWidgetItem(secondToTime(routeInfo.at(i).spendTime));
		item3->setTextAlignment(Qt::AlignHCenter);
		ui->tableWidget->setItem(i, 2, item3);
	}
}

QString SelectRouteWgt::secondToTime(const double second)
{
	QString str;

	int time_1 = int(second);//整数部分
	double time_2 = second - time_1;//小数部分

	//小时
	int hour = time_1 / 3600;
	if (hour!=0)
	{
		str = QString::number(hour) + QStringLiteral("小时");
	}

	//分钟
	int minute = time_1 % 3600 / 60;
	if (minute!=0)
	{
		str += QString::number(minute) + QStringLiteral("分钟");
	}

	//秒
	int secondd = time_1 % 3600 % 60;
	if (secondd!=0)
	{
		double s = secondd + time_2;
		str += QString::number(s) + QStringLiteral("秒");
	}

	return str;
}

QByteArray SelectRouteWgt::listToArrry(QList<QList<QPair<double, double>>> routePoint)
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

QList<QList<QPair<double, double>>> SelectRouteWgt::arrryToList(QByteArray arrry)
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

void SelectRouteWgt::drawLine(QList<QPair<double, double>> route)
{
	if (!mpGlobeCtr)
	{
		return;
	}

	EarthView::World::Spatial::Geometry::CLineString* line1 = EV_NEW EarthView::World::Spatial::Geometry::CLineString();
	int index = 0;

	for (int i = 0;i < route.size();i++)
	{
		double height = mpGlobeCtr->getSceneManager()->getHeightAtTile(route.at(i).second, route.at(i).first);
		EarthView::World::Spatial::Geometry::CCoordinate p1 = EarthView::World::Spatial::Geometry::CCoordinate(route.at(i).second, route.at(i).first, height);
		line1->add(p1, index++);
	}
	line1->setSpatialReferenceRef(CCoordinateSystemFactory::createCoordSys(GEO_WGS84));
	line1->update();
	m_pTrackingLayer->addItem(_lineItem);
	_lineItem->addGeometry(line1);

	//3d绘制
	CSimpleLine3DSymbol * lineSymbol = EV_NEW CSimpleLine3DSymbol();
	CRgbColor color;
	color.setRGB(255, 255, 0);
	lineSymbol->setLineColor(&color);
	//lineSymbol->setBoldEnabled(true);
	lineSymbol->setAltitudeMode(AM_Absolute);
	lineSymbol->setTransectShapeType(TST_Point);
	CGeoObject* obj2 = EV_NEW CGeoObject();
	obj2->setGeometry(line1, false, true);//第三个参数为true，说明setGeometry时clone了一下,外面可以释放line1了;
	obj2->setSymbol(lineSymbol, false, true);//第三个参数为true，说明setSymbol时clone了一下,外面可以释放lineSymbol了;
	EV_DELETE line1;
	EV_DELETE lineSymbol;

	CMultiGeometry3DExtension* extensions = EV_NEW CMultiPolyline3DExtension(mpGlobeCtr->getSceneManager(), "extensionTest");
	extensions->appendGeoObject(obj2);
	extensions->build();
	extensions->render();
	_3dLineList.push_back(extensions);
}

void SelectRouteWgt::drawText(double X, double Y, QString text)
{
	if (!m_pTrackingLayer)
	{
		return;
	}
	CTrackingTextItem* TextItem = new CTrackingTextItem;
	_allTextItem.append(TextItem);
	CRgbColor textColor(255, 0, 0);
	CSimpleTextSymbol labelSymbol;
	labelSymbol.setColor(&textColor);
	labelSymbol.setSize(10.0);
	labelSymbol.setBold(true);
	TextItem->setPositionType(TIP_LeftBottom);
	TextItem->setSymbol(&labelSymbol);
	TextItem->setVisible(true);
	m_pTrackingLayer->addItem(TextItem);
	TextItem->setPos(X , Y );
	TextItem->setText(text.toLocal8Bit().data());
}

void SelectRouteWgt::drawText_3D(double X, double Y, EVString text)
{
	//text = "无名大道";
	//创建广告版点风格,
	CBillboard3DSymbol* symbol = EV_NEW CBillboard3DSymbol();
	symbol->setName("PointExtensionSymbol");
	symbol->setAltitudeMode(AM_Absolute);
	symbol->setHightenValue(0);
	symbol->setShowPicture(true);
	symbol->setPictureScale(1.0);
	symbol->setBillboardWidth(25.0);
	symbol->setBillboardHeight(25.0);

	CImage image;
	image.load("A.png", "geometry3d");

	DataStreamPtr streamptr = image.encode("png");
	MemoryDataStreamPtr memoryStreamPtr(streamptr);
	symbol->setImage(0, "A.png", memoryStreamPtr);

	symbol->setShowText(true);
	symbol->setPropertyName("name");
	symbol->setFontName("宋体");
	symbol->setFontSize(20);
	symbol->setColor((255, 0, 0));

	EarthView::World::Spatial::Geometry::CPoint* p = EV_NEW EarthView::World::Spatial::Geometry::CPoint(X, Y, 0);
	p->setSpatialReferenceRef(CCoordinateSystemFactory::createCoordSys(GEO_WGS84));
	p->update();

	CGeoObject* obj1 = EV_NEW CGeoObject();
	obj1->setGeometry(p, false, true);//第三个参数为true，说明setGeometry时clone了一下,外面可以释放p了;
	obj1->setSymbol(symbol, false, true);//第三个参数为true，说明setSymbol时clone了一下，外面可以释放symbol了;
	EV_DELETE symbol;
	EV_DELETE p;

	obj1->addProperty("name", text);

	CMultiGeometry3DExtension* extensions = EV_NEW CMultiPoint3DExtension(mpGlobeCtr->getSceneManager(), "extensionTest");
	extensions = EV_NEW CMultiPoint3DExtension(mpGlobeCtr->getSceneManager(), "extensionTest");
	extensions->appendGeoObject(obj1);
	extensions->build();
	extensions->render();
	_3dPointList.push_back(extensions);
}

void SelectRouteWgt::clearLine()
{
	if (_lineItem != NULL)
	{
		_lineItem->clear();
	}

	if (_3dLineList.size() == 0)
	{
		return;
	}
	for (int i = 0; i < _3dLineList.size(); i++)
	{
		if (_3dLineList[i])
		{
			for (int j = 0; j < _3dLineList[i]->getGeoObjectCount(); j++)
			{
				CGeoObject* go = _3dLineList[i]->getGeoObject(j);
				EV_DELETE go;
				go = NULL;
			}
			EV_DELETE _3dLineList[i];
			_3dLineList[i] = NULL;
		}
	}
	_3dLineList.clear();
	for (int i = 0; i < _3dPointList.size(); i++)
	{
		if (_3dPointList[i])
		{
			for (int j = 0; j < _3dPointList[i]->getGeoObjectCount(); j++)
			{
				CGeoObject* go = _3dPointList[i]->getGeoObject(j);
				EV_DELETE go;
				go = NULL;
			}
			EV_DELETE _3dPointList[i];
			_3dPointList[i] = NULL;
		}
	}
	_3dPointList.clear();
	if (mpGlobeCtr)
	if (mpGlobeCtr)
	{
		mpGlobeCtr->getSceneManager()->updateQuadImage(false, false, false);
	}
}

void SelectRouteWgt::clearText()
{
	//清空文字
	for (int i = 0; i < _allTextItem.size();i++)
	{
		_allTextItem.at(i)->setText("");
		_allTextItem.at(i)->setVisible(false);
	}
	_allTextItem.clear();
}


void SelectRouteWgt::okSlot()
{
	if (ui->tableWidget->rowCount()==1)
	{
		_routeInfoLst[0].runSpeed = QString::number(ui->doubleSpinBox->value());
		emit selectRoute(_routeInfoLst.at(0));
		close();
		return;
	}

	int current = ui->tableWidget->currentRow();
	if (_currentRow<0)
	{
		QMessageBox::information(this,QStringLiteral("提示"),QStringLiteral("请先选择一条路线"));
		return;
	}
	else
	{
		_routeInfoLst[_currentRow].runSpeed = QString::number(ui->doubleSpinBox->value());
		emit selectRoute(_routeInfoLst.at(_currentRow));
		close();
		return;
	}
}

void SelectRouteWgt::cancelSlot()
{
	emit signalCancle();
	QDialog::close();
}

void SelectRouteWgt::lstItemWgtSlot(QListWidgetItem *item)
{
	//找到点击的这段路
	QUuid theId = item->data(Qt::UserRole).toUuid();
	RouteInfo routeInfo = _routeInfoLst.at(_currentRow);
	int pos = -1;
	for (int i = 0;i < routeInfo.drivingInfoLst.size();i++)
	{
		if (routeInfo.drivingInfoLst.at(i).id==theId)
		{
			pos = i;
			break;
		}
	}
	if (pos==-1)
	{
		return;
	}
	QList<QPair<double, double>> theRoad= routeInfo.routePoint.at(pos);
	QString theRoadName = routeInfo.drivingInfoLst.at(pos).loadName;
	if (theRoadName=="")
	{
		theRoadName = QStringLiteral("无名大道");
	}

	//清空上一次高亮
	clearLine();
	clearText();
	//在地图上高亮这段路
	drawLine(theRoad);
	drawText(theRoad.at((int)theRoad.size()/2).second, theRoad.at((int)theRoad.size() / 2).first,theRoadName);
	drawText_3D(theRoad.at((int)theRoad.size() / 2).second, theRoad.at((int)theRoad.size() / 2).first, theRoadName.toLocal8Bit().data());
}

void SelectRouteWgt::hideSlot()
{
	//ui->pushButton_hide->hide();
	ui->listWidget->hide();
}

void SelectRouteWgt::itemClickedSlot(QTableWidgetItem* item)
{
	_currentRow = item->row();
	QString usrname = DataBaseOperatorSingleton::getInstance()->getCurrentUser();
	if (_currentRow==0)
	{
		emit preViewRoute(_routeInfoLst[0]);

		GroupTrainDataInstance::getInstance()->set2dDrawRoute(_routeInfoLst[0].routePoint);
		//通知绘制插件在二维上绘制路线
		QString interName = "baseInter.FederationInteract.TwoDRoute";
		QHash<QString, QVariant> params;
		params.insert("otherinfo", "route" + usrname);
		bool isSuccess = EVRTIReceiver::GetInstance()->SendInterToEngine(interName, params);
	}
	if (_currentRow==1)
	{
		emit preViewRoute(_routeInfoLst[1]);

		GroupTrainDataInstance::getInstance()->set2dDrawRoute(_routeInfoLst[1].routePoint);
		//通知绘制插件在二维上绘制路线
		QString interName = "baseInter.FederationInteract.TwoDRoute";
		QHash<QString, QVariant> params;
		params.insert("otherinfo", "route"+ usrname);
		bool isSuccess = EVRTIReceiver::GetInstance()->SendInterToEngine(interName, params);
	}

	ui->listWidget->clear();

	QList<DrivingInfo> drivingInfoLst = _routeInfoLst.at(_currentRow).drivingInfoLst;

	for (int i = 0; i < drivingInfoLst.size(); i++)
	{
		QString image = drivingInfoLst.at(i).image;
		if (image == "")
		{
			if (i == 0)
			{
				image = "start";
			}
			else if (i == drivingInfoLst.size() - 1)
			{
				image = "end";
			}
			else
			{
				image = "straight";
			}
		}

		DrivingInfoWgt* wgt = new DrivingInfoWgt(image, drivingInfoLst.at(i).loadName, drivingInfoLst.at(i).drivingDistance, this);

		QListWidgetItem *item = new QListWidgetItem(ui->listWidget, 0);
		item->setData(Qt::UserRole, drivingInfoLst.at(i).id);
		int a = wgt->width();
		int b = wgt->height();
		item->setSizeHint(QSize(wgt->width(), wgt->height()));
		ui->listWidget->setItemWidget(item, wgt);
	}

	ui->listWidget->show();
}

void SelectRouteWgt::itemDoubleClickedSlot(QTableWidgetItem * item)
{
	if (_currentRow==-1)
	{
		return;
	}

	ui->listWidget->clear();

	QList<DrivingInfo> drivingInfoLst = _routeInfoLst.at(_currentRow).drivingInfoLst;

	for (int i = 0; i < drivingInfoLst.size();i++)
	{
		QString image = drivingInfoLst.at(i).image;
		if (image=="")
		{
			if (i == 0 )
			{
				image = "start";
			}
			else if (i == drivingInfoLst.size()-1 )
			{
				image = "end";
			}
			else
			{
				image = "straight";
			}
		}

		DrivingInfoWgt* wgt = new DrivingInfoWgt(image, drivingInfoLst.at(i).loadName, drivingInfoLst.at(i).drivingDistance,this);

		QListWidgetItem *item = new QListWidgetItem(ui->listWidget, 0);
		int a = wgt->width();
		int b = wgt->height();
		item->setSizeHint(QSize(wgt->width(), wgt->height()));
		ui->listWidget->setItemWidget(item, wgt);
	}

	ui->listWidget->show();
	//ui->pushButton_hide->show();
}

void SelectRouteWgt::saveRouteSlot()
{
	QTableWidgetItem* currentItem = ui->tableWidget->currentItem();
	if (!currentItem)
	{
		QMessageBox::information(this, QStringLiteral("提示"), QStringLiteral("请先选择一条要保存到数据库的路线"));
		return;
	}
	int currentRow = currentItem->row();
	NameRouteDlg dlg;
	connect(&dlg, &NameRouteDlg::nameFinshed, [&](QString routename)
	{
		if (DataBaseOperatorSingleton::getInstance()->isRename(routename))
		{
			QMessageBox::information(this, QStringLiteral("提示"), QStringLiteral("该路径名已经存在！"));
			return;
		}
		RouteInfo info = _routeInfoLst.at(currentRow);
		QList<QList<QPair<double, double>>> point = info.routePoint;
		QByteArray arrry = listToArrry(point);
		QList<QList<QPair<double, double>>> test = arrryToList(arrry);
		DBRouteStruct route;
		route.RouteName = routename;
		route.RouteDistance = info.length;
		route.RoutePoint = arrry;
		bool isSuccess = DataBaseOperatorSingleton::getInstance()->uploadeRoute(route);
		if (isSuccess)
		{
			QMessageBox::information(this, QStringLiteral("提示"), QStringLiteral("成功提交到数据库"));
			return;
		}
		else
		{
			QMessageBox::information(this, QStringLiteral("提示"), QStringLiteral("提交失败，请检查网络"));
			return;
		}
	});
	dlg.exec();
}

void SelectRouteWgt::initGeometryItem()
{
	_3dLineList.clear();
	_allTextItem.clear();

	//实线-白色
	_lineItem = new CTrackingGeomsItem;
	CSimpleLineSymbol lineSymbol;
	CRgbColor rgb1(255, 0, 0, 150);
	//CRgbColor rgb1(255, 255, 255);
	lineSymbol.setWidth(6);
	lineSymbol.setColor(&rgb1);
	_lineItem->setSymbol(&lineSymbol);

}
