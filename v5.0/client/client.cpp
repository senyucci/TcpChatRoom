#include "client.h"
#include "ui_client.h"
QString tmp_usr_name = 0;

Client::Client(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Client)
{
    ui->setupUi(this);
    ui->record->append("服务器未连接...");

    //Connect to socket
    client = new QTcpSocket(this);
    client->abort();
    connect(client,SIGNAL(connected()),SLOT(doConnected()));
    connect(client,SIGNAL(disconnected()),SLOT(doDisconnected()));
    connect(client,SIGNAL(readyRead()),SLOT(doReceiveData()));
    client->connectToHost("81.69.236.101",8888);

    //font_color
    connect(ui->colorBtn,&QPushButton::clicked,this,[=]()
    {
        QColor c = QColorDialog::getColor();
        if(c.isValid())
        {
            ui->record->setTextColor(c);
        }
    });

    //font_size
    connect(ui->fontsizeBtn,&QPushButton::clicked,this,[=]()
    {
        bool ok = true;
        QFont f = QFontDialog::getFont(&ok);
        ui->record->setFont(f);
    });


}

Client::~Client()
{
    delete ui;
    delete client;
}

void Client::doDisconnected()
{
    ui->record->append(tr("服务器已断开..."));
}

void Client::doConnected()
{
    ui->record->clear();
    ui->record->append(tr("服务器已连接..."));

    if(sq.ConnectMysql())
    {
        QString s = QString("select username,nickname from chatdata.user where username = '%1';").arg(tmp_usr_name);
        QString nic,welcome;
        QSqlQuery q;

        // 查询昵称
        q.exec(s);
        if(q.next())
        {
            nic = q.value(1).toString();
        }
        welcome = QString("Welcome,%1").arg(nic);
        ui->record->append(welcome);
        sendMsg(nic);
        QString tmp = '1' + nic;
        client->write(tmp.toUtf8());
    }
    else
    {
        ui->record->append(tr("数据库连接失败..."));
    }
}

void Client::doReceiveData()
{
    // recv data
    connect(client,&QTcpSocket::readyRead,this,[=]()
    {
        QByteArray buffer;
        buffer = client->readAll();
        if(!buffer.isEmpty())
        {
            QString str = tr(buffer).toUtf8();
            ui->record->append(str);
        }
    });
}

void Client::sendMsg(QString nic)
{
    connect(ui->sendBtn,&QPushButton::clicked,this,[=]()
    {
        client->write('0'+ui->message->text().toUtf8());
        QString msg =nic + ": "+ui->message->text().toUtf8();
        ui->record->append(msg);
        ui->message->clear();
    });
}

