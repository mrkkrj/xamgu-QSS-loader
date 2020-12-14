/** @file   QssLoaderDemoApp.cpp
   @author  Marek Krajewski (mrkkrj), www.ib-krajewski.de
   @date    20.10.2020

   @brief Implementation of Xamgu's demo app

   @copyright  Copyright 2020, Marek Krajewski, released under the terms of LGPL v3
*/

#include "QssLoaderDemoApp.h"

#include <QtWidgets/QMessageBox>
#include <QtGui/QImageReader>

#include "../Source/QsseFileLoader.h"


// OPEN TODO::: enable configurable logging???
#ifdef QSSLD_LOGGING_ENABLED
#  include "Logging/Logger.h"
#else
#  include <QDebug>

#  define QSSLD_LOGGING_DEFINE_SCOPE(a)

#  define QTUTILS_LOG_TRACE(theme, a) do {} while (0)
#  define QTUTILS_LOG_INFO(theme, a) do {} while (0)
#  define QTUTILS_LOG_ERROR(theme, a) do {} while (0)
#endif


namespace {

    // scope for local log
    QSSLD_LOGGING_DEFINE_SCOPE(StylingTestApp);

}

using namespace ibkrj::xamgu;


// public methods

QssLoaderDemoApp::QssLoaderDemoApp(QWidget* parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);
   
#ifdef QSSLD_LOGGING_ENABLED
    // test logging
    auto list = QImageReader::supportedImageFormats();
    for (auto& elem : list)
    {
        QTUTILS_LOG_INFO(tags::Infrastructure, QString("Supported format: \"%1\"").arg(QString(elem)));
    }
#endif
   
    // pre-configure
    on_comboBoxTheme_activated("Light");
    fillTableWidget("text", 6);

    // connect
    connect(ui.horizontalSlider, &QSlider::valueChanged, this, [this](int value) { ui.progressBar->setProperty("value", value); });
}


// private slots

void QssLoaderDemoApp::on_pushButtonOK_clicked()
{
    QMessageBox::critical(this, "Xamgu ERROR", "Not Yet Implemented ...");
}


void QssLoaderDemoApp::on_pushButtonCancel_clicked()
{
    if (QMessageBox::question(this, "Xamgu QUESTION", "Do you want to close the application?") == QMessageBox::Yes)
    {
        close();
     }
}


void QssLoaderDemoApp::on_toolButton_clicked()
{
    QMessageBox::information(this, "Xamgu INFO", 
        "\n"
        "This Tool Button is doing nothing (yet) !!!"
        "\n");
}


void QssLoaderDemoApp::on_infoToolButton_clicked()
{
    QMessageBox::information(this, "Xamgu INFO",
        "Demo application for the Xamgu Qss-Loader."
        "\n\n\""
        "Lorem ipsum dolor sit amet, consectetur adipisici elit, sed eiusmod tempor incidunt ut "
        "labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco "
        "laboris nisi ut aliquid ex ea commodi consequat. Quis aute iure reprehenderit in voluptate "
        "velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint obcaecat cupiditat non "
        "proident, sunt in culpa qui officia deserunt mollit anim id est laborum."
        "\"\n");
}


void QssLoaderDemoApp::on_datePickToolButton_clicked()
{
    QMessageBox::critical(this, "Xamgu ERROR", "Not Yet Implemented ...");
}


void QssLoaderDemoApp::on_comboBoxTheme_activated(const QString& text)
{
    if (text == "No Style")
    {
        qApp->setStyleSheet("");
        return;
    }   
    QsseFileLoader ld(":/Resources/StyleSheets");
   
    if (text == "Light")
    {      
        ld.createVariable("$Theme", "Light");
    }
    else if (text == "Dark")
    {
        ld.createVariable("$Theme", "Dark");
    }     

    QString stylesheet = ld.loadStyleSheet("DemoAppStyleSheet.qsse");

    if (stylesheet.isEmpty())
    {
        QMessageBox::critical(this, "Xamgu ERROR", ld.messages().join('\n'));
        return;
    }

    qApp->setStyleSheet(stylesheet);
}


// private methods

void QssLoaderDemoApp::fillTableWidget(const QString& txt, int rowsToInit)
{
    int rows = ui.tableWidget->rowCount();
    int columns = ui.tableWidget->columnCount();

    for (int r = 0; r < rowsToInit && r < rows; ++r)
    {
        for (int c = 0; c < columns; ++c)
        {
            ui.tableWidget->setItem(r, c, new QTableWidgetItem(txt + " " + QString::number(r + 1)));
        }
    }

    for (int r = 0; r < rows - rowsToInit; ++r)
    {
        for (int c = 0; c < columns; ++c)
        {
            ui.tableWidget->setItem(rowsToInit + r, c, new QTableWidgetItem("---"));
        }
    }
}

