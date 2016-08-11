#include "worker.h"
#include <QPainter>
#include <QPainterPath>
#include <QPixmap>
#include <QDateTime>
#include <QCoreApplication>
#include <QFileInfo>
#include <iostream>

using namespace std;

Worker::Worker(QObject *parent) : QObject(parent)
{
    dName = QCoreApplication::applicationDirPath()+"/../../public/depth/";
    QFileInfo fileInfo(dName);
    dName = fileInfo.absolutePath();
    iNum = 0;
}
void Worker::setMin(uint dtime, iDepth src, mPair m ){
    ++iNum;
    /*cout << "\r" << "                                     " << end;*/
    cout << iNum << endl;
    depMin[dtime] = src;
    depPair[dtime] = m;
    QDateTime current = QDateTime::currentDateTime();
    uint tdime = current.toTime_t();
    uint remove = tdime - 3600*24;
    QHash <uint, iDepth>::const_iterator dIterDep = depMin.constBegin();
    while (dIterDep != depMin.constEnd()) {
        uint dIter = dIterDep.key();
        if(dIter < remove){
            depMin.remove(dIter);
            depPair.remove(dIter);
            --iNum;
        }
        ++dIterDep;
    }
}
void Worker::generate(int id, QString &dep){
    QDateTime current = QDateTime::currentDateTime();
    uint tdime = current.toTime_t() -1;
    uint startScan = tdime - 3600*24;
    QPixmap pixmap( 4320, 2100 );
    //QPixmap pixmap( 4320, 4320 );
    pixmap.fill( Qt::white );
    QPainter painter( &pixmap );
    if(!depMin.isEmpty()){
        float max = 0.0;
        float min = 0.0;
        QHash <uint, mPair>::const_iterator dIterDp = depPair.constBegin();
        while (dIterDp != depPair.constEnd()) {
            mPair pr = dIterDp.value();
            if(min == 0.0)
                min = pr.min;
            if(max < pr.max)
                max = pr.max;
            if(min > pr.min || min == 0.0)
                min = pr.min;
            ++dIterDp;
        }
        float step = (max-min)/2000;
        int countImg = 0;
        QHash <uint, iTypes> img;
        QHash <uint, iDepth>::const_iterator dIterDep = depMin.constBegin();
        while (dIterDep != depMin.constEnd()) {
            uint position = (dIterDep.key() - startScan)/60;
            if (position == 0)
                position = 1;
            else
                position *=3;
            iDepth dp = dIterDep.value();
            if(dp.contains(dep)){
                ++countImg;
                //здесь только /btc_usd
                iTypes infoBlock = dp[dep];
                //asks
                iPairs asks = infoBlock["asks"];
                iPairs newAsks;
                newAsks.resize(2000);
                for(uint si = 0; si < 2000; ++si){
                    newAsks[si].price = min + step*si;
                    newAsks[si].amount = 0.0;
                }
                foreach (iPair pr, asks) {
                    int positionLine = (int)((pr.price-min)/step);
                    newAsks[positionLine].amount += pr.amount;
                }
                img[position]["asks"] = newAsks;
                //bids
                iPairs bids = infoBlock["bids"];
                iPairs newBids;
                newBids.resize(2000);
                for(uint si = 0; si < 2000; ++si){
                    newBids[si].price = min + step*si;
                    newBids[si].amount = 0.0;
                }
                foreach (iPair pr, bids) {
                    int positionLine = (int)((pr.price-min)/step);
                    newBids[positionLine].amount += pr.amount;
                }
                img[position]["bids"] = newBids;
            }
            ++dIterDep;
        }
        if(countImg > 0){
            QHash <uint, iTypes>::const_iterator dIterImg = img.constBegin();
            while (dIterImg != img.constEnd()) {
                int pos = (int)dIterImg.key();
                iTypes tp = dIterImg.value();
                //asks красные (снизу)
                float maxColor = 0.0;
                float minColor = tp["asks"][0].amount;
                foreach (iPair pr, tp["asks"]){
                    if(maxColor < pr.amount)
                        maxColor = pr.amount;
                    if(minColor > pr.amount || minColor == 0.0){
                        minColor = pr.amount;
                    }
                }
                float norm = (maxColor - minColor)/255;
                int num = 0;
                if (norm >= 0.001){
                    int minNum = 0;
                    int maxNum = 0;
                    foreach (iPair pr, tp["asks"]){
                        ++num;
                        int color = 255-(int)((pr.amount-minColor)/norm);
                        if(color < 254){
                            QPainterPath path;
                            path.moveTo( pos, 2000-num );
                            path.lineTo( pos+3, 2000-num );
                            painter.setRenderHint( QPainter::Antialiasing );
                            painter.setPen( QColor(155,0,0,color) );
                            painter.setBrush( QColor(155,0,0,color) );
                            painter.drawPath( path );
                            //cout << "asks color: " << color << "(" << pos << ", " << 2000-num << ")" << endl;
                            if(maxNum == 0)
                                maxNum = 2000-num;
                            minNum = 2000-num;
                        }
                    }
                    cout << "asks minMax: " << minNum << ": " << maxNum << endl;
                }
                //bids #FF9800 (сверху)
                maxColor = 0.0;
                minColor = 0.0;
                foreach (iPair pr, tp["bids"]){
                    if(maxColor < pr.amount)
                        maxColor = pr.amount;
                    if(minColor > pr.amount || minColor == 0.0){
                        minColor = pr.amount;
                    }
                }
                norm = (maxColor - minColor)/255;
                if (norm >= 0.001){
                    int minNum = 0;
                    int maxNum = 0;
                    num = 0;
                    foreach (iPair pr, tp["bids"]){
                        ++num;
                        int color = 255-(int)((pr.amount-minColor)/norm);
                        if(color < 254){
                            QPainterPath path;
                            path.moveTo( pos, 2000-num );
                            path.lineTo( pos+3, 2000-num );
                            painter.setRenderHint( QPainter::Antialiasing );
                            painter.setPen( QColor(15,15,15,color) );
                            painter.setBrush( QColor(15,15,15,color) );
                            painter.drawPath( path );
                            //cout << "bids color: " << color << "(" << pos << ", " << 2000-num << ")" << endl;
                            if(maxNum == 0)
                                maxNum = 2000-num;
                            minNum = 2000-num;
                        }
                    }
                    cout << "bids minMax: " << minNum << ": " << maxNum << endl;
                }
                ++dIterImg;
            }
        }
    }
    QString qn = QString::number(tdime)+".png";
    pixmap.save( dName+"/"+qn );

    emit response(id, qn);
}
