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

    local N = trim("`: di %15.0gc _N'")
    local r `:di %9.2f `time_sort' / `time_qsort''
    di "    benchmark (obs: `N', ratio: `r', varlist: `0') gsort: `time_sort', qsort: `time_qsort'"
    return scalar time_sort  = `time_sort'
    return scalar time_qsort = `time_qsort'
end
