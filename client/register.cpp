#include "register.h"
#include "login.h"
#include "QApplication"
#include "ui_register.h"

Register::Register(QWidget *parent) :
    QDialog(parent),
    RegisterUI(new Ui::Register)
{
    RegisterUI->setupUi(this);
    sq = new Mysql;
    connect(RegisterUI->registerBtn,&QPushButton::clicked,this,&Register::doRegister);
    connect(RegisterUI->cancelBtn,&QPushButton::clicked,this,&Register::doCancel);
}

void Register::doRegister()
{
    user = RegisterUI->Username->text().trimmed();
    pswd = RegisterUI->Passwd->text().trimmed();
    confPswd = RegisterUI->confPasswd->text().trimmed();
    nick = RegisterUI->Nickname->text().trimmed();

    if(!(emptyJudge(user,pswd,confPswd,nick)&&passwdJudge(pswd,confPswd)&&legalityCheck(user,pswd)))
    {
        return;
    }

    if(sq->ConnectMysql())
    {
        QString reg = QString("insert into chatdata.user values (0,'%1','%2','%3');").arg(user).arg(pswd).arg(nick);
        QString check = QString("select * from chatdata.user where username= '%1'").arg(user);
        QSqlQuery q;

        if(q.exec(check)&&q.first())
        {
            QMessageBox::warning(NULL,"Error","用户名已存在,请重试");
            return;
        }

        else if(q.exec(reg))
        {
            QMessageBox::information(NULL,"Success","注册成功",QMessageBox::Yes);
            Register::doCancel();
        }
        else
        {
            QMessageBox::warning(NULL,"Error","注册失败,请重试");
            return;
        }
    }
    else
    {
        QMessageBox::warning(NULL,"Error","服务器连接失败,请重试");
        return;
    }

}

void Register::doCancel()
{
    this->close();
}

void Register::clearEditline()
{
    RegisterUI->Username->clear();
    RegisterUI->Passwd->clear();
    RegisterUI->confPasswd->clear();
    RegisterUI->Nickname->clear();
}

bool Register::emptyJudge(QString &user,QString &pswd,QString &confPswd,QString &nick)
{
    if(user==""||pswd==""||confPswd==""||nick=="")
    {
        QMessageBox::warning(NULL,"Error","注册信息不能为空,请重试");
        clearEditline();
        return false;
    }
    return true;
}

bool Register::passwdJudge(QString &pswd,QString &confPswd)
{
    if(pswd != confPswd)
    {
        QMessageBox::warning(NULL,"Error","确认密码不一致，请重试");
        clearEditline();
        return false;
    }
    return true;
}

bool Register::legalityCheck(QString &user,QString &pswd)
{
    if(user.length() < 6||user.length()>12)
    {
        QMessageBox::warning(NULL,"Error","用户名长度应在6~12位");
        clearEditline();
        return false;
    }
    else if(pswd.length()<6||user.length()>18)
    {
        QMessageBox::warning(NULL,"Error","密码长度应大于6位");
        clearEditline();
        return false;
    }
    return true;
}



Register::~Register()
{
    delete RegisterUI;
    delete sq;
}

