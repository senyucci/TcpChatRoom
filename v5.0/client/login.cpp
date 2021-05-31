#include "client.h"
#include "ui_login.h"
#include <QMessageBox>
#include <QString>


Login::Login(QWidget *parent) :
    QDialog(parent),
    LoginUI(new Ui::Login)
{
    LoginUI->setupUi(this);
    this->setWindowTitle(tr("Login"));

    msql = new Mysql;

    connect(LoginUI->loginBtn,&QPushButton::clicked,this,&Login::doLogin);
    connect(LoginUI->registerBtn,&QPushButton::clicked,this,&Login::doRegister);
}

void Login::doLogin()
{
    user = LoginUI->User->text().trimmed();
    pswd = LoginUI->Passwd->text().trimmed();

    if(msql->ConnectMysql())
    {
        QSqlQuery q;
        QString str = QString("select * from chatdata.user where username = '%1' and password= '%2';").arg(user).arg(pswd);
        q.exec(str);
        if(q.first())
        {
            tmp_usr_name = user;
            accept(); //关闭窗体，并设置返回值为Accepted
        }
        else
        {
            QMessageBox::warning(this,tr("waring"),tr("登录凭证有误,请重新输入"),QMessageBox::Yes);
            LoginUI->User->clear();
            LoginUI->Passwd->clear();
            LoginUI->User->setFocus();
        }
    }
}

void Login::doRegister()
{
    registerDiolog = new Register(this);
    registerDiolog->exec();
}

Login::~Login()
{
    delete LoginUI;
}
