GRDIR = /usr/local/gr
QMAKE = qmake-qt5
UNAME_S := $(shell uname -s)
QTINCDIR := $(shell $(QMAKE) -query QT_INSTALL_HEADERS)
QTLIBDIR := $(shell $(QMAKE) -query QT_INSTALL_LIBS)
ifeq ($(UNAME_S),Darwin)
	CXXFLAGS = -std=c++11 -fPIC -I$(QTINCDIR) -I$(QTLIBDIR)/QtCore.framework/Versions/5/Headers -I$(QTLIBDIR)/QtGui.framework/Versions/5/Headers -I$(QTLIBDIR)/QtWidgets.framework/Versions/5/Headers -I$(GRDIR)/include -F$(QTLIBDIR)
	LIBS = -F$(QTLIBDIR) -L$(GRDIR)/lib -framework QtWidgets -framework QtGui -framework QtCore -lGR -lqt5gr -Xlinker -rpath -Xlinker $(QTLIBDIR) -Xlinker -rpath -Xlinker $(GRDIR)/lib
else
	CXXFLAGS = -std=c++11 -fPIC -I$(QTINCDIR) -I$(QTINCDIR)/QtCore -I$(QTINCDIR)/QtGui -I$(QTINCDIR)/QtWidgets -I$(GRDIR)/include
	LIBS = -L$(QTLIBDIR) -L$(GRDIR)/lib -lQt5Widgets -lQt5Gui -lQt5Core -lGR -lqt5gr -Xlinker -rpath -Xlinker $(QTLIBDIR) -Xlinker -rpath -Xlinker $(GRDIR)/lib
endif


all: main

main: main.o main_window.o
	$(CXX) -o $@ $^ $(LIBS)

%.o: %.cpp
	$(CXX) -c $(CXXFLAGS) $<

clean:
	rm -f main *.o
