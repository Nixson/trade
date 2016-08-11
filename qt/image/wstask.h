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
    iTask *step;
    iTaskResult *result;
    iTradesData *trade;
    QHash<uint,iDepth> *depth;

    void getMax();
    ufBlock getStep();
    void getRange(ufBlock &listDepth);
};

#endif // WSTASK_H
