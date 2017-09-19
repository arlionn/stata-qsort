version 13
clear all
set more off
set varabbrev off
* set seed 42
set seed 1729
set linesize 128

* sysuse auto,  clear
* cap noi qsort foreign rep78 make, v b
* cap noi qsort foreign rep78 make, v b
* l

set rmsg on
* clear
* * set obs 300000
* set obs 10000000
* gen x = floor(rnormal())
* gen y = rnormal()
* qui ralpha rstr, l(3)
* qui ralpha rst2, l(3)
* gen double id2 = _n
* gen byte dummy = 0
* gen idx    = _n - 1
* gen st_idx = _n - 1
* save tmp, replace

* use /home/mauricio/code/stata-qsort-fresh/tmp.dta in 1/1000000, clear
* cap noi qsort rstr dummy x
* sort rstr dummy x
use /home/mauricio/code/stata-qsort-fresh/tmp.dta, clear
cap noi qsort rstr x rst2 y, v b
* replace st_idx = _n - 1
* sort idx
* sort rstr x rst2 y
* assert st_idx == (_n - 1)
