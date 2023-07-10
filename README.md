# Supernice

Set the I/O-scheduling class, priority and niceness to relaxed values when launching a process.

This was originally a fork of `ionice` from `util-linux`.

Running `supernice` is the equivalent of running both `ionice -c3` and `nice -n20`.

This can be useful to prevent processes from hogging the CPU and/or the I/O.

If you have ever watched the mouse cursor freeze when compiling a large project, this utility is for you.


## Example use

    supernice make


## Requirements

* Linux
* `make`
* A working C compiler, like `gcc`


## Installation

Build

    make

Install (use `sudo`, `doas` or run as root):
    
    sudo make install


## License

The original source code of `ionice` is dual-licenced under either GPL2:

```
Copyright (C) 2004 Jens Axboe <jens@axboe.dk>
Released under the terms of the GNU General Public License version 2
```

Or under `No copyright is claimed. This code is in the public domain; do with it what you wish`:

```
Copyright (C) 2010 Karel Zak <kzak@redhat.com>
Copyright (C) 2010 Davidlohr Bueso <dave@gnu.org>
No copyright is claimed. This code is in the public domain; do with it what you wish.
```

I wish to relicense all code under these terms as GPL2.

The changes made in 2021 and beyond are:

```
Copyright (C) 2021-2022 Alexander F. Rødseth <xyproto@archlinux.org>
Released under the terms of the GNU General Public License version 2
```

The entire code of `supernice` is now released under GPL2.

The full license text is in the `COPYING` file.


## General info

* Version: 1.0.0
* License: GPL2
