#include "parsertest.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    parsertest w;
    w.show();

    return a.exec();
}
