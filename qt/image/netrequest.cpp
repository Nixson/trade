#include "netrequest.h"

NetRequest::NetRequest(QObject *parent) : QObject(parent), manager(new QNetworkAccessManager(this))
{
    connect(manager, &QNetworkAccessManager::finished, this, &NetRequest::replyFinished);
    codec = QTextCodec::codecForName("utf8");
}
void NetRequest::get(QUrl uri){
    manager->get(QNetworkRequest(uri));
}
void NetRequest::replyFinished(QNetworkReply *reply){
    if (reply->error() == QNetworkReply::NoError){
        QByteArray content = reply->readAll();
        emit responseJSON(content);
        QString text = codec->toUnicode(content.data());
        emit response(text);
    }
    reply->deleteLater();
}
