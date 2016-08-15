#include "wstask.h"
#include "memory.h"
#include <iostream>
#include <QDateTime>

WsTask::WsTask(iTask *stepTask, QObject *parent) : QObject(parent),
                                                   step(stepTask),
                                                   result(new iTaskResult),
                                                   trade(new iTradesData),
                                                   findTrade(new iTradesData),
                                                   depth(new QHash<uint,iDepth>),
                                                   findDepth(new QHash<uint,iDepth>){}

void WsTask::run(){
    Memory::get(step->PeriodStart,step->PeriodStop,*trade);
    Memory::get(step->PeriodStart,step->PeriodStop,*depth);

    //Собираем последние 20 минут
    std::cout << "reRange step 0" << std::endl;
    uint last20 = step->PeriodStop - 60*20;
    for(uint pos = last20; pos <= step->PeriodStop; ++pos){
        if(trade->contains(pos)){
            findTrade->insert(pos,trade->value(pos));
            trade->remove(pos);
        }
        if(depth->contains(pos)){
            findDepth->insert(pos,depth->value(pos));
            depth->remove(pos);
        }
    }




    rate.lastPeriod = 0;
    getMax();

    //step->rate
    std::cout << "reRange step 1" << std::endl;
    ufBlock list = getStep();
    getRange(list);
    updTmpTable(list);
    reRange();
    std::cout << "reRange step 2" << std::endl;

    for(uint pos = last20; pos <= step->PeriodStop; ++pos){
        trade->clear();
        depth->clear();
        if(findDepth->contains(pos))
            depth->insert(pos,findDepth->value(pos));
        if(findTrade->contains(pos))
            trade->insert(pos,findTrade->value(pos));
        std::cout << "reRange step 3" << std::endl;
        ufBlock list = getLastDep();
        std::cout << "reRange step 4" << std::endl;
        getRange(list);
        std::cout << "reRange step 5: " << list.size() << std::endl;
        updTmpTable(list);
        std::cout << "reRange step 6" << std::endl;
        reRange();
    }

    std::cout << "WsTask" <<  rangeUser.size() << std::endl;

    emit response(step, result);
}

ufBlock& WsTask::getLastDep(){
    ufBlock nextStep;
    for(auto iter = depth->begin();iter!=depth->end();  ++iter){
        if(!iter.value().contains(step->type))
            continue;
        iPairs ascPair = iter.value()[step->type]["asks"];
        qSort(ascPair.begin(),ascPair.end(),Asort);
        iPairs bidPair = iter.value()[step->type]["bids"];
        qSort(bidPair.begin(),bidPair.end(),Dsort);
        int cntA = 0;
        int cntB = 0;
        float summ = 0.0;
        foreach (iPair pr, ascPair){
            ++cntA;
            summ += pr.amount;
            if(summ >=step->rate)
                break;
        }
        summ = 0.0;
        foreach (iPair pr, bidPair){
            ++cntB;
            summ += pr.amount;
            if(summ >=step->rate)
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
        std::cout << "reRange getLastDep: " << bl.dtime << std::endl;
        nextStep.insert(bl.dtime,bl);
    }
    std::cout << "reRange nextStep: " << nextStep.size() << std::endl;
    return getLastTrades(nextStep);
}

ufBlock& WsTask::getLastTrades(ufBlock &listDepth){
    for(auto iter = trade->cbegin();iter!=trade->cend();  ++iter){
        uint period = iter.key();
        std::cout << "reRange getLastTrades: " << period << std::endl;
        if(!iter.value().contains(step->type))
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
        iTrades td = iter.value()[step->type];
        foreach (iTrade tradeElement, td) {
            listDepth[period].price.append(tradeElement.price);
        }
    }
    std::cout << "reRange getLastTrades: " << listDepth.size() << std::endl;
    return listDepth;
}

void WsTask::getMax(){
    float max = 0.0;
    for(auto iter = trade->begin();iter!=trade->end();  ++iter){
        if(!iter.value().contains(step->type))
            continue;
        iTrades td = iter.value()[step->type];
        foreach (iTrade tradeElement, td) {
            if(tradeElement.amount > max)
                max = tradeElement.amount;
        }
    }
    step->rate *=max;
}
bool WsTask::Asort(iPair a, iPair b){
    return a.price < b.price;
}
bool WsTask::Dsort(iPair a, iPair b){
    return a.price > b.price;
}
ufBlock WsTask::getStep(){
    ufBlock nextStep;
    for(auto iter = depth->cbegin();iter!=depth->cend();  ++iter){
        if(!iter.value().contains(step->type))
            continue;
        iPairs ascPair = iter.value()[step->type]["asks"];
        qSort(ascPair.begin(),ascPair.end(),Asort);
        iPairs bidPair = iter.value()[step->type]["bids"];
        qSort(bidPair.begin(),bidPair.end(),Dsort);
        int cntA = 0;
        int cntB = 0;
        float summ = 0.0;
        foreach (iPair pr, ascPair){
            ++cntA;
            summ += pr.amount;
            if(summ >=step->rate)
                break;
        }
        summ = 0.0;
        foreach (iPair pr, bidPair){
            ++cntB;
            summ += pr.amount;
            if(summ >=step->rate)
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
    return nextStep;
}
void WsTask::getRange(ufBlock &listDepth){
    for(auto iter = trade->cbegin();iter!=trade->cend();  ++iter){
        uint period = iter.key();
        if(!iter.value().contains(step->type))
            continue;
        if(!listDepth.contains(period)){
            bool find = false;
            infoBlock bdata;
            bdata.dtime = period;
            bdata.asks = 0;
            bdata.bids = 0;
            bdata.range = 0.0;
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
        iTrades td = iter.value()[step->type];
        foreach (iTrade tradeElement, td) {
            listDepth[period].price.append(tradeElement.price);
        }
    }
}







void WsTask::updTmpTable(ufBlock &rest){
    QDateTime current = QDateTime::currentDateTime();
    uint cTime = current.toTime_t();

    std::cout << "updTmpTable rest:" << rest.size() << std::endl;

    for (auto i = rest.cbegin(); i != rest.cend(); ++i){
        infoBlock info = i.value();
        std::cout << "updTmpTable rest begin:" << i.key() << std::endl;
        if(info.dtime > rate.lastPeriod){
            rate.lastRange = info.range;
        }
        if(info.price.size() == 0)
            continue;
        std::cout << "updTmpTable rest tmpUser:" << i.key() << std::endl;
        tmpUser[i.key()] = info;
    }
    rate.min = 0.0;
    rate.max = 0.0;
    rateUser.clear();
    std::cout << "updTmpTable:" << tmpUser.size() << std::endl;
    for (auto i = tmpUser.begin(); i != tmpUser.end(); ++i){
        bool isLast = false;
        infoBlock info = i.value();
        if(info.dtime > rate.lastPeriod){
            isLast = true;
            rate.lastPeriod = info.dtime;
        }
        if (rate.min == 0.0)
            rate.min = info.range;
        if(rate.min > info.range)
            rate.min = info.range;
        if(rate.max < info.range)
            rate.max = info.range;
        foreach(float price, info.price){
            char pos;
            if(price > rate.lastPrice)
                pos = '>';
            else if(price < rate.lastPrice)
                pos = '<';
            else
                pos = '=';

            if(isLast && (pos=='>' || pos=='<')){
                if(lastAsc.contains(0)){
                    if(info.range >= lastAsc[0].min && info.range <= lastAsc[0].max){
                        strResponse rsp;
                        rsp.dtime = cTime;
                        rsp.price = price;
                        rsp.range = info.range;
                        if(lastAsc[0].pos==pos){
                            rsp.response = true;
                        }
                        else
                            rsp.response = false;
                            rangeUser.append(rsp);
                        lastAsc.remove(0);
                    }
                }
            }



            bool find = false;
            for(auto it = rateUser.begin(); it!=rateUser.end();++it){
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
                rateUser.append(dt);
            }
            rate.lastPrice = price;

        }
    }
    std::cout << "subRange: " << rateUser.length() << std::endl;
    subRange();
}
void WsTask::subRange(){
    if(rate.max - rate.min < 0.001)
        return;
    float ranges = (rate.max-rate.min)/10.0;
    bool hasMin = false;
    int summ = 0;
    for(int i = 0; i < 10; ++i){
        float min = rate.min + ranges*i;
        float max = rate.min + ranges*(i+1);
        int cnt = 0;
        foreach(tmpTable dt, rateUser){
            if(dt.range >= min && dt.range <= max)
                ++cnt;
        }
        if(i == 0 && cnt <= 5 ){
            hasMin = true;
            //std::cout << "cnt (0): " << cnt << std::endl;
            rate.min += ranges;
        }
        summ +=cnt;
        if(i == 9 && cnt <= 5 ){
            hasMin = true;
            //std::cout << "cnt (9): " << cnt << std::endl;
            rate.max -= ranges;
        }
    }
    if(summ > 25 && hasMin && rate.min < rate.max){
        subRange();
    }
}
void WsTask::reRange(){
    QVector <strTable> sdata;
    float ranges = 0.1;
    int rangesCnt = (int)((rate.max-rate.min)/ranges);
    bool rrange = true;
    for(int i = 0; i < rangesCnt; ++i){
        float min = rate.min + ranges*i;
        float max = rate.min + ranges*(i+1);
        int cntF = 0;
        int cntM = 0;
        int cntR = 0;
        strTable stF;
        {
            stF.min = min;
            stF.max = max;
            stF.pos = '>';
            stF.count = 0;
            foreach(tmpTable dt, rateUser){
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
            foreach(tmpTable dt, rateUser){
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
            foreach(tmpTable dt, rateUser){
                bool w = dt.range < stR.max;
                if(i==(rangesCnt-1))
                    w = dt.range <= stR.max;
                if(dt.range >= stR.min && w && dt.pos==stR.pos)
                    stR.count++;
            }
            cntR = stR.count;
        }
        int rSumm = cntM+cntF+cntR;
        if(rSumm >= 2 ){
            sdata.append(stF);
            sdata.append(stM);
            sdata.append(stR);
        }
        if(min <= rate.lastRange && max > rate.lastRange){
            //rate.perc;
            // {попадает ли в диапазон 20%}
            // если да, тогда ждем покупку в этом диапазоне
            int summ = cntM + cntF;
            int absRange = abs(cntM - cntF);
            if(summ > 0){
                float prc = 100*(float)absRange/(float)summ;
                if(prc > rate.perc){
                    strTable st;
                    st.min = min;
                    st.max = max;
                    if(cntF < cntM)
                        st.pos = '<';
                    else
                        st.pos = '>';
                    lastAsc[0] = st;
                    rrange = false;
                }
            }
        }
    }
    if(rrange){
        lastAsc.remove(0);
    }
    //return sdata;
}
