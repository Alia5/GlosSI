#include "UWPSelectDialog.h"



UWPSelectDialog::UWPSelectDialog(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
}


UWPSelectDialog::~UWPSelectDialog()
{

}

void UWPSelectDialog::setUWPList(QList<UWPPair> uwpPairs)
{
	this->uwpPairs = uwpPairs;
	QStringList items;
	for (auto uwp : uwpPairs)
	{
		QString space = " ";
		for (int i = 0; i < 60 - uwp.AppName.size(); i++)
		{
			space += " ";
		}
		items << "Name: " + uwp.AppName + space + "AppId: " + uwp.AppUMId;
	}
	ui.listWidget->addItems(items);
}

void UWPSelectDialog::on_pushButton_2_clicked()
{
	done(-1);
}


void UWPSelectDialog::on_pushButton_clicked()
{
	done(ui.listWidget->currentRow());
}