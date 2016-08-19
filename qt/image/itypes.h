#ifndef ITYPES_H
#define ITYPES_H


#include <QVector>
#include <QHash>
#include <QMap>
#include <QDataStream>
#include <iostream>

typedef struct iPair_str {
    float price;
    float amount;
} iPair;


typedef struct iRate_str {
    QString type;
    int find;
    int view;
    int rate;
    float perc;
    float rateFloat;
    float lastPrice;
    float lastRange;
    uint lastPeriod;
    float min;
    float max;
} iRate;

typedef struct iTask_struct {
    uint iduser;
    QString type;
    uint PeriodStart;
    uint PeriodStop;
    float rate;
    float perc;
} iTask;

typedef struct iTaskResult_struct {
    uint lost;
    uint bad;
    uint good;
} iTaskResult;





typedef struct Pair_str {
    float max;
    float min;
} mPair;

typedef struct Trade_struct {
    int tid;
    QString type;
    float price;
    float amount;
} iTrade;

typedef struct infoBlock_str {
    uint dtime;
    QVector<float> price;
    uint asks;
    uint bids;
    float range;
} infoBlock;

typedef struct tmpTable_str {
    float range;
    char pos;
    uint count;
} tmpTable;

typedef struct strTable_str {
    float max;
    float min;
    char pos;
    uint count;
} strTable;

typedef struct strResponse_str {
    uint dtime;
    float price;
    float range;
    bool response;
} strResponse;


QDataStream &operator<<(QDataStream &out, const iPair_str &str);
QDataStream &operator>>(QDataStream &out, iPair_str &str);
QDataStream &operator<<(QDataStream &out, const Trade_struct &str);
QDataStream &operator>>(QDataStream &out, Trade_struct &str);

typedef QVector<iPair> iPairs;
typedef QHash<uint, infoBlock> ufBlock;
typedef QMap<uint, infoBlock> umBlock;
typedef QHash<QString, iPairs> iTypes;
typedef QHash<QString, iTypes> iDepth;

typedef QHash<QString, float> iFloat;

typedef QVector<iTrade> iTrades;
typedef QHash <QString,iTrades> iTradesLst;
typedef QHash<uint, iTradesLst> iTradesData;

typedef QHash<uint, float> ufMap;
typedef QHash<uint, QString> usMap;

typedef QVector<QPair<uint, float>> ufVector;



#endif // ITYPES_H
