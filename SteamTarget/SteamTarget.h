#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_SteamTarget.h"

class SteamTarget : public QMainWindow
{
    Q_OBJECT

public:
    SteamTarget(QWidget *parent = Q_NULLPTR);

private:
    Ui::SteamTargetClass ui;
};
