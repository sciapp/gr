      GRDIR = /usr/local/gr
     CONFIG = xft=no

UNAME := $(shell uname)

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

install: default
	$(MAKE) -C lib/gks GRDIR=$(GRDIR) install
	$(MAKE) -C lib/gr GRDIR=$(GRDIR) install
	$(MAKE) -C lib/gr3 GRDIR=$(GRDIR) install

clean:
	rm -f Makedefs
	$(MAKE) -C lib/gks clean
	$(MAKE) -C lib/gr clean
	$(MAKE) -C lib/gr3 clean
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
	sudo chown -R -h root:wheel tmp/
	pkgbuild --identifier de.fz-juelich.gr --root tmp --install-location /usr/local --ownership preserve gr.pkg
	sudo rm -rf tmp

code-format:
ifeq ($(UNAME), Darwin)
	@find -E . -type f -regex '.*\.(c|cpp|cxx|m|h|hpp|hxx)' ! -path './3rdparty/*' -exec clang-format -i -verbose -style=file {} \;
	@CMAKE_FORMAT="$$(./.setup_cmakeformat.sh)" && \
	find -E . -type f -regex '(.*/CMakeLists\.txt)|(.*\.cmake)' ! -path './3rdparty/*' \
	          -exec echo "Formatting "{} \; -exec "$${CMAKE_FORMAT}" -i {} \;
else
	@find . -type f -regextype posix-extended -regex '.*\.(c|cpp|cxx|m|h|hpp|hxx)' ! -path './3rdparty/*' -exec clang-format -i -verbose -style=file {} \;
	@CMAKE_FORMAT="$$(./.setup_cmakeformat.sh)" && \
	find . -type f -regextype posix-extended -regex '(.*/CMakeLists\.txt)|(.*\.cmake)' ! -path './3rdparty/*' \
	       -exec echo "Formatting "{} \; -exec "$${CMAKE_FORMAT}" -i {} \;
endif


.PHONY: default pre-check all install clean realclean self osxpkg code-format
