#include <QApplication>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    // устанавливаем размеры секций сплиттера
    w.setSplitSize();

    return a.exec();
}
