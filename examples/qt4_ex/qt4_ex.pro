GRDIR = $$(GRDIR)
isEmpty(GRDIR) {
  GRDIR = /usr/local/gr
}

TEMPLATE = app
TARGET = qt4_ex
DEPENDPATH += .
INCLUDEPATH += .

INCLUDEPATH += $$GRDIR/include
LIBS += -L$$GRDIR/lib -lGR -lGRM -lqt4gr
QMAKE_LFLAGS += -Wl,-rpath,$$GRDIR/lib

# Input
HEADERS += main_window.h
SOURCES += main.cpp main_window.cpp
