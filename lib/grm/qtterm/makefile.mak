UNAME := $(shell uname)
TMP_QMAKE ?= $(shell which qmake6 2>/dev/null || which qmake-qt6 2>/dev/null || which qmake-qt5 2>/dev/null || which qmake5 2>/dev/null || which qmake 2>/dev/null || echo '')
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

QMAKE_VER_MAJOR := 0
QMAKE_VER_MAJOR := $(shell $(TMP_QMAKE) -query QT_VERSION | cut -f1 -d.)
QMAKE_GREATER_EQUAL_5 := $(shell [ $(QMAKE_VER_MAJOR) -gt 5 -o $(QMAKE_VER_MAJOR) -eq 5 ] && echo true)
ifeq ($(QMAKE_GREATER_EQUAL_5), true)
  QMAKE ?= $(TMP_QMAKE)
endif

QMakefile:
	@if [ "$(QMAKE)" != "" ]; then $(QMAKE) -o QMakefile; fi

default: QMakefile
	@if [ "$(QMAKE)" != "" ]; then $(MAKE) -f QMakefile; fi

install: default
ifneq ($(QMAKE),)
ifeq ($(UNAME),Darwin)
	@if [ ! -d $(DESTDIR)$(GRDIR)/Applications ]; then \
	mkdir -m 755 $(DESTDIR)$(GRDIR)/Applications; fi
	@ditto qtterm.app \
	$(DESTDIR)$(GRDIR)/Applications/qtterm.app
else
	cp -p qtterm $(DESTDIR)$(GRDIR)/bin
endif
else
    $(info Not building qtterm (No suitable qt version found))
endif

clean:
ifneq ($(QMAKE),)
	@if [ -f QMakefile ]; then make -f QMakefile distclean; \
	rm -f QMakefile; fi
	@if [ -f Makefile ]; then make distclean; fi
endif

.PHONY: default install clean
