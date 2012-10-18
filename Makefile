CC=gcc
CFLAGS=-Wall -O2 -g
LIBS=-ludis86

_OBJS = tracer.o elf.o elfs.o rat.o

SRCDIR = src
OBJS = $(patsubst %,$(SRCDIR)/%,$(_OBJS))

all : $(OBJS)
	$(CC) $(SRCDIR)/rnpt.c -o rnpt $(CFLAGS) $(OBJS) $(LIBS)

%.o : %.cc %.h
	$(CC) -c -o $@ $< $(CFLAGS)

%.o : %.cc
	$(CC) -c -o $@ $< $(CFLAGS)

clean :
	rm -f $(SRCDIR)/*.o
	rm -f rnpt