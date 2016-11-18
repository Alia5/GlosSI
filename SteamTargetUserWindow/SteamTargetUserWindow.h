#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_SteamTargetUserWindow.h"

#include <iostream>

#include <QSettings>

class SteamTargetUserWindow : public QMainWindow
{
    Q_OBJECT

public:
    SteamTargetUserWindow(QWidget *parent = Q_NULLPTR);

private:
    Ui::SteamTargetUserWindowClass ui;

	bool enableIPC = false;

private slots:
	void on_pushButton_resetControllers_clicked();
	void on_checkBox_showOverlay_toggled();
	void on_checkBox_enableControllers_toggled();
	void on_checkBox_showDebugConsole_toggled();
};
