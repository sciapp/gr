GRDIR = /usr/local/gr
INCLUDES = -I..
CFLAGS = $(INCLUDES) -std=c89 -Wall -Wextra -Wpedantic -Wno-unused-parameter -g
LIBS = -L$(GRDIR)/lib -lGR -lm
LDFLAGS = $(LIBS) -Wl,-rpath,$(GRDIR)/lib


all: hold_append event_handling plot multi_plot subplots receiver sender custom_receiver custom_sender merge_args

hold_append: hold_append.o
	$(CC) -o $@ $^ $(LDFLAGS)

event_handling: event_handling.o
	$(CC) -o $@ $^ $(LDFLAGS)

plot: plot.o
	$(CC) -o $@ $^ $(LDFLAGS)

multi_plot: multi_plot.o
	$(CC) -o $@ $^ $(LDFLAGS)

subplots: subplots.o
	$(CC) -o $@ $^ $(LDFLAGS)

receiver: receiver.o
	$(CC) -o $@ $^ $(LDFLAGS)

sender: sender.o
	$(CC) -o $@ $^ $(LDFLAGS)

custom_receiver: custom_receiver.o
	$(CC) -o $@ $^ $(LDFLAGS)

custom_sender: custom_sender.o
	$(CC) -o $@ $^ $(LDFLAGS)

merge_args: merge_args.o
	$(CC) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) -c $(CFLAGS) $^

clean:
	rm -f event_handling plot multi_plot subplots receiver sender custom_receiver custom_sender merge_args \
	      custom_sender.out *.o *.a *.so

.PHONY: all clean
