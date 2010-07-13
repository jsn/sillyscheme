CFLAGS = -Wall # -O3 -fomit-frame-pointer
OBJS = util.o

all: main

main: main.c $(OBJS)
	$(CC) $(CFLAGS) -o $@ $< $(OBJS)

clean:
	rm  -f $(OBJS) main
