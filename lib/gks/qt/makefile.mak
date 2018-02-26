 BINDIR = $(DESTDIR)$(GRDIR)/bin
TMP_QMAKE ?= $(shell which qmake 2>/dev/null || which qmake-qt5 2>/dev/null || which qmake-qt4 2>/dev/null || echo '')
ifneq ($(QT4_QMAKE),)
ifneq ($(QT4_QMAKE),false)
  TMP_QMAKE := $(QT4_QMAKE)
endif
endif
ifneq ($(QT5_QMAKE),)
ifneq ($(QT5_QMAKE),false)
  TMP_QMAKE := $(QT5_QMAKE)
endif
endif
QMAKE ?= $(TMP_QMAKE)

default:
	@if [ "$(QMAKE)" != "" ]; then $(QMAKE) -o QMakefile; \
	make -f QMakefile; fi

install:
	@if [ -f gksqt ]; then cp -p gksqt $(BINDIR); fi

clean:
	@if [ -f QMakefile ]; then make -f QMakefile distclean; \
	rm -f QMakefile; fi
