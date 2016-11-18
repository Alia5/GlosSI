#pragma once

#include <QDialog>
#include "ui_UWPSelectDialog.h"

#include "UWPPair.h"


#include <QList>

class UWPSelectDialog : public QDialog
{
	Q_OBJECT

public:
	UWPSelectDialog(QWidget *parent = Q_NULLPTR);
	~UWPSelectDialog();

	void setUWPList(QList<UWPPair> uwpPairs);	

private:
	Ui::UWPSelectDialog ui;
	QList<UWPPair> uwpPairs;

private slots:
	void on_pushButton_clicked();
	void on_pushButton_2_clicked();

};
