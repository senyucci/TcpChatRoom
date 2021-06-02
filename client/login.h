#ifndef LOGIN_H
#define LOGIN_H
#include "client.h"
#include <QString>
#include <QPushButton>
#include <QDialog>


class Client;
class Register;
class Mysql;

namespace Ui {
class Login;
}

class Login : public QDialog
{
    Q_OBJECT

public:
    explicit Login(QWidget *parent = nullptr);
    ~Login();

signals:
    void sendUsername(QString);

private slots:
    void doLogin();
    void doRegister();

private:
    Ui::Login *LoginUI;
    Register *registerDiolog;
    QString user;
    QString pswd;
    QPushButton *loginBtn;
    QPushButton *registerBtn;
    Mysql *msql;

};

#endif // LOGIN_H
