*! version 0.1.0 28Jul2017 Mauricio Caceres Bravo, mauricio.caceres.bravo@gmail.com
*! implementation of -sort- and -gsort- using C-plugins

capture program drop qsort
program define qsort
    version 13
    syntax varlist(min = 1), [Verbose Benchmark overhead]
    if ( "`c(os)'" != "Unix" ) di as err "Not available for `c(os)`; only Unix."

    * Verbose and benchmark printing
    * ------------------------------

    if ( "`verbose'" == "" ) {
        local verbose = 0
    }
    else {
        local verbose = 1
    }

    if ( "`benchmark'" == "" ) {
        local benchmark = 0
    }
    else {
        local benchmark = 1
    }

    scalar __qsort_verbose   = `verbose'
    scalar __qsort_benchmark = `benchmark'

    if ( `verbose'  | `benchmark' ) local noi noisily

    * Parse sort variables and the rest of the variabes
    * -------------------------------------------------

    qui ds *
    local memvars `r(varlist)'
    qui ds `varlist'
    local sortvars `r(varlist)'
    local restvars: list memvars - sortvars

    scalar __qsort_kvars_sort = `:list sizeof sortvars'
    scalar __qsort_kvars_rest = `:list sizeof restvars'
    parse_sort_types `sortvars', `multi'
    parse_rest_types `restvars', `multi'

    * Run the plugin
    * --------------

    * cap `noi' plugin call qsort_plugin `varlist' `restvars', sort
    di "plugin call qsort_plugin `varlist' `restvars', sort"
    plugin call qsort_plugin `varlist' `restvars', sort

    * Clean up after yourself
    * -----------------------

    cap matrix drop __qsort_strpos_rest
    cap matrix drop __qsort_numpos_rest
    cap matrix drop __qsort_restvars

    cap matrix drop __qsort_strpos
    cap matrix drop __qsort_numpos

    cap matrix drop __qsort_sortvars
    cap matrix drop __qsort_sortmin
    cap matrix drop __qsort_sortmax

    cap matrix drop c_qsort_sortmiss
    cap matrix drop c_qsort_sortmin
    cap matrix drop c_qsort_sortmax

    cap scalar drop __qsort_kvars_sort
    cap scalar drop __qsort_kvars_rest
    cap scalar drop __qsort_is_int
end

cap program drop qsort_plugin
program qsort_plugin, plugin using(`"qsort_`:di lower("`c(os)'")'.plugin"')

cap program drop qsortmulti_plugin
cap program qsortmulti_plugin, plugin using(`"qsort_`:di lower("`c(os)'")'_multi.plugin"')

* Parse sort variable types; encode as numbers
* --------------------------------------------

capture program drop parse_sort_types
program parse_sort_types
    syntax varlist(min = 1), [multi]

    cap matrix drop __qsort_strpos
    cap matrix drop __qsort_numpos

    cap matrix drop __qsort_sortvars
    cap matrix drop __qsort_sortmin
    cap matrix drop __qsort_sortmax

    cap matrix drop c_qsort_sortmiss
    cap matrix drop c_qsort_sortmin
    cap matrix drop c_qsort_sortmax

    * If any strings, skip integer check
    * ----------------------------------

    local kmaybe  = 1
    foreach sortvar of varlist `varlist' {
        if regexm("`:type `sortvar''", "str") local kmaybe = 0
    }

    * Check whether we only have integers
    * -----------------------------------

    local varnum  ""
    local knum    = 0
    local khash   = 0
    local intlist ""
    foreach sortvar of varlist `varlist' {
        if ( `kmaybe' ) {
            if inlist("`:type `sortvar''", "byte", "int", "long") {
                local ++knum
                local varnum `varnum' `sortvar'
                local intlist `intlist' 1
            }
            else if inlist("`:type `sortvar''", "float", "double") {
                if ( `=_N > 0' ) {
                    cap plugin call qsort`multi'_plugin `sortvar', isint
                    if ( _rc ) exit _rc
                }
                else scalar __qsort_is_int = 0
                if ( `=scalar(__qsort_is_int)' ) {
                    local ++knum
                    local varnum `varnum' `sortvar'
                    local intlist `intlist' 1
                }
                else {
                    local kmaybe = 0
                    local ++khash
                    local intlist `intlist' 0
                }
            }
            else {
                local kmaybe = 0
                local ++khash
                local intlist `intlist' 0
            }
        }
        else {
            local ++khash
            local intlist `intlist' 0
        }
    }
    else {
        foreach sortvar of varlist `varlist' {
            local intlist `intlist' 0
        }
    }

    * Set up max-min for integer sort in C
    * ------------------------------------

    * If so, set up min and max in C. Later we will check whether we can use a
    * bijection of the sort variables to the whole numbers as our index
    if ( (`knum' > 0) & (`khash' == 0) ) {
        matrix c_qsort_sortmiss = J(1, `knum', 0)
        matrix c_qsort_sortmin  = J(1, `knum', 0)
        matrix c_qsort_sortmax  = J(1, `knum', 0)
        if ( `=_N > 0' ) {
            cap plugin call qsort`multi'_plugin `varnum', setup
            if ( _rc ) exit _rc
        }
        matrix __qsort_sortmin = c_qsort_sortmin
        matrix __qsort_sortmax = c_qsort_sortmax + c_qsort_sortmiss
    }

    * Encode type of each variable
    * ----------------------------

    * See 'help data_types'; we encode string types as their length,
    * integer types as -1, and other numeric types as 0. Each are
    * handled differently when sorting:
    *     - All integer types: Try to map them to the natural numbers
    *     - All same type: Invoke loop that reads the same type
    *     - A mix of types: Invoke loop that reads a mix of types
    *
    * The loop that reads a mix of types switches from reading strings
    * to reading numeric variables in the order the user specified the
    * sort variables, which is necessary for the sort to be consistent.
    * But this version of the loop is marginally slower than the version
    * that reads the same type throughout.
    *
    * Last, we need to know the length of the data to read them into
    * C and sort them. Numeric data are 8 bytes (we will read them
    * as double) and strings are read into a string buffer, which is
    * allocated the length of the longest sort string variable.

    local sort_post  0
    local sort_types ""
    foreach sortvar of varlist `varlist' {
        local ++sort_post
        gettoken is_int intlist: intlist
        local stype: type `sortvar'
        if ( (`is_int' | inlist("`stype'", "byte", "int", "long")) ) {
            local sort_types `sort_types' num
            matrix __qsort_sortvars = nullmat(__qsort_sortvars), -1
            matrix __qsort_numpos   = nullmat(__qsort_numpos), `sort_post'
        }
        else {
            matrix __qsort_sortmin = J(1, `:list sizeof varlist', 0)
            matrix __qsort_sortmax = J(1, `:list sizeof varlist', 0)
            if regexm("`stype'", "str([1-9][0-9]*|L)") {
                local sort_types `sort_types' str
                if ( regexs(1) == "L" ) {
                    tempvar strlen
                    gen `strlen' = length(`sortvar')
                    qui sum `strlen'
                    matrix __qsort_sortvars = nullmat(__qsort_sortvars), `r(max)'
                    matrix __qsort_strpos   = nullmat(__qsort_strpos),   `sort_post' 
                }
                else {
                    matrix __qsort_sortvars = nullmat(__qsort_sortvars), `=regexs(1)'
                    matrix __qsort_strpos   = nullmat(__qsort_strpos),   `sort_post' 
                }
            }
            else {
                local sort_types `sort_types' num
                if inlist("`stype'", "float", "double") {
                    matrix __qsort_sortvars = nullmat(__qsort_sortvars), 0
                    matrix __qsort_numpos   = nullmat(__qsort_numpos), `sort_post'
                }
                else if ( inlist("`stype'", "byte", "int", "long") ) {
                    matrix __qsort_sortvars = nullmat(__qsort_sortvars), 0
                    matrix __qsort_numpos   = nullmat(__qsort_numpos), `sort_post'
                }
                else {
                    di as err "variable `byvar' has unknown type '`stype''"
                }
            }
        }
    }
end

* Parse the tpe of the rest of the variables
* ------------------------------------------

capture program drop parse_rest_types
program parse_rest_types
    syntax varlist, [multi]

    cap matrix drop __qsort_strpos_rest
    cap matrix drop __qsort_numpos_rest
    cap matrix drop __qsort_restvars

    * Encode type of each variable
    * ----------------------------

    local sort_post  0
    foreach sortvar of varlist `varlist' {
        local ++sort_post
        local stype: type `sortvar'
        if regexm("`stype'", "str([1-9][0-9]*|L)") {
            if ( regexs(1) == "L" ) {
                tempvar strlen
                gen `strlen' = length(`sortvar')
                qui sum `strlen'
                matrix __qsort_restvars    = nullmat(__qsort_restvars),    `r(max)'
                matrix __qsort_strpos_rest = nullmat(__qsort_strpos_rest), `sort_post' 
            }
            else {
                matrix __qsort_restvars = nullmat(__qsort_restvars),       `=regexs(1)'
                matrix __qsort_strpos_rest = nullmat(__qsort_strpos_rest), `sort_post' 
            }
        }
        else {
            matrix __qsort_restvars    = nullmat(__qsort_restvars),    0
            matrix __qsort_numpos_rest = nullmat(__qsort_numpos_rest), `sort_post'
        }
    }
end
