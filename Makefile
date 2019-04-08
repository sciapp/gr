      GRDIR = /usr/local/gr
     CONFIG = xft=no
       DIRS = lib/gr lib/gr3
ALL_DISTROS = centos centos6 debian suse
ifeq ($(DISTROS),all)
	override DISTROS = $(ALL_DISTROS)
endif

UNAME := $(shell uname)

default: pre-check Makedefs

pre-check:
	@lib/Precheck "${GRDIR}"  || \
	( echo "FATAL: Source and target directory are identical"; exit 1 )
	$(MAKE) `uname`

Makedefs:
	@lib/Preflight $(CONFIG) >Makedefs

Linux: all
Darwin: all

all:
	@for d in $(DIRS); do $(MAKE) -C $$d GRDIR=$(GRDIR); done
ifeq ($(UNAME), Darwin)
	(env CC=cc xcodebuild -project lib/gks/quartz/GKSTerm.xcodeproj)
endif

install: default
	@for d in $(DIRS); do $(MAKE) -C $$d GRDIR=$(GRDIR) install; done
ifeq ($(UNAME), Darwin)
	@if [ ! -d $(DESTDIR)$(GRDIR)/Applications ]; then \
	mkdir -m 755 $(DESTDIR)$(GRDIR)/Applications; fi
	@ditto lib/gks/quartz/build/Release/GKSTerm.app \
	$(DESTDIR)$(GRDIR)/Applications/GKSTerm.app
	@ditto lib/gks/qt/gksqt.app \
	$(DESTDIR)$(GRDIR)/Applications/gksqt.app
endif

clean:
	rm -f Makedefs
	@for d in $(DIRS) 3rdparty; do $(MAKE) -C $$d clean; done
ifeq ($(UNAME), Darwin)
	(env CC=cc xcodebuild -project lib/gks/quartz/GKSTerm.xcodeproj clean)
endif
	cp -p lib/gks/quartz/project.pbxproj lib/gks/quartz/GKSTerm.xcodeproj/
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
else
	@find . -type f -regextype posix-extended -regex '.*\.(c|cpp|cxx|m|h|hpp|hxx)' ! -path './3rdparty/*' -exec clang-format -i -verbose -style=file {} \;
endif


.PHONY: default pre-check Linux Darwin all install clean realclean self osxpkg code-format
