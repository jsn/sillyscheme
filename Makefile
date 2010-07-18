CFLAGS = -Wall # -O3 -fomit-frame-pointer
OBJS =  scanner.o value.o parse.o env.o eval.o builtin.o memory.o
HEADERS = scanner.h scheme.h
LIBS = -lfl

all: main runtests

main: main.c $(OBJS)
	$(CC) $(CFLAGS) -o $@ $< $(OBJS) $(LIBS)

runtests: runtests.c $(OBJS)
	$(CC) $(CFLAGS) -o $@ $< $(OBJS) $(LIBS)

scanner.c scanner.h: scanner.l

$(OBJS): $(HEADERS)

clean:
	rm  -f $(OBJS) main runtests scanner.[ch] 
