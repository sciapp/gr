ifeq ($(strip $(PREFIX)),)
override PREFIX = $(abspath $(CURDIR)/../build)
endif

VERSION = 1.1.1
THEORA_EXTRA_CONFIGURE_FLAGS ?=
ifeq ($(shell uname),Darwin)
THEORA_EXTRA_CONFIGURE_FLAGS += CFLAGS=-mmacosx-version-min=10.11
endif

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
DOWNLOAD_CMD := echo "Error: Unable to find curl or wget."; exit 1; \#
endif

default: install

$(PREFIX)/src/libtheora-$(VERSION).tar.gz:
	mkdir -p $(PREFIX)/src
	cd $(PREFIX)/src/ && $(DOWNLOAD_CMD) https://gr-framework.org/downloads/3rdparty/libtheora-$(VERSION).tar.gz

$(PREFIX)/src/libtheora-$(VERSION)/configure: $(PREFIX)/src/libtheora-$(VERSION).tar.gz
	cd $(PREFIX)/src/ && tar -xf libtheora-$(VERSION).tar.gz
	cd $(PREFIX)/src/ && patch -p0 < $(CURDIR)/libtheora.patch
	touch $@

$(PREFIX)/src/libtheora-$(VERSION)/Makefile: $(PREFIX)/src/libtheora-$(VERSION)/configure $(PREFIX)/lib/libogg.a
	cd $(PREFIX)/src/libtheora-$(VERSION) && \
	./configure \
		--prefix=$(PREFIX) \
		--libdir=$(PREFIX)/lib \
		--enable-static \
		--disable-shared \
		--with-pic \
		--disable-dependency-tracking \
		--disable-spec \
		--disable-examples \
		--includedir=$(PREFIX)/include \
		--with-ogg-includes=$(PREFIX)/include \
		--with-ogg-libraries=$(PREFIX)/lib \
		$(THEORA_EXTRA_CONFIGURE_FLAGS)

$(PREFIX)/lib/libtheora.a: $(PREFIX)/src/libtheora-$(VERSION)/Makefile
	$(MAKE) -C $(PREFIX)/src/libtheora-$(VERSION) -j4
	$(MAKE) -C $(PREFIX)/src/libtheora-$(VERSION) install

install: $(PREFIX)/lib/libtheora.a


$(PREFIX)/lib/libogg.a:
	$(MAKE) -C ../ogg PREFIX="$(PREFIX)" install

.PHONY: default install
