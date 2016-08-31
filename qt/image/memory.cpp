#include "memory.h"

Memory::Memory(){}
Rdata *Memory::dLink = new Rdata();

void Memory::set(uint date, iDepth &from){
    dLink->set(date,from);
}
void Memory::set(uint date, iTradesLst &from){
    dLink->set(date,from);
}
void Memory::get(uint date, iDepth &to){
    dLink->get(date,to);
}
void Memory::get(uint date, iTradesLst &to){
    dLink->get(date,to);
}
void Memory::get(uint datein, uint dateout, QMap<uint, iDepth> &to){
    dLink->get(datein,dateout,to);
}
void Memory::get(uint datein, uint dateout, iTradesData &to){
    dLink->get(datein,dateout,to);
}
void Memory::rm(uint date, QString type){
    dLink->rm(date,type);
}
