* ---------------------------------------------------------------------
* Project: qsort
* Program: qsort_tests.do
* Author:  Mauricio Caceres Bravo <mauricio.caceres.bravo@gmail.com>
* Created: Sun Jul 30 20:41:49 EDT 2017
* Updated: Sun Jul 30 20:46:26 EDT 2017
* Purpose: Unit tests for qsort
* Version: 0.1.0
* Manual:  help qsort

* Stata start-up options
* ----------------------

/*

* TODO: Counting sort // 2017-07-31 12:22 EDT
* TODO: Sort the rest of the data along with the sort variables // 2017-07-31 12:22 EDT
* TODO: Unit tests // 2017-07-31 12:22 EDT

version 13
clear all
set more off
set varabbrev off
* set seed 42
set seed 1729
set linesize 128

sysuse auto,  clear
qsort mpg,       v b
qsort headroom,  v b
qsort make,      v b
qsort foreign rep78 make, v b
l

* sort rstr x rst2 y
sort dummy rstr rst2
cap drop st_idx
gen st_idx = _n
sort idx
* qsort rstr x rst2 y, v b
qsort dummy rstr rst2, v b
assert st_idx == _n
preserve
    keep in 1/1000
    gen my_idx = _n
    qsort rstr x rst2 y, v b
restore
clear

set rmsg on
clear
set obs 10000000
gen x1 = ceil(_N * rnormal() / 1000000)
gen x2 = ceil(_N * rnormal() / 100000)
gen x3 = ceil(_N * rnormal() / 10000)
gen x4 = ceil(_N * rnormal() / 1000)
gen x5 = ceil(_N * rnormal() / 100)
gen x6 = ceil(_N * rnormal() / 10)
gen x7 = rnormal()
gen alpha = "something"
gen idx = _n
sort x1-x7 alpha
sort idx
qsort x1-x7 alpha, v b

set rmsg on
clear
set obs 1000000
qui ralpha x, l(3)
qui ralpha y, l(3)
expand 4
gen idx = _n
sort x
sort idx
qsort x y, v b

. qsort x y, v b
Sort by strings
        Sort (1): Allocated memory; 0.244 seconds.
        Sort (2): Read string variables; 0.192 seconds.
        Sort (3): Sorted array; 2.074 seconds.
        Sort (4): Copied back sorted variables; 0.447 seconds.
        Sort (5): Free up memory; 0.423 seconds.
r; t=4.16 12:14:52

. qsort x y, v b
Sort by strings
        Sort (1): Allocated memory; 0.000 seconds.
        Sort (2): Read string variables; 0.408 seconds.
        Sort (3): Sorted array; 2.044 seconds.
        Sort (4): Copied back sorted variables; 0.496 seconds.
        Sort (5): Free up memory; 0.005 seconds.
r; t=3.75 12:18:32

*/

* Main program wrapper
* --------------------

program main
    syntax, [CAPture NOIsily *]

    * Set up
    * ------

    local  progname tests
    local  start_time "$S_TIME $S_DATE"
    di "Start: `start_time'"

    * Run the things
    * --------------

    `capture' `noisily' {
        * do qsort_checks.do

        di ""
        di "-------------------------------------"
        di "Basic unit-tests $S_TIME $S_DATE"
        di "-------------------------------------"

        unit_test, `noisily' test(checks_options_qsort, debug_force_single)
        unit_test, `noisily' test(checks_call_qsort,    debug_force_single)
        if !inlist("`c(os)'", "Windows") {
            unit_test, `noisily' test(checks_options_qsort, debug_force_multi)
            unit_test, `noisily' test(checks_call_qsort,    debug_force_multi)
        }

        di ""
        di "--------------------------------------------------"
        di "Consistency checks (vs qsort) $S_TIME $S_DATE"
        di "--------------------------------------------------"

        consistency_qsort, `noisily' oncollision(error) debug_force_single
        if !inlist("`c(os)'", "Windows") {
            consistency_qsort, `noisily' oncollision(error) debug_force_multi
        }
    }
    local rc = _rc

    exit_message, rc(`rc') progname(`progname') start_time(`start_time') `capture'
    exit `rc'
end

* ---------------------------------------------------------------------
* Aux programs

capture program drop exit_message
program exit_message
    syntax, rc(int) progname(str) start_time(str) [CAPture]
    local end_time "$S_TIME $S_DATE"
    local time     "Start: `start_time'" _n(1) "End: `end_time'"
    di ""
    if (`rc' == 0) {
        di "End: $S_TIME $S_DATE"
        local paux      ran
        local message "`progname' finished running" _n(2) "`time'"
        local subject "`progname' `paux'"
    }
    else if ("`capture'" == "") {
        di "WARNING: $S_TIME $S_DATE"
        local paux ran with non-0 exit status
        local message "`progname' ran but Stata gave error code r(`rc')" _n(2) "`time'"
        local subject "`progname' `paux'"
    }
    else {
        di "ERROR: $S_TIME $S_DATE"
        local paux ran with errors
        local message "`progname' stopped with error code r(`rc')" _n(2) "`time'"
        local subject "`progname' `paux'"
    }
    di "`subject'"
    di ""
    di "`message'"
end

* Wrapper for easy timer use
cap program drop mytimer
program mytimer, rclass
    * args number what step
    syntax anything, [minutes ts]

    tokenize `anything'
    local number `1'
    local what   `2'
    local step   `3'

    if ("`what'" == "end") {
        qui {
            timer clear `number'
            timer off   `number'
        }
        if ("`ts'" == "ts") mytimer_ts `step'
    }
    else if ("`what'" == "info") {
        qui {
            timer off `number'
            timer list `number'
        }
        local seconds = r(t`number')
        local prints  `:di trim("`:di %21.2gc `seconds''")' seconds
        if ("`minutes'" != "") {
            local minutes = `seconds' / 60
            local prints  `:di trim("`:di %21.3gc `minutes''")' minutes
        }
        mytimer_ts Step `step' took `prints'
        qui {
            timer clear `number'
            timer on    `number'
        }
    }
    else {
        qui {
            timer clear `number'
            timer on    `number'
            timer off   `number'
            timer list  `number'
            timer on    `number'
        }
        if ("`ts'" == "ts") mytimer_ts `step'
    }
end

capture program drop mytimer_ts
program mytimer_ts
    display _n(1) "{hline 79}"
    if ("`0'" != "") display `"`0'"'
    display `"        Base: $S_FN"'
    display  "        In memory: `:di trim("`:di %21.0gc _N'")' observations"
    display  "        Timestamp: $S_TIME $S_DATE"
    display  "{hline 79}" _n(1)
end

capture program drop unit_test
program unit_test
    syntax, test(str) [NOIsily tab(int 4)]
    local tabs `""'
    forvalues i = 1 / `tab' {
        local tabs "`tabs' "
    }
    cap `noisily' `test'
    if ( _rc ) {
        di as error `"`tabs'test(failed): `test'"'
        exit _rc
    }
    else di as txt `"`tabs'test(passed): `test'"'
end

* ---------------------------------------------------------------------
* Run the things

main
