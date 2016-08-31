#ifndef RDATA_H
#define RDATA_H

#include <QReadWriteLock>
#include "itypes.h"

class Rdata
{
public:
    Rdata();
    void rm(uint date, QString type);
    void set(uint date, iDepth &from);
    void get(uint date, iDepth &to);
    void get(uint datein, uint dateout, QMap<uint, iDepth> &to);

    void set(uint date, iTradesLst &from);
    void get(uint date, iTradesLst &to);
    void get(uint datein, uint dateout, iTradesData &to);

private:
    QMap<QString, QReadWriteLock *> qm;
    QMap<uint, iDepth> depth;
    iTradesData trades;
};

#endif // RDATA_H
