/*
Copyright 2018 Peter Repukat - FlatspotSoftware

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http ://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/
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
		for (int i = 0; i < 40 - uwp.AppName.size(); i++)
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