#include <QApplication>
#include "TableCreator.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    TableCreator tableCreator;
    tableCreator.show();

    return a.exec();
}
