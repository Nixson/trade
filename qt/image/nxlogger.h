#ifndef NXLOGGER_H
#define NXLOGGER_H

#include <QObject>
#include <QTimer>

class NxLogger : public QObject
{
    Q_OBJECT
public:
    explicit NxLogger(QObject *parent = 0);

public slots:
    void echo(QString);
    void update();
    void remove(int number);

private:
        QTimer *timer;
        QStringList data;
        QString logDir;
};

#endif // NXLOGGER_H
