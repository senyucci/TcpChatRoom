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
    client->connectToHost(HOST,PORT);

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
        welcome = welcomeTime();
        welcome += QString(", %1").arg(nic);
        ui->record->append(welcome);

        QString tmp = NAME + nic;
        client->write(tmp.toUtf8());
        doSendMsg();
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

void Client::doSendMsg()
{
    connect(ui->sendBtn,&QPushButton::clicked,this,[=]()
    {
        client->write( MSG + ui->message->text().toUtf8());
        ui->message->clear();
    });
}

QString Client::welcomeTime()
{
    QTime *time = new QTime();
    QString welcome,strTime,strHour;
    strTime = time->currentTime().toString("hh:mm:ss");
    strHour = strTime.mid(0,2);
    int hour = strHour.toInt();
    if(hour>=5&&hour<11)
    {
        welcome = "早上好";
        return welcome;
    }
    else if(hour>=11&&hour<13)
    {
        welcome = "中午好";
        return welcome;
    }
    else if(hour>=13&&hour<18)
    {
        welcome = "下午好";
        return welcome;
    }
    else
    {
        welcome = "晚上好";
        return welcome;
    }
    welcome = "error";
    return welcome;
}
