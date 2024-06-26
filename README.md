# sgbust
Optimized multi-threaded SameGame solver in C++

[![license](https://img.shields.io/github/license/chausner/sgbust.svg)](https://github.com/chausner/sgbust/blob/master/LICENSE)

sgbust is a command-line tool that uses [beam search](https://en.wikipedia.org/wiki/Beam_search) to find good solutions for [SameGame](https://en.wikipedia.org/wiki/SameGame) puzzles.

## Building

To build sgbust, a C++20-compliant compiler is required.
sgbust is tested with recent versions of MSVC, gcc and Clang.
As of writing, Clang is only supported when libstdc++ is used as STL implementation.
libc++ is not supported.

[vcpkg](https://github.com/microsoft/vcpkg) is recommended for installing third-party library dependencies.
Follow [these instructions](https://github.com/microsoft/vcpkg?tab=readme-ov-file#getting-started) to setup vcpkg before building sgbust.

You may build either via CMake (supported on all platforms) or via MSBuild (Windows-only).

## Usage

### Generating grids

Use the `generate` command to generate a random grid using the specified width, height, number of colors and minimum group size,
and save it to the specified path as a BGF file:

```
.\sgbust generate sample.bgf --width 15 --height 15 --num-colors 4 --min-group-size 2
```

You may optionally pass a custom randomization seed using `--seed` to generate grids deterministically.
If no seed is set, a random seed is used.

### Solving grids

Use the `solve` command to search for a good solution of the grid saved at the specified path:

```
.\sgbust solve sample.bgf --scoring greedy --scoring-group-score n^2-n
```

Parameters `--scoring` and `--scoring-group-score` define the optimization objective for the search algorithm
and are further described below.

#### Solution strings

A solution is a sequence of groups that are removed from a grid until no group remains.
Solutions are represented as compact strings using the following encoding scheme:

* The group in each step of the sequence is represented by a numeric ID,
  where the first group in a grid (when iterating column-first from top to bottom) is assigned the ID 0, the second the ID 1, etc.
* Each ID is encoded in base 26 using digits 'A'..'Z'
* The encoded IDs are concatenated. IDs with more than one digit are enclosed in parentheses.

For example, the solution "(AD)(AA)GPBA" denotes the sequence consisting of the 30th, 27th, 7th, 16th, 2nd and 1st group.

#### Configuring the optimization objective

Since SameGame implementations exist with a variety of different scoring rules and game objectives,
sgbust requires the user to specify what to optimize for.
The following scoring schemes are currently implemented and can be chosen via the `--scoring` parameter:

* `greedy` (default)
  * A simple scoring scheme where game states are evaluated based on the current game score which is optimized greedily.
* `potential`
  * A more advanced variant of `greedy` where not only the current game score is optimized but also potential future scoring opportunities are taken into account.
  * This scheme is recommended for maximization of the final game score.
* `num-blocks-not-in-groups`
  * Game states are evaluated based on how many blocks in the grid are not part of any group.
    By minimizing this number, the algorithm favors solutions that allow as many blocks to be removed as possible.
  * Recommended if you want to minimize the number of blocks remaining at the end but do not care about the final game score or the number of steps in the solution.

#### Configuring scoring rules

The following parameters can be used to define the scoring rules:

* `--scoring-group-score`: the score of a group, as a polynomial function of the group size
* `--scoring-clearance-bonus`: bonus added to the final game score if no blocks remain (defaults to zero if not specified)
* `--scoring-leftover-penalty`: penalty subtracted from the final game score if blocks remain, as a polynomial function of the number of blocks remaining (defaults to zero if not specified)

Examples of polynomial expressions are `n^2`, `2n+1` or `2n^3+2n^2-3n+2`.
Only integer coefficients are supported.

Note that, depending on the selected optimization objective (`greedy`/`potential`/`num-blocks-not-in-groups`), some of the parameters may be mandatory, optional or not supported at all.

#### Limiting the search space

By default, the application attempts to search the complete game tree
and is thereby guaranteed to find the best possible solution.
However, this is only feasible for very small grids.
It is usually necessary to limit the search space using the `--max-beam-size` option
as otherwise the application would quickly use up all available memory:

```
.\sgbust solve sample.bgf --max-beam-size 10000000
```

The number following the `--max-beam-size` parameter specifies the beam width during beam search,
i.e. the maximum number of grid candidates kept in memory.

#### Starting at a partial solution

It is possible to start the search at an intermediate state by specifying a partial solution string using `--prefix`, e.g.:

```
.\sgbust solve sample.bgf --prefix "XQW(AA)KK"
```

This will search for a good solution beginning with "XQW(AA)KK".

#### Advanced options

There a couple of other advanced options that can be useful in certain cases.
Run `.\sgbust solve --help` for more information.

### Displaying grids

Use the `show` command to display the grid saved in the specified BGF file:

```
.\sgbust show sample.bgf
```

If a solution string is passed using the optional `--solution` parameter,
each step in the solution is printed out with the resulting intermediate state of the grid, e.g.:

```
.\sgbust show sample.bgf --solution "(AD)ZP(AG)(AH)(AB)(AG)(AD)(AA)ZPPLJFHBPOSPBHHJIAAFAADCAAAAA"
```

### Running benchmarks

Use the `benchmark` command to generate an arbitrary number of random grids and solve them using specified parameters.
Not only can the command be used to benchmark the performance of the solver itself,
it also outputs statistics such as the percentage of grids that could be fully cleared,
the average score and the average number of remaining blocks.

Most of the parameters accepted by the `generate` and `solve` commands are supported, e.g.:

```
.\sgbust benchmark --width 15 --height 15 --num-colors 4 --min-group-size 2 --scoring-group-score n^2-n --max-beam-size 10000 --num-grids 1000
```

This will generate 1000 grids with the specified dimensions and parameters and solve them.
