#ifndef REST_H
#define REST_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>

class Rest : public QObject
{
    Q_OBJECT
public:
    explicit Rest(QObject *parent = 0);

signals:
    void generate(int,QString &dep);
    void getMaxTrades(int id, QString &dep, uint FindPeriod, uint ViewPeriod, uint rate);

public slots:
    void response(int id, QString &rest);
    void restConnection();
    void txRx(QTcpSocket *clientSocket, int idusersocs);
private:
    QTcpServer *server;
    QHash<int, QTcpSocket *> SClients;

};

#endif // REST_H
