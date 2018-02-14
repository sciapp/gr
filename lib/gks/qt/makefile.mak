 BINDIR = $(DESTDIR)$(GRDIR)/bin
  QMAKE ?= $(shell which qmake 2>/dev/null || which qmake-qt5 2>/dev/null || which qmake-qt4 2>/dev/null || echo '')

default:
	@if [ "$(QMAKE)" != "" ]; then $(QMAKE) -o QMakefile; \
	make -f QMakefile; fi

install:
	@if [ -f gksqt ]; then cp -p gksqt $(BINDIR); fi

clean:
	@if [ -f QMakefile ]; then make -f QMakefile distclean; \
	rm -f QMakefile; fi
