ifeq ($(strip $(PREFIX)),)
override PREFIX = $(abspath $(CURDIR)/../build)
endif

VERSION = 1.14.6
CAIRO_EXTRA_CONFIGURE_FLAGS ?=
ifeq ($(shell uname),Darwin)
CAIRO_EXTRA_CONFIGURE_FLAGS += CFLAGS=-mmacosx-version-min=10.11
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

$(PREFIX)/src/cairo-$(VERSION).tar.xz:
	mkdir -p $(PREFIX)/src
	cd $(PREFIX)/src/ && $(DOWNLOAD_CMD) https://gr-framework.org/downloads/3rdparty/cairo-$(VERSION).tar.xz

$(PREFIX)/src/cairo-$(VERSION)/configure: $(PREFIX)/src/cairo-$(VERSION).tar.xz
	cd $(PREFIX)/src/ && tar -xf cairo-$(VERSION).tar.xz
	touch $@

$(PREFIX)/src/cairo-$(VERSION)/Makefile: $(PREFIX)/src/cairo-$(VERSION)/configure $(PREFIX)/lib/libz.a $(PREFIX)/lib/libpng.a $(PREFIX)/lib/libfreetype.a $(PREFIX)/lib/libpixman-1.a
	cd $(PREFIX)/src/cairo-$(VERSION) && \
	export CFLAGS=-O && \
	export LIBS=$(PREFIX)/lib/libz.a && \
	export png_REQUIRES=libpng16 && \
	export png_CFLAGS=-I$(PREFIX)/include && \
	export png_LIBS=$(PREFIX)/lib/libpng.a && \
	export png_NONPKGCONFIG_CFLAGS=-I$(PREFIX)/include && \
	export png_NONPKGCONFIG_LIBS=$(PREFIX)/lib/libpng.a && \
	export pixman_CFLAGS=-I$(PREFIX)/include/pixman-1 && \
	export pixman_LIBS=$(PREFIX)/lib/libpixman-1.a && \
	export FREETYPE_CFLAGS=-I$(PREFIX)/include/ && \
	export FREETYPE_LIBS=$(PREFIX)/lib/libfreetype.a && \
	export ARFLAGS="rcs" && \
	./configure \
		--prefix=$(PREFIX) \
		--libdir=$(PREFIX)/lib \
		--enable-static \
		--disable-shared \
		--with-pic \
		--disable-dependency-tracking \
		--disable-quartz \
		--enable-ft \
		--disable-fc \
		--disable-ps \
		--disable-pdf \
		--disable-svg \
		--disable-script \
		--disable-trace \
		--disable-interpreter \
		--disable-valgrind \
		--without-x \
		--disable-xlib \
		--disable-xlib-xrender \
		--disable-xcb \
		--disable-xlib-xcb \
		$(CAIRO_EXTRA_CONFIGURE_FLAGS)
	# Do not build tests or demos
	cp dummy.Makefile $(PREFIX)/src/cairo-$(VERSION)/test/Makefile
	cp dummy.Makefile $(PREFIX)/src/cairo-$(VERSION)/perf/Makefile
	# configure doesn't update the Makefile mtime correctly
	touch $@

$(PREFIX)/lib/libcairo.a: $(PREFIX)/src/cairo-$(VERSION)/Makefile
	export CFLAGS=-O && \
	export LIBS=$(PREFIX)/lib/libz.a && \
	export png_REQUIRES=libpng16 && \
	export png_CFLAGS=-I$(PREFIX)/include && \
	export png_LIBS=$(PREFIX)/lib/libpng.a && \
	export png_NONPKGCONFIG_CFLAGS=-I$(PREFIX)/include && \
	export png_NONPKGCONFIG_LIBS=$(PREFIX)/lib/libpng.a && \
	export pixman_CFLAGS=-I$(PREFIX)/include/pixman-1 && \
	export pixman_LIBS=$(PREFIX)/lib/libpixman-1.a && \
	export FREETYPE_CFLAGS=-I$(PREFIX)/include/ && \
	export FREETYPE_LIBS=$(PREFIX)/lib/libfreetype.a && \
	$(MAKE) -C $(PREFIX)/src/cairo-$(VERSION) -j4 && \
	$(MAKE) -C $(PREFIX)/src/cairo-$(VERSION) install
	# Fix broken libcairo static library issues on some systems (e.g. Darwin)
	mkdir -p $(PREFIX)/build/cairo-$(VERSION)
	cd $(PREFIX)/build/cairo-$(VERSION) && $(AR) x $(PREFIX)/lib/libcairo.a
	cd $(PREFIX)/build/cairo-$(VERSION) && $(AR) crv libcairo.a *.o
	mv $(PREFIX)/build/cairo-$(VERSION)/libcairo.a $(PREFIX)/lib/libcairo.a

install: $(PREFIX)/lib/libcairo.a

$(PREFIX)/lib/libz.a:
	$(MAKE) -C ../zlib PREFIX="$(PREFIX)" install

$(PREFIX)/lib/libpng.a:
	$(MAKE) -C ../libpng16 PREFIX="$(PREFIX)" install

$(PREFIX)/lib/libfreetype.a:
	$(MAKE) -C ../freetype PREFIX="$(PREFIX)" install

$(PREFIX)/lib/libpixman-1.a:
	$(MAKE) -C ../pixman PREFIX="$(PREFIX)" install

.PHONY: default install
