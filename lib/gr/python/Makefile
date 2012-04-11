      GRDIR = /usr/local/gr
     LIBDIR = $(GRDIR)/lib

    INSTALL = cp
    DEFINES =
   INCLUDES = -I.. -I../../gks
     CFLAGS = -fPIC -Wall $(DEFINES) $(INCLUDES) $(ARCHS)
    SOFLAGS = -shared
    LDFLAGS = $(ARCHS)
  EXTRALIBS =
   XLIBPATH = /usr/X11R6/lib
      XLIBS = -L$(XLIBPATH) -lXt -lX11
       LIBS = -lc -lm

.c.o:
	$(CC) -c $(CFLAGS) $<

default:
	@make `./Config`

Darwin:
	@make all SOFLAGS="-F. -bundle" static=true \
	LDFLAGS="-framework Carbon -Wl,-install_name,$(LIBDIR)/gr.so"
Linux:
	make all SOFLAGS="-shared" LDFLAGS="-Wl,-rpath=$(LIBDIR)" \
	EXTRALIBS="-lutil"

all: gr.so gr

gr.so: py_gr.o
ifdef static
	$(CC) -o $@ $(SOFLAGS) $(LDFLAGS) $? -L.. -lGR -L../../gks -lGKS \
	$(PYTHON_HOME)/lib/python$(PY_VER)/config/libpython$(PY_VER).a \
	$(EXTRALIBS) -L$(XLIBPATH) -lXt -lX11 $(LIBS)
else
	$(CC) -o $@ $(SOFLAGS) $(LDFLAGS) $? -L.. -lGR -L../../gks -lGKS \
	-L$(PYTHON_HOME)/lib -lpython$(PY_VER) \
	$(EXTRALIBS) -L$(XLIBPATH) -lXt -lX11 $(LIBS)
endif

py_gr.o: py_gr.c
	$(CC) -c $(CFLAGS) -I$(PYTHON_HOME)/include/python$(PY_VER) -o $@ $<

gr: py_gr.c
ifdef static
	$(CC) -D__MAIN__ -o $@ $(LDFLAGS) $< -L.. -lGR -L../../gks -lGKS \
	-I.. -I$(PYTHON_HOME)/include/python$(PY_VER) \
	$(PYTHON_HOME)/lib/python$(PY_VER)/config/libpython$(PY_VER).a \
	$(EXTRALIBS) -L$(XLIBPATH) -lXt -lX11 $(LIBS)
else
	$(CC) -D__MAIN__ -o $@ $< $(LDFLAGS) -L.. -lGR -L../../gks -lGKS \
	-I.. -I$(PYTHON_HOME)/include/python$(PY_VER) \
	-L$(PYTHON_HOME)/lib -lpython$(PY_VER) \
	$(EXTRALIBS) -L$(XLIBPATH) -lXt -lX11 $(LIBS)
endif

install: gr.so
	$(INSTALL) -p gr.so $(LIBDIR)/gr.so

clean:
	rm -f gr so_locations *.so *.a *.o *.bak *~
