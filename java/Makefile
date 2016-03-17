# Simple makefile to help me generate the JNI header file
# This is for internal use only.

pkg     = net.herle.random.mtarc4random
pkgpath = $(subst .,/,$(pkg))
pkgdir  = $(dir $(pkgpath))
jsrc    = $(pkgpath).java
jcls    = $(patsubst %.java, %.class, $(jsrc))


platform := $(shell uname)

Darwin_INCDIRS := /System/Library/Frameworks/JavaVM.framework/Headers
Darwin_SOFLAGS := -Wl,-dylib

Linux_SOFLAGS := -Wl,-shared

all = jranduuid.h randuuid.so

vpath %.c . ..

objs = jranduuid.o randuuid.o arc4random.o posix_entropy.o

# Don't change these
INCS = $(addprefix -I, $($(platform)_INCDIRS))
CFLAGS = -Wall -O3 -g $(INCS)
SOFLAGS = $($(platform)_SOFLAGS)

all: $(all)

jranduuid.h: $(jcls)
	javah -verbose -jni -o $@ $(pkg)

$(jcls): $(jsrc)
	javac $<

randuuid.so: $(objs)
	$(CC) -o $@ $(SOFLAGS) $^

.PHONY: clean

clean:
	-rm -f $(jcls) $(all) $(objs)

#vim: noexpandtab:sw=4=ts=4:notextmode:
