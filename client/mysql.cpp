#include "mysql.h"


Mysql::Mysql()
{
    db = new QSqlDatabase;
}

Mysql::~Mysql()
{
    delete db;
}

bool Mysql::ConnectMysql()
{
    *db = QSqlDatabase::addDatabase("QMYSQL");
    db->setHostName("81.69.236.101");
    db->setPort(3310);
    db->setDatabaseName("chatdata");
    db->setUserName("zjj");
    db->setPassword("Plmoiuqsz123..");
    if(!db->open())
    {
        QMessageBox::warning(NULL,"error",db->lastError().text());
        return false;
    }
    return true;
}

void Mysql::CloseMysql()
{
    db->close();
}
