 TARGETS = freetype jpeg libpng16 zlib
  EXTRAS = mupdf ogg theora vpx ffmpeg cmake glfw zeromq openjp2
     DIR = 

default:
	@for d in $(TARGETS); do make -C $$d; done

extras:
	@for d in $(EXTRAS); do ( cd $$d && ./build.sh $(DIR) ); done

clean:
	@for d in $(TARGETS); do make -C $$d clean; done

realclean: clean
	rm -rf ./build
