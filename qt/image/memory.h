#ifndef MEMORY_H
#define MEMORY_H

#include "itypes.h"
#include "rdata.h"

class Memory
{
public:
    Memory();
    static Rdata *dLink;
    static void rm(uint date, QString type);
    static void set(uint date, iDepth &from);
    static void get(uint date, iDepth &to);
    static void get(uint datein, uint dateout, QMap<uint, iDepth> &to);

    static void set(uint date, iTradesLst &from);
    static void get(uint date, iTradesLst &to);
    static void get(uint datein, uint dateout, iTradesData &to);
};

#endif // MEMORY_H
