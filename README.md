AMBLAQ2 - Another Multithreaded Bounded Lockfree Atomic Queue (Version 2)
=========================================================================
By Richard Maxwell

This is a collection of single header style, C11, strongly typed, 
compile time sized, multithreaded, bounded, mostly lockfree, atomic queue fifos.

* MPMC: Multiple producers, multiple consumers
* SPMC: Single producer, multiple consumers
* MPSC: Multiple producers, single consumer
* SPSC: single producer, single consumers

based on the work by Dmitry Vyukov.

(http://www.1024cores.net/home/lock-free-algorithms/queues/bounded-mpmc-queue)

This is a rework of [AMBLAQ](https://github.com/JodiTheTigger/amblaq) so that:

* The queue size if fixed at compile time
* Queue data can now be put on the stack
* Sepeate files per queue type as opposed to a macro heavy single file
* Remove macros in favour of the user using text editor search and replace to
  modify the code for their liking

[![Build Status](https://travis-ci.org/JodiTheTigger/amblaq2.svg?branch=master)](https://travis-ci.org/JodiTheTigger/amblaq2)

Example Usage
-------------

Using the default file, function and data type names.

```c
// TODO
```

### Example as a header + c file

my_struct.h
```c
// TODO
```

my_queue.h:
```c
// TODO
```

my_queue.c:
```c
// TODO
```

Status
------
* In development

Development Notes
-----------------
After AMBLAQ I addressed my concerns with a run time queue size by makeing it
compiler time, and the macro heavyness to removing all the macros :-)

I still wanted queue type safety, so I make the onus on the user to change
the data type and function names to their liking instead, making it easier
for the user to search and replace the code to do that.

Still have the problem that with using C11, windows MSVC will not build it as 
it doesn't even support C99, let alone C11. I'll need to change the code to make
it complie as C++11, and then port the test code to use a C11->C++11 thread 
interface.

License
-------
The code is licesned under the MIT License. See [LICENSE].
