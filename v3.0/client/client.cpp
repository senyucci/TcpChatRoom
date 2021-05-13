#include "client.h"
#include "ui_client.h"


Client::Client(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Client)
{
    ui->setupUi(this);


    // init
    client = new QTcpSocket(this);

    // connect
    client->connectToHost("81.69.236.101",8888);

    // recv data
    connect(client,&QTcpSocket::readyRead,this,[=]()
    {
        QByteArray arr = client->readAll();
        ui->record->append(arr);

    });

    connect(ui->sendBtn,&QPushButton::clicked,this,[=]()
    {
        client->write(ui->message->text().toUtf8());
        ui->record->append("Me: "+ui->message->text());
        ui->message->clear();
    });

}

Client::~Client()
{
    delete ui;
}
