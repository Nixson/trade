#ifndef WSREST_H
#define WSREST_H

#include "itypes.h"
#include <QObject>
#include <QWebSocketServer>
#include <QWebSocket>


class WSrest : public QObject
{
    Q_OBJECT
public:
    explicit WSrest(QObject *parent = 0);
    ~WSrest();

signals:
    void closed();
    void getMaxTrades(int id, QString &dep, uint FindPeriod, uint ViewPeriod, uint rate);
    void getLastTrades(int id, QString &dep, uint period, float rate);

public slots:
    void onNewConnection();
    void processTextMessage(QString message);
    void processBinaryMessage(QByteArray message);
    void socketDisconnected();

    void response(int id, ufBlock &rest, float range);
    void responseLast(int id,ufBlock &rest, float range);

private:
    QWebSocketServer            *m_pWebSocketServer;
    QHash<QWebSocket *, int>     m_clients;
    QHash<int, QWebSocket *>     am_clients;
    QHash <int, iRate>           rate;
    QHash <int, iReal>           ireal;
    QHash <int, QVector <tmpTable>> rateUser;
    QHash <int, strTable>        lastAsc;
    QHash <int, QVector< strResponse > > rangeUser;
    int                         last;
    uint                        lastUpd;
    QMap <int, ufBlock>         tmpUser;

    void                updTmpTable(int id, QVector <tmpTable> &tt,  ufBlock &rest);
    QVector<strTable>   reRange(int id);
    void                print(int id, QVector <strTable> &sdata, ufBlock &rest, bool view);
    void                subRange(int id, QVector <tmpTable> &tt);
};

#endif // WSREST_H
