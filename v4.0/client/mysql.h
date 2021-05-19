#ifndef MYSQL_H
#define MYSQL_H

#include <QMessageBox>
#include <QtSql>
#include <QSqlQuery>
#include <QtSql/QSqlDatabase>

class Mysql
{
public:
    Mysql();
    ~Mysql();
    bool ConnectMysql();
    void CloseMysql();


private:
    QSqlDatabase* db;
};


#endif // MYSQL_H

