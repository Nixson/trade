#include "wsrest.h"
#include <QTimer>
#include <iostream>

WSrest::WSrest(QObject *parent) : QObject(parent), last(1)
{
    m_pWebSocketServer = new QWebSocketServer(QStringLiteral("WSrest"),
                                            QWebSocketServer::NonSecureMode, this);
    m_pWebSocketServer->listen(QHostAddress::Any, 5602);
    connect(m_pWebSocketServer, &QWebSocketServer::newConnection, this, &WSrest::onNewConnection);
    connect(m_pWebSocketServer, &QWebSocketServer::closed, this, &WSrest::closed);
    std::cout << "listen ws" << std::endl;
}

WSrest::~WSrest(){
    m_pWebSocketServer->close();
}
void WSrest::onNewConnection(){
    QDateTime current = QDateTime::currentDateTime();
    QWebSocket *pSocket = m_pWebSocketServer->nextPendingConnection();
    connect(pSocket, &QWebSocket::textMessageReceived, this, &WSrest::processTextMessage);
    connect(pSocket, &QWebSocket::binaryMessageReceived, this, &WSrest::processBinaryMessage);
    connect(pSocket, &QWebSocket::disconnected, this, &WSrest::socketDisconnected);
    m_clients.insert(pSocket, last);
    am_clients.insert(last,pSocket);
    iRate cl;
    cl.type = "btc_usd";
    cl.find = 3600;
    cl.view = 100;
    cl.maxSt = 30;
    cl.rate = 10;
    cl.perc = 20.0;
    cl.rateFloat = 10.0;
    cl.lastPrice = 0.0;
    cl.min = 0.0;
    cl.max = 0.0;
    cl.lastRange = 0.0;
    cl.reverse = false;
    cl.lastPeriod =current.toTime_t();
    rate.insert(last,cl);
    iReal rc;
    rc.count = 0;
    rc.price = 0.0;
    ireal.insert(last,rc);
    ++last;
}

void WSrest::socketDisconnected(){
    QWebSocket *pClient = qobject_cast<QWebSocket *>(sender());
    if(pClient){
        if(m_clients.contains(pClient)){
            int lst = m_clients.value(pClient);
            rate.remove(lst);
            m_clients.remove(pClient);
            am_clients.remove(lst);
            rateUser.remove(lst);
            lastAsc.remove(lst);
            rangeUser.remove(lst);
            tmpUser.remove(lst);
            std::cout << "end: " << lst << " ";
        }
        try {
            pClient->deleteLater();
            std::cout << "no error" << std::endl;
        }
        catch (...){
            std::cout << "error!" << std::endl;
        }
    }
}
void WSrest::processBinaryMessage(QByteArray message)
{
    QWebSocket *pClient = qobject_cast<QWebSocket *>(sender());
    if (pClient) {
        pClient->sendBinaryMessage(message);
    }
}

void WSrest::updTmpTable(int id, QVector <tmpTable> &tt,  ufBlock &rest){
    QDateTime current = QDateTime::currentDateTime();
    uint cTime = current.toTime_t();
    if(!tmpUser.contains(id)){
        ufBlock tmp;
        tmpUser[id] = tmp;
    }
    uint remove = cTime - rate[id].view;
    //std::cout << "tmpUser.remove: " << remove << "rate[id].view: " << rate[id].view << std::endl;
    for(auto dIter = tmpUser[id].begin(); dIter!=tmpUser[id].end();){
        uint dIterTime = dIter.key();
        if(dIterTime < remove){
            //std::cout << "clear tmpUser: " << dIterTime << " < " << remove << std::endl;
            dIter = tmpUser[id].erase(dIter);
        }
        else
            ++dIter;
    }
    //std::cout << "tmpUser.size: " << tmpUser.size() << std::endl;
    for (auto i = rest.begin(); i != rest.end(); ++i){
        infoBlock info = i.value();
        if(info.dtime > rate[id].lastPeriod){
            rate[id].lastRange = info.range;
        }
        if(info.price.size() == 0)
            continue;
        tmpUser[id].insert(i.key(),info);
    }
    rate[id].min = 0.0;
    rate[id].max = 0.0;
    //std::cout << "tt.size: " << tt.size() << std::endl;
    tt.clear();
    //std::cout << "tt.size: " << tt.size() << std::endl;
    double zCmin = ireal[id].volatilityRange/ireal[id].volatilitymin;
    double zCmax = ireal[id].volatilityRange/ireal[id].volatilitymax;
    for (auto i = tmpUser[id].begin(); i != tmpUser[id].end(); ++i){
        bool isLast = false;
        infoBlock info = i.value();
        if(info.dtime > rate[id].lastPeriod){
            isLast = true;
            rate[id].lastPeriod = info.dtime;
        }
        if (rate[id].min == 0.0)
            rate[id].min = info.range;
        if(rate[id].min > info.range)
            rate[id].min = info.range;
        if(rate[id].max < info.range)
            rate[id].max = info.range;
        foreach(float price, info.price){
            char pos;
            if(price > rate[id].lastPrice)
                pos = '>';
            else if(price < rate[id].lastPrice)
                pos = '<';
            else
                pos = '=';
            if(isLast && ireal[id].depth.length() > 0){
                emit echo("Последний. Ожиданий фиксаций: "+QString::number(ireal[id].depth.length()));
                double depthRespMax = (double)price - zCmax;
                double depthRespMin = (double)price + zCmin;
                if(rate[id].reverse){
                    depthRespMax = (double)price - zCmin;
                    depthRespMin = (double)price + zCmax;
                }
                emit echo("Порог фиксации верхний: "+QString::number(depthRespMax));
                emit echo("Порог фиксации нижний : "+QString::number(depthRespMin));
                for(int positionTrDep = 0; positionTrDep < ireal[id].depth.size(); ++positionTrDep) {
                    trDepth element = ireal[id].depth.at(positionTrDep);
                    if(element.type){
                        emit echo("Фиксация. Покупка: "+QString::number(element.value));
                        // была покупка
                        if(ireal[id].volatility > 0 &&
                                (ireal[id].volatility == 1 ||
                                    (depthRespMax >= element.value || depthRespMin <= element.value)
                                 )
                             ){     //продаем в плюс                //продаем в минус
                            ireal[id].count--;
                            ireal[id].price += price;
                            ireal[id].depth.removeAt(positionTrDep);
                            --positionTrDep;
                            strResponse rsp;
                            rsp.dtime = info.dtime;
                            rsp.price = price;
                            rsp.range = info.range;
                            rsp.diffR = 0.0;
                            rsp.diffM = std::abs(rsp.price - rate[id].lastPrice);
                            rangeUser[id].append(rsp);
                        }
                    }
                    else {
                        emit echo("Фиксация. Продажа: "+QString::number(element.value));
                        // была продажа
                        if(ireal[id].volatility > 0 &&
                                (ireal[id].volatility == 1 ||
                                    (depthRespMax >= element.value || depthRespMin <= element.value)
                                 )
                            ){      //покупаем в минус                //покупаем в плюс
                            ireal[id].count++;
                            ireal[id].price -= price;
                            ireal[id].depth.removeAt(positionTrDep);
                            --positionTrDep;
                            strResponse rsp;
                            rsp.dtime = info.dtime;
                            rsp.price = price;
                            rsp.range = info.range;
                            rsp.diffR = 0.0;
                            rsp.diffM = std::abs(rsp.price - rate[id].lastPrice);
                            rangeUser[id].append(rsp);
                        }
                    }
                    ++positionTrDep;

                }

            }

            if(isLast && (pos=='>' || pos=='<')){
                if(lastAsc.contains(id)){
                    emit echo("Ожидание. Диапазон: "+QString::number(info.range));
                    emit echo("Ожидание. Максимум: "+QString::number(lastAsc[id].min));
                    emit echo("Ожидание. Минимум : "+QString::number(lastAsc[id].max));
                    if(info.range >= lastAsc[id].min && info.range <= lastAsc[id].max){
                        trDepth trD;
                        strResponse rsp;
                        rsp.dtime = info.dtime;
                        rsp.price = price;
                        rsp.range = info.range;
                        rsp.diffR = 0.0;
                        if(lastAsc[id].pos==pos){
                            rsp.response = true;
                        }
                        else {
                            rsp.response = false;
                        }
                        if(rate[id].reverse)
                            rsp.response = !rsp.response;
                        if(lastAsc[id].pos=='>'){
                            if(rate[id].reverse){
                                trD.type = false;
                                ireal[id].count--;
                                ireal[id].price += price;
                            }
                            else {
                                trD.type = true;
                                ireal[id].count++;
                                ireal[id].price -= price;
                            }
                        }
                        else {
                            if(rate[id].reverse){
                                trD.type = true;
                                ireal[id].count++;
                                ireal[id].price -= price;
                            }
                            else {
                                trD.type = false;
                                ireal[id].count--;
                                ireal[id].price += price;
                            }
                        }
                        trD.value = price;
                        std::cout << "tr: " << lastAsc[id].pos << " " << pos << rate[id].reverse << rsp.response << std::endl;
                        rsp.diffM = std::abs(rsp.price - rate[id].lastPrice);
                        rangeUser[id].append(rsp);
                        ireal[id].depth.append(trD);
                        lastAsc.remove(id);
                    }
                }
            }



            bool find = false;
            for(auto it = tt.begin(); it!=tt.end();++it){
                if(it->pos==pos && it->range==info.range){
                    it->count++;
                    find = true;
                    break;
                }
            }
            if(!find){
                tmpTable dt;
                dt.count = 1;
                dt.pos = pos;
                dt.range = info.range;
                tt.append(dt);
            }
            rate[id].lastPrice = price;

        }
    }
    subRange(id,tt);
}
void WSrest::subRange(int id, QVector <tmpTable> &tt){
    if(rate[id].max - rate[id].min < 0.001)
        return;
    //std::cout << "subRange min: " << rate[id].min<< ", max: " << rate[id].max << std::endl;
    //float ranges = 0.1;
    //int rangesCnt = (int)(rate[id].max-rate[id].min)/ranges;
    float ranges = (rate[id].max-rate[id].min)/10.0;
    bool hasMin = false;
    int summ = 0;
    for(int i = 0; i < 10; ++i){
        float min = rate[id].min + ranges*i;
        float max = rate[id].min + ranges*(i+1);
        int cnt = 0;
        foreach(tmpTable dt, tt){
            if(dt.range >= min && dt.range <= max)
                ++cnt;
        }
        if(i == 0 && cnt <= 5 ){
            hasMin = true;
            //std::cout << "cnt (0): " << cnt << std::endl;
            rate[id].min += ranges;
        }
        summ +=cnt;
        if(i == 9 && cnt <= 5 ){
            hasMin = true;
            //std::cout << "cnt (9): " << cnt << std::endl;
            rate[id].max -= ranges;
        }
    }
    if(summ > 25 && hasMin && rate[id].min < rate[id].max){
        //std::cout << "subRange step min: " << rate[id].min << ", max: " << rate[id].max << std::endl;
        subRange(id,tt);
    }
}
QVector <strTable> WSrest::reRange(int id){
    QVector <tmpTable> tt = rateUser[id];
    QVector <strTable> sdata;
    //float ranges = (rate[id].max-rate[id].min)/10.0;
    float ranges = 0.1;
    int rangesCnt = (int)((rate[id].max-rate[id].min)/ranges);
//    std::cout << "rate: " << rate[id].max << " x " << rate[id].min << std::endl;
//    std::cout << "count: " << rangesCnt << std::endl;
    bool rrange = true;
    for(int i = 0; i < rangesCnt; ++i){
        float min = rate[id].min + ranges*i;
        float max = rate[id].min + ranges*(i+1);
        int cntF = 0;
        int cntM = 0;
        int cntR = 0;
        strTable stF;
        {
            stF.min = min;
            stF.max = max;
            stF.pos = '>';
            stF.count = 0;
            foreach(tmpTable dt, tt){
                bool w = dt.range < stF.max;
                if(i==(rangesCnt-1))
                    w = dt.range <= stF.max;
                if(dt.range >= stF.min && w && dt.pos==stF.pos)
                    stF.count++;
            }
            cntF = stF.count;
        }
        strTable stM;
        {
            stM.min = min;
            stM.max = max;
            stM.pos = '<';
            stM.count = 0;
            foreach(tmpTable dt, tt){
                bool w = dt.range < stM.max;
                if(i==(rangesCnt-1))
                    w = dt.range <= stM.max;
                if(dt.range >= stM.min && w && dt.pos==stM.pos)
                    stM.count++;
            }
            cntM = stM.count;
        }
        strTable stR;
        {
            stR.min = min;
            stR.max = max;
            stR.pos = '=';
            stR.count = 0;
            foreach(tmpTable dt, tt){
                bool w = dt.range < stR.max;
                if(i==(rangesCnt-1))
                    w = dt.range <= stR.max;
                if(dt.range >= stR.min && w && dt.pos==stR.pos)
                    stR.count++;
            }
            cntR = stR.count;
        }
        int rSumm = cntM+cntF+cntR;
        if(rSumm >= rate[id].maxSt ){
            sdata.append(stF);
            sdata.append(stM);
            sdata.append(stR);
        }
        if(min <= rate[id].lastRange && max > rate[id].lastRange){
            //rate[id].perc;
            // {попадает ли в диапазон 20%}
            // если да, тогда ждем покупку в этом диапазоне
            int summ = cntM + cntF;
            int absRange = abs(cntM - cntF);
            if(summ >= rate[id].maxSt){
                float prc = 100*(float)absRange/(float)summ;
                //std:: << rate[id].lastRange << " summ:" << summ << ", range:" << prc << std::endl;
                if(prc > rate[id].perc){
                    strTable st;
                    st.min = min;
                    st.max = max;
                    if(cntF < cntM)
                        st.pos = '<';
                    else
                        st.pos = '>';
                    lastAsc[id] = st;
                    rrange = false;
                }
            }
        }
    }
    if(rrange){
        lastAsc.remove(id);
    }
    return sdata;
}
void WSrest::response(int id, ufBlock &rest, float range){
    std::cout << "response" << id << std::endl;
    if(!am_clients.contains(id)){
        return;
    }
    rate[id].rateFloat = range;
    QVector <tmpTable> tt;
    updTmpTable(id, tt, rest);
    rateUser.insert(id,tt);
    QVector <strTable> sdata = reRange(id);
    print(id,sdata,rest,false);

    QTimer *subTimer = new QTimer();
    subTimer->setInterval(1000);
    connect(subTimer,&QTimer::timeout,[=](){
        if(!am_clients.contains(id)){
            subTimer->stop();
        }
        else {
            emit getLastTrades(id,rate[id].type,rate[id].lastPeriod+1,rate[id].rateFloat);
        }
    });
    subTimer->start();

}
void WSrest::responseLast(int id, ufBlock &rest, float range){
    Q_UNUSED(range);
    if(rateUser.contains(id)){
        updTmpTable(id, rateUser[id], rest);
        QVector <strTable> sdata = reRange(id);
        print(id,sdata,rest,true);
    }
}
void WSrest::print(int id, QVector <strTable> &sdata, ufBlock &rest, bool view){
    if(!am_clients.contains(id)){
        return;
    }
    QWebSocket *pClient = am_clients[id];
    if(pClient->isValid()){
        QStringList stl;
        QStringList sts;
        QStringList stu;
        QStringList tmp;
        QStringList dep;
        QString sta = "{}";
        //bool isPrice = false;
        if(view){
            for (auto i = rest.begin(); i != rest.end(); ++i){
                infoBlock ib = i.value();
    //        foreach (infoBlock ib, rest) {
                QString stlPrice;
                if(ib.price.size() > 0){
                    //isPrice = true;
                    QStringList priceStr;
                    foreach (float price, ib.price) {
                        priceStr << QString::number(price);
                    }
                    stlPrice = "\""+priceStr.join(",")+"\"";
                }
                else
                    stlPrice = "\"\"";
                stl << "{\"dtime\":"+QString::number(ib.dtime)+",\"asks\":"+QString::number(ib.asks)+",\"bids\":"+QString::number(ib.bids)+",\"range\":"+QString::number(ib.range)+",\"price\":"+stlPrice+"}";
            }
        }

        foreach (strTable id, sdata) {
            sts << "{\"min\":"+QString::number(id.min)+",\"max\":"+QString::number(id.max)+",\"count\":"+QString::number(id.count)+",\"pos\":\""+(QString)id.pos+"\"}";
        }
        if(lastAsc.contains(id)){
            sta ="{\"min\":"+QString::number(lastAsc[id].min)+",\"max\":"+QString::number(lastAsc[id].max)+",\"pos\":\""+(QString)lastAsc[id].pos+"\"}";
        }
        if(rangeUser.contains(id)){
            foreach (strResponse inf, rangeUser[id]) {
                stu << "{\"dtime\":"+QString::number(inf.dtime)
                       +",\"price\":"+QString::number(inf.price)
                       +",\"range\":"+QString::number(inf.range)
                       +",\"response\":"+QString::number(inf.response)
                       +",\"diffR\":"+QString::number(inf.diffR)
                       +",\"diffM\":"+QString::number(inf.diffM)
                       +"}";
            }
        }
        QString resp;
        for(auto tmpUserState = tmpUser[id].cbegin(); tmpUserState!=tmpUser[id].cend(); ++tmpUserState){
            infoBlock val = tmpUserState.value();
            QStringList priceStr;
            foreach (float price, val.price) {
                priceStr << QString::number(price);
            }
            tmp << "{\"dtime\":\""+QString::number(tmpUserState.key())+"\", \"data\":\""+QString::number(val.range)+"::"+priceStr.join(", ")+"\"}";
        }
        if(ireal[id].depth.length() > 0)
            foreach (trDepth elem, ireal[id].depth) {
                QString position = "<";
                if(elem.type)
                    position = ">";
                dep << "\""+position+QString::number(elem.value)+"\"";
            }
        resp = "{\"rate\":["+stl.join(",")+"], \"tab\": ["+sts.join(",")+"],\"last\":{\"range\":"
                +QString::number(rate[id].lastRange)+",\"asc\": "+sta+" }, \"rtables\": ["+stu.join(",")+"],\"tmp\": ["+tmp.join(",")
                +"], \"summ\": {\"count\":"+QString::number(ireal[id].count)+",\"price\":"+QString::number(ireal[id].price)+"}, \"depth\": ["+dep.join(',')+"] }";
        pClient->sendTextMessage(resp);
    }
}

void WSrest::processTextMessage(QString message)
{
    QWebSocket *pClient = qobject_cast<QWebSocket *>(sender());
    if (pClient) {
        if(!m_clients.contains(pClient)){
            return;
        }
        int idusersocs = m_clients.value(pClient);
        QStringList rsp = message.split(";");
        //std::cout << message.toStdString() << std::endl;
        if(rsp.size() < 5){
            return;
        }
        QDateTime current = QDateTime::currentDateTime();


        try {
            rate[idusersocs].type = rsp[0];
            rate[idusersocs].find = rsp[1].toInt();
            rate[idusersocs].view = rsp[2].toInt();
            rate[idusersocs].rate = rsp[3].toInt();
            rate[idusersocs].perc = rsp[4].toFloat();
            rate[idusersocs].maxSt = rsp[8].toInt();
            if(rsp[5]=="1")
                rate[idusersocs].reverse = true;
            std::cout << "reverse: " << rate[idusersocs].reverse << std::endl;

            ireal[idusersocs].count = rsp[6].toInt();
            ireal[idusersocs].price = rsp[7].toFloat();
            ireal[idusersocs].volatilitymin = rsp[9].toDouble();
            ireal[idusersocs].volatilitymax = rsp[10].toDouble();
            ireal[idusersocs].volatilityRange = rsp[11].toDouble();
            ireal[idusersocs].volatility = rsp[12].toInt();
            QStringList depthList = rsp[12].split(":");
            if(depthList.length() > 0){
                foreach (QString depth, depthList) {
                    if(depth.length() > 0){
                        trDepth trD;
                        trD.type = false;
                        if(depth.left(1)==">")
                            trD.type = true;
                        trD.value = depth.mid(1).toDouble();
                        ireal[idusersocs].depth.append(trD);
                    }
                }
            }
        }
        catch(...){}
        rate[idusersocs].rateFloat = (float)rate[idusersocs].rate;
        rate[idusersocs].view = rate[idusersocs].find;
        rate[idusersocs].find = current.toTime_t() - rate[idusersocs].find;
        //std::cout << rate[idusersocs].perc << std::endl;

        emit getMaxTrades(idusersocs,rate[idusersocs].type,rate[idusersocs].find, rate[idusersocs].view, rate[idusersocs].rate);
//        pClient->sendTextMessage(message);
    }
}
