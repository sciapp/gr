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
#	xcodebuild -project lib/gks/quartz/GKSTerm.xcodeproj ARCHS=i386
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

clean:
	rm -f Makedefs
	@for d in $(DIRS); do make -C $$d clean; done
ifeq ($(UNAME), Darwin)
	xcodebuild -project lib/gks/quartz/GKSTerm.xcodeproj clean
endif
