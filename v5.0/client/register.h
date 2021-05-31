#ifndef REGISTER_H
#define REGISTER_H
#include <client.h>
#include <QString>
#include <QDialog>

class Login;
class Mysql;

namespace Ui {
class Register;
}

class Register : public QDialog
{
    Q_OBJECT;

public:
    explicit Register(QWidget *parent = nullptr);
    void clearEditline();
    bool emptyJudge(QString &user,QString &pswd,QString &confPswd,QString &nick);
    bool passwdJudge(QString &pswd,QString &confPswd);
    ~Register();

private slots:
    void doCancel();
    void doRegister();
private:
    Mysql *sq;
    Ui::Register *RegisterUI;
    Login *loginDiolog;
    QString user;
    QString pswd;
    QString confPswd;
    QString nick;
};

#endif // REGISTER_H
