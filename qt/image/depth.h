#ifndef DEPTH_H
#define DEPTH_H

#include <QObject>
#include <QThread>
#include <QTimer>
#include <QJsonObject>

#include "netrequest.h"
#include "itypes.h"
#include "worker.h"
#include "rest.h"


class Depth : public QObject
{
    Q_OBJECT
    QThread netThread;
public:
    explicit Depth(QObject *parent = 0);
    void setRest(Rest *rst);
    void setWork(Worker *wrk);
    static bool Asort(iPair a, iPair b);
    static bool Dsort(iPair a, iPair b);

signals:
    void getUrl(QUrl url);
    void setMin(uint, iDepth, mPair);
    void setMax(const QString &dep, ufMap &resp);
    void setMaxWs(int id, QString &dep, uint FindPeriod, ufBlock &listDepth, float resp);
    void setLastWs(int id, QString &dep, uint FindPeriod, ufBlock &listDepth, float resp);
    void response(int id, QString &result);

public slots:
    void update();
    void responseJSON(QByteArray &info);
    void updateMin();
    void clearMin();
    void getMax(int id, const QString &dep, uint period, float cntElements, usMap &listTrades);
    void getMaxWs(int id, QString &dep, uint FindPeriod, uint ViewPeriod, float resp);
    void getLastWs(int id, QString &dep, uint period, float rate);
private:
    NetRequest *req;
    QTimer *timer, *timerMin;
    QHash <uint, iDepth> dep;
    int pNum;
    void print(QString, iPairs);
    void loadOb();

    Worker *work;
    Rest *rest;
    QString dirPath;


    void parse(QJsonObject &json);
};

#endif // DEPTH_H
