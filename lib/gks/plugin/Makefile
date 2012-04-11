    GRDIR = /usr/local/gr
   LIBDIR = $(GRDIR)/lib

  DEFINES = -DGRDIR=\"$(GRDIR)\"
 CXXFLAGS = -g -Wall -fPIC $(DEFINES) $(INCLUDES) 
  SOFLAGS = -shared
  LDFLAGS =
   QTLIBS = -L$(QTDIR)/lib -lQtGui -lQtCore
    XLIBS = -L/usr/X11R6/lib -lXt -lX11
  PNGLIBS = -lpng
   GSLIBS = -lgs
    GSINC = -I/usr/include/ghostscript
     LIBS = -lc -lm
    ZLIBS = -lz

  TARGETS = qtplugin.so svgplugin.so figplugin.so gsplugin.so wmfplugin.so

.cxx.o:
	$(CXX) -c $(CXXFLAGS) $<

default:
	@make `uname`

Darwin:
	@make $(TARGETS) quartzplugin.so \
	INCLUDES="-I.. -I../../png -I/usr/local/include" \
	PNGLIBS="-L../../png -lpng" SOFLAGS="-F. -bundle" \
	GSINC="-I/usr/local/include/ghostscript" LIBS="-liconv"
Linux:
	@make $(TARGETS) \
	INCLUDES="-I.. -I../../png" PNGLIBS="-L../../png -lpng" \
	SOFLAGS="-shared" LDFLAGS="-Wl,-rpath,$(QTDIR)/lib"

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

qtplugin.o: qtplugin.cxx
	$(CXX) -c $(CXXFLAGS) -I$(QTDIR)/include -o $@ $<

svgplugin.o: svgplugin.cxx
	$(CXX) -c $(CXXFLAGS) -o $@ $<

figplugin.o: figplugin.cxx
	$(CXX) -c $(CXXFLAGS) -o $@ $<

gsplugin.o: gsplugin.cxx
	$(CXX) -c $(CXXFLAGS) $(GSINC) -o $@ $<

wmfplugin.o: wmfplugin.cxx
	$(CXX) -c $(CXXFLAGS) -o $@ $<

quartzplugin.o: quartzplugin.m
	$(CXX) -c $(CXXFLAGS) -o $@ $<

font.o: font.cxx
	$(CXX) -c $(CXXFLAGS) -o $@ $<
afm.o: afm.cxx
	$(CXX) -c $(CXXFLAGS) -o $@ $<
util.o: util.cxx
	$(CXX) -c $(CXXFLAGS) -o $@ $<
dl.o: dl.cxx
	$(CXX) -c $(CXXFLAGS) -o $@ $<
malloc.o: malloc.cxx
	$(CXX) -c $(CXXFLAGS) -o $@ $<
error.o: error.cxx
	$(CXX) -c $(CXXFLAGS) -o $@ $<
io.o: io.cxx
	$(CXX) -c $(CXXFLAGS) -o $@ $<

install: default $(TARGETS)
	cp -p *.so $(LIBDIR)

clean:
	rm -f so_locations *.so *.o *.bak *~ *.tmp
