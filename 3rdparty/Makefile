 TARGETS = jbig2dec jpeg openjpeg png zlib
  EXTRAS = mupdf ffmpeg glfw zeromq
     DIR = 

default:
	@for d in $(TARGETS); do make -C $$d; done

extras:
	@for d in $(EXTRAS); do ( cd $$d && ./build.sh $(DIR) ); done

clean:
	@for d in $(TARGETS); do make -C $$d clean; done
	rm -rf ./build
