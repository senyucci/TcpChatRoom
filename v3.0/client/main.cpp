#include "client.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Client w;
    w.show();
    w.setWindowTitle("TcpClient");
    return a.exec();
}
