       GRDIR = /usr/local/gr
      DOCDIR = $(DESTDIR)$(GRDIR)/share/GR
      LIBDIR = $(DESTDIR)$(GRDIR)/lib
      INCDIR = $(DESTDIR)$(GRDIR)/include
PKGCONFIGDIR = $(LIBDIR)/pkgconfig
      CONFIG = xft=no

UNAME := $(shell uname)

PREFERRED_CLANG_FORMAT_VERSION="13"
ifeq ($(shell command -v "clang-format-$(PREFERRED_CLANG_FORMAT_VERSION)"),)
  CLANG_FORMAT="clang-format"
else
  CLANG_FORMAT="clang-format-$(PREFERRED_CLANG_FORMAT_VERSION)"
endif

default: all

pre-check:
	@lib/Precheck "${GRDIR}"  || \
	( echo "FATAL: Source and target directory are identical"; exit 1 )

Makedefs:
	@lib/Preflight $(CONFIG) >Makedefs

all: pre-check
	$(MAKE) -C lib/gks GRDIR=$(GRDIR)
	$(MAKE) -C lib/gr GRDIR=$(GRDIR)
	$(MAKE) -C lib/gr3 GRDIR=$(GRDIR)
	$(MAKE) -C lib/grm GRDIR=$(GRDIR)

install: default
	$(MAKE) -C lib/gks GRDIR=$(GRDIR) install
	$(MAKE) -C lib/gr GRDIR=$(GRDIR) install
	$(MAKE) -C lib/gr3 GRDIR=$(GRDIR) install
	$(MAKE) -C lib/grm GRDIR=$(GRDIR) install
	@if [ ! -d $(DESTDIR)$(GRDIR) ]; then mkdir -m 755 $(DESTDIR)$(GRDIR); fi
	@if [ ! -d $(DOCDIR) ]; then mkdir -m 755 -p $(DOCDIR); fi
	cp -p LICENSE.md $(DOCDIR)
	@if [ ! -d $(INCDIR) ]; then mkdir -m 755 $(INCDIR); fi
	@if [ ! -d $(LIBDIR) ]; then mkdir -m 755 $(LIBDIR); fi
	@if [ ! -d $(PKGCONFIGDIR) ]; then mkdir -m 755 $(PKGCONFIGDIR); fi
	@for package in gks gr gr3 grm; do PREFIX=${GRDIR} lib/configure-pkg-config $${package}.pc.in > $(PKGCONFIGDIR)/$${package}.pc; done

clean:
	rm -f Makedefs
	$(MAKE) -C lib/gks clean
	$(MAKE) -C lib/gr clean
	$(MAKE) -C lib/gr3 clean
	$(MAKE) -C lib/grm clean
	$(MAKE) -C 3rdparty clean
	rm -f gr.pkg

realclean: clean
	$(MAKE) -C 3rdparty realclean
	rm -rf build
	find packaging -type f \( -name '*.deb' -o -name '*.rpm' \) -exec rm \{\} \;
	rm -rf tmp

self:
	sh 3rdparty/makeself.sh

osxpkg:
	mkdir -p tmp/bin tmp/gr
	rsync -a /usr/local/gr/ tmp/gr/
	ln -s ../gr/bin/gr tmp/bin/gr
	ln -s ../gr/Applications/glgr.app/Contents/MacOS/glgr tmp/bin/glgr
	ln -s ../gr/Applications/gksqt.app/Contents/MacOS/gksqt tmp/bin/gksqt
	@if [ -e ../gr/Applications/grplot.app/Contents/MacOS/grplot ]; then \
	ln -s ../gr/Applications/grplot.app/Contents/MacOS/grplot tmp/bin/grplot; fi
	sudo chown -R -h root:wheel tmp/
	pkgbuild --identifier de.fz-juelich.gr --root tmp --install-location /usr/local --ownership preserve gr.pkg
	sudo rm -rf tmp

code-format:
ifeq ($(UNAME), Darwin)
	@find -E . -type f \
	           -regex '.*\.(c|cpp|cxx|m|h|hpp|hxx)' \
	         ! -path './3rdparty/*' \
	         ! -path './apps/*' \
	         ! -path './build/*' \
	           -exec "$(CLANG_FORMAT)" -i -verbose -style=file {} \;
	@CMAKE_FORMAT="$$(./.setup_cmakeformat.sh)" && \
	find -E . -type f \
	          -regex '(.*/CMakeLists\.txt)|(.*\.cmake)' \
	        ! -path './3rdparty/*' \
	        ! -path './apps/*' \
	        ! -path './build/*' \
	          -exec echo "Formatting "{} \; \
	          -exec "$${CMAKE_FORMAT}" -i {} \;
else
	@find . -type f \
	        -regextype posix-extended \
	        -regex '.*\.(c|cpp|cxx|m|h|hpp|hxx)' \
	      ! -path './3rdparty/*' \
	      ! -path './apps/*' \
	      ! -path './build/*' \
	        -exec "$(CLANG_FORMAT)" -i -verbose -style=file {} \;
	@CMAKE_FORMAT="$$(./.setup_cmakeformat.sh)" && \
	find . -type f \
	       -regextype posix-extended \
	       -regex '(.*/CMakeLists\.txt)|(.*\.cmake)' \
	     ! -path './3rdparty/*' \
	     ! -path './apps/*' \
	     ! -path './build/*' \
	       -exec echo "Formatting "{} \; \
	       -exec "$${CMAKE_FORMAT}" -i {} \;
endif


.PHONY: default pre-check all install clean realclean self osxpkg code-format
