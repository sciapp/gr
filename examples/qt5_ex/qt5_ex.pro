GRDIR = $$(GRDIR)
isEmpty(GRDIR) {
  GRDIR = /usr/local/gr
}
TEMPLATE = app
TARGET = qt5_ex
INCLUDEPATH += $$GRDIR/include
LIBS += -L$$GRDIR/lib -lGR -lqt5gr

QT += widgets

QMAKE_RPATHDIR += $$GRDIR/lib

HEADERS += main_window.h
SOURCES += main.cpp \
           main_window.cpp
