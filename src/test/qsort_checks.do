capture program drop checks_options_qsort
program checks_options_qsort
    syntax, [tol(real 1e-6) NOIsily *]
    di _n(1) "{hline 80}" _n(1) "checks_options_qsort, `options'" _n(1) "{hline 80}" _n(1)
end

capture program drop checks_call_qsort
program checks_call_qsort
    syntax, [tol(real 1e-6) NOIsily *]
    di _n(1) "{hline 80}" _n(1) "checks_call_qsort, `options'" _n(1) "{hline 80}" _n(1)
end

capture program drop consistency_qsort
program consistency_qsort
    syntax, [tol(real 1e-6) NOIsily *]
    di _n(1) "{hline 80}" _n(1) "consistency_qsort, `options'" _n(1) "{hline 80}" _n(1)
end
