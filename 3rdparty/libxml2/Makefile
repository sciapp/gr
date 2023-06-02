ifeq ($(strip $(PREFIX)),)
override PREFIX = $(abspath $(CURDIR)/../build)
endif

VERSION = 2.10.4
LIBXML2_EXTRA_CONFIGURE_FLAGS ?=
ifeq ($(shell uname),Darwin)
LIBXML2_EXTRA_CONFIGURE_FLAGS += CFLAGS=-mmacosx-version-min=10.14
endif

ifeq ($(DOWNLOAD_CMD),)
ifneq ($(shell curl --version 2>/dev/null),)
DOWNLOAD_CMD := curl -f -k -OL
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

$(PREFIX)/src/libxml2-$(VERSION).tar.xz:
	mkdir -p $(PREFIX)/src
	cd $(PREFIX)/src/ && $(DOWNLOAD_CMD) https://gr-framework.org/downloads/3rdparty/libxml2-$(VERSION).tar.xz

$(PREFIX)/src/libxml2-$(VERSION)/configure: $(PREFIX)/src/libxml2-$(VERSION).tar.xz
	cd $(PREFIX)/src/ && tar -xf libxml2-$(VERSION).tar.xz
	touch $@

$(PREFIX)/src/libxml2-$(VERSION)/Makefile: $(PREFIX)/src/libxml2-$(VERSION)/configure
	cd $(PREFIX)/src/libxml2-$(VERSION) && \
	./configure \
		--prefix=$(PREFIX) \
		--libdir=$(PREFIX)/lib \
		--enable-static \
		--disable-shared \
		--with-pic \
		--with-zlib=$(PREFIX) \
		--without-python \
		$(LIBXML2_EXTRA_CONFIGURE_FLAGS)
	# configure doesn't update the Makefile mtime correctly
	touch $@

$(PREFIX)/lib/libxml2.a: $(PREFIX)/src/libxml2-$(VERSION)/Makefile
	$(MAKE) -C $(PREFIX)/src/libxml2-$(VERSION) -j4 && \
	$(MAKE) -C $(PREFIX)/src/libxml2-$(VERSION) install

libxml2.a: $(PREFIX)/lib/libxml2.a
	cp $(PREFIX)/lib/libxml2.a libxml2.a

clean:
	rm -f libxml2.a

install: $(PREFIX)/lib/libxml2.a

.PHONY: default clean install
