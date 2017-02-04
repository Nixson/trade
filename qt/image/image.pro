QT += core network websockets

# gui widgets

CONFIG += c++11

TARGET = image
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += itypes.cpp \
    main.cpp \
    depth.cpp \
    trades.cpp \
    netrequest.cpp \
    rest.cpp \
    worker.cpp \
    wsrest.cpp \
    memory.cpp \
    rdata.cpp \
    wsworker.cpp \
    wstask.cpp \
    nxlogger.cpp

HEADERS += itypes.h \
    depth.h \
    trades.h \
    netrequest.h \
    rest.h \
    worker.h \
    wsrest.h \
    memory.h \
    rdata.h \
    wsworker.h \
    wstask.h \
    nxlogger.h
