/********************************************************************************
** Form generated from reading UI file 'UWPSelectDialog.ui'
**
** Created by: Qt User Interface Compiler version 5.7.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_UWPSELECTDIALOG_H
#define UI_UWPSELECTDIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QDialog>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QPushButton>

QT_BEGIN_NAMESPACE

class Ui_UWPSelectDialog
{
public:
    QListWidget *listWidget;
    QPushButton *pushButton;
    QPushButton *pushButton_2;

    void setupUi(QDialog *UWPSelectDialog)
    {
        if (UWPSelectDialog->objectName().isEmpty())
            UWPSelectDialog->setObjectName(QStringLiteral("UWPSelectDialog"));
        UWPSelectDialog->resize(666, 421);
        UWPSelectDialog->setMinimumSize(QSize(666, 421));
        UWPSelectDialog->setMaximumSize(QSize(666, 420));
        listWidget = new QListWidget(UWPSelectDialog);
        listWidget->setObjectName(QStringLiteral("listWidget"));
        listWidget->setGeometry(QRect(0, 0, 666, 381));
        listWidget->setStyleSheet(QStringLiteral("font: 8pt \"MS Shell Dlg 2\";"));
        pushButton = new QPushButton(UWPSelectDialog);
        pushButton->setObjectName(QStringLiteral("pushButton"));
        pushButton->setGeometry(QRect(490, 390, 75, 23));
        pushButton_2 = new QPushButton(UWPSelectDialog);
        pushButton_2->setObjectName(QStringLiteral("pushButton_2"));
        pushButton_2->setGeometry(QRect(580, 390, 75, 23));

        retranslateUi(UWPSelectDialog);

        QMetaObject::connectSlotsByName(UWPSelectDialog);
    } // setupUi

    void retranslateUi(QDialog *UWPSelectDialog)
    {
        UWPSelectDialog->setWindowTitle(QApplication::translate("UWPSelectDialog", "Select UWP-App", 0));
        pushButton->setText(QApplication::translate("UWPSelectDialog", "OK", 0));
        pushButton_2->setText(QApplication::translate("UWPSelectDialog", "Cancel", 0));
    } // retranslateUi

};

namespace Ui {
    class UWPSelectDialog: public Ui_UWPSelectDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_UWPSELECTDIALOG_H
