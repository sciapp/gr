    GRDIR = /usr/local/gr
   LIBDIR = $(GRDIR)/lib

 CXXFLAGS = -g -Wall -fPIC
 INCLUDES = -I.. -I../../png -I/usr/local/include
  SOFLAGS = -shared
  LDFLAGS =
    QTINC = -I$(QTDIR)/include
   QTLIBS = -L$(QTDIR)/lib -lQtGui -lQtCore
    XLIBS = -L/usr/X11R6/lib -lXt -lX11
  PNGLIBS = -L../../png -lpng
   GSLIBS = -lgs
    GSINC = -I/usr/include/ghostscript
     LIBS = -lc -lm
    ZLIBS = -lz

.SUFFIXES: .o .m .cxx

.m.o:
	$(CXX) -c $(CXXFLAGS) -DGRDIR=\"$(GRDIR)\" $(DEFINES) $(QTINC) $(GSINC) $(INCLUDES) $<
.cxx.o:
	$(CXX) -c $(CXXFLAGS) -DGRDIR=\"$(GRDIR)\" $(DEFINES) $(QTINC) $(GSINC) $(INCLUDES) $<

default:
	@make `uname`

targets: qtplugin.so svgplugin.so figplugin.so gsplugin.so wmfplugin.so

Darwin:
	@make targets quartzplugin.so \
	GSINC="-I/usr/local/include/ghostscript" LIBS="-liconv"
Linux:
	@make targets \
	SOFLAGS="-shared" LDFLAGS="-Wl,-rpath=$(QTDIR)/lib"

qtplugin.so: qtplugin.o font.o afm.o util.o dl.o malloc.o error.o io.o
	$(CXX) -o $@ $(SOFLAGS) $? $(QTLIBS) $(LDFLAGS) $(XLIBS) $(LIBS)

svgplugin.so: svgplugin.o font.o afm.o util.o malloc.o error.o io.o
	$(CXX) -o $@ $(SOFLAGS) $? $(PNGLIBS) $(ZLIBS) $(LIBS)

figplugin.so: figplugin.o font.o afm.o util.o malloc.o error.o io.o
	$(CXX) -o $@ $(SOFLAGS) $? $(PNGLIBS) $(ZLIBS) $(LIBS)

gsplugin.so: gsplugin.o font.o afm.o util.o malloc.o error.o io.o
	$(CXX) -o $@ $(SOFLAGS) $? $(GSLIBS) $(XLIBS) $(LIBS)

wmfplugin.so: wmfplugin.o font.o afm.o util.o malloc.o error.o io.o
	$(CXX) -o $@ $(SOFLAGS) $? $(LIBS)

quartzplugin.so: quartzplugin.o font.o afm.o util.o dl.o malloc.o error.o io.o
	$(CXX) -o $@ $(SOFLAGS) $? $(LIBS) \
	-lobjc -framework Foundation -framework ApplicationServices

install: default $(TARGETS)
	cp -p *.so $(LIBDIR)

clean:
	rm -f so_locations *.so *.o *.bak *~ *.tmp
