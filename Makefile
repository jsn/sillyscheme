CFLAGS = -Wall # -O3 -fomit-frame-pointer
OBJS = util.o parse.o scanner.o
LIBS = -lfl

all: main

main: main.c $(OBJS)
	$(CC) $(CFLAGS) -o $@ $< $(OBJS) $(LIBS)

scanner.c scanner.h: scanner.l

clean:
	rm  -f $(OBJS) main scanner.[ch] 
