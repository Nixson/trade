#include "depth.h"
#include <QDateTime>
#include <QJsonDocument>
#include <QJsonArray>
#include <iostream>
#include <QFile>
#include <QCoreApplication>

#include "memory.h"

using namespace std;

Depth::Depth(QObject *parent) : QObject(parent)
{
    dirPath = QCoreApplication::applicationDirPath();
    req = new NetRequest();
    req->moveToThread(&netThread);
    connect(this,&Depth::getUrl,req,&NetRequest::get);
    connect(req,&NetRequest::responseJSON,this,&Depth::responseJSON);
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
    pNum = 0;
}
void Depth::update(){
    emit getUrl(QUrl("https://btc-e.nz/api/3/depth/btc_usd?limit=1000"));
}

void Depth::setRest(Rest *rst){
    rest = rst;
}

void Depth::setWork(Worker *wrk){
    work = wrk;
    connect(this,&Depth::setMin,work,&Worker::setMin);
}

void Depth::responseJSON(QByteArray &info){
    QJsonParseError  parseError;
    QJsonDocument jdoc = QJsonDocument::fromJson(info,&parseError);
    if(parseError.error == QJsonParseError::NoError){
        if(jdoc.isObject()){
            QJsonObject jo = jdoc.object();
            parse(jo);
        }
    }else
        cout << "error parse JSON" << endl;
}
void Depth::parse(QJsonObject &json){
    QDateTime current = QDateTime::currentDateTime();
    uint tdime = current.toTime_t();
    iDepth depIn;
    for(auto iter = json.begin();iter!=json.end();  ++iter){
        QString idepth = iter.key();
        QJsonObject lists = iter.value().toObject();
        QJsonArray asks = lists["asks"].toArray();
        iTypes typeLst;
        iPairs pairsA;
        foreach (const QJsonValue &iterAsks, asks){
            iPair p;
            QJsonArray jo = iterAsks.toArray();
            p.price = jo[0].toVariant().toFloat();
            p.amount = jo[1].toVariant().toFloat();
            pairsA.append(p);
        }
        typeLst["asks"] = pairsA;
        QJsonArray bids = lists["bids"].toArray();
        iPairs pairsB;
        foreach (const QJsonValue &iterBids, bids){
            iPair p;
            QJsonArray jo = iterBids.toArray();
            p.price = jo[0].toVariant().toFloat();
            p.amount = jo[1].toVariant().toFloat();
            pairsB.append(p);
        }
        for(auto iterBids = bids.begin();iterBids!=bids.end();  ++iterBids){
        }
        typeLst["bids"] = pairsB;
        depIn[idepth] = typeLst;
    }
    dep[tdime] = depIn;
    Memory::set(tdime,depIn);
    if(!timerMin->isActive()){
        timerMin->start();
    }
    else {
        ++pNum;
    }


}
void Depth::print(QString name, iPairs pr){
    foreach(iPair p, pr){
        cout << name.toStdString() << ": " << p.price << endl;
    }
}
bool Depth::Asort(iPair a, iPair b){
    return a.price < b.price;
}
bool Depth::Dsort(iPair a, iPair b){
    return a.price > b.price;
}
void Depth::getMaxWs(int id, QString &depName, uint FindPeriod, uint ViewPeriod, float rate){
    ufBlock nextStep;
    for(auto iter = dep.begin();iter!=dep.end();  ++iter){
        if(iter.key() < ViewPeriod)
            continue;
        if(!iter.value().contains(depName))
            continue;
        iPairs ascPair = iter.value()[depName]["asks"];
        qSort(ascPair.begin(),ascPair.end(),Asort);
        iPairs bidPair = iter.value()[depName]["bids"];
        qSort(bidPair.begin(),bidPair.end(),Dsort);
        int cntA = 0;
        int cntB = 0;
        float summ = 0.0;
        foreach (iPair pr, ascPair){
            ++cntA;
            summ += pr.amount;
            if(summ >=rate)
                break;
        }
        summ = 0.0;
        foreach (iPair pr, bidPair){
            ++cntB;
            summ += pr.amount;
            if(summ >=rate)
                break;
        }
        infoBlock bl;
        bl.dtime = iter.key();
        bl.asks = (uint)cntA;
        bl.bids = (uint)cntB;
        if(cntB > 0)
            bl.range = (float)cntA/(float)cntB;
        else
            bl.range = 0.0;
        nextStep.insert(bl.dtime,bl);
    }
    emit setMaxWs(id,depName,FindPeriod,nextStep,rate);
}

void Depth::getLastWs(int id, QString &depName, uint period, float rate){
    ufBlock nextStep;
    for(auto iter = dep.begin();iter!=dep.end();  ++iter){
        if(iter.key() < period)
            continue;
        if(!iter.value().contains(depName))
            continue;
        iPairs ascPair = iter.value()[depName]["asks"];
        qSort(ascPair.begin(),ascPair.end(),Asort);
        iPairs bidPair = iter.value()[depName]["bids"];
        qSort(bidPair.begin(),bidPair.end(),Dsort);
        int cntA = 0;
        int cntB = 0;
        float summ = 0.0;
        foreach (iPair pr, ascPair){
            ++cntA;
            summ += pr.amount;
            if(summ >=rate)
                break;
        }
        summ = 0.0;
        foreach (iPair pr, bidPair){
            ++cntB;
            summ += pr.amount;
            if(summ >=rate)
                break;
        }
        infoBlock bl;
        bl.dtime = iter.key();
        bl.asks = (uint)cntA;
        bl.bids = (uint)cntB;
        if(cntB > 0)
            bl.range = (float)cntA/(float)cntB;
        else
            bl.range = 0.0;
        nextStep.insert(bl.dtime,bl);
    }
    emit setLastWs(id,depName,period,nextStep,rate);

}

void Depth::getMax(int id, const QString &depName, uint period, float cntElements, usMap &listTrades){
    //ufMap resp;
    QStringList respStr;
    for(auto iter = dep.begin();iter!=dep.end();  ++iter){
        if(iter.key() < period)
            continue;
        if(!iter.value().contains(depName))
            continue;
        iPairs ascPair = iter.value()[depName]["asks"];
        qSort(ascPair.begin(),ascPair.end(),Asort);
        iPairs bidPair = iter.value()[depName]["bids"];
        qSort(bidPair.begin(),bidPair.end(),Dsort);
        //print("ascPair",ascPair);
        //print("bidPair",bidPair);
        int cntA = 0;
        int cntB = 0;
        float summ = 0.0;
        foreach (iPair pr, ascPair){
            ++cntA;
            summ += pr.amount;
            if(summ >=cntElements)
                break;
        }
        summ = 0.0;
        foreach (iPair pr, bidPair){
            ++cntB;
            summ += pr.amount;
            if(summ >=cntElements)
                break;
        }
        QString trades = "";
        if(listTrades.contains(iter.key()))
            trades = listTrades[iter.key()];
        respStr << "["+QString::number(iter.key())+","+QString::number(cntA)+","+QString::number(cntB)+",\\\""+trades+"\\\"]";
    }
    QString rsp = "["+respStr.join(",")+"]";
    emit response(id,rsp);
}


void Depth::clearMin(){
    pNum = 0;
    QDateTime current = QDateTime::currentDateTime();
    QString data = current.toString("yyyy-MM-dd HH:00:00");
    QDateTime currentHour = QDateTime::fromString(data,"yyyy-MM-dd HH:00:00");
    uint tSafe = currentHour.toTime_t();
    //std::cout << data.toStdString() << std::endl;
    uint tdime = current.toTime_t();
    uint remove = tdime - 3600*24;
    uint lastRemove = tSafe - 3600*24;
    QHash <uint, iDepth> depSafe;
    for(auto dIterDep = dep.begin(); dIterDep!=dep.end();){
        uint dIter = dIterDep.key();
        if(dIter < remove){
            //cout << "clear Depth: " << dIter << " < " << remove << endl;
            dIterDep = dep.erase(dIterDep);
            Memory::rm(dIter,"depth");

        }
        else{
            if(dIter > tSafe){
                depSafe.insert(dIter,dIterDep.value());
            }
            ++dIterDep;
        }
    }
    if(QFile::exists(dirPath+"/depth"+QString::number(lastRemove)+".ob")){
        QFile::remove(dirPath+"/depth"+QString::number(lastRemove)+".ob");
    }
    //lastRemove
    QString filename = dirPath+"/depth"+QString::number(tSafe)+".ob";
       QFile depthFile(filename);
       if (!depthFile.open(QIODevice::WriteOnly))
       {
           qDebug() << "Could not write to file:" << filename << "Error string:" << depthFile.errorString();
           return;
       }
       QDataStream out(&depthFile);
       out.setVersion(QDataStream::Qt_5_3);
       out << depSafe;


}
void Depth::loadOb(){
    QDateTime current = QDateTime::currentDateTime();
    QString data = current.toString("yyyy-MM-dd HH:00:00");
    QDateTime currentHour = QDateTime::fromString(data,"yyyy-MM-dd HH:00:00");
    uint tSafe = currentHour.toTime_t();
    for( int i = 23; i >= 0; --i){
        QHash <uint, iDepth> depSafe;
        uint lastFile = tSafe - 3600*i;
        QString filename = dirPath+"/depth"+QString::number(lastFile)+".ob";
        QFile depthFile(filename);
        if(depthFile.exists()){
            if (depthFile.open(QIODevice::ReadOnly)){
                QDataStream in(&depthFile);
                in.setVersion(QDataStream::Qt_5_3);
                in >> depSafe;
            }
        }
        for(auto dI=depSafe.begin(); dI!=depSafe.end();++dI){
            dep.insert(dI.key(),dI.value());
            Memory::set(dI.key(),dI.value());
        }
        depSafe.clear();
    }

    /*QString filename = "depth.ob";
    QFile depthFile(filename);
    if(depthFile.exists()){
        if (!depthFile.open(QIODevice::ReadOnly))
        {
            qDebug() << "Could not read file:" << filename << "Error string:" << depthFile.errorString();
            return;
        }
        QDataStream in(&depthFile);
        in.setVersion(QDataStream::Qt_5_3);
        in >> dep;
    }*/
}

void Depth::updateMin(){
    pNum = 0;
    QDateTime current = QDateTime::currentDateTime();
    uint tdime = current.toTime_t();
    uint tdimeMin = tdime - 60;
    uint remove = tdime - 3600*24;
    /*Создаем массив за минуту*/
    QHash <uint, iDepth> tmp;
    QHash <QString, iFloat > block;
    uint cnt = 0;
    QHash <uint, iDepth>::const_iterator dIterDep = dep.constBegin();
    while (dIterDep != dep.constEnd()) {
        uint dIter = dIterDep.key();
        if(dIter > remove){
            if(dIter >= tdimeMin && dIter <= tdime){
                tmp[dIter] = dep[dIter];
                ++cnt;
                QHashIterator<QString, iTypes> iter(dep[dIter]);
                while(iter.hasNext()){
                    iter.next();
                    QString iName = iter.key();
                    if(!block.contains(iName)){
                        block[iName]["maxA"] = 0.0;
                        block[iName]["maxD"] = 0.0;
                        block[iName]["minA"] = 0.0;
                        block[iName]["minD"] = 0.0;
                    }
                    iTypes nxt = iter.value();
                    foreach (iPair pr, nxt["asks"]) {
                        if(block[iName]["maxA"] < pr.price)
                            block[iName]["maxA"] = pr.price;
                        if(block[iName]["minA"] > pr.price || block[iName]["minA"] == 0.0)
                            block[iName]["minA"] = pr.price;
                    }
                    foreach (iPair pr, nxt["bids"]) {
                        if(block[iName]["maxD"] < pr.price)
                            block[iName]["maxD"] = pr.price;
                        if(block[iName]["minD"] > pr.price || block[iName]["minD"] == 0.0)
                            block[iName]["minD"] = pr.price;
                    }
                }
            }
        }
        else
            dep.remove(dIter);
        ++dIterDep;
    }
    /*Генерируем загоровку*/
    mPair pair;
    pair.min = 0;
    pair.max = 0;
    iDepth src;
    QHash<QString, iFloat >::const_iterator i = block.constBegin();
    while (i != block.constEnd()) {
        QString name = i.key();
        if(pair.max < block[name]["maxA"])
            pair.max = block[name]["maxA"];
        if(pair.max < block[name]["maxD"])
            pair.max = block[name]["maxD"];
        if(pair.min > block[name]["minA"] || pair.min==0.0)
            pair.min = block[name]["minA"];
        if(pair.min > block[name]["minD"] || pair.min==0.0)
            pair.min = block[name]["minD"];
        block[name]["stepA"] = (block[name]["maxA"] - block[name]["minA"])/1000;
        block[name]["stepD"] = (block[name]["maxD"] - block[name]["minD"])/1000;
        iPairs pr;
        for(uint si = 0; si < 1000; ++si){
            iPair p;
            p.price = block[name]["minA"] + block[name]["stepA"]*si;
            p.amount = 0.0;
            pr.append(p);
        }
        src[name]["asks"] = pr;
        iPairs prB;
        for(uint si = 0; si < 1000; ++si){
            iPair p;
            p.price = block[name]["minA"] + block[name]["stepA"]*si;
            p.amount = 0.0;
            prB.append(p);
        }
        src[name]["bids"] = prB;
        ++i;
    }
    if(cnt > 0){
        /* Суммируем элементы массива и записываем в заготовку */
        foreach(iDepth dTmp, tmp){
            iDepth::const_iterator iDtmp = dTmp.constBegin();
            while (iDtmp != dTmp.constEnd()) {
                QString name = iDtmp.key();
                //asks
                //block[name]["stepA"]
                iPairs asks = iDtmp.value()["asks"];
                foreach (iPair pr, asks) {
                    int position = (int)((pr.price-block[name]["minA"])/block[name]["stepA"]);
                    if(position >= 1000)
                        position = 999;
                    src[name]["asks"][position].amount+=pr.amount;
                }



                //bids
                iPairs bids = iDtmp.value()["bids"];
                foreach (iPair pr, bids) {
                    int position = (int)((pr.price-block[name]["minD"])/block[name]["stepD"]);
                    if(position >= 1000)
                        position = 999;
                    src[name]["bids"][position].amount+=pr.amount;
                }
                ++iDtmp;
            }
        }
        iDepth::const_iterator iDSrc = src.constBegin();
        while (iDSrc != src.constEnd()) {
            QString name = iDSrc.key();
            //asks
            iPairs asks = iDSrc.value()["asks"];
            uint num = 0;
            foreach (iPair pr, asks) {
                src[name]["asks"][num].amount = pr.amount/cnt;
                ++num;
            }


            //bids
            num = 0;
            iPairs bids = iDSrc.value()["bids"];
            foreach (iPair pr, bids) {
                src[name]["bids"][num].amount = pr.amount/cnt;
                ++num;
            }
            ++iDSrc;
        }
    }
    tmp.clear();
    //emit setMin(tdimeMin,src, pair);
}
//        cout << iter.key().toStdString() << endl;
