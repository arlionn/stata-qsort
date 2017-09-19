Change Log
==========

## qsort-0.1.0 (2017-07-28)

### Features

* `qsort` is a Stata wrapper for C's qsort
* At the moment, it can only sort on 1 or 2 variables of the same type.

### Planned

- Have two comparison functions, with one flipping the sign, for +-.
- `qsort, stable`: Create additional variable, 1 to N, and sort by that at the
  end as well as part of the nexted sorting.
- `qsort, mfirst`: Replace missing values with its negative (for doubles) and
  have a special sorting function for strings (if mfirst is requested; check
  if it is faster to check for missing values before calling the special
  function). Missings are the largest double, so fliping to negative would
  sort them first.
- `qsort, gen()`: Simply write the index to a variable.
