#include <QThreadPool>
#include "wsworker.h"
#include <iostream>
#include "wstask.h"

WsWorker::WsWorker(QObject *parent) : QObject(parent), last(0)
{
    m_pWebSocketServer = new QWebSocketServer(QStringLiteral("WsTask"),
                                            QWebSocketServer::NonSecureMode, this);
    m_pWebSocketServer->listen(QHostAddress::Any, 5603);
    connect(m_pWebSocketServer, &QWebSocketServer::newConnection, this, &WsWorker::onNewConnection);
    connect(m_pWebSocketServer, &QWebSocketServer::closed, this, &WsWorker::closed);
    std::cout << "listen WsWorker 5603" << std::endl;

}
WsWorker::~WsWorker(){
    m_pWebSocketServer->close();
}
void WsWorker::onNewConnection(){
    QWebSocket *pSocket = m_pWebSocketServer->nextPendingConnection();
    connect(pSocket, &QWebSocket::textMessageReceived, this, &WsWorker::processTextMessage);
    connect(pSocket, &QWebSocket::disconnected, this, &WsWorker::socketDisconnected);
    m_clients.insert(pSocket, last);
    am_clients.insert(last,pSocket);
    iWsUser client;
    client.User = last;
    client.type = "btc_usd";
    client.timeIn = 1200;
    client.timeOut = 3600;
    client.timeRange = 2;
    client.rateIn = 1;
    client.rateOut = 10;
    client.rateRange = 5;
    client.priceIn = 10.0;
    client.priceOut = 20.0;
    client.priceRange = 2;
    client.rangeLabel = 1.5;
    client.tasks = 0;
    user.insert(last,client);
    ++last;
}
void WsWorker::socketDisconnected(){
    QWebSocket *pClient = qobject_cast<QWebSocket *>(sender());
    if(pClient){
        if(m_clients.contains(pClient)){
            int lst = m_clients.value(pClient);
            m_clients.remove(pClient);
            am_clients.remove(lst);
            tmpUser.remove(lst);
        }
        try {
            pClient->deleteLater();
        }
        catch (...){
            std::cout << "error!" << std::endl;
        }
    }
}
void WsWorker::processTextMessage(QString message){
    QWebSocket *pClient = qobject_cast<QWebSocket *>(sender());
    if (pClient) {
        if(!m_clients.contains(pClient)){
            return;
        }
        int idusersocs = m_clients.value(pClient);
        QStringList rsp = message.split(";");
        if(rsp.size() < 9){
            return;
        }
        QDateTime current = QDateTime::currentDateTime();
        uint currentInt = current.toTime_t();

        try {
            user[idusersocs].type       = rsp[0];
            user[idusersocs].timeIn     = 0;//rsp[1].toInt();
            user[idusersocs].timeOut    = rsp[1].toInt();
            user[idusersocs].timeRange  = rsp[2].toInt();
            user[idusersocs].rateIn     = rsp[3].toInt();
            user[idusersocs].rateOut    = rsp[4].toInt();
            user[idusersocs].rateRange  = rsp[5].toInt();
            user[idusersocs].priceIn    = rsp[6].toFloat();
            user[idusersocs].priceOut   = rsp[7].toFloat();
            user[idusersocs].priceRange = rsp[8].toInt();
            user[idusersocs].rangeLabel = rsp[9].toFloat();
            std::cout << rsp[9].toStdString() << ": " << rsp[9].toFloat() << std::endl;

        }
        catch(...){}

        user[idusersocs].taskLast = 0;
        user[idusersocs].tasks = 0;
        user[idusersocs].task.clear();
        user[idusersocs].result.clear();

        uint timeRange  = (user[idusersocs].timeOut - user[idusersocs].timeIn)/user[idusersocs].timeRange;
        uint rateRange  = (user[idusersocs].rateOut - user[idusersocs].rateIn)/user[idusersocs].rateRange;

        uint priceRange = (int)((user[idusersocs].priceOut - user[idusersocs].priceIn)/user[idusersocs].priceRange);

        std::cout << "QThreadPool init" << std::endl;
        for(uint rangeTime = 1; rangeTime < user[idusersocs].timeRange; ++rangeTime){
            uint timeStart = currentInt - user[idusersocs].timeIn - timeRange*rangeTime;
            if(currentInt - timeStart < 60*20)
                continue;
            for(uint rangeRate = 0; rangeRate < user[idusersocs].rateRange; ++rangeRate){
                uint rate = user[idusersocs].rateIn + rateRange*rangeRate;
                for(uint rangePrice = 0; rangePrice < user[idusersocs].priceRange; ++rangePrice){
                    float price = user[idusersocs].priceIn + (float)priceRange*rangePrice;
                    iTask task;
                    task.iduser = idusersocs;
                    task.type = user[idusersocs].type;
                    task.PeriodStart = timeStart;
                    task.PeriodStop = currentInt;
                    task.rate = (float)rate;
                    task.perc = price;
                    poolIn(task);
                    ++user[idusersocs].tasks;
                    ++user[idusersocs].taskLast;
                }
            }
        }
        user[idusersocs].task.reserve(user[idusersocs].tasks);
        user[idusersocs].result.reserve(user[idusersocs].tasks);
        std::cout << "QThreadPool start: " << user[idusersocs].tasks << std::endl;
    }

}
void WsWorker::poolIn(iTask task){
    WsTask *wtask = new WsTask(task);
    connect(wtask,&WsTask::response,this,&WsWorker::response);
    QThreadPool::globalInstance()->start(wtask);
}
void WsWorker::response(iTask *step, iTaskResult *result){
    int key = user[step->iduser].tasks - user[step->iduser].taskLast;
    --user[step->iduser].taskLast;
    user[step->iduser].task.append(step);
    user[step->iduser].result.append(result);
    std::cout << key << std::endl;
    if((uint)user[step->iduser].task.length() == user[step->iduser].tasks){
        QDateTime current = QDateTime::currentDateTime();
        std::cout << "\n responseOut" << std::endl;
        float bad = 0.0;
        float good = 0.0;
        iTask *goodStep = new iTask();
        goodStep->PeriodStart =current.toTime_t();
        goodStep->perc = 0;
        goodStep->rate = 0;

        iTask *badStep = new iTask();
        badStep->PeriodStart =current.toTime_t();
        badStep->perc = 0;
        badStep->rate = 0;

        bool find = false;
        bool findReverse = false;

        for (auto key = 0; key < user[step->iduser].result.length(); ++key) {
            auto info = user[step->iduser].result[key];
            float est = 0.0;
            if(info->bad > 0){
                est = (float)info->good/info->bad;
                if(bad==0.0){
                    bad = est;
                    if(user[step->iduser].rangeLabel < 1/bad){
                        findReverse = true;
                        badStep = user[step->iduser].task[key];
                    }
                }
                else if(bad > est) {
                    bad = est;
                    if(user[step->iduser].rangeLabel < 1/bad){
                        findReverse = true;
                        badStep = user[step->iduser].task[key];
                    }
                }
            }
            if(good < est){
                good = est;
                if(good > user[step->iduser].rangeLabel){
                    find = true;
                    std::cout << "result: " << key << std::endl;
                    goodStep = user[step->iduser].task[key];
                }
            }
            if(bad > est && est > 0.01) {
                bad = est;
                if(user[step->iduser].rangeLabel < 1/bad){
                    findReverse = true;
                    badStep = user[step->iduser].task[key];
                }
            }
            //std::cout << "result: " << info->good << ":\t" << info->bad << ":\t" << est << std::endl;
        }
        std::cout << "best: " << good << std::endl;
        std::cout << "bad: " << bad << std::endl;
        if(find || findReverse){
            if(find)
                std::cout << "best: find" << std::endl;
            if(findReverse)
                std::cout << "bad: find" << std::endl;

            if(find && findReverse){
                if(good > 1/bad)
                    findReverse = false;
                else
                    find = false;
            }
            QString msg;
            if(find){
                uint currentInt = current.toTime_t() - goodStep->PeriodStart;
                msg = "{\"dtime\":"+QString::number((int)currentInt)
                        +",\"perc\":"+QString::number((int)goodStep->perc)
                        +",\"rate\":"+QString::number((int)goodStep->rate)
                        +",\"reverse\": 0"
                        +",\"good\":\""+QString::number(good)+"\",\"bad\":\""+QString::number(bad)+"\"}";
            }
            else {
                uint currentInt = current.toTime_t() - badStep->PeriodStart;
                msg = "{\"dtime\":"+QString::number((int)currentInt)
                        +",\"perc\":"+QString::number((int)badStep->perc)
                        +",\"rate\":"+QString::number((int)badStep->rate)
                        +",\"reverse\": 1"
                        +",\"good\":\""+QString::number(good)+"\",\"bad\":\""+QString::number(bad)+"\"}";
            }
            if(am_clients.contains(step->iduser))
                am_clients[step->iduser]->sendTextMessage(msg);
        }else {
            QString msg = "{\"dtime\":0,\"perc\":0,\"rate\":0,\"good\":\""+QString::number(good)+"\",\"reverse\":0,\"bad\":\""+QString::number(bad)+"\"}";
            if(am_clients.contains(step->iduser))
                am_clients[step->iduser]->sendTextMessage(msg);
        }
        user[step->iduser].tasks = 0;
    }
}
