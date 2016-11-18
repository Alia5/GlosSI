/*
Copyright 2016 Peter Repukat - FlatspotSoftware

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
