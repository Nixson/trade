#ifndef TRADES_H
#define TRADES_H

#include <QObject>
#include <QTimer>
#include <QJsonObject>
#include <QThread>
#include "netrequest.h"
#include "itypes.h"
#include "worker.h"
#include "rest.h"

class Trades : public QObject
{
    Q_OBJECT
    QThread netThread;

public:
    explicit Trades(QObject *parent = 0);
    void setRest(Rest *rst);
    void setWork(Worker *wrk);

signals:
    void getUrl(QUrl url);
    void setMin(uint, iDepth, mPair);
    void setMax(int id, QString &dep, uint period, float resp, usMap &listTrades);
    void setMaxWs(int id, QString &dep, uint FindPeriod, uint ViewPeriod, float resp);
    void responseWs(int id,ufBlock &listDepth, float resp);
    void responseLastWs(int id,ufBlock &listDepth, float resp);

public slots:
    void update();
    void responseJSON(QByteArray &info);
    void updateMin();
    void clearMin();
    void getMax(int id, QString &dep, uint FindPeriod, uint ViewPeriod, uint rate);
    void getMaxWs(int id, QString &dep, uint FindPeriod, uint ViewPeriod, uint rate);
    void getLastWsRange(int id, QString &dep, uint FindPeriod, ufBlock &listDepth, float resp);
    void getMaxWsRange(int id, QString &dep, uint FindPeriod, ufBlock &listDepth, float resp);
private:
    NetRequest *req;
    QTimer *timer, *timerMin;
    QMap <uint, iDepth> dep;
    iTradesData trade;
    int pNum;
    void loadOb();

    Worker *work;
    Rest *rest;
    QString dirPath;



    void parse(QJsonObject);
};

#endif // TRADES_H
