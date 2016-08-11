#ifndef WORKER_H
#define WORKER_H

#include <QObject>
#include "itypes.h"

class Worker : public QObject
{
    Q_OBJECT
public:
    explicit Worker(QObject *parent = 0);

signals:
    void response(int, QString &data);

public slots:
    void setMin(uint, iDepth, mPair m);
    void generate(int id, QString &dep);
private:
    QHash <uint, iDepth> depMin;
    QHash <uint, mPair> depPair;
    QString dName;
    int iNum;

};

#endif // WORKER_H
