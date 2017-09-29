qsort
=====

[Overview](#faster-sorting)
| [Installation](#installation)
| [Benchmarks](#benchmarks)
| [Building](#building)
| [FAQs](#faqs)
| [License](#license)

Proof-of-concept Stata package that provides a (usually) faster implementation
of `sort` and an always faster implementation of `gsort`. It is a wrapper
for BSD's implementation of quicksort, but the speed improvement comes at a
potentially expensive memory overhead.

`version 0.1.0 29Sep2017`

Faster Sorting
--------------

This package's aim is to provide a proof-of-concept implementation of `qsort`.
The speed improvements are specially large when sorting in descending order,
specially strings, or sorting numbers, specially integers. The speed increase
uses, in part, the insights from [this statalist.org](https://www.statalist.org/forums/forum/general-stata-discussion/mata/172131-big-data-recalling-previous-sort-orders) thread.

Though faster, this plugin can be VERY memory hungry, specially when sorting
by a large number of variables. Hence the more sort variables there are the
smaller the speed increase (this only applies if the sorting variables include
strings).

Installation
------------

I only have access to Stata 13.1, so I impose that to be the minimum.
```stata
net install qsort, from(https://raw.githubusercontent.com/mcaceresb/stata-qsort/master/build/)
* adoupdate, update
* ado uninstall qsort
```

The syntax is identical to `sort` and `gsort`, except `mfirst` and `gen` are not available.
```stata
sysuse auto
qsort foreign rep78 make
qsort -foreign rep78 -make, v b stable
```

Benchmarks
----------

Benchmarks were performed on a personal laptop running Linux:

    Program:   Stata/IC 13.1 (1 core)
    OS:        x86_64 GNU/Linux
    Processor: Intel(R) Core(TM) i7-6500U CPU @ 2.50GHz
    Cores:     2 cores with 2 virtual threads per core.
    Memory:    15.6GiB
    Swap:      15.6GiB

See the log files in `./src/test/qsort_tests*log` for details.  We benchmark
on a dataset with a million (1,000,000) observations and 12 variables.  (See
below for details on the data used). We can see a marginal improvement
relative to `sort`, and a massive improvement relative to `gsort`.

### Versus `sort`

| sort | qsort | ratio (s / q) | sorted by (stable)                                         |
| ---- | ----- | ------------- | ---------------------------------------------------------- |
| 1.64 | .94   | 1.74          | str_12                                                     |
| 2.08 | 1.31  | 1.59          | str_12 str_32                                              |
| 2.3  | 1.52  | 1.51          | str_12 str_32 str_4                                        |
| 1.74 | .87   | 2             | double1                                                    |
| 1.95 | .96   | 2.03          | double1 double2                                            |
| 1.99 | 1.07  | 1.86          | double1 double2 double3                                    |
| 1.73 | .71   | 2.44          | int1                                                       |
| 2.37 | .92   | 2.58          | int1 int2                                                  |
| 2.1  | .92   | 2.28          | int1 int2 int3                                             |
| 2.41 | 1.21  | 1.99          | int1 str_32 double1                                        |
| 2.48 | 1.94  | 1.28          | int1 str_32 double1 int2 str_12 double2                    |
| 2.87 | 2.72  | 1.06          | int1 str_32 double1 int2 str_12 double2 int3 str_4 double3 |

### Versus `gsort`

| gsort | qsort | ratio (s / q) | sorted by (stable)                                             |
| ----- | ----- | ------------- | ----------------------------------------------------------     |
| 8.14  | .91   | 8.95          | -str_12                                                        |
| 8.03  | 1.28  | 6.27          | str_12 -str_32                                                 |
| 11.43 | 1.61  | 7.1           | str_12 -str_32 str_4                                           |
| 7.89  | .7    | 11.27         | -double1                                                       |
| 7.15  | .74   | 9.66          | double1 -double2                                               |
| 9.4   | .81   | 11.6          | double1 -double2 double3                                       |
| 5.65  | .52   | 10.87         | -int1                                                          |
| 7.13  | .63   | 11.32         | int1 -int2                                                     |
| 8.08  | .73   | 11.07         | int1 -int2 int3                                                |
| 11.16 | .94   | 11.87         | -int1 -str_32 -double1                                         |
| 13.67 | 1.7   | 8.04          | int1 -str_32 double1 -int2 str_12 -double2                     |
| 17.58 | 2.1   | 8.37          | int1 -str_32 double1 -int2 str_12 -double2 int3 -str_4 double3 |

### Dataset description

```
  obs:     1,000,000
 vars:            12                          29 Sep 2017 10:54
 size:    95,000,000
----------------------------------------------------------------
              storage   display    value
variable name   type    format     label      variable label
----------------------------------------------------------------
str_long        str5    %9s
str_mid         str3    %9s
str_short       str3    %9s
str_32          str32   %32s
str_12          str12   %12s
str_4           str4    %9s
int1            long    %12.0g
int2            long    %12.0g
int3            long    %12.0g
double1         double  %10.0g
double2         double  %10.0g
double3         double  %10.0g
----------------------------------------------------------------
Sorted by:
```

The first three string variables were created using the `ralpha` package. The
numbered string variables were concatenated from the named string variables
and string constants. All the doubles are random uniforms or random normals.
All the integers are random integers. Some summary stats:

```
    Variable |       Obs        Mean    Std. Dev.       Min        Max
-------------+--------------------------------------------------------
    str_long |         0
     str_mid |         0
   str_short |         0
      str_32 |         0
      str_12 |         0
-------------+--------------------------------------------------------
       str_4 |         0
        int1 |   1000000      -.4975    1.041314         -5          4
        int2 |   1000000    499.1961    288.4051          0        999
        int3 |   1000000     9.50008    5.009741        -13         32
     double1 |   1000000    .0040237    1.001515  -4.387256   4.301971
-------------+--------------------------------------------------------
     double2 |   1000000    500.9148    288.8438   .0060915   999.9914
     double3 |   1000000    9.982373    4.998421  -14.39133   31.13625
```

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
radix sort.

For mixed data types `qsort` is not much faster. The only reason there is a
speed bump is because it takes advantage of Stata's own `sortpreserve` function.
If you give Stata a sorting permutation, it can rearrange the data reasonably
fast. Thus, sorting just the sort variables and then rearranging the entire
data set is faster than sorting the entire data from the get go.

### Why can this get memory hungry?

To sort the data using C I need to copy the soting variables from Stata. Stata
does not create a copy of the data and simply sorts in in place.

### Important differences from `sort` and `qsort`

`qsort` does not produce a stable sort, unless requested. This means that if
the order of the variables specified is not unique, the resulting data set
would not necessarily be in the same overall order.

Last, neither `mfirst` nor `gen` have been implemented.

## TODO

- [ ] Implement standard options (`mfirst`, `gen`)
- [ ] Better implementation of `stable`.

License
-------

`quicksortBSD.c` is [BSD-licensed](). The rest of the code-base is
[MIT-licensed](https://github.com/mcaceresb/stata-gtools/blob/master/LICENSE)
