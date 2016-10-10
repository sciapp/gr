LIBDIR = $(DESTDIR)$(GRDIR)/lib
INCDIR = $(DESTDIR)$(GRDIR)/include

UNAME_S := $(shell uname -s)

CFLAGS = -O3 -Wall -fPIC $(EXTRA_CFLAGS)
LDFLAGS = $(EXTRA_LDFLAGS)

ifneq ($(QT4_QMAKE),false)
QT4INCDIR = `$(QT4_QMAKE) -query QT_INSTALL_HEADERS`
QT4INC = -I$(QT4INCDIR) -I$(QT4INCDIR)/QtCore -I$(QT4INCDIR)/QtGui
ifeq ($(UNAME_S),Darwin)
ifneq ($(QT4INC),)
	QT4INC += -mmacosx-version-min=10.7
endif
endif
QT4LIBDIR = `$(QT4_QMAKE) -query QT_INSTALL_LIBS`
QT4LIBS = -L$(QT4LIBDIR) -lQtGui -lQtCore
endif

ifneq ($(QT5_QMAKE),false)
QT5DEFS =
QT5INCDIR = `$(QT5_QMAKE) -query QT_INSTALL_HEADERS`
ifeq ($(UNAME_S),Darwin)
	QT5INC = -I$(QT5INCDIR) -I$(QT5LIBDIR)/QtCore.framework/Versions/5/Headers -I$(QT5LIBDIR)/QtGui.framework/Versions/5/Headers -I$(QT5LIBDIR)/QtWidgets.framework/Versions/5/Headers -F$(QT5LIBDIR)
else
	QT5INC = -I$(QT5INCDIR) -I$(QT5INCDIR)/QtCore -I$(QT5INCDIR)/QtGui -I$(QT5INCDIR)/QtWidgets
endif
QT5INC += -std=c++11
QT5LIBDIR = `$(QT5_QMAKE) -query QT_INSTALL_LIBS`
ifeq ($(UNAME_S),Darwin)
	QT5LIBS = -F$(QT5LIBDIR) -framework QtWidgets -framework QtGui -framework QtCore
else
	QT5LIBS = -L$(QT5LIBDIR) -lQt5Widgets -lQt5Gui -lQt5Core
endif
endif

GRINC = -I..
GRLIBS = -L.. -lGR


default:
	@make -C ../../.. Makedefs
	@make `cat ../../../Makedefs`

Linux:
	@make targets
Darwin:
	@make targets

targets: libqt4gr.so libqt5gr.so

ifneq ($(QT4_QMAKE),false)
libqt4gr.so: grwidget_qt4.o
	$(CXX) -shared -o $@ $^ $(LDFLAGS) $(QT4LIBS) $(GRLIBS)

grwidget_qt4.o: grwidget.cxx
	$(CXX) -o $@ -c $(CFLAGS) $(QT4INC) $(GRINC) $<
else
libqt4gr.so:
endif

ifneq ($(QT5_QMAKE),false)
libqt5gr.so: grwidget_qt5.o
	$(CXX) -shared -o $@ $^ $(LDFLAGS) $(QT5LIBS) $(GRLIBS)

grwidget_qt5.o: grwidget.cxx
	$(CXX) -o $@ -c $(CFLAGS) $(QT5INC) $(GRINC) $<
else
libqt5gr.so:
endif

install:
	@if [ ! -d $(LIBDIR) ]; then mkdir -m 755 $(LIBDIR); fi
	cp -p libqt*.* $(LIBDIR)
ifneq (false false,$(filter false,$(QT4_QMAKE) $(QT5_QMAKE)))   # copy grwidget.h only if Qt4 or/and Qt5 are present
	@if [ ! -d $(INCDIR) ]; then mkdir -m 755 $(INCDIR); fi
	cp -p grwidget.h $(INCDIR)
endif

clean:
	rm -f *.so *.o

.PHONY: default Linux Darwin targets install clean
