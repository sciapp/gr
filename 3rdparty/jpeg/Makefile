  CFLAGS = -O -I. -fPIC -fno-common $(ARCHS)
    OBJS = \
        jaricom.o \
        jcapimin.o \
        jcapistd.o \
        jcarith.o \
        jccoefct.o \
        jccolor.o \
        jcdctmgr.o \
        jchuff.o \
        jcinit.o \
        jcmainct.o \
        jcmarker.o \
        jcmaster.o \
        jcomapi.o \
        jcparam.o \
        jcprepct.o \
        jcsample.o \
        jctrans.o \
        jdapimin.o \
        jdapistd.o \
        jdarith.o \
        jdatadst.o \
        jdatasrc.o \
        jdcoefct.o \
        jdcolor.o \
        jddctmgr.o \
        jdhuff.o \
        jdinput.o \
        jdmainct.o \
        jdmarker.o \
        jdmaster.o \
        jdmerge.o \
        jdpostct.o \
        jdsample.o \
        jdtrans.o \
        jerror.o \
        jfdctflt.o \
        jfdctfst.o \
        jfdctint.o \
        jidctflt.o \
        jidctfst.o \
        jidctint.o \
        jmemmgr.o \
        jmemnobs.o \
        jquant1.o \
        jquant2.o \
        jutils.o

libjpeg.a: $(OBJS)
	$(AR) crv $@ $?
	$(AR) ts $@

distclean: clean

clean:
	rm -f libjpeg.a *.o *.bak

depend:
	makedepend -Y -- \
        jaricom.c \
        jcapimin.c \
        jcapistd.c \
        jcarith.c \
        jccoefct.c \
        jccolor.c \
        jcdctmgr.c \
        jchuff.c \
        jcinit.c \
        jcmainct.c \
        jcmarker.c \
        jcmaster.c \
        jcomapi.c \
        jcparam.c \
        jcprepct.c \
        jcsample.c \
        jctrans.c \
        jdapimin.c \
        jdapistd.c \
        jdarith.c \
        jdatadst.c \
        jdatasrc.c \
        jdcoefct.c \
        jdcolor.c \
        jddctmgr.c \
        jdhuff.c \
        jdinput.c \
        jdmainct.c \
        jdmarker.c \
        jdmaster.c \
        jdmerge.c \
        jdpostct.c \
        jdsample.c \
        jdtrans.c \
        jerror.c \
        jfdctflt.c \
        jfdctfst.c \
        jfdctint.c \
        jidctflt.c \
        jidctfst.c \
        jidctint.c \
        jmemmgr.c \
        jmemnobs.c \
        jquant1.c \
        jquant2.c \
        jutils.c 2> /dev/null

# DO NOT DELETE THIS LINE -- make depend depends on it.

jaricom.o: jinclude.h jpeglib.h jmorecfg.h jpegint.h jerror.h
jcapimin.o: jinclude.h jpeglib.h jmorecfg.h jpegint.h jerror.h
jcapistd.o: jinclude.h jpeglib.h jmorecfg.h jpegint.h jerror.h
jcarith.o: jinclude.h jpeglib.h jmorecfg.h jpegint.h jerror.h
jccoefct.o: jinclude.h jpeglib.h jmorecfg.h jpegint.h jerror.h
jccolor.o: jinclude.h jpeglib.h jmorecfg.h jpegint.h jerror.h
jcdctmgr.o: jinclude.h jpeglib.h jmorecfg.h jpegint.h jerror.h jdct.h
jchuff.o: jinclude.h jpeglib.h jmorecfg.h jpegint.h jerror.h
jcinit.o: jinclude.h jpeglib.h jmorecfg.h jpegint.h jerror.h
jcmainct.o: jinclude.h jpeglib.h jmorecfg.h jpegint.h jerror.h
jcmarker.o: jinclude.h jpeglib.h jmorecfg.h jpegint.h jerror.h
jcmaster.o: jinclude.h jpeglib.h jmorecfg.h jpegint.h jerror.h
jcomapi.o: jinclude.h jpeglib.h jmorecfg.h jpegint.h jerror.h
jcparam.o: jinclude.h jpeglib.h jmorecfg.h jpegint.h jerror.h
jcprepct.o: jinclude.h jpeglib.h jmorecfg.h jpegint.h jerror.h
jcsample.o: jinclude.h jpeglib.h jmorecfg.h jpegint.h jerror.h
jctrans.o: jinclude.h jpeglib.h jmorecfg.h jpegint.h jerror.h
jdapimin.o: jinclude.h jpeglib.h jmorecfg.h jpegint.h jerror.h
jdapistd.o: jinclude.h jpeglib.h jmorecfg.h jpegint.h jerror.h
jdarith.o: jinclude.h jpeglib.h jmorecfg.h jpegint.h jerror.h
jdatadst.o: jinclude.h jpeglib.h jmorecfg.h jpegint.h jerror.h
jdatasrc.o: jinclude.h jpeglib.h jmorecfg.h jpegint.h jerror.h
jdcoefct.o: jinclude.h jpeglib.h jmorecfg.h jpegint.h jerror.h
jdcolor.o: jinclude.h jpeglib.h jmorecfg.h jpegint.h jerror.h
jddctmgr.o: jinclude.h jpeglib.h jmorecfg.h jpegint.h jerror.h jdct.h
jdhuff.o: jinclude.h jpeglib.h jmorecfg.h jpegint.h jerror.h
jdinput.o: jinclude.h jpeglib.h jmorecfg.h jpegint.h jerror.h
jdmainct.o: jinclude.h jpeglib.h jmorecfg.h jpegint.h jerror.h
jdmarker.o: jinclude.h jpeglib.h jmorecfg.h jpegint.h jerror.h
jdmaster.o: jinclude.h jpeglib.h jmorecfg.h jpegint.h jerror.h
jdmerge.o: jinclude.h jpeglib.h jmorecfg.h jpegint.h jerror.h
jdpostct.o: jinclude.h jpeglib.h jmorecfg.h jpegint.h jerror.h
jdsample.o: jinclude.h jpeglib.h jmorecfg.h jpegint.h jerror.h
jdtrans.o: jinclude.h jpeglib.h jmorecfg.h jpegint.h jerror.h
jerror.o: jinclude.h jpeglib.h jmorecfg.h jpegint.h jerror.h jversion.h
jfdctflt.o: jinclude.h jpeglib.h jmorecfg.h jpegint.h jerror.h jdct.h
jfdctfst.o: jinclude.h jpeglib.h jmorecfg.h jpegint.h jerror.h jdct.h
jfdctint.o: jinclude.h jpeglib.h jmorecfg.h jpegint.h jerror.h jdct.h
jidctflt.o: jinclude.h jpeglib.h jmorecfg.h jpegint.h jerror.h jdct.h
jidctfst.o: jinclude.h jpeglib.h jmorecfg.h jpegint.h jerror.h jdct.h
jidctint.o: jinclude.h jpeglib.h jmorecfg.h jpegint.h jerror.h jdct.h
jmemmgr.o: jinclude.h jpeglib.h jmorecfg.h jpegint.h jerror.h jmemsys.h
jmemnobs.o: jinclude.h jpeglib.h jmorecfg.h jpegint.h jerror.h jmemsys.h
jquant1.o: jinclude.h jpeglib.h jmorecfg.h jpegint.h jerror.h
jquant2.o: jinclude.h jpeglib.h jmorecfg.h jpegint.h jerror.h
jutils.o: jinclude.h jpeglib.h jmorecfg.h jpegint.h jerror.h
