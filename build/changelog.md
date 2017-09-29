Change Log
==========

## qsort-0.1.0 (2017-07-29)

### Features

* `qsort` is a Stata wrapper for C's qsort that is usually faster than `sort`
  and is always faster than `gsort`.
* The speed increase uses the insights from [this statalist.org](https://www.statalist.org/forums/forum/general-stata-discussion/mata/172131-big-data-recalling-previous-sort-orders) thread.

### Planned

- `qsort, mfirst`: Replace missing values with its negative (for doubles) and
  have a special sorting function for strings (if mfirst is requested; check
  if it is faster to check for missing values before calling the special
  function). Missings are the largest double, so fliping to negative would
  sort them first.
