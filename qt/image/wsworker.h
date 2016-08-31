#ifndef WSWORKER_H
#define WSWORKER_H

#include "itypes.h"
#include <QObject>
#include <QWebSocketServer>
#include <QWebSocket>


typedef struct iWsUser_str {
    uint User;
    QString type;
    uint timeIn;
    uint timeOut;
    uint timeRange;
    uint rateIn;
    uint rateOut;
    uint rateRange;
    float priceIn;
    float priceOut;
    float rangeLabel;
    uint priceRange;
    uint tasks;
    uint taskLast;
    QVector <iTask *> task;
    QVector <iTaskResult *> result;
} iWsUser;



class WsWorker : public QObject
{
    Q_OBJECT
public:
    explicit WsWorker(QObject *parent = 0);
    ~WsWorker();

signals:
    void closed();

public slots:
    void onNewConnection();
    void processTextMessage(QString message);
    void socketDisconnected();


    void response(iTask *step, iTaskResult *result);
private:
    QWebSocketServer            *m_pWebSocketServer;
    QHash<QWebSocket *, int>     m_clients;
    QHash<int, QWebSocket *>     am_clients;
    QHash <int, ufBlock>         tmpUser;
    QHash <int, iWsUser>         user;
    uint                         last;
    void                         poolIn(iTask task);

};

#endif // WSWORKER_H
