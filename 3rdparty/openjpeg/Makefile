  CFLAGS = -O -I. -fPIC -fno-common $(ARCHS)
    OBJS = \
        bio.o \
        cio.o \
        dwt.o \
        event.o \
        image.o \
        j2k.o \
        j2k_lib.o \
        jp2.o \
        jpt.o \
        mct.o \
        mqc.o \
        openjpeg.o \
        pi.o \
        raw.o \
        t1.o \
        t2.o \
        tcd.o \
        tgt.o \
        cidx_manager.o \
        phix_manager.o \
        ppix_manager.o \
        thix_manager.o \
        tpix_manager.o

libopenjpeg.a: $(OBJS)
	$(AR) crv $@ $?
	$(AR) ts $@

distclean: clean

clean:
	rm -f libopenjpeg.a *.o *.bak

depend:
	makedepend -Y -- \
        bio.c \
        cio.c \
        dwt.c \
        event.c \
        image.c \
        j2k.c \
        j2k_lib.c \
        jp2.c \
        jpt.c \
        mct.c \
        mqc.c \
        openjpeg.c \
        pi.c \
        raw.c \
        t1.c \
        t2.c \
        tcd.c \
        tgt.c \
        cidx_manager.c \
        phix_manager.c \
        ppix_manager.c \
        thix_manager.c \
        tpix_manager.c 2> /dev/null

# DO NOT DELETE THIS LINE -- make depend depends on it.

bio.o: opj_includes.h openjpeg.h j2k_lib.h opj_malloc.h event.h bio.h cio.h
bio.o: image.h j2k.h jp2.h jpt.h mqc.h raw.h tgt.h pi.h tcd.h t1.h dwt.h t2.h
bio.o: mct.h int.h fix.h cidx_manager.h indexbox_manager.h
cio.o: opj_includes.h openjpeg.h j2k_lib.h opj_malloc.h event.h bio.h cio.h
cio.o: image.h j2k.h jp2.h jpt.h mqc.h raw.h tgt.h pi.h tcd.h t1.h dwt.h t2.h
cio.o: mct.h int.h fix.h cidx_manager.h indexbox_manager.h
dwt.o: opj_includes.h openjpeg.h j2k_lib.h opj_malloc.h event.h bio.h cio.h
dwt.o: image.h j2k.h jp2.h jpt.h mqc.h raw.h tgt.h pi.h tcd.h t1.h dwt.h t2.h
dwt.o: mct.h int.h fix.h cidx_manager.h indexbox_manager.h
event.o: opj_includes.h openjpeg.h j2k_lib.h opj_malloc.h event.h bio.h cio.h
event.o: image.h j2k.h jp2.h jpt.h mqc.h raw.h tgt.h pi.h tcd.h t1.h dwt.h
event.o: t2.h mct.h int.h fix.h cidx_manager.h indexbox_manager.h
image.o: opj_includes.h openjpeg.h j2k_lib.h opj_malloc.h event.h bio.h cio.h
image.o: image.h j2k.h jp2.h jpt.h mqc.h raw.h tgt.h pi.h tcd.h t1.h dwt.h
image.o: t2.h mct.h int.h fix.h cidx_manager.h indexbox_manager.h
j2k.o: opj_includes.h openjpeg.h j2k_lib.h opj_malloc.h event.h bio.h cio.h
j2k.o: image.h j2k.h jp2.h jpt.h mqc.h raw.h tgt.h pi.h tcd.h t1.h dwt.h t2.h
j2k.o: mct.h int.h fix.h cidx_manager.h indexbox_manager.h
j2k_lib.o: opj_includes.h openjpeg.h j2k_lib.h opj_malloc.h event.h bio.h
j2k_lib.o: cio.h image.h j2k.h jp2.h jpt.h mqc.h raw.h tgt.h pi.h tcd.h t1.h
j2k_lib.o: dwt.h t2.h mct.h int.h fix.h cidx_manager.h indexbox_manager.h
jp2.o: opj_includes.h openjpeg.h j2k_lib.h opj_malloc.h event.h bio.h cio.h
jp2.o: image.h j2k.h jp2.h jpt.h mqc.h raw.h tgt.h pi.h tcd.h t1.h dwt.h t2.h
jp2.o: mct.h int.h fix.h cidx_manager.h indexbox_manager.h
jpt.o: opj_includes.h openjpeg.h j2k_lib.h opj_malloc.h event.h bio.h cio.h
jpt.o: image.h j2k.h jp2.h jpt.h mqc.h raw.h tgt.h pi.h tcd.h t1.h dwt.h t2.h
jpt.o: mct.h int.h fix.h cidx_manager.h indexbox_manager.h
mct.o: opj_includes.h openjpeg.h j2k_lib.h opj_malloc.h event.h bio.h cio.h
mct.o: image.h j2k.h jp2.h jpt.h mqc.h raw.h tgt.h pi.h tcd.h t1.h dwt.h t2.h
mct.o: mct.h int.h fix.h cidx_manager.h indexbox_manager.h
mqc.o: opj_includes.h openjpeg.h j2k_lib.h opj_malloc.h event.h bio.h cio.h
mqc.o: image.h j2k.h jp2.h jpt.h mqc.h raw.h tgt.h pi.h tcd.h t1.h dwt.h t2.h
mqc.o: mct.h int.h fix.h cidx_manager.h indexbox_manager.h
openjpeg.o: opj_config.h opj_includes.h openjpeg.h j2k_lib.h opj_malloc.h
openjpeg.o: event.h bio.h cio.h image.h j2k.h jp2.h jpt.h mqc.h raw.h tgt.h
openjpeg.o: pi.h tcd.h t1.h dwt.h t2.h mct.h int.h fix.h cidx_manager.h
openjpeg.o: indexbox_manager.h
pi.o: opj_includes.h openjpeg.h j2k_lib.h opj_malloc.h event.h bio.h cio.h
pi.o: image.h j2k.h jp2.h jpt.h mqc.h raw.h tgt.h pi.h tcd.h t1.h dwt.h t2.h
pi.o: mct.h int.h fix.h cidx_manager.h indexbox_manager.h
raw.o: opj_includes.h openjpeg.h j2k_lib.h opj_malloc.h event.h bio.h cio.h
raw.o: image.h j2k.h jp2.h jpt.h mqc.h raw.h tgt.h pi.h tcd.h t1.h dwt.h t2.h
raw.o: mct.h int.h fix.h cidx_manager.h indexbox_manager.h
t1.o: opj_includes.h openjpeg.h j2k_lib.h opj_malloc.h event.h bio.h cio.h
t1.o: image.h j2k.h jp2.h jpt.h mqc.h raw.h tgt.h pi.h tcd.h t1.h dwt.h t2.h
t1.o: mct.h int.h fix.h cidx_manager.h indexbox_manager.h t1_luts.h
t2.o: opj_includes.h openjpeg.h j2k_lib.h opj_malloc.h event.h bio.h cio.h
t2.o: image.h j2k.h jp2.h jpt.h mqc.h raw.h tgt.h pi.h tcd.h t1.h dwt.h t2.h
t2.o: mct.h int.h fix.h cidx_manager.h indexbox_manager.h
tcd.o: opj_includes.h openjpeg.h j2k_lib.h opj_malloc.h event.h bio.h cio.h
tcd.o: image.h j2k.h jp2.h jpt.h mqc.h raw.h tgt.h pi.h tcd.h t1.h dwt.h t2.h
tcd.o: mct.h int.h fix.h cidx_manager.h indexbox_manager.h
tgt.o: opj_includes.h openjpeg.h j2k_lib.h opj_malloc.h event.h bio.h cio.h
tgt.o: image.h j2k.h jp2.h jpt.h mqc.h raw.h tgt.h pi.h tcd.h t1.h dwt.h t2.h
tgt.o: mct.h int.h fix.h cidx_manager.h indexbox_manager.h
cidx_manager.o: opj_includes.h openjpeg.h j2k_lib.h opj_malloc.h event.h
cidx_manager.o: bio.h cio.h image.h j2k.h jp2.h jpt.h mqc.h raw.h tgt.h pi.h
cidx_manager.o: tcd.h t1.h dwt.h t2.h mct.h int.h fix.h cidx_manager.h
cidx_manager.o: indexbox_manager.h
phix_manager.o: opj_includes.h openjpeg.h j2k_lib.h opj_malloc.h event.h
phix_manager.o: bio.h cio.h image.h j2k.h jp2.h jpt.h mqc.h raw.h tgt.h pi.h
phix_manager.o: tcd.h t1.h dwt.h t2.h mct.h int.h fix.h cidx_manager.h
phix_manager.o: indexbox_manager.h
ppix_manager.o: opj_includes.h openjpeg.h j2k_lib.h opj_malloc.h event.h
ppix_manager.o: bio.h cio.h image.h j2k.h jp2.h jpt.h mqc.h raw.h tgt.h pi.h
ppix_manager.o: tcd.h t1.h dwt.h t2.h mct.h int.h fix.h cidx_manager.h
ppix_manager.o: indexbox_manager.h
thix_manager.o: opj_includes.h openjpeg.h j2k_lib.h opj_malloc.h event.h
thix_manager.o: bio.h cio.h image.h j2k.h jp2.h jpt.h mqc.h raw.h tgt.h pi.h
thix_manager.o: tcd.h t1.h dwt.h t2.h mct.h int.h fix.h cidx_manager.h
thix_manager.o: indexbox_manager.h
tpix_manager.o: opj_includes.h openjpeg.h j2k_lib.h opj_malloc.h event.h
tpix_manager.o: bio.h cio.h image.h j2k.h jp2.h jpt.h mqc.h raw.h tgt.h pi.h
tpix_manager.o: tcd.h t1.h dwt.h t2.h mct.h int.h fix.h cidx_manager.h
tpix_manager.o: indexbox_manager.h
