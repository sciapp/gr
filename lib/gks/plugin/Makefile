   UNAME_S := $(shell uname -s)

      GRDIR = /usr/local/gr
     LIBDIR = $(DESTDIR)$(GRDIR)/lib

     CFLAGS = -O -Wall -fPIC $(EXTRA_CFLAGS)
   CXXFLAGS = -O -Wall -fPIC $(EXTRA_CXXFLAGS)
   INCLUDES = -I.. -I../../../3rdparty
    SOFLAGS = -shared
    LDFLAGS = $(EXTRA_LDFLAGS)
  WX_CONFIG = false
     WXDEFS =
      WXINC = `$(WX_CONFIG) --cxxflags 2>/dev/null`
     WXLIBS = `$(WX_CONFIG) --libs 2>/dev/null`
ifeq ($(UNAME_S),Darwin)
ifneq ($(WXLIBS),)
     WXLIBS += -mmacosx-version-min=10.7
endif
endif
ifneq ($(QT4_QMAKE),false)
     QT4DEFS =
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
 GTK_CONFIG = false
    GTKDEFS =
     GTKINC = `$(GTK_CONFIG) gtk+-2.0 --cflags 2>/dev/null`
    GTKLIBS = `$(GTK_CONFIG) gtk+-2.0 --libs 2>/dev/null`
   JPEGLIBS = ../../../3rdparty/jpeg/libjpeg.a
   JPEGDEFS = -I../../../3rdparty/jpeg/
    PNGLIBS = ../../../3rdparty/libpng16/libpng.a
    BZ2LIBS = ../../../3rdparty/bzip2/libbz2.a
     GSDEFS =
      GSINC =
     GSLIBS = -lgs
   GLFWDEFS =
    GLFWINC =
    GLFWLIB = glfw
ifeq ($(UNAME_S),Darwin)
   GLFWLIBS = -l$(GLFWLIB) -framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo
else
   GLFWLIBS = -l$(GLFWLIB) -lGL
endif
    XFTDEFS =
     FTDEFS =
      FTINC = -I../../../3rdparty/freetype/include
     FTLIBS = ../../../3rdparty/freetype/libfreetype.a
     AVDEFS =
     AVLIBS = -lavdevice -lavformat -lavfilter -lavcodec -lswscale -lavutil -ltheora -logg -lvpx $(BZ2LIBS)
  MUPDFDEFS =
  MUPDFLIBS = -lmupdf $(FTLIBS) -ljbig2dec $(JPEGLIBS) -lopenjp2 $(BZ2LIBS)
  CAIRODEFS =
ifdef USE_STATIC_CAIRO_LIBS
  CAIROLIBS = ../../../3rdparty/build/lib/libcairo.a ../../../3rdparty/build/lib/libpixman-1.a
else
  CAIROLIBS = -lcairo -lpixman-1 -lpthread
endif
    SSLLIBS = -lssl -lcrypto
    X11DEFS =
ifeq ($(UNAME_S), Darwin)
     X11INC = -I/opt/X11/include
else
     X11INC = -I/usr/X11R6/include
endif
    XFTLIBS = -lXft -lfontconfig
    X11LIBS = $(X11PATH) $(XFTLIBS) -lXt -lX11
    ZMQDEFS =
    ZMQLIBS = -L../../../3rdparty/build/lib/ -lzmq -lpthread
  EXTRALIBS =
       LIBS = -lc -lm
       ZINC = -I../../../3rdparty/zlib
      ZLIBS = ../../../3rdparty/zlib/libz.a
    GKSLIBS = ../libGKS.a

.SUFFIXES: .o .m .cxx

.m.o:
	$(CXX) -c -DGRDIR=\"$(GRDIR)\" $(DEFINES) $(INCLUDES) $(CXXFLAGS) $<
.c.o:
	$(CC) -c -DGRDIR=\"$(GRDIR)\" $(DEFINES) $(CFLAGS) $(INCLUDES) $(GTKDEFS) $(GTKINC) $(GSDEFS) $(GSINC) $(GLFWDEFS) $(GLFWINC) $(ZMQDEFS) $(XFTDEFS) $(FTDEFS) $(FTINC) $(AVDEFS) $(MUPDFDEFS) $(CAIRODEFS) $(X11DEFS) $(X11INC) $(ZINC) $<
.cxx.o:
	$(CXX) -c -DGRDIR=\"$(GRDIR)\" $(DEFINES) $(CXXFLAGS) $(INCLUDES) $(WXDEFS) $(WXINC) $(QT4DEFS) $(QT4INC) $(GTKDEFS) $(GTKINC) $(GSDEFS) $(GSINC) $(GLFWDEFS) $(GLFWINC) $(ZMQDEFS) $(XFTDEFS) $(FTDEFS) $(FTINC) $(AVDEFS) $(MUPDFDEFS) $(CAIRODEFS) $(X11DEFS) $(X11INC) $(ZINC) $<

default:
	@make -C ../../.. Makedefs
	@make `cat ../../../Makedefs`

targets: prerequisites gks \
         x11plugin.so wxplugin.so qtplugin.so qt5plugin.so svgplugin.so \
         gtkplugin.so figplugin.so gsplugin.so wmfplugin.so glplugin.so \
         zmqplugin.so movplugin.so htmplugin.so pgfplugin.so cairoplugin.so \
         videoplugin.so

prerequisites:
	@for d in libpng16; do make -C ../../../3rdparty/$$d; done

gks:
	@make -C .. libGKS.a

Darwin:
	@make targets quartzplugin.so \
	EXTRALIBS="-framework VideoDecodeAcceleration -framework CoreVideo -framework CoreFoundation" LIBS="-liconv"
Linux:
	@make targets \
	SOFLAGS="-shared" \
	LDFLAGS="$(EXTRA_LDFLAGS)" \
	EXTRA_LDFLAGS_QT4="-Wl,-rpath=$(QT4LIBDIR)" \
	EXTRA_LDFLAGS_QT5="-Wl,-rpath=$(QT5LIBDIR)"

qtplugin.o: qtplugin.cxx
	$(CXX) -o $@ -c -DGRDIR=\"$(GRDIR)\" $(DEFINES) $(WXDEFS) $(WXINC) $(QT4DEFS) $(QT4INC) $(GTKDEFS) $(GTKINC) $(GSDEFS) $(GSINC) $(GLFWDEFS) $(GLFWINC) $(ZMQDEFS) $(INCLUDES) $(XFTDEFS) $(FTDEFS) $(FTINC) $(AVDEFS) $(MUPDFDEFS) $(CAIRODEFS) $(X11DEFS) $(X11INC) $(CXXFLAGS) $(ZINC) $<

qt5plugin.o: qt5plugin.cxx
	$(CXX) -o $@ -c -DGRDIR=\"$(GRDIR)\" $(DEFINES) $(WXDEFS) $(WXINC) $(QT5DEFS) $(QT5INC) $(GTKDEFS) $(GTKINC) $(GSDEFS) $(GSINC) $(GLFWDEFS) $(GLFWINC) $(ZMQDEFS) $(INCLUDES) $(XFTDEFS) $(FTDEFS) $(FTINC) $(AVDEFS) $(MUPDFDEFS) $(CAIRODEFS) $(X11DEFS) $(X11INC) $(CXXFLAGS) $(ZINC) $<

x11plugin.so: x11plugin.o
	$(CC) -o $@ $(SOFLAGS) $^ $(LDFLAGS) $(GKSLIBS) $(FTLIBS) $(X11LIBS) $(LIBS)

wxplugin.so: wxplugin.o
	$(CXX) -o $@ $(SOFLAGS) $^ $(LDFLAGS) $(GKSLIBS) $(WXLIBS) $(X11LIBS) $(LIBS)

qtplugin.so: qtplugin.o
	$(CXX) -o $@ $(SOFLAGS) $^ $(LDFLAGS) $(GKSLIBS) $(EXTRA_LDFLAGS_QT4) $(QT4LIBS) $(X11LIBS) $(LIBS)

qt5plugin.so: qt5plugin.o
	$(CXX) -o $@ $(SOFLAGS) $^ $(LDFLAGS) $(GKSLIBS) $(EXTRA_LDFLAGS_QT5) $(QT5LIBS) $(X11LIBS) $(LIBS)

svgplugin.so: svgplugin.o
	$(CC) -o $@ $(SOFLAGS) $^ $(LDFLAGS) $(GKSLIBS) $(PNGLIBS) $(ZLIBS) $(LIBS)

gtkplugin.so: gtkplugin.o
	$(CC) -o $@ $(SOFLAGS) $^ $(LDFLAGS) $(GKSLIBS) $(GTKLIBS) $(X11LIBS) $(LIBS)

figplugin.so: figplugin.o
	$(CC) -o $@ $(SOFLAGS) $^ $(LDFLAGS) $(GKSLIBS) $(PNGLIBS) $(ZLIBS) $(LIBS)

gsplugin.so: gsplugin.o
	$(CC) -o $@ $(SOFLAGS) $^ $(LDFLAGS) $(GKSLIBS) $(GSLIBS) $(X11LIBS) $(LIBS)

wmfplugin.so: wmfplugin.o
	$(CC) -o $@ $(SOFLAGS) $^ $(LDFLAGS) $(GKSLIBS) $(LIBS)

quartzplugin.so: quartzplugin.o
	$(CXX) -o $@ $(SOFLAGS) $^ $(GKSLIBS) $(LIBS) $(ZMQLIBS) \
	-lobjc -framework Foundation -framework ApplicationServices -framework AppKit

glplugin.so: glplugin.o
	$(CC) -o $@ $(SOFLAGS) $^ $(LDFLAGS) $(GKSLIBS) $(GLFWLIBS) $(FTLIBS) $(X11LIBS) $(LIBS)

zmqplugin.so: zmqplugin.o
	$(CXX) -o $@ $(SOFLAGS) $^ $(LDFLAGS) $(GKSLIBS) $(ZMQLIBS) $(LIBS)

movplugin.so: movplugin.o vc.o pdf.o gif.o
	$(CC) -o $@ $(SOFLAGS) $^ $(LDFLAGS) $(GKSLIBS) $(AVLIBS) $(MUPDFLIBS) $(SSLLIBS) $(EXTRALIBS) $(ZLIBS) $(LIBS)

htmplugin.so: htmplugin.o
	$(CC) -o $@ $(SOFLAGS) $^ $(LDFLAGS) $(GKSLIBS) $(PNGLIBS) $(ZLIBS) $(LIBS)

pgfplugin.so: pgfplugin.o
	$(CC) -o $@ $(SOFLAGS) $^ $(LDFLAGS) $(GKSLIBS) $(PNGLIBS) $(ZLIBS) $(LIBS)

cairoplugin.o: cairoplugin.c
	$(CC) -c -DGRDIR=\"$(GRDIR)\" $(DEFINES) $(INCLUDES) $(FTDEFS) $(FTINC) $(JPEGDEFS) $(CAIRODEFS) $(CXXFLAGS) $(ZINC) -DNO_X11 $<

cairoplugin.so: cairoplugin.o
	$(CC) -o $@ $(SOFLAGS) $^ $(LDFLAGS) $(GKSLIBS) $(CAIROLIBS) $(FTLIBS) $(JPEGLIBS) $(PNGLIBS) $(ZLIBS) $(LIBS) -DNO_X11

videoplugin.so: videoplugin.o vc.o
	$(CC) -o $@ $(SOFLAGS) $^ $(LDFLAGS) $(GKSLIBS) $(AVLIBS) $(FTLIBS) $(EXTRALIBS) $(ZLIBS) $(LIBS)

install: default $(TARGETS)
	cp -p *.so $(LIBDIR)

clean:
	rm -f so_locations *.so *.o *.bak *~ *.tmp

qtplugin.o: qtplugin_impl.cxx
qt5plugin.o: qtplugin_impl.cxx

.PHONY: clean install default gks Darwin Linux
