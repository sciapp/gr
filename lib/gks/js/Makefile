  JSC = /System/Library/Frameworks/JavaScriptCore.framework/Versions/Current/Resources/jsc
  EMCC = emcc

default: demo.js
	$(JSC) demo.js >demo.pdf

demo.js: demo.c
	emcc -DEMSCRIPTEN -D__SVR4 -DGRDIR=\"\" \
	--embed-file fonts/gksfont.dat demo.c -o demo.js \
	gks.c font.c afm.c util.c malloc.c error.c wiss.c js.c io.c

clean:
	rm -f demo.js demo.pdf
