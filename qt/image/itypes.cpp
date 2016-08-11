#include "itypes.h"


#ifndef ITYPES_INIT
#define ITYPES_INIT

QDataStream &operator<<(QDataStream &out, const iPair_str &str)
{
    out << str.price;
    out << str.amount;
    return out;
}
QDataStream &operator>>(QDataStream &out, iPair_str &str)
{
    out >> str.price;
    out >> str.amount;
    return out;
}
QDataStream &operator<<(QDataStream &out, const Trade_struct &str)
{
    out << str.tid;
    out << str.type;
    out << str.price;
    out << str.amount;
    return out;
}
QDataStream &operator>>(QDataStream &out, Trade_struct &str)
{
    out >> str.tid;
    out >> str.type;
    out >> str.price;
    out >> str.amount;
    return out;
}

#endif // ITYPES_INIT
