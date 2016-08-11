#ifndef NETREQUEST_H
#define NETREQUEST_H

#include <QObject>
#include <QTextCodec>
#include <QNetworkReply>
#include <QNetworkAccessManager>

class NetRequest : public QObject
{
    Q_OBJECT
public:
    explicit NetRequest(QObject *parent = 0);

signals:
    void response(QString &data);
    void responseJSON(QByteArray &info);

public slots:
    void replyFinished(QNetworkReply *);
    void get(QUrl uri);
private:
    QNetworkAccessManager *manager;
    QTextCodec *codec;
};

#endif // NETREQUEST_H
