# BlocCpp
SameGame solver in C++

[![license](https://img.shields.io/github/license/chausner/BlocCpp.svg)](https://github.com/chausner/BlocCpp/blob/master/LICENSE)

BlocCpp is a command-line tool that implements a [beam search](https://en.wikipedia.org/wiki/Beam_search) algorithm to find good solutions for [SameGame](https://en.wikipedia.org/wiki/SameGame) puzzles.

## Features

* Generate random grids
* Find good solutions for grids
* Print grids and visualize each step of a solution

## Usage

### Generate grids

Use the `generate` command to generate a random grid using the specified width, height, number of colors and minimum group size,
and save it to the specified path as a BGF file:

```
.\BlocCpp generate sample.bgf --width 15 --height 15 --num-colors 4 --smallest-group-size 2
```

You may optionally pass a custom randomization seed using `--seed` to generate grids deterministically.
If no seed is set, a random seed is used.

### Solve grids

Use the `solve` command to search for good solutions to the grid saved at the specified path:

```
.\BlocCpp solve sample.bgf
```

It is usually a good idea to limit the search space using the `--max-db-size` option
as otherwise the application may quickly use up all available memory:

```
.\BlocCpp solve sample.bgf --max-db-size 10000000
```

It is possible to start the search at an intermediate state by specifying a partial solution string using `--prefix`, e.g.:

```
.\BlocCpp solve sample.bgf --prefix "XQW(AA)KK"
```

This will search for a good solution beginning with "XQW(AA)KK".

There a couple of advanced options that can be useful in certain cases.
Run `.\BlocCpp solve --help` for more information.

### Show grids

Use the `show` command to display the grid saved in the specified BGF file:

```
.\BlocCpp show sample.bgf
```

If a solution string is passed using the optional `--solution` parameter,
each step in the solution is printed out with the resulting intermediate state of the grid.