# Retro

A numerical matrix environment with low resource requirements

The project started from the 1993 program Euler version 3.04 that run,
at the time, on an Atari ST (I used it a long long time ago!), but supported an X11 interface.

Dr Grothmann has developped Euler until now and made it become

>	[Euler Math Toolbox](http://euler.rene-grothmann.de/)

You probably should get it if you want a full feature up-to-date
version.

For now, a new makefile, and here it just runs on Linux this old
piece of code.

The goal is to fix the bugs that version had and to backport some
of the features of subsequent versions of Euler, but try to get 
it as low resource hungry as possible, and why not, make it run on
a microcontroller.

## Install (Linux)

    make
    ./xretro

## Documentation

* [Changelog](doc/update.md)
* Retro [manual](doc/retro.md)
* [Examples](doc/examples.md)
* [Extending](doc/extend.md) retro with your own functions
* for the [insiders](doc/insider.md)
