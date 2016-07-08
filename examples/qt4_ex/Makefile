GRDIR = /usr/local/gr
QMAKE = qmake
QTINCDIR := $(shell $(QMAKE) -query QT_INSTALL_HEADERS)
QTLIBDIR := $(shell $(QMAKE) -query QT_INSTALL_LIBS)
CXXFLAGS = -I$(QTINCDIR) -I$(QTINCDIR)/QtCore -I$(QTINCDIR)/QtGui -I$(GRDIR)/include
LIBS = -L$(QTLIBDIR) -L$(GRDIR)/lib -lQtGui -lQtCore -lGR -lqt4gr -Xlinker -rpath -Xlinker $(QTLIBDIR) -Xlinker -rpath -Xlinker $(GRDIR)/lib


all: main

main: main.o main_window.o
	$(CXX) -o $@ $^ $(LIBS)

%.o: %.cpp
	$(CXX) -c $(CXXFLAGS) $<

clean:
	rm -f main *.o
