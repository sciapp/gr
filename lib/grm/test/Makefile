GRDIR = /usr/local/gr
INCLUDES = -I$(GRDIR)/include
CFLAGS = $(INCLUDES) -std=c89 -Wall -Wextra -Wpedantic -Wno-unused-parameter -g
LIBS = -L$(GRDIR)/lib -lGRM -lm
LDFLAGS = $(LIBS) -Wl,-rpath,$(GRDIR)/lib


all: custom_receiver custom_sender event_handling hold_append imshow isosurface line_errorbar merge_args multi_plot plot receiver sender subplots

imshow: imshow.o
	$(CC) -o $@ $^ $(LDFLAGS)

isosurface: isosurface.o
	$(CC) -o $@ $^ $(LDFLAGS)

line_errorbar: line_errorbar.o
	$(CC) -o $@ $^ $(LDFLAGS)

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
	rm -f imshow isosurface line_errorbar hold_append event_handling plot multi_plot subplots receiver sender custom_receiver custom_sender merge_args \
	      *.o *.a *.so

.PHONY: all clean
