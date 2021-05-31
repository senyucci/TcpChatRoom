#ifndef CLIENT_H
#define CLIENT_H

#include "login.h"
#include "register.h"
#include "mysql.h"
#include "common.h"
#include <QWidget>
#include <QTcpSocket>
#include <QtNetwork>
#include <QPushButton>
#include <QLineEdit>
#include <QDialog>
#include <QFontDialog>
#include <QColorDialog>

extern QString tmp_usr_name;

class Mysql;

QT_BEGIN_NAMESPACE
namespace Ui { class Client; }
QT_END_NAMESPACE

class Client : public QWidget
{
    Q_OBJECT

public:
    Client(QWidget *parent = nullptr);
    void sendMsg(QString Nic);
    ~Client();

private:
    Mysql sq;
    Ui::Client *ui;
    QTcpSocket* client;



private slots:
    void doDisconnected();
    void doConnected();
    void doReceiveData();
};

#endif // CLIENT_H
