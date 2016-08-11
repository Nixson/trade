#include "trades.h"
#include <QDateTime>
#include <QJsonDocument>
#include <QJsonArray>
#include <iostream>
#include <QFile>
#include <QCoreApplication>
#include "memory.h"

using namespace std;


Trades::Trades(QObject *parent) : QObject(parent)
{
    dirPath = QCoreApplication::applicationDirPath();
    req = new NetRequest();
    req->moveToThread(&netThread);
    connect(this,&Trades::getUrl,req,&NetRequest::get);
    connect(req,&NetRequest::responseJSON,this,&Trades::responseJSON);
    connect(&netThread, &QThread::finished, req, &QObject::deleteLater);
    netThread.start();
    loadOb();
    timer = new QTimer(this);
    timer->setInterval(1000);
    connect(timer, SIGNAL(timeout()), this, SLOT(update()));
    timer->start();
    timerMin = new QTimer(this);
    timerMin->setInterval(60000);
//    connect(timerMin, SIGNAL(timeout()), this, SLOT(updateMin()));
    connect(timerMin, SIGNAL(timeout()), this, SLOT(clearMin()));
}
void Trades::update(){
    emit getUrl(QUrl("https://btc-e.nz/api/3/trades/btc_usd?limit=1000"));
}

void Trades::setRest(Rest *rst){
    rest = rst;
}
void Trades::setWork(Worker *wrk){
    work = wrk;
    connect(this,&Trades::setMin,work,&Worker::setMin);
}
void Trades::responseJSON(QByteArray &info){
    QJsonParseError  parseError;
    QJsonDocument jdoc = QJsonDocument::fromJson(info,&parseError);
    if(parseError.error == QJsonParseError::NoError){
        if(jdoc.isObject()){
            parse(jdoc.object());
        }
    }else
        cout << "error parse JSON" << endl;
}

void Trades::getMax(int id, QString &dep, uint FindPeriod, uint ViewPeriod, uint rate){
    float max = 0.0;
    usMap ml;
    for(auto iter = trade.begin();iter!=trade.end();  ++iter){
        if(iter.key() < FindPeriod)
            continue;
        if(!iter.value().contains(dep))
            continue;
        iTrades td = iter.value()[dep];
        QStringList lCnt;
        foreach (iTrade tradeElement, td) {
            if(tradeElement.amount > max)
                max = tradeElement.amount;
            lCnt << QString::number(tradeElement.price)+"("+QString::number(tradeElement.amount)+")";
        }
        ml.insert(iter.key(),lCnt.join(", "));

    }
    emit setMax(id, dep,ViewPeriod, max*(float)rate, ml);

}
void Trades::getMaxWs(int id, QString &dep, uint FindPeriod, uint ViewPeriod, uint rate){
    float max = 0.0;
    for(auto iter = trade.begin();iter!=trade.end();  ++iter){
        if(iter.key() < FindPeriod)
            continue;
        if(!iter.value().contains(dep))
            continue;
        iTrades td = iter.value()[dep];
        foreach (iTrade tradeElement, td) {
            if(tradeElement.amount > max)
                max = tradeElement.amount;
        }
    }
    emit setMaxWs(id, dep, FindPeriod,ViewPeriod, max*(float)rate);
}

void Trades::getLastWsRange(int id, QString &dep, uint FindPeriod, ufBlock &listDepth, float resp){
    for(auto iter = trade.begin();iter!=trade.end();  ++iter){
        uint period = iter.key();
        if(period < FindPeriod)
            continue;
        if(!iter.value().contains(dep))
            continue;
        if(!listDepth.contains(period)){
            bool find = false;
            infoBlock bdata;
            bdata.dtime = period;
            for(uint subperiod = period; subperiod > period-10; --subperiod){
                if(listDepth.contains(subperiod)){
                    find = true;
                    bdata.asks = listDepth[subperiod].asks;
                    bdata.bids = listDepth[subperiod].bids;
                    bdata.range = listDepth[subperiod].range;
                }
            }
            if(!find)
                continue;
            listDepth.insert(period,bdata);
        }
        iTrades td = iter.value()[dep];
        foreach (iTrade tradeElement, td) {
            listDepth[period].price.append(tradeElement.price);
        }
    }
    emit responseLastWs(id,listDepth, resp);
}

void Trades::getMaxWsRange(int id, QString &dep, uint FindPeriod, ufBlock &listDepth, float resp){
    for(auto iter = trade.begin();iter!=trade.end();  ++iter){
        uint period = iter.key();
        if(period < FindPeriod)
            continue;
        if(!iter.value().contains(dep))
            continue;
        if(!listDepth.contains(period)){
            bool find = false;
            infoBlock bdata;
            bdata.dtime = period;
            for(uint subperiod = period; subperiod > period - 10; --subperiod){
                if(listDepth.contains(subperiod)){
                    find = true;
                    bdata.asks = listDepth[subperiod].asks;
                    bdata.bids = listDepth[subperiod].bids;
                    bdata.range = listDepth[subperiod].range;
                }
            }
            if(!find)
                continue;
            listDepth.insert(period,bdata);
        }
        iTrades td = iter.value()[dep];
        foreach (iTrade tradeElement, td) {
            listDepth[period].price.append(tradeElement.price);
        }
    }
    cout << "responseWs" << id << endl;
    emit responseWs(id,listDepth, resp);
}
void Trades::parse(QJsonObject json){
    QList <uint> tsp;
    for(auto iter = json.begin();iter!=json.end();  ++iter){
        QString idepth = iter.key();//btc_usd
        QJsonArray lists = iter.value().toArray();
        foreach (const QJsonValue &iterList, lists){
            QJsonObject jo = iterList.toObject();
            iTrade tr;
            tr.type = jo["type"].toString();
            tr.tid = jo["tid"].toInt();
            tr.price = (float)jo["price"].toDouble();
            tr.amount = (float)jo["amount"].toDouble();
            int timestamp = (uint)jo["timestamp"].toInt();
            if(!trade.contains(timestamp)){
                tsp.append(timestamp);
                iTrades tRs;
                tRs.append(tr);
                iTradesLst lst;
                lst[idepth] = tRs;
                trade.insert(timestamp,lst);

            }
            else {
                if(!trade[timestamp].contains(idepth)){
                    iTrades tRs;
                    tRs.append(tr);
                    trade[timestamp][idepth] = tRs;
                    tsp.append(timestamp);
                }else {
                    bool isIn = false;
                    foreach (iTrade tTemp, trade[timestamp][idepth]){
                        if(tTemp.tid==tr.tid){
                            isIn = true;
                            break;
                        }
                    }
                    if(!isIn){
                        trade[timestamp][idepth].append(tr);
                        tsp.append(timestamp);
                    }
                }
            }
        }
    }
    if(tsp.length() > 0){
        foreach (uint tstamp, tsp) {
            Memory::set(tstamp,trade[tstamp]);
        }
    }
    if(!timerMin->isActive()){
        timerMin->start();
    }
}

void Trades::updateMin(){
}
void Trades::clearMin(){
    QDateTime current = QDateTime::currentDateTime();
    uint tdime = current.toTime_t();
    uint remove = tdime - 3600*24;
    for(auto dIter = trade.begin(); dIter!=trade.end();){
        uint dIterTime = dIter.key();
        if(dIterTime < remove){
            //cout << "clear Trades: " << dIterTime << " < " << remove << endl;
            dIter = trade.erase(dIter);
            Memory::rm(dIterTime,"trade");
        }
        else
            ++dIter;
    }
    QString filename = dirPath+"/trades.ob";
       QFile depthFile(filename);
       if (!depthFile.open(QIODevice::WriteOnly))
       {
           qDebug() << "Could not write to file:" << filename << "Error string:" << depthFile.errorString();
           return;
       }
       QDataStream out(&depthFile);
       out.setVersion(QDataStream::Qt_5_3);
       out << trade;

}
void Trades::loadOb(){
    QString filename = dirPath+"/trades.ob";
    QFile depthFile(filename);
    if(depthFile.exists()){
        if (!depthFile.open(QIODevice::ReadOnly))
        {
            qDebug() << "Could not read file:" << filename << "Error string:" << depthFile.errorString();
            return;
        }
        QDataStream in(&depthFile);
        in.setVersion(QDataStream::Qt_5_3);
        in >> trade;
        for(auto tu = trade.begin(); tu!= trade.end();++tu){
            Memory::set(tu.key(),tu.value());

        }
    }
}
