LIBDIR = $(DESTDIR)$(GRDIR)/lib
INCDIR = $(DESTDIR)$(GRDIR)/include

UNAME_S := $(shell uname -s)

CFLAGS = -O3 -Wall -fPIC $(EXTRA_CFLAGS)
LDFLAGS = $(EXTRA_LDFLAGS)
ifeq ($(UNAME_S),Linux)
LDFLAGS += -Wl,-soname,$@
endif


ifeq ($(UNAME_S),Darwin)
GR_SHARED_LIBRARY_SUFFIX ?= .dylib
else
GR_SHARED_LIBRARY_SUFFIX ?= .so
endif

ifneq ($(QT4_QMAKE),false)
QT4INCDIR = `$(QT4_QMAKE) -query QT_INSTALL_HEADERS`
QT4INC = -I$(QT4INCDIR) -I$(QT4INCDIR)/QtCore -I$(QT4INCDIR)/QtGui
ifeq ($(UNAME_S),Darwin)
ifneq ($(QT4INC),)
	QT4INC += -mmacosx-version-min=10.15
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

ifneq ($(QT6_QMAKE),false)
QT6DEFS =
QT6INCDIR = `$(QT6_QMAKE) -query QT_INSTALL_HEADERS`
ifeq ($(UNAME_S),Darwin)
	QT6INC = -I$(QT6INCDIR) -I$(QT6LIBDIR)/QtCore.framework/Versions/A/Headers -I$(QT6LIBDIR)/QtGui.framework/Versions/A/Headers -I$(QT6LIBDIR)/QtWidgets.framework/Versions/A/Headers -F$(QT6LIBDIR)
else
	QT6INC = -I$(QT6INCDIR) -I$(QT6INCDIR)/QtCore -I$(QT6INCDIR)/QtGui -I$(QT6INCDIR)/QtWidgets
endif
QT6INC += -std=c++17
QT6LIBDIR = `$(QT6_QMAKE) -query QT_INSTALL_LIBS`
ifeq ($(UNAME_S),Darwin)
	QT6LIBS = -F$(QT6LIBDIR) -framework QtWidgets -framework QtGui -framework QtCore
else
	QT6LIBS = -L$(QT6LIBDIR) -lQt6Widgets -lQt6Gui -lQt6Core
endif
endif

ifeq ($(UNAME_S),Darwin)
	LIBQT4GR_INSTALL_NAME = -Wl,-install_name,@rpath/libqt4gr${GR_SHARED_LIBRARY_SUFFIX}
	LIBQT5GR_INSTALL_NAME = -Wl,-install_name,@rpath/libqt5gr${GR_SHARED_LIBRARY_SUFFIX}
	LIBQT6GR_INSTALL_NAME = -Wl,-install_name,@rpath/libqt6gr${GR_SHARED_LIBRARY_SUFFIX}
else
	LIBQT4GR_INSTALL_NAME =
	LIBQT5GR_INSTALL_NAME =
	LIBQT6GR_INSTALL_NAME =
endif

GRINC = -I..
GRLIBS = -L.. -lGR


default:
	$(MAKE) -C ../../.. Makedefs
	$(MAKE) `cat ../../../Makedefs`

all: targets

targets: libqt4gr${GR_SHARED_LIBRARY_SUFFIX} libqt5gr${GR_SHARED_LIBRARY_SUFFIX} libqt6gr${GR_SHARED_LIBRARY_SUFFIX}

ifneq ($(QT4_QMAKE),false)
libqt4gr${GR_SHARED_LIBRARY_SUFFIX}: grwidget_qt4.o
	$(CXX) -shared -o $@ $^ $(LDFLAGS) $(QT4LIBS) $(GRLIBS) $(LIBQT4GR_INSTALL_NAME)

grwidget_qt4.o: grwidget.cxx
	$(CXX) -o $@ -c $(CFLAGS) $(QT4INC) $(GRINC) $<
else
libqt4gr${GR_SHARED_LIBRARY_SUFFIX}:
endif

ifneq ($(QT5_QMAKE),false)
libqt5gr${GR_SHARED_LIBRARY_SUFFIX}: grwidget_qt5.o
	$(CXX) -shared -o $@ $^ $(LDFLAGS) $(QT5LIBS) $(GRLIBS) $(LIBQT5GR_INSTALL_NAME)

grwidget_qt5.o: grwidget.cxx
	$(CXX) -o $@ -c $(CFLAGS) $(QT5INC) $(GRINC) $<
else
libqt5gr${GR_SHARED_LIBRARY_SUFFIX}:
endif

ifneq ($(QT6_QMAKE),false)
libqt6gr${GR_SHARED_LIBRARY_SUFFIX}: grwidget_qt6.o
	$(CXX) -shared -o $@ $^ $(LDFLAGS) $(QT6LIBS) $(GRLIBS) $(LIBQT6GR_INSTALL_NAME)

grwidget_qt6.o: grwidget.cxx
	$(CXX) -o $@ -c $(CFLAGS) $(QT6INC) $(GRINC) $<
else
libqt6gr${GR_SHARED_LIBRARY_SUFFIX}:
endif

install:
ifneq (false false,$(filter false,$(QT4_QMAKE) $(QT5_QMAKE) $(QT6_QMAKE)))   # install only if any Qt is present
	@if [ ! -d $(LIBDIR) ]; then mkdir -m 755 $(LIBDIR); fi
	cp -p libqt*.* $(LIBDIR)
	@if [ ! -d $(INCDIR) ]; then mkdir -m 755 $(INCDIR); fi
	cp -p grwidget.h $(INCDIR)
endif

clean:
	rm -f *.so *.o *${GR_SHARED_LIBRARY_SUFFIX}

.PHONY: default all targets install clean
