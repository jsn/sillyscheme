CFLAGS = -Wall # -O3 -fomit-frame-pointer
OBJS =  scanner.o value.o util.o parse.o assoc.o env.o eval.o builtin.o
LIBS = -lfl

all: main runtests

main: main.c $(OBJS)
	$(CC) $(CFLAGS) -o $@ $< $(OBJS) $(LIBS)

runtests: runtests.c $(OBJS)
	$(CC) $(CFLAGS) -o $@ $< $(OBJS) $(LIBS)

scanner.c scanner.h: scanner.l

clean:
	rm  -f $(OBJS) main runtests scanner.[ch] 
