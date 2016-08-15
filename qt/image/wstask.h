#ifndef WSTASK_H
#define WSTASK_H

#include "itypes.h"
#include <QObject>
#include <QRunnable>

class WsTask : public QObject, public QRunnable
{
    Q_OBJECT
public:
    explicit WsTask(iTask *stepTask, QObject *parent = 0);
    static bool Asort(iPair a, iPair b);
    static bool Dsort(iPair a, iPair b);

signals:
    void response(iTask *stepTask, iTaskResult *resultTask);

public slots:
    void run();

private:
    umBlock             tmpUser;
    iTask               *step;
    iRate               rate;
    iTaskResult         *result;
    iTradesData         *tradeLink, findTrade;
    QHash<uint,iDepth>  *depthLink, findDepth;

    iTradesData         trade;
    QHash<uint,iDepth>  depth;



    QHash <int, strTable> lastAsc;
    QVector<tmpTable>  rateUser;
    QVector< strResponse > rangeUser;

    void getMax();
    ufBlock getStep();
    void getRange(ufBlock &listDepth);
    void updTmpTable(ufBlock &rest);
    void subRange();
    void reRange();

    umBlock &getLastDep(uint pos);
    umBlock& getLastTrades(umBlock &listDepth);
};

#endif // WSTASK_H
