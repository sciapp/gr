  CFLAGS = -O -I. -fPIC -fno-common $(ARCHS)
    OBJS = \
        jaricom.o jcapimin.o jcapistd.o jcarith.o \
        jccoefct.o jccolor.o jcdctmgr.o jchuff.o \
        jcinit.o jcmainct.o jcmarker.o jcmaster.o \
        jcomapi.o jcparam.o jcprepct.o jcsample.o \
        jctrans.o jdapimin.o jdapistd.o jdarith.o \
        jdatadst.o jdatasrc.o jdcoefct.o jdcolor.o \
        jddctmgr.o jdhuff.o jdinput.o jdmainct.o \
        jdmarker.o jdmaster.o jdmerge.o jdpostct.o \
        jdsample.o jdtrans.o jerror.o jfdctflt.o \
        jfdctfst.o jfdctint.o jidctflt.o jidctfst.o \
        jidctint.o jquant1.o jquant2.o jutils.o \
        jmemmgr.o jmemnobs.o

libjpeg.a: $(OBJS)
	$(AR) crv $@ $?
	$(AR) ts $@

distclean: clean

clean:
	rm -f libjpeg.a *.o *.bak

depend:
	makedepend -Y -- \
        jaricom.c jcapimin.c jcapistd.c jcarith.c \
        jccoefct.c jccolor.c jcdctmgr.c jchuff.c \
        jcinit.c jcmainct.c jcmarker.c jcmaster.c \
        jcomapi.c jcparam.c jcprepct.c jcsample.c \
        jctrans.c jdapimin.c jdapistd.c jdarith.c \
        jdatadst.c jdatasrc.c jdcoefct.c jdcolor.c \
        jddctmgr.c jdhuff.c jdinput.c jdmainct.c \
        jdmarker.c jdmaster.c jdmerge.c jdpostct.c \
        jdsample.c jdtrans.c jerror.c jfdctflt.c \
        jfdctfst.c jfdctint.c jidctflt.c jidctfst.c \
        jidctint.c jquant1.c jquant2.c jutils.c \
        jmemmgr.c jmemnobs.c 2> /dev/null

# DO NOT DELETE THIS LINE -- make depend depends on it.

jaricom.o: jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h
jcapimin.o: jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h
jcapistd.o: jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h
jcarith.o: jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h
jccoefct.o: jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h
jccolor.o: jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h
jcdctmgr.o: jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h
jcdctmgr.o: jdct.h
jchuff.o: jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h
jcinit.o: jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h
jcmainct.o: jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h
jcmarker.o: jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h
jcmaster.o: jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h
jcomapi.o: jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h
jcparam.o: jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h
jcprepct.o: jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h
jcsample.o: jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h
jctrans.o: jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h
jdapimin.o: jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h
jdapistd.o: jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h
jdarith.o: jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h
jdatadst.o: jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h
jdatasrc.o: jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h
jdcoefct.o: jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h
jdcolor.o: jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h
jddctmgr.o: jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h
jddctmgr.o: jdct.h
jdhuff.o: jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h
jdinput.o: jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h
jdmainct.o: jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h
jdmarker.o: jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h
jdmaster.o: jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h
jdmerge.o: jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h
jdpostct.o: jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h
jdsample.o: jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h
jdtrans.o: jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h
jerror.o: jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h
jerror.o: jversion.h
jfdctflt.o: jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h
jfdctflt.o: jdct.h
jfdctfst.o: jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h
jfdctfst.o: jdct.h
jfdctint.o: jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h
jfdctint.o: jdct.h
jidctflt.o: jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h
jidctflt.o: jdct.h
jidctfst.o: jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h
jidctfst.o: jdct.h
jidctint.o: jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h
jidctint.o: jdct.h
jquant1.o: jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h
jquant2.o: jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h
jutils.o: jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h
jmemmgr.o: jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h
jmemmgr.o: jmemsys.h
jmemnobs.o: jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h
jmemnobs.o: jmemsys.h
