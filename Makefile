CFLAGS = -Wall # -O3 -fomit-frame-pointer
OBJS =  scanner.o value.o util.o parse.o
LIBS = -lfl

all: main

main: main.c $(OBJS)
	$(CC) $(CFLAGS) -o $@ $< $(OBJS) $(LIBS)

scanner.c scanner.h: scanner.l

clean:
	rm  -f $(OBJS) main scanner.[ch] 
