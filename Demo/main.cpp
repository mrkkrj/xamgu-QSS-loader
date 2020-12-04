#include "QssLoaderDemoApp.h"

#include <QtWidgets/QApplication>
#include <QtWidgets/QStyleFactory>


int main(int argc, char* argv[])
{
    QApplication a(argc, argv);

    QStyle* style = QStyleFactory::create("Fusion");
    QApplication::setStyle(style);

    QssLoaderDemoApp w;
    w.show();

    return a.exec();
}
