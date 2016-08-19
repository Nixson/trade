//#include <QApplication>
#include <QCoreApplication>
#include <QThread>

#include <unistd.h>
#include <iostream>

#include "itypes.h"
#include "depth.h"
#include "trades.h"
#include "rest.h"
#include "worker.h"
#include "wsrest.h"
#include "wsworker.h"

Q_DECLARE_METATYPE(iDepth)
Q_DECLARE_METATYPE(iDepth*)
Q_DECLARE_METATYPE(ufMap)
Q_DECLARE_METATYPE(ufMap*)
Q_DECLARE_METATYPE(usMap)
Q_DECLARE_METATYPE(usMap*)
Q_DECLARE_METATYPE(mPair)
Q_DECLARE_METATYPE(ufBlock)
Q_DECLARE_METATYPE(mPair*)
Q_DECLARE_METATYPE(QByteArray*)


int wok (int argc, char *argv[]){
    QCoreApplication a(argc, argv);
    qRegisterMetaType<iDepth>("iDepth&");
    qRegisterMetaType<ufMap>("ufMap&");
    qRegisterMetaType<ufMap>("usMap&");
    qRegisterMetaType<QByteArray>("QByteArray&");
    qRegisterMetaType<QString>("QString&");
    qRegisterMetaType<ufBlock>("ufBlock&");
    QThread *depthThread = new QThread;
    QThread *tradeThread = new QThread;
    QThread *workThread = new QThread;
    QThread *restThread = new QThread;
    QThread *wrestThread = new QThread;
    QThread *wworkThread = new QThread;
    Worker *work = new Worker();
    Rest *rest = new Rest();
    Depth *dep = new Depth();
    Trades *tr = new Trades();
    WSrest *ws = new WSrest();
    WsWorker *wwork = new WsWorker();
    dep->setRest(rest);
    dep->setWork(work);
    QObject::connect(rest,&Rest::generate,work,&Worker::generate);
    QObject::connect(rest,&Rest::getMaxTrades,tr,&Trades::getMax);
    QObject::connect(ws,&WSrest::getMaxTrades,tr,&Trades::getMaxWs);
    QObject::connect(ws,&WSrest::getLastTrades,dep,&Depth::getLastWs);

    QObject::connect(work,&Worker::response,rest,&Rest::response);
    QObject::connect(tr,&Trades::setMax,dep,&Depth::getMax);
    QObject::connect(tr,&Trades::setMaxWs,dep,&Depth::getMaxWs);
    QObject::connect(dep,&Depth::setMaxWs,tr,&Trades::getMaxWsRange);
    QObject::connect(dep,&Depth::setLastWs,tr,&Trades::getLastWsRange);

    QObject::connect(dep,&Depth::response,rest,&Rest::response);
    QObject::connect(tr,&Trades::responseWs,ws,&WSrest::response);
    QObject::connect(tr,&Trades::responseLastWs,ws,&WSrest::responseLast);


    QObject::connect(depthThread, &QThread::finished, dep, &QObject::deleteLater);
    QObject::connect(tradeThread, &QThread::finished, tr, &QObject::deleteLater);

    QObject::connect(restThread, &QThread::finished, rest, &QObject::deleteLater);
    QObject::connect(workThread, &QThread::finished, work, &QObject::deleteLater);
    QObject::connect(wrestThread, &QThread::finished, dep, &QObject::deleteLater);
    QObject::connect(wworkThread, &QThread::finished, wwork, &QObject::deleteLater);


    tr->moveToThread(tradeThread);
    dep->moveToThread(depthThread);
    rest->moveToThread(restThread);
    work->moveToThread(workThread);
    ws->moveToThread(wrestThread);
    wwork->moveToThread(wworkThread);

    restThread->start();
    workThread->start();
    wrestThread->start();
    wworkThread->start();

    tradeThread->start();
    depthThread->start();


      //return 0;
    return  a.exec();
}

int main(int argc, char *argv[])
{
    std::cout << "argc: " << argc << std::endl;
    std::cout << "argv: " << *argv << std::endl;
    if (argc != 2){
        return wok(argc,argv);
    }
    int pid;
    pid = fork();
    if (pid == -1){
        std::cout << "Not daemonize" << std::endl;
        return -1;
    }
    if(!pid){
        setsid();
        int stat = chdir("/");
        Q_UNUSED(stat);
        close(STDIN_FILENO);
        close(STDOUT_FILENO);
        close(STDERR_FILENO);
        return wok(argc,argv);
    }
    return 0;
}
