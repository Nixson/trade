#include "nxlogger.h"
#include <QFile>
#include <QFileInfo>
#include <QCoreApplication>
#include <QDateTime>

NxLogger::NxLogger(QObject *parent) : QObject(parent)
{
    logDir = QCoreApplication::applicationDirPath()+"/../../public/log/";
    timer = new QTimer(this);
    timer->setInterval(1000);
    connect(timer, SIGNAL(timeout()), this, SLOT(update()));
    timer->start();
}

void NxLogger::echo(QString info){
    data << QDateTime::currentDateTime().toString("yyyy.MM.dd hh:mm:ss.zzz") + " " + info;
}

void NxLogger::update(){
    if(data.length() == 0)
        return;
    QStringList sub = data;
    data.clear();
    QFileInfo fi(logDir+"info.txt");
    if(fi.exists() && fi.size() > 1048576) {
        remove(1);
    }
    QFile log(logDir+"info.txt");
    log.open(QIODevice::Append | QIODevice::Text);
    QByteArray ba;
    foreach (QString line, sub) {
        ba.append(line+"\n");
    }
    log.write(ba);
    log.close();
}
void NxLogger::remove(int number){
    QFile log(logDir+"info"+QString::number(number)+".txt");
    if(number < 10){
        QFileInfo fi(log);
        if(fi.exists()){
            remove(number+1);
        }
        log.rename(logDir+"info"+QString::number(number+1)+".txt");
        return;
    }
    log.remove();


}
