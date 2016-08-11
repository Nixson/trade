#include "wstask.h"
#include "memory.h"
#include <iostream>

WsTask::WsTask(iTask *stepTask, QObject *parent) : QObject(parent),
                                                   step(stepTask),
                                                   result(new iTaskResult),
                                                   trade(new iTradesData),
                                                   depth(new QHash<uint,iDepth>){}

void WsTask::run(){
    Memory::get(step->PeriodStart,step->PeriodStop,*trade);
    Memory::get(step->PeriodStart,step->PeriodStop,*depth);

    getMax();
    ufBlock list = getStep();
    getRange(list);

    std::cout << "WsTask" <<  list->size() << std::endl;

    emit response(step, result);
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
