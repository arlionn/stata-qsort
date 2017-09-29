* ---------------------------------------------------------------------
* Project: qsort
* Program: qsort_tests.do
* Author:  Mauricio Caceres Bravo <mauricio.caceres.bravo@gmail.com>
* Created: Sun Jul 30 20:41:49 EDT 2017
* Updated: Fri Sep 29 12:42:53 EDT 2017
* Purpose: Unit tests for qsort
* Version: 0.1.0
* Manual:  help qsort

* Stata start-up options
* ----------------------

version 13
clear all
set more off
set varabbrev off
* set seed 42
set seed 1729
set linesize 128
ssc install ralpha

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

        unit_test, `noisily' test(checks_options_qsort)

        di ""
        di "--------------------------------------------------"
        di "Consistency checks (vs qsort) $S_TIME $S_DATE"
        di "--------------------------------------------------"

        consistency_qsort
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

capture program drop checks_options_qsort
program checks_options_qsort
    syntax, [tol(real 1e-6) NOIsily *]
    di _n(1) "{hline 80}" _n(1) "checks_options_qsort, `options'" _n(1) "{hline 80}" _n(1)
    sysuse auto
    gen idx = _n
    qsort -foreign rep78 make -mpg
    qsort idx
    qsort -foreign rep78, stable
    qsort idx, qsort
    qsort foreign rep78 mpg
    qsort idx, v b
end

capture program drop consistency_qsort
program consistency_qsort
    syntax, [tol(real 1e-6) NOIsily *]

    cap gen_data, n(100000)
    expand 10

    compare_sort str_12
    compare_sort str_12 str_32
    compare_sort str_12 str_32 str_4

    compare_sort double1
    compare_sort double1 double2
    compare_sort double1 double2 double3

    compare_sort int1
    compare_sort int1 int2
    compare_sort int1 int2 int3

    compare_sort int1 str_32 double1
    compare_sort int1 str_32 double1 int2 str_12 double2
    compare_sort int1 str_32 double1 int2 str_12 double2 int3 str_4 double3

    di _n(2)

    compare_gsort -str_12
    compare_gsort str_12 -str_32
    compare_gsort str_12 -str_32 str_4

    compare_gsort -double1
    compare_gsort double1 -double2
    compare_gsort double1 -double2 double3

    compare_gsort -int1
    compare_gsort int1 -int2
    compare_gsort int1 -int2 int3

    compare_gsort -int1 -str_32 -double1
    compare_gsort int1 -str_32 double1 -int2 str_12 -double2
    compare_gsort int1 -str_32 double1 -int2 str_12 -double2 int3 -str_4 double3

    di _n(1) "{hline 80}" _n(1) "consistency_qsort, `options'" _n(1) "{hline 80}" _n(1)
end

capture program drop gen_data
program gen_data
    syntax, [n(int 100)]
    clear
    set obs `n'
    qui ralpha str_long,  l(5)
    qui ralpha str_mid,   l(3)
    qui ralpha str_short, l(1)
    gen str32 str_32   = str_long + "this is some string padding"
    gen str12 str_12   = str_mid  + "padding" + str_short + str_short
    gen str4  str_4    = str_mid  + str_short

    gen long int1  = floor(rnormal())
    gen long int2  = floor(uniform() * 1000)
    gen long int3  = floor(rnormal() * 5 + 10)

    gen double double1 = rnormal()
    gen double double2 = uniform() * 1000
    gen double double3 = rnormal() * 5 + 10
end

capture program drop compare_sort
program compare_sort, rclass
    tempvar ix
    gen long `ix' = _n

    timer clear
    preserve
        timer on 42
        sort `0' `ix'
        timer off 42
        tempfile file_sort
        qui save `file_sort'
    restore
    qui timer list
    local time_sort `:di %9.2f r(t42)'

    timer clear
    preserve
        timer on 43
        qui qsort `0' `ix', v b
        timer off 43
        cf * using `file_sort'
    restore
    qui timer list
    local time_qsort `:di %9.2f r(t43)'

    local N = trim("`: di %15.0gc _N'")
    local r `:di %9.2f `time_sort' / `time_qsort''
    di "    benchmark (obs: `N', ratio: `r', varlist: `0') sort: `time_sort', qsort: `time_qsort'"
    return scalar time_sort  = `time_sort'
    return scalar time_qsort = `time_qsort'
end

capture program drop compare_gsort
program compare_gsort, rclass
    tempvar ix
    gen long `ix' = _n

    timer clear
    preserve
        timer on 42
        gsort `0' `ix'
        timer off 42
        tempfile file_sort
        qui save `file_sort'
    restore
    qui timer list
    local time_sort `:di %9.2f r(t42)'

    timer clear
    preserve
        timer on 43
        qui qsort `0' `ix', v b
        timer off 43
        cf * using `file_sort'
    restore
    qui timer list
    local time_qsort `:di %9.2f r(t43)'

    local N trim("`: di %15.0gc _N'")
    local r `:di %9.2f `time_sort' / `time_qsort''
    di "    benchmark (obs: `N', ratio: `r', varlist: `0') gsort: `time_sort', qsort: `time_qsort'"
    return scalar time_sort  = `time_sort'
    return scalar time_qsort = `time_qsort'
end

* ---------------------------------------------------------------------
* Run the things

main
