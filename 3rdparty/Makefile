 TARGETS = jpeg libpng16 zlib
  EXTRAS = mupdf ffmpeg cmake glfw zeromq openjp2 theora ogg yasm vpx
     DIR = 

default:
	@for d in $(TARGETS); do make -C $$d; done

extras:
	@for d in $(EXTRAS); do ( cd $$d && ./build.sh $(DIR) ); done

clean:
	@for d in $(TARGETS); do make -C $$d clean; done

realclean: clean
	rm -rf ./build
