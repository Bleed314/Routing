#include "NameRouteDlg.h"

#include "ui_NameRouteDlg.h"

#include <QMessageBox>

NameRouteDlg::NameRouteDlg(QWidget *parent)
	: BaseDialogEx(parent),ui(new Ui::NameRouteDlg)
{
	QWidget* widget = new QWidget;
	ui->setupUi(widget);
	setCentralWidget(widget);
	setWindowTitle(QString::fromLocal8Bit("·������"));

	connect(ui->pushButton_ok, &QPushButton::clicked, this, &NameRouteDlg::okSlot);
	connect(ui->pushButton_cancel, &QPushButton::clicked, this, &NameRouteDlg::cancelSlot);
}

NameRouteDlg::~NameRouteDlg()
{
}

void NameRouteDlg::okSlot()
{
	QString name = ui->lineEdit->text();
	if (name.isEmpty())
	{
		QMessageBox::information(this, QStringLiteral("��ʾ"), QStringLiteral("��������Ϊ��"));
		return;
	}
	emit nameFinshed(name);
	close();
}

void NameRouteDlg::cancelSlot()
{
	close();
}
