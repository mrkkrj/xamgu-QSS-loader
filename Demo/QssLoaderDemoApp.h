/** @file   QssLoaderDemoApp.h
   @author  Marek Krajewski (mrkkrj), www.ib-krajewski.de
   @date    28.09.2020

   @brief Declaration of Xamgu's QssFileLoader's demo app class

   @copyright  Copyright 2020, Marek Krajewski, released under the terms of LGPL v3
*/

#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_QssLoaderDemoApp.h"


/**
   @brief: Demo app for the QSS-e loader
 */
class QssLoaderDemoApp
    : public QMainWindow
{
    Q_OBJECT

public:
    QssLoaderDemoApp(QWidget* parent = nullptr);

private slots:
    void on_pushButtonOK_clicked();
    void on_pushButtonCancel_clicked();
    void on_comboBoxTheme_activated(const QString& text);
    void on_toolButton_clicked();
    void on_infoToolButton_clicked();
    void on_datePickToolButton_clicked();

private:
    void fillTableWidget(const QString& txt, int rowsToInit = 1);

    Ui::QssLoaderDemoApp ui;
};

