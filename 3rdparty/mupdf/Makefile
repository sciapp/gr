ifeq ($(strip $(PREFIX)),)
override PREFIX = $(abspath $(CURDIR)/../build)
endif

VERSION = 1.6
MUPDF_MAKE_FLAGS := prefix=$(PREFIX) HAVE_MUJS=no HAVE_CURL=no XCFLAGS=-fPIC
ifeq ($(shell uname), Darwin)
MUPDF_MAKE_FLAGS += HAVE_X11=no
endif
MUPDF_EXTRA_MAKE_FLAGS ?=
MUPDF_MAKE_FLAGS += $(MUPDF_EXTRA_MAKE_FLAGS)

ifeq ($(DOWNLOAD_CMD),)
ifneq ($(shell curl --version 2>/dev/null),)
DOWNLOAD_CMD := curl -k -OL
endif
endif
ifeq ($(DOWNLOAD_CMD),)
ifneq ($(shell wget --version 2>/dev/null),)
DOWNLOAD_CMD := wget --no-check-certificate
endif
endif
ifeq ($(DOWNLOAD_CMD),)
$(error Unable to find curl or wget)
endif

default: install

$(PREFIX)/src/mupdf-$(VERSION)-source.tar.gz:
	mkdir -p $(PREFIX)/src
	cd $(PREFIX)/src/ && $(DOWNLOAD_CMD) https://gr-framework.org/downloads/3rdparty/mupdf-$(VERSION)-source.tar.gz

$(PREFIX)/src/mupdf-$(VERSION)-source/Makefile: $(PREFIX)/src/mupdf-$(VERSION)-source.tar.gz
	cd $(PREFIX)/src/ && tar -xf mupdf-$(VERSION)-source.tar.gz
	cd $(PREFIX)/src/ && patch -p0 < $(CURDIR)/mupdf.patch
	touch $@

$(PREFIX)/lib/libmupdf.a: $(PREFIX)/src/mupdf-$(VERSION)-source/Makefile
	make -C $(PREFIX)/src/mupdf-$(VERSION)-source -j4 generate CC=cc CXX=c++ AR=ar OS=""
	make -C $(PREFIX)/src/mupdf-$(VERSION)-source -j4 $(MUPDF_MAKE_FLAGS)
	make -C $(PREFIX)/src/mupdf-$(VERSION)-source install $(MUPDF_MAKE_FLAGS)

install: $(PREFIX)/lib/libmupdf.a

.PHONY: default install
