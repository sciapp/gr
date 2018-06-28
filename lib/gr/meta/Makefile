GRDIR = /usr/local/gr
INCLUDES = -I..
CFLAGS = $(INCLUDES) -std=c89 -Wall -Wextra -Wpedantic -Wno-unused-parameter -g
LIBS = -L$(GRDIR)/lib -lGR -lm
LDFLAGS = $(LIBS) -Wl,-rpath,$(GRDIR)/lib


all: plot receiver sender

plot: plot.o
	$(CC) -o $@ $^ $(LDFLAGS)

receiver: receiver.o
	$(CC) -o $@ $^ $(LDFLAGS)

sender: sender.o
	$(CC) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) -c $(CFLAGS) $^

clean:
	rm -f plot receiver sender *.o *.a *.so

.PHONY: all clean
