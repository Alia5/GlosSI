#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_GloSC.h"

#include <qdir.h>
#include <qfile.h>
#include <qsettings.h>
#include <QFileDialog>
#include <qlist.h>

#include <Windows.h>
#include <appmodel.h>

#include "UWPPair.h"
#include "UWPSelectDialog.h"

class GloSC : public QMainWindow
{
    Q_OBJECT

public:
    GloSC(QWidget *parent = Q_NULLPTR);

private:
    Ui::GloSCClass ui;

	void updateEntryList();
	void writeIni(QString entryName);

	QList<UWPPair> uwpPairs;

private slots:
	void on_pbSave_clicked();
	void on_pbDelete_clicked();
	void on_pbAddToSteam_clicked();
	void on_pbSearchPath_clicked();
	void on_pbUWP_clicked();
	void on_lwInstances_currentRowChanged(int row);


};
