#include "DrivingInfoWgt.h"

#include "ui_DrivingInfoWgt.h"

DrivingInfoWgt::DrivingInfoWgt(const QString& image, const QString& roadName, const double& drivingDistance, QWidget *parent)
	: QWidget(parent),ui(new Ui::DrivingInfoWgt)
{
	ui->setupUi(this);

	//方向image解析

	//ui->label_image->setScaledContents(true);
	QString iconPAth= toLocalPixPath(image);
	ui->label_image->setPixmap(QPixmap(iconPAth));
	

	//road
	if (roadName=="")
	{
		ui->label_roadName->setText("         " + QStringLiteral("无名大道"));
	}
	else
	{
		ui->label_roadName->setText("         "+roadName);
	}	

	//distance
	if (drivingDistance<=1000)
	{
		ui->label_drivingDisdance->setText(QString::number(drivingDistance) + "m");
	}
	else
	{
		ui->label_drivingDisdance->setText(QString::number(drivingDistance/1000, 'f', 2) + "Km");
	}
}

DrivingInfoWgt::~DrivingInfoWgt()
{

}

QString DrivingInfoWgt::toLocalPixPath(const QString & imageName) const
{
	//QString path = QString::fromLocal8Bit("%1%2").arg(QApplication::applicationDirPath()).arg(QStringLiteral("/../userData/data/widgetImage/widget/routePlanning/straight.png"));
	QString path = "";
	if (imageName.contains("left")&&imageName.contains("slight"))
	{
		path = QString::fromLocal8Bit("%1%2").arg(QApplication::applicationDirPath()).arg(QStringLiteral("/../userData/data/widgetImage/widget/routePlanning/slight_left.png"));
		return path;
	}
	if (imageName.contains("left") )
	{
		path = QString::fromLocal8Bit("%1%2").arg(QApplication::applicationDirPath()).arg(QStringLiteral("/../userData/data/widgetImage/widget/routePlanning/left.png"));
		return path;
	}
	if (imageName.contains("right") && imageName.contains("slight"))
	{
		path = QString::fromLocal8Bit("%1%2").arg(QApplication::applicationDirPath()).arg(QStringLiteral("/../userData/data/widgetImage/widget/routePlanning/slight_right.png"));
		return path;
	}
	if (imageName.contains("right") )
	{
		path = QString::fromLocal8Bit("%1%2").arg(QApplication::applicationDirPath()).arg(QStringLiteral("/../userData/data/widgetImage/widget/routePlanning/right.png"));
		return path;
	}
	if (imageName.contains("straight"))
	{
		path = QString::fromLocal8Bit("%1%2").arg(QApplication::applicationDirPath()).arg(QStringLiteral("/../userData/data/widgetImage/widget/routePlanning/straight.png"));
		return path;
	}
	if (imageName.contains("end"))
	{
		path = QString::fromLocal8Bit("%1%2").arg(QApplication::applicationDirPath()).arg(QStringLiteral("/../userData/data/widgetImage/widget/routePlanning/end.png"));
		return path;
	}
	if (imageName.contains("start"))
	{
		path = QString::fromLocal8Bit("%1%2").arg(QApplication::applicationDirPath()).arg(QStringLiteral("/../userData/data/widgetImage/widget/routePlanning/start.png"));
		return path;
	}
	if (imageName.contains("uturn"))
	{
		path = QString::fromLocal8Bit("%1%2").arg(QApplication::applicationDirPath()).arg(QStringLiteral("/../userData/data/widgetImage/widget/routePlanning/uturn.png"));
		return path;
	}
	return path;
}
