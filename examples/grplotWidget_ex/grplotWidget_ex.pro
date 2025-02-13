GRDIR = $$(GRDIR)
isEmpty(GRDIR) {
  GRDIR = /usr/local/gr
}
TEMPLATE = app
TARGET = grplotWidget_ex
INCLUDEPATH += $$GRDIR/include $$GRDIR/include/grplotWidget
LIBS += -L$$GRDIR/lib -lGR -lGRM -lgrplotWidget

QT += widgets

QMAKE_RPATHDIR += $$GRDIR/lib

HEADERS += main_window.h
SOURCES += main.cpp \
           main_window.cpp
