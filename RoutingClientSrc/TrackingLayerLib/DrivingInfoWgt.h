#pragma once

#include <QWidget>

namespace Ui { class	DrivingInfoWgt; };

class DrivingInfoWgt : public QWidget
{
	Q_OBJECT

public:
	DrivingInfoWgt(const QString& image,const QString& roadName,const double& drivingDistance,QWidget *parent = Q_NULLPTR);
	~DrivingInfoWgt();

private:
	QString toLocalPixPath(const QString& imageName) const;

private:
	Ui::DrivingInfoWgt *ui;
};
