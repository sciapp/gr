      GRDIR = /usr/local/gr
     LIBDIR = $(GRDIR)/lib

   CXXFLAGS = -g -Wall -fPIC
   INCLUDES = -I.. -I../../../3rdparty/png -I/usr/local/include
    SOFLAGS = -shared
    LDFLAGS =
  WX_CONFIG = false
     WXDEFS =
      WXINC = `$(WX_CONFIG) --cxxflags 2>/dev/null`
     WXLIBS = `$(WX_CONFIG) --libs 2>/dev/null`
     QTDEFS =
      QTINC = -I$(QTDIR)/include
     QTLIBS = -L$(QTDIR)/lib -lQtGui -lQtCore
 PKG_CONFIG = false
    GTKDEFS =
     GTKINC = `$(PKG_CONFIG) gtk+-2.0 --cflags 2>/dev/null`
    GTKLIBS = `$(PKG_CONFIG) gtk+-2.0 --libs 2>/dev/null`
      XLIBS = -L/usr/X11R6/lib -lXt -lX11
    PNGLIBS = -L../../../3rdparty/png -lpng
     GSDEFS = 
      GSINC = -I/usr/include/ghostscript
     GSLIBS = -lgs
   GLFWDEFS = -DXFT
    GLFWINC = -I/usr/include/freetype2 
   GLFWLIBS = -lfreetype -lglfw -lGL -lXrandr -lX11 -lrt
       LIBS = -lc -lm
      ZLIBS = -lz

.SUFFIXES: .o .m .cxx

.m.o:
	$(CXX) -c $(CXXFLAGS) -DGRDIR=\"$(GRDIR)\" $(DEFINES) $(INCLUDES) $<
.cxx.o:
	$(CXX) -c $(CXXFLAGS) -DGRDIR=\"$(GRDIR)\" $(DEFINES) $(WXDEFS) $(WXINC) $(QTDEFS) $(QTINC) $(GTKDEFS) $(GTKINC) $(GSDEFS) $(GSINC) $(GLFWDEFS) $(GLFWINC) $(INCLUDES) $<

default:
	@make `uname`

targets: wxplugin.so qtplugin.so svgplugin.so gtkplugin.so figplugin.so \
         gsplugin.so wmfplugin.so glplugin.so

Darwin:
	@make targets quartzplugin.so \
	GSINC="-I/usr/local/include/ghostscript" \
	GLFWINC="-I/usr/X11/include/freetype2" \
	GLFWLIBS="-lfreetype -lglfw -framework OpenGL -framework Cocoa -framework IOKit" \
	LIBS="-liconv"
Linux:
	@make targets \
	SOFLAGS="-shared" LDFLAGS="-Wl,-rpath=$(QTDIR)/lib"

wxplugin.so: wxplugin.o font.o afm.o util.o dl.o malloc.o error.o io.o
	$(CXX) -o $@ $(SOFLAGS) $? $(WXLIBS) $(LDFLAGS) $(XLIBS) $(LIBS)

qtplugin.so: qtplugin.o font.o afm.o util.o dl.o malloc.o error.o io.o
	$(CXX) -o $@ $(SOFLAGS) $? $(QTLIBS) $(LDFLAGS) $(XLIBS) $(LIBS)

svgplugin.so: svgplugin.o font.o afm.o util.o malloc.o error.o io.o
	$(CXX) -o $@ $(SOFLAGS) $? $(PNGLIBS) $(ZLIBS) $(LIBS)

gtkplugin.so: gtkplugin.o font.o afm.o util.o dl.o malloc.o error.o io.o
	$(CXX) -o $@ $(SOFLAGS) $? $(GTKLIBS) $(LDFLAGS) $(XLIBS) $(LIBS)

figplugin.so: figplugin.o font.o afm.o util.o malloc.o error.o io.o
	$(CXX) -o $@ $(SOFLAGS) $? $(PNGLIBS) $(ZLIBS) $(LIBS)

gsplugin.so: gsplugin.o font.o afm.o util.o malloc.o error.o io.o
	$(CXX) -o $@ $(SOFLAGS) $? $(GSLIBS) $(XLIBS) $(LIBS)

wmfplugin.so: wmfplugin.o font.o afm.o util.o malloc.o error.o io.o
	$(CXX) -o $@ $(SOFLAGS) $? $(LIBS)

quartzplugin.so: quartzplugin.o font.o afm.o util.o dl.o malloc.o error.o io.o
	$(CXX) -o $@ $(SOFLAGS) $? $(LIBS) \
	-lobjc -framework Foundation -framework ApplicationServices

glplugin.so: glplugin.o font.o afm.o util.o ft.o dl.o malloc.o error.o io.o
	$(CXX) -o $@ $(SOFLAGS) $? $(GLFWLIBS) $(XLIBS) $(LIBS)

install: default $(TARGETS)
	cp -p *.so $(LIBDIR)

clean:
	rm -f so_locations *.so *.o *.bak *~ *.tmp
