 GRDIR = /usr/local/gr
  DIRS = lib/gr lib/gr3

UNAME := $(shell uname)

default: Makedefs
	@make `uname`

Makedefs:
	@lib/Preflight >Makedefs

Linux: all
Darwin: all

all:
	@for d in $(DIRS); do make -C $$d GRDIR=$(GRDIR); done
ifeq ($(UNAME), Darwin)
	xcodebuild -project lib/gks/quartz/GKSTerm.xcodeproj
endif

install: default
	@for d in $(DIRS); do make -C $$d GRDIR=$(GRDIR) install; done
ifeq ($(UNAME), Darwin)
	@if [ ! -d $(DESTDIR)$(GRDIR)/Applications ]; then \
	mkdir -m 755 $(DESTDIR)$(GRDIR)/Applications; fi
	@ditto lib/gks/quartz/build/Release/GKSTerm.app \
	$(DESTDIR)$(GRDIR)/Applications/GKSTerm.app 
endif
	cp -p bin/gr.sh $(DESTDIR)$(GRDIR)/bin/gr
	cp -p bin/anaconda.sh $(DESTDIR)$(GRDIR)/bin/anaconda

clean:
	rm -f Makedefs
	@for d in $(DIRS); do make -C $$d clean; done
ifeq ($(UNAME), Darwin)
	xcodebuild -project lib/gks/quartz/GKSTerm.xcodeproj clean
endif
	cp -p lib/gks/quartz/project.pbxproj lib/gks/quartz/GKSTerm.xcodeproj/
	rm -f gr.pkg

pypi: clean
	python setup.py sdist upload -r https://pypi.python.org/pypi

testpypi: clean
	python setup.py sdist upload -r https://testpypi.python.org/pypi

osxpkg:
	mkdir -p tmp/bin tmp/gr
	rsync -a /usr/local/gr/ tmp/gr/
	ln -s ../gr/bin/gr tmp/bin/gr
	ln -s ../gr/Applications/glgr.app/Contents/MacOS/glgr tmp/bin/glgr
	ln -s ../gr/Applications/gksqt.app/Contents/MacOS/gksqt tmp/bin/gksqt
	sudo chown -R -h root:wheel tmp/
	pkgbuild --identifier de.fz-juelich.gr --root tmp --install-location /usr/local --ownership preserve gr.pkg
	sudo rm -rf tmp

sphinxdoc:
	make -C doc html
	rsync -av --delete --exclude=/media doc/_build/html/ iffwww:/WebServer/Documents/gr/
	rsync -av --delete doc/media/ iffwww:/WebServer/Documents/gr/media/

mirror:
	(cd ../gr-github && git fetch && git push)
