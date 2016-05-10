# mt-arc4random
This is a thread-Aware, thread-safe version of OpenBSD arc4random (chacha20 based).
I cleaned it up and made it portable to all POSIX like OSes. It
builds cleanly on OS X, Linux and OpenBSD. In theory, it should just
work on any modern Unix.


## How is it Thread Aware?
First, I converted every internal function to accept a context
variable and prevented them from assuming any global state.

Next, I stashed a per-thread context using the pthread TLS API
`pthread_getspecific()` and `pthread_setspecific()`.

Lastly, I rewrote the publicly visible functions to fetch the
context before calling any internal functions.

## How do you provide entropy (seed) in a portable way?
This implementation uses an "external" function named
`getentropy()`. On OpenBSD, this is a syscall. 
I have provided an implementation of `getentropy()` for POSIX systems
via `/dev/urandom`.

## Building and Using this
Using this is very simple:

* If you are on OpenBSD:

   - Include the supplied `arc4random.h` in every place where you expect
     to call `arc4random()`. This is *mandatory* - otherwise, you
     will have a fun time debugging crashes.

   - Add `arc4random.c` to your build.

* If you are any Unix like platform:

    - Add the two files `arc4random.c` and `posix_entropy.c` to your
      build system.

   - Include the supplied `arc4random.h` in every place where you expect
     to call `arc4random()`

* Use the well-known APIs - `arc4random()` and
  `arc4random_buf()` as you always do.

## Testing and Performance

There's a small benchmark program called `t_arcrand`; to build it
just run `make`. It should work on any modern Unix. Tested on
OpenBSD, Linux, OS X Darwin.

When run on a retina MacBook Pro 13” (2013) running OS X Yosemite:

    ./t_arc4rand 16 32 64 256 512
    size,      arc4rand,	sysrand,	speed-up
        16,   12.2966,	 	279.9628,	 22.77
        32,   11.3687,	 	268.2029,	 23.59
        64,    9.9161,	 	238.8743,	 24.09
       256,    9.3164,	 	217.1194,	 23.30
       512,    8.3054,	 	204.1270,	 24.58


The results show the CPU cycles consumed by `arc4random_buf()` to
generate "n" bytes of random data. The third column labeled
`sysrand` is the CPU cycles consumed by reading from `/dev/urandom`.
And the last column is the relative speedup between the two.

On debian Linux (sid) x86_64 on a Core-i7 Laptop running the
Linux 4.5 kernel, I see:

    ./t_arc4rand 16 32 64 256 512
    size,      arc4rand,	sysrand,	speed-up
        16,    9.6997,	 	255.6211,	 26.35
        32,    7.9821,	 	251.2072,	 31.47
        64,    7.5916,	 	220.8430,	 29.09
       256,    7.4764,	 	206.5079,	 27.62
       512,    7.2225,	 	201.9913,	 27.97


## RFC 4122 UUID Generation
There is a short implementatin of RFC 4122
Random number based UUID generation in randuuid.c. This 
uses the underlying `arc4random()` implementation. The
signature for that function is simple enough
`void randuuid(uint8_t* buf, size_t n)`. 

### Java bindings 
The UUID generator has a JNI binding specified in the java/
directory.

## How is it licensed?
I don't have any special licensing terms; my changes are subject to
the original licensing terms in the file `arc4random.c`.

--
Sudhi Herle <sudhi@herle.net>
Wed Oct  7 15:07:53 PDT 2015
