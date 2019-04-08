ifeq ($(strip $(PREFIX)),)
override PREFIX = $(abspath $(CURDIR)/../build)
endif

VERSION = 0.34.0
PIXMAN_EXTRA_CONFIGURE_FLAGS ?=
ifeq ($(shell uname),Darwin)
PIXMAN_EXTRA_CONFIGURE_FLAGS += CFLAGS=-mmacosx-version-min=10.11
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

$(PREFIX)/src/pixman-$(VERSION).tar.gz:
	mkdir -p $(PREFIX)/src
	cd $(PREFIX)/src/ && $(DOWNLOAD_CMD) https://gr-framework.org/downloads/3rdparty/pixman-$(VERSION).tar.gz

$(PREFIX)/src/pixman-$(VERSION)/configure: $(PREFIX)/src/pixman-$(VERSION).tar.gz
	cd $(PREFIX)/src/ && tar -xf pixman-$(VERSION).tar.gz
	touch $@

$(PREFIX)/src/pixman-$(VERSION)/Makefile: $(PREFIX)/src/pixman-$(VERSION)/configure
	cd $(PREFIX)/src/pixman-$(VERSION) && ./configure \
		--prefix=$(PREFIX) \
		--libdir=$(PREFIX)/lib \
		--disable-shared \
		--enable-static \
		--with-pic \
		--disable-dependency-tracking \
		$(PIXMAN_EXTRA_CONFIGURE_FLAGS)
	# Do not build tests or demos
	cp dummy.Makefile $(PREFIX)/src/pixman-$(VERSION)/test/Makefile
	cp dummy.Makefile $(PREFIX)/src/pixman-$(VERSION)/demos/Makefile

$(PREFIX)/lib/libpixman-1.a: $(PREFIX)/src/pixman-$(VERSION)/Makefile
	$(MAKE) -C $(PREFIX)/src/pixman-$(VERSION) -j4
	$(MAKE) -C $(PREFIX)/src/pixman-$(VERSION) install

install: $(PREFIX)/lib/libpixman-1.a

.PHONY: default install
