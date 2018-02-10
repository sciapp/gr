 BINDIR = $(DESTDIR)$(GRDIR)/bin
  QMAKE =

ifneq ($(QT5_QMAKE),false)
  QMAKE = $(QT5_QMAKE)
endif

default:
	@if [ "$(QMAKE)" != "" ]; then $(QMAKE) -o QMakefile; \
	make -f Qmakefile; fi

install:
	@if [ -f gksqt ]; then cp -p gksqt $(BINDIR); fi

clean:
	@if [ -f QMakefile ]; then make -f QMakefile distclean; \
	rm -f QMakefile; fi
