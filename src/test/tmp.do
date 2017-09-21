version 13
clear all
set more off
set varabbrev off
* set seed 42
set seed 1729
set linesize 128

* sysuse auto,  clear
* set rmsg on
* clear
* set obs 2000000
* * set obs 100000
* gen x = floor(rnormal())
* gen y = rnormal()
* qui ralpha rstr, l(5)
* qui ralpha rst2, l(3)
* replace rstr = rstr + "s"
* replace rst2 = rst2 + "someting longer"
* gen double id2 = _n
* gen byte dummy = runiform()
* gen idx = _n
* save /tmp/qsort, replace

set rmsg on
* use /tmp/qsort in 1/150000, clear
use /tmp/qsort, clear
* gen testing = "This is a very long string; I don't see how mem wouldn't go through the roof."
* desc
* expand 3
* replace dummy = runiform()
* sort rstr x rst2 y dummy
* cap drop st_idx
* gen st_idx = _n
* sort idx
* sort x y dummy

cap noi qsort x y dummy, v b
cap noi qsort idx, v b
cap noi qsort x idx, v b
cap noi qsort idx, v b
cap noi qsort rstr x rst2 y dummy, v b
l if st_idx != _n
assert st_idx == _n
