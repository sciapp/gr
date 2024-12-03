UNAME := $(shell uname)
TMP_QMAKE ?= $(shell which qmake 2>/dev/null || which qmake6 2>/dev/null || which qmake-qt5 2>/dev/null || echo '')
ifneq ($(QT5_QMAKE),)
ifneq ($(QT5_QMAKE),false)
  TMP_QMAKE := $(QT5_QMAKE)
endif
endif
ifneq ($(QT6_QMAKE),)
ifneq ($(QT6_QMAKE),false)
  TMP_QMAKE := $(QT6_QMAKE)
endif
endif
QMAKE ?= $(TMP_QMAKE)

default: QMakefile
	@if [ "$(QMAKE)" != "" ]; then $(MAKE) -f QMakefile; fi

QMakefile:
	@if [ "$(QMAKE)" != "" ]; then $(QMAKE) -o QMakefile THIRDPARTYDIR=$(THIRDPARTYDIR); fi

install: default
ifeq ($(UNAME), Darwin)
	@if [ ! -d $(DESTDIR)$(GRDIR)/Applications ]; then \
	mkdir -m 755 $(DESTDIR)$(GRDIR)/Applications; fi
	@ditto grplot.app \
	$(DESTDIR)$(GRDIR)/Applications/grplot.app
	@if [ ! -d $(DESTDIR)$(GRDIR)/bin ]; then \
		mkdir -m 755 $(DESTDIR)$(GRDIR)/bin; fi
	@if [ ! -d $(DESTDIR)$(GRDIR)/share/doc/grplot ]; then \
		mkdir -m 755 -p $(DESTDIR)$(GRDIR)/share/doc/grplot; fi
	@ditto README.md \
    $(DESTDIR)$(GRDIR)/share/doc/grplot/grplot.man.md
else
	@if [ ! -d $(DESTDIR)$(GRDIR)/bin ]; then \
		mkdir -m 755 $(DESTDIR)$(GRDIR)/bin; fi
	cp -p grplot $(DESTDIR)$(GRDIR)/bin/
	@if [ ! -d $(DESTDIR)$(GRDIR)/share/doc/grplot ]; then \
		mkdir -m 755 -p $(DESTDIR)$(GRDIR)/share/doc/grplot; fi
	cp -p README.md $(DESTDIR)$(GRDIR)/share/doc/grplot/grplot.man.md
endif

clean:
	@if [ -f QMakefile ]; then make -f QMakefile distclean; \
	rm -f QMakefile; fi
	@if [ -f Makefile ]; then make distclean; fi

.PHONY: default install clean
