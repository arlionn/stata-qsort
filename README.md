qsort
=====

[Overview](#faster-sorting)
| [Installation](#installation)
| [Benchmarks](#benchmarks)
| [Building](#building)
| [FAQs](#faqs)
| [License](#license)

Proof-of-concept Stata package that provides a faster implementation of `sort`
and `gsort`. It is a wrapper for BSD's implementation of quicksort, but the
speed improvement comes at a really bad memory overhead.

`version 0.1.0 21Sep2017`

Faster Sorting
--------------

This package's aim is to provide a proof-of-concept implementation of `qsort`.
The speed improvements are specially large when sorting in descending order,
specially strings, or sorting numbers, specially integers. (Stata fails to
take advantage of a simple transformation when all the sorting variables
are integers, which would allow using a sorting algorithm that has O(n)
complaxity, while the best sorting alorithms finish in O(n log(n)) time).

Though faster, this plugin is VERY memory hungry. I am not particularly adept
in C and I don't really know how to create a mixed data type container where
each element has a different type and length. The internal implementation
works but it is very crude.

Stata uses little to no additional memory when sorting. This leads me to
believe the algorithm is being applied directly on the data in memory. If I
subtract the overhead from copying the data to and from C, the sorting portion
of the plugin is actually much faster.

Installation
------------

I only have access to Stata 13.1, so I impose that to be the minimum.
```stata
net install qsort, from(https://raw.githubusercontent.com/mcaceresb/stata-qsort/develop/build/)
* adoupdate, update
* ado uninstall qsort
```

The syntax is identical to `sort` and `gsort`, except `mfirst` and `gen` are not available.
```stata
sysuse auto
qsort foreign rep78 make
qsort -foreign rep78 -make, v b
```

Benchmarks
----------

Pending...

Building
--------

### Requirements

If you want to compile the plugin yourself, you will need
- The GNU Compiler Collection (`gcc`)
- v2.0 or above of the [Stata Plugin Interface](https://stata.com/plugins/version2) (SPI).

I keep a copy of Stata's Plugin Interface in this repository.  However,
you will have to make sure you have `gcc` installed and in your system's
`PATH`. Last, on windows, you will additionally need
- [Cygwin](https://cygwin.com) with gcc, make, libgomp, x86_64-w64-mingw32-gcc-5.4.0.exe
  (Cygwin is pretty massive by default; I would install only those packages).

### Compilation

```bash
git clone https://github.com/mcaceresb/stata-gtools
cd stata-gtools
make clean
make
```

If that is successful then run `./build/qsort_tests.do` to test the program
will work as expected. If successful, the exit message should be "tests
finished running" followed by the start and end time.

FAQs
----

### Why is this faster?

For descending order, this is faster because Stata's implementation is very
inefficient. It creates a temporary variable that, if sorted normally, would
give an equivalent result to sorting the variable in the reverse order. For
integers, I map the set of integers to the counting numbers and perform a
radix sort.  For numbers, it seems Stata's implementation is very inefficient
at sorting numbers.

For mixed data types `qsort` is not much faster. Presumably it has an adge because
the modern compiler optimizations make it faster.

### Why is this so memory hungry?

I am a C novice and, unfortunately, I don't really know how to implement data
structures properly. Internally, I create a manually-indexed array that can
hold numbers or character pointers. Since array elements must be of equal
size, each element takes 8 bytes. For characters, I then allocate additional
space since string variables can _de facto_ have arbitrary lengths. Hence for
K variables, the overhead is K * 8 bytes per variable, plus the sum of all the
string variable lengths.

### Important differences from `sort` and `qsort`

`qsort` does not produce a stable sort, unless requested. This means that if
the order of the variables specified is not unique, the resulting data set
would not necessarily be in the same overall order.

Last, neither `mfirst` nor `gen` have been implemented.

## TODO

- [ ] Implement standard options (`mfirst`, `gen`)
- [ ] Better implementation of `stable`.
- [ ] Read and write data in parallel.

License
-------

`quicksort.c` is [BSD-licensed](). The rest of the code-base is
[MIT-licensed](https://github.com/mcaceresb/stata-gtools/blob/master/LICENSE)
