# sgbust
Optimized multi-threaded SameGame solver in C++

[![license](https://img.shields.io/github/license/chausner/sgbust.svg)](https://github.com/chausner/sgbust/blob/master/LICENSE)

sgbust is a command-line tool that uses [beam search](https://en.wikipedia.org/wiki/Beam_search) to find good solutions for [SameGame](https://en.wikipedia.org/wiki/SameGame) puzzles.

## Features

* Generate random grids
* Find good solutions for grids
* Print grids and visualize each step of a solution
* Benchmarking and solver statistics

## Building

To build sgbust, a C++20-compliant compiler is required.
sgbust is tested with recent versions of MSVC, gcc and Clang.
As of writing, Clang is only supported when libstdc++ is used as STL implementation
since libc++ currently does not implement Parallel STL algorithms that sgbust relies on for multi-threading.

[vcpkg](https://github.com/microsoft/vcpkg) is recommended for installing third-party library dependencies.
Follow the [quick start guide](https://github.com/microsoft/vcpkg#quick-start-windows) to setup vcpkg before building sgbust.

You may either build via CMake (supported on all platforms) or MSBuild (Windows-only).

## Usage

### Generate grids

Use the `generate` command to generate a random grid using the specified width, height, number of colors and minimum group size,
and save it to the specified path as a BGF file:

```
.\sgbust generate sample.bgf --width 15 --height 15 --num-colors 4 --min-group-size 2
```

You may optionally pass a custom randomization seed using `--seed` to generate grids deterministically.
If no seed is set, a random seed is used.

### Solve grids

Use the `solve` command to search for good solutions to the grid saved at the specified path:

```
.\sgbust solve sample.bgf
```

By default, the application attempts to search the complete game tree
and is guaranteed to find the best possible solution.
However, this is only feasible for very small grids.
It is usually necessary to limit the search space using the `--max-db-size` option
as otherwise the application may quickly use up all available memory:

```
.\sgbust solve sample.bgf --max-db-size 10000000
```

The number following the `--max-db-size` parameter specifies the beam width during beam search,
i.e. the maximum number of grid candidates kept in memory.

It is possible to start the search at an intermediate state by specifying a partial solution string using `--prefix`, e.g.:

```
.\sgbust solve sample.bgf --prefix "XQW(AA)KK"
```

This will search for a good solution beginning with "XQW(AA)KK".

There a couple of advanced options that can be useful in certain cases.
Run `.\sgbust solve --help` for more information.

### Show grids

Use the `show` command to display the grid saved in the specified BGF file:

```
.\sgbust show sample.bgf
```

If a solution string is passed using the optional `--solution` parameter,
each step in the solution is printed out with the resulting intermediate state of the grid.

## Run benchmarks

Use the `benchmark` command to generate an arbitrary number of random grids and solve them using specified parameters.
Not only can the command be used to benchmark the performance of the solver itself,
it also outputs statistics such as percentage of grids that could be fully cleared,
the average score and the average number of remaining blocks.

Most of the parameters accepted by the `generate` and `solve` commands are supported, e.g.:

```
.\sgbust benchmark --width 15 --height 15 --num-colors 4 --min-group-size 2 --scoring-group-score n^2-n --max-db-size 10000 --num-grids 1000
```

This will generate 1000 grids with the specified dimensions and parameters and solve them.
