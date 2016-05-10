
platform := $(shell uname)

OpenBSD_objs = 
Linux_objs = posix_entropy.o
Darwin_objs = posix_entropy.o

objs = arc4random.o error.o $($(platform)_objs)

bench = t_arc4rand


Darwin_ldflags =
OpenBSD_ldflags = -lpthread
Linux_ldflags = -lpthread

CC = gcc
CFLAGS = -O3 -Wall -D__$(platform)__=1 -I.
LDFLAGS = $($(platform)_ldflags)

all: $(bench)

t_arc4rand: t_arc4rand.o $(objs)
	$(CC) -o $@ $^ $(LDFLAGS)


.PHONY: clean

clean:
	-rm -f $(objs) t_arc4rand.o $(bench)


