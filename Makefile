
platform := $(shell uname)

OpenBSD_objs = 
Linux_objs = posix_entropy.o
Darwin_objs = posix_entropy.o

objs = arc4random.o $($(platform)_objs)

bench = t_arc4rand

CC = gcc
CFLAGS = -O3 -Wall -D__$(platform)__=1 -I.


all: $(bench)

t_arc4rand: t_arc4rand.o $(objs)
	$(CC) -o $@ $^


.PHONY: clean

clean:
	-rm -f $(objs) $(bench)


