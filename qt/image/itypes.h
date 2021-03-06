#ifndef ITYPES_H
#define ITYPES_H


#include <QVector>
#include <QHash>
#include <QMap>
#include <QDataStream>

typedef struct iPair_str {
    float price;
    float amount;
} iPair;


typedef struct iRate_str {
    QString type;
    int find;
    int view;
    int rate;
    int maxSt;
    float perc;
    float rateFloat;
    float lastPrice;
    float lastPriceUser;
    float lastRange;
    uint lastPeriod;
    float min;
    float max;
    bool  reverse;
} iRate;

typedef struct trDepth_str {
    bool type;
    double value;
} trDepth;

typedef struct iReal_str {
    int         count;
    float       price;
    double      volatilitymin;
    double      volatilitymax;
    double      volatilityRange;
    int         volatility;
    QList<trDepth> depth;
} iReal;

typedef struct iTask_struct {
    uint iduser;
    QString type;
    uint PeriodStart;
    uint PeriodStop;
    float rate;
    float perc;
    double min;
    double max;
} iTask;

typedef struct iTaskResult_struct {
    uint lost;
    uint bad;
    uint good;
    double min;
    double max;
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
    float diffR;
    float diffM;
} strResponse;


QDataStream &operator<<(QDataStream &out, const iPair_str &str);
QDataStream &operator>>(QDataStream &out, iPair_str &str);
QDataStream &operator<<(QDataStream &out, const Trade_struct &str);
QDataStream &operator>>(QDataStream &out, Trade_struct &str);

typedef QVector<iPair> iPairs;
typedef QMap<uint, infoBlock> ufBlock;
typedef QMap<uint, infoBlock> umBlock;
typedef QMap<QString, iPairs> iTypes;
typedef QMap<QString, iTypes> iDepth;

typedef QMap<QString, float> iFloat;

typedef QVector<iTrade> iTrades;
typedef QMap <QString,iTrades> iTradesLst;
typedef QMap<uint, iTradesLst> iTradesData;

typedef QMap<uint, float> ufMap;
typedef QMap<uint, QString> usMap;

typedef QVector<QPair<uint, float>> ufVector;



#endif // ITYPES_H
