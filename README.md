# Linux system monitor

Based on the System Monitor Project in the Object Oriented Programming Course of
the [Udacity C++ Nanodegree
Program](https://www.udacity.com/course/c-plus-plus-nanodegree--nd213), with a
lot of refactorings and improvements.

## Dependencies

For display output, the [ncurses](https://www.gnu.org/software/ncurses/) library
is used. You can install it with `sudo apt install libncurses5-dev
libncursesw5-dev`. 

## Building and running the project

For building the project, both Make and [CMake](https://cmake.org/) is
necessary. If everything is set up, you can build with `make build`, and run the
resulting executable `./build/monitor`.
