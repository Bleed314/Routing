#pragma once

#include <QWidget>

#include "UILibrary/BaseDialogEx.h"

namespace Ui { class	NameRouteDlg; };

class NameRouteDlg : public BaseDialogEx
{
	Q_OBJECT

public:
	NameRouteDlg(QWidget *parent = Q_NULLPTR);
	~NameRouteDlg();

signals:
	void nameFinshed(QString name);

private slots:
	void okSlot();
	void cancelSlot();

private:
	Ui::NameRouteDlg* ui;
};
