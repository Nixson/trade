#include "rdata.h"
#include <iostream>

Rdata::Rdata()
{
    qm["depth"] = new QReadWriteLock();
    qm["trade"] = new QReadWriteLock();
}
void Rdata::set(uint date, iDepth &from){
    qm["depth"]->lockForWrite();
    depth[date] = from;
    qm["depth"]->unlock();
}
void Rdata::rm(uint date, QString type){
    if(type=="trade"){
        qm["trade"]->lockForWrite();
        trades.remove(date);
        qm["trade"]->unlock();
    }
    else {
        qm["depth"]->lockForWrite();
        depth.remove(date);
        qm["depth"]->unlock();
    }
}
void Rdata::set(uint date, iTradesLst &from){
    qm["trade"]->lockForWrite();
    trades[date] = from;
    qm["trade"]->unlock();
}
void Rdata::get(uint date, iDepth &to){
    qm["depth"]->lockForRead();
    if(depth.contains(date))
        to = depth[date];
    else {
        iDepth blank;
        to = blank;
    }
    qm["depth"]->unlock();
}
void Rdata::get(uint datein, uint dateout, QHash<uint, iDepth> &to){
    qm["depth"]->lockForRead();
    uint start, stop;
    if(datein > dateout){
        start = dateout;
        stop = datein;
    }
    else {
        start = datein;
        stop = dateout;
    }
    /*for(auto iter = depth.cbegin(); iter!=depth.cend(); ++iter){
        uint k = iter.key();
        std::cout << "find depth: " << k << std::endl;
        if(k >= start && k <= stop)
            to[k] = iter.value();
    }*/
    for(auto iter = start; iter <= stop; ++iter){
        if(depth.contains(iter)){
            to[iter] = depth[iter];
        }
    }
    //std::cout << "find depth: " << start << ":" << stop << " size: " << to.size() << "^^" << depth.size() << std::endl;
    qm["depth"]->unlock();
}

void Rdata::get(uint date, iTradesLst &to){
    qm["trade"]->lockForRead();
    if(trades.contains(date))
        to = trades[date];
    else {
        iTradesLst tl;
        to = tl;
    }
    qm["trade"]->unlock();
}
void Rdata::get(uint datein, uint dateout, iTradesData &to){
    qm["trade"]->lockForRead();
    uint start, stop;
    if(datein > dateout){
        start = dateout;
        stop = datein;
    }
    else {
        start = datein;
        stop = dateout;
    }
    /*for(auto iter = trades.cbegin(); iter!=trades.cend(); ++iter){
            uint k = iter.key();
            std::cout << "find trade: " << k << std::endl;
            if(k >= start && k <= stop)
                to[k] = iter.value();
       }*/
    for(auto iter = start; iter <= stop; ++iter){
        if(trades.contains(iter)){
            to[iter] = trades[iter];
        }
    }
    //std::cout << "find trade: " << start << ":" << stop << " size: " << to.size() << "^^" << trades.size() << std::endl;
    qm["trade"]->unlock();
}
