#ifndef WSTASK_H
#define WSTASK_H

#include "itypes.h"
#include <QObject>
#include <QRunnable>

class WsTask : public QObject, public QRunnable
{
    Q_OBJECT
public:
    explicit WsTask(iTask stepTask, QObject *parent = 0);
    static bool Asort(iPair a, iPair b);
    static bool Dsort(iPair a, iPair b);

signals:
    void response(iTask *stepTask, iTaskResult *resultTask);

public slots:
    void run();

private:
    umBlock             tmpUser;
    iTask               stepOb, *step;
    uint                stepNum;
    iRate               rate;
    float               stepRate;
    iTaskResult         *result;
    iTradesData         *tradeLink, findTrade;
    QMap<uint,iDepth>  *depthLink, findDepth;

    iTradesData         trade;
    QMap<uint,iDepth>  depth;



    QMap <int, strTable> lastAsc;
    QVector<tmpTable>  rateUser;
    QVector< strResponse > rangeUser;
    uint                missing;

    void getMax();
    ufBlock getStep();
    void getRange(ufBlock &listDepth);
    void getRange(uint per, umBlock &listDepth);
    void updTmpTable(ufBlock &rest);
    void subRange();
    void reRange();

    void getLastDep(uint pos);
    void getLastTrades(uint pos);

    umBlock listDepth;

};

#endif // WSTASK_H
