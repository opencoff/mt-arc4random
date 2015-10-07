# mt-arc4random
This is a thread-Aware, thread-safe version of OpenBSD arc4random (chacha20 based).
I cleaned it up and made it portable to all POSIX like OSes. It
builds cleanly on OS X, Linux and all BSDs.


## How is it Thread Aware?
First, I converted every internal function to accept a context
variable and prevented them from assuming any global state.

Next, I stashed a per-thread context using the pthread TLS API
`pthread_getspecific()` and `pthread_setspecific()`.

Lastly, I rewrote the publicly visible functions to fetch the
context before calling any internal functions.

## How do you provide entropy (seed) in a portable way?
This implementation uses an "external" function named
`sys_entropy()`. I have provided an implementation for POSIX systems
via use of `/dev/random`.

## Building and Using this
Using this is very simple:

* Add the two files `arc4random.c` and `posix_entropy.c` to your
  build system.

* Use the well-known OpenBSD APIs - `arc4random()` and
  `arc4random_buf()` as you always do.


## How is it licensed?
I don't have any special licensing terms; my changes are subject to
the original licensing terms in the file `arc4random.c`.

--
Sudhi Herle <sudhi@herle.net>
Wed Oct  7 15:07:53 PDT 2015
