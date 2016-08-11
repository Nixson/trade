#include "rest.h"
#include <iostream>
#include <QDateTime>
#include <QUrl>
#include <QUrlQuery>

using namespace std;

Rest::Rest(QObject *parent) : QObject(parent)
{
    server = new QTcpServer(this);
    connect(server, SIGNAL(newConnection()),this, SLOT(restConnection()));
    server->listen(QHostAddress::Any,5601);
    cout << "listen: 5601 " << endl;
}

void Rest::restConnection(){
    QTcpSocket* socket = server->nextPendingConnection();
    int idusersocs=socket->socketDescriptor();
    SClients.insert(idusersocs,socket);
    connect(socket, &QTcpSocket::readyRead,[this, socket, idusersocs](){
        this->txRx(socket, idusersocs);
    });
}
void Rest::txRx(QTcpSocket* clientSocket, int idusersocs){
    QByteArray arr =  clientSocket->readAll();
    QString info(arr);
    QStringList subInfo = info.split("\n");
    QStringList req = subInfo[0].split(" ");
    if(req[0]!="GET" || req[1]=="/" || req[1]=="/favicon.ico"){
        QTextStream os(clientSocket);
        os.setAutoDetectUnicode(true);
        os << "HTTP/1.0 404 Not Found\r\n"
              "Content-Type: application/json; charset=\"utf-8\"\r\n"
              "\r\n"
              "{\"error\":\"not found\"}\n";
        clientSocket->close();
        SClients.remove(idusersocs);
        return;
    }
    QStringList he = req[1].split("/");
    if(he.size()==2)
        emit generate(idusersocs,he[1]);
    else {
        if(he.size() > 2) {
            QUrl q(req[1]);
            QUrlQuery query(q);
            QString findRangeStr = query.queryItemValue("findRange");
            QString viewRangeStr = query.queryItemValue("viewRange");
            QString rateStr = query.queryItemValue("rate");
            int rate, findRange, viewRange;
            QDateTime current = QDateTime::currentDateTime();
            //current.toTime_t()-
            try {
                rate = rateStr.toInt();
                findRange = findRangeStr.toInt();
                viewRange = viewRangeStr.toInt();
            }
            catch(...){
                rate = 10;
                findRange = 3600;
                viewRange = 100;
            }
            findRange = current.toTime_t() - findRange;
            viewRange = current.toTime_t() - viewRange;
            emit getMaxTrades(idusersocs,he[1],findRange, viewRange, rate);
        }

    }
}
void Rest::response(int id, QString &rest){
    QTcpSocket* clientSocket = SClients[id];
    QTextStream os(clientSocket);
    os.setAutoDetectUnicode(true);
    os << "HTTP/1.0 200 Ok\r\n"
          "Content-Type: application/json; charset=\"utf-8\"\r\n"
          "\r\n"
          "{\"response\":\""+rest+"\"}\n";
    clientSocket->close();
    SClients.remove(id);
}
