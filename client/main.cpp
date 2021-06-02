#include "client.h"
#include "mysql.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Login l;
    l.show();

    if(l.exec() == QDialog::Accepted)
    {
        Client client;
        client.show();

        return a.exec();
    }
    else
        return 0;
}
