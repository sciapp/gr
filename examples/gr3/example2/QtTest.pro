#-------------------------------------------------
#
# Project created by QtCreator 2012-09-03T15:34:37
#
#-------------------------------------------------

QT       += core gui opengl

TARGET = QtTest
TEMPLATE = app


SOURCES += main.cpp

HEADERS  += \
    GR3Widget.h

INCLUDEPATH += /usr/local/gr/include

LIBS += -L/usr/local/gr/lib -lGR3
