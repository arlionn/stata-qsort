/*********************************************************************
 * Program: qsort_plugin.c
 * Author:  Mauricio Caceres Bravo <mauricio.caceres.bravo@gmail.com>
 * Created: Sun Jul 30 11:00:27 EDT 2017
 * Updated: Thu Aug 24 13:41:40 EDT 2017
 * Purpose: Stata plugin to sort data using BSD's quicksort
 * Note:    See stata.com/plugins for more on Stata plugins
 * Version: 0.1.0
 *********************************************************************/

/**
 * @file qsort_plugin.c
 * @author Mauricio Caceres Bravo
 * @date 30 Jul 2017
 * @brief Stata plugin to sort data using BSD's quicksort
 *
 * This file should only ever be called from qsort.ado
 *
 * @see help gcollapse, help egen, gcollapse.c, gegen.c
 * @see http://www.stata.com/plugins for more on Stata plugins
 */

#define _GNU_SOURCE
#include "qsort_plugin.h"
#include "spi/stplugin.h"
#include "spt/st_gentools.c"
#include "qsort_utils.c"
#include "qsort_generic.c"

// -DGMUTI=1 flag compiles multi-threaded version of the plugin
// #if GMULTI
// #include "qsort_multi.c"
// #else
// #include "qsort_single.c"
// #endif

STDLL stata_call(int argc, char *argv[])
{
    if ( argc < 1 ) {
        sf_errprintf ("Nothing to do. Available: -isint- and -sort-\n");
        return (198);
    }

    ST_retcode rc;
    setlocale (LC_ALL, "");
    struct StataInfo st_info;
    char todo[8];
    strcpy (todo, argv[0]);

    if ( strcmp(todo, "check") == 0 ) {
        // Exit; if you're here the plugin was loaded fine
        return (0);
    }
    else if ( strcmp(todo, "isint") == 0 ) {
        // Figure out if float|double variable is actually all integers
        if ( (rc = sf_isint()) ) return (rc);
        return (0);
    }
    else if ( strcmp(todo, "setup") == 0 ) {
        // Computes min, max, and rage for all numeric sort variables
        if ( (rc = sf_numsetup()) ) return (rc);
        return (0);
    }
    else if ( strcmp(todo, "sort") == 0 ) {
        if ( (rc = sf_parse_info(&st_info)) ) return (rc);

        if ( st_info.integers_ok ) {
            if ( st_info.verbose ) sf_printf("Sort by integers\n", st_info.kvars_sort_num);
            return (42001);
        }
        else if ( st_info.sortvars_maxlen > 0 ) {
            if ( st_info.sortvars_minlen > 0 ) {
                if ( st_info.verbose ) sf_printf("Sort by strings\n", st_info.kvars_sort_str);
                // if ( (rc = sf_ssort (&st_info)) );
                return (42003);
            }
            else {
                if ( st_info.verbose ) sf_printf("Sort by %d strings, %d numeric\n",
                                                 st_info.kvars_sort_str, st_info.kvars_sort_num);
                if ( (rc = sf_msort (&st_info)) ) return (rc);
                return (42004);
            }
        }
        else {
            if ( st_info.verbose ) sf_printf("Sort by numeric\n", st_info.kvars_sort_num);
            return (42002);
            // if ( (rc = sf_dsort (&st_info)) );
        }

        return(0);
    }

    sf_printf ("Nothing to do; pugin should be called from -gcollapse- or -gegen-\n");
    return(0);
}

int sf_parse_info (struct StataInfo *st_info)
{
    int i, k;
    ST_retcode rc ;

    /*********************************************************************
     *                           Basic parsing                           *
     *********************************************************************/

    // Number of observations
    size_t in1 = SF_in1();
    size_t in2 = SF_in2();
    size_t N   = in2 - in1 + 1;

    // Number of variables to sort by
    int kvars_sort;
    ST_double kvars_sort_double;
    if ( (rc = SF_scal_use("__qsort_kvars_sort", &kvars_sort_double)) ) {
        return (rc);
    }
    else {
        kvars_sort = (int) kvars_sort_double;
    }

    // Number of variables left
    int kvars_rest;
    ST_double kvars_rest_double ;
    if ( (rc = SF_scal_use("__qsort_kvars_rest", &kvars_rest_double)) ) {
        return (rc);
    }
    else {
        kvars_rest = (int) kvars_rest_double;
    }
    int kvars_rest_start = kvars_sort + 1;

    // Verbose printing
    int verbose;
    ST_double verb_double ;
    if ( (rc = SF_scal_use("__qsort_verbose", &verb_double)) ) {
        return(rc) ;
    }
    else {
        verbose = (int) verb_double;
    }

    // Benchmark printing
    int benchmark;
    ST_double bench_double ;
    if ( (rc = SF_scal_use("__qsort_benchmark", &bench_double)) ) {
        return(rc) ;
    }
    else {
        benchmark = (int) bench_double;
    }

    /*********************************************************************
     *                 Parse sort variables info vectors                 *
     *********************************************************************/

    // Length of sort meta vectors
    // int kvars_sort = sf_get_vector_length("__qsort_sortvars");
    // if ( kvars_sort < 0 ) {
    //     sf_errprintf("Failed to parse __qsort_sortvars\n");
    //     return(198);
    // }

    // sortvars_lens:
    //     - For strings, the variable length
    //     - We store floats and doubles as double; we code "length" as 0
    //     - We store integers as uint64_t; we code "length" as -1
    // sortvars_mins:
    //     - Smallest string length. If 0 or -1 we can figure out
    //       whether we have doubles or integers in the sort variables.
    // sortvars_maxs:
    //     - Largest string length. If 0 or -1 we can figure out
    //       whether we only have numbers for sort variables.
    st_info->sortvars_lens = calloc(kvars_sort, sizeof st_info->sortvars_lens);
    st_info->sortvars_mins = calloc(kvars_sort, sizeof st_info->sortvars_mins);
    st_info->sortvars_maxs = calloc(kvars_sort, sizeof st_info->sortvars_maxs);

    if ( st_info->sortvars_lens == NULL ) return(sf_oom_error("sf_parse_info", "st_info->sortvars_lens"));
    if ( st_info->sortvars_mins == NULL ) return(sf_oom_error("sf_parse_info", "st_info->sortvars_mins"));
    if ( st_info->sortvars_maxs == NULL ) return(sf_oom_error("sf_parse_info", "st_info->sortvars_maxs"));

    double sortvars_lens_double[kvars_sort],
           sortvars_mins_double[kvars_sort],
           sortvars_maxs_double[kvars_sort];

    if ( (rc = sf_get_vector("__qsort_sortvars", sortvars_lens_double)) ) return(rc);
    if ( (rc = sf_get_vector("__qsort_sortmin",  sortvars_mins_double)) ) return(rc);
    if ( (rc = sf_get_vector("__qsort_sortmax",  sortvars_maxs_double)) ) return(rc);

    for (i = 0; i < kvars_sort; i++) {
        st_info->sortvars_lens[i] = (int) sortvars_lens_double[i];
        st_info->sortvars_mins[i] = (int) sortvars_mins_double[i];
        st_info->sortvars_maxs[i] = (int) sortvars_maxs_double[i];
    }

    // Get count of numeric and string sort variables
    size_t kvars_sort_str = 0;
    for (i = 0; i < kvars_sort; i++) {
        kvars_sort_str += (st_info->sortvars_lens[i] > 0);
    }
    size_t kvars_sort_num = kvars_sort - kvars_sort_str;

    // If only integers, check worst case of the bijection would not overflow.
    // Given K variables, sort_1 to sort_K, where sort_k belongs to the set
    // B_k, the general problem we face is devising a function f such that
    // f: B_1 x ... x B_K -> N, where N are the natural (whole) numbers. For
    // integers, we don't need to conjure qsort---we can use the bijection.
    //
    //     1. The first variable: z[i, 1] = f(1)(x[i, 1]) = x[i, 1] - min(x[, 1]) + 1
    //     2. The kth variable: z[i, k] = f(k)(x[i, k]) = i * range(z[, k - 1]) + (x[i, k - 1] - min(x[, 2]))
    //
    // If we have too many sort variables, it is possible our integers will
    // overflow. We check whether this may happen below.

    int integers_ok;
    int sortvars_minlen = mf_min_signed(st_info->sortvars_lens, kvars_sort);
    int sortvars_maxlen = mf_max_signed(st_info->sortvars_lens, kvars_sort);
    if ( sortvars_maxlen < 0 ) {
        if (kvars_sort > 1) {
            integers_ok = 1;
            size_t worst = st_info->sortvars_maxs[0] - st_info->sortvars_mins[0] + 1;
            size_t range = st_info->sortvars_maxs[1] - st_info->sortvars_mins[1] + 1;
            for (k = 1; k < kvars_sort; k++) {
                if ( worst > (ULONG_MAX / range)  ) {
                    if ( verbose ) sf_printf("variables all intergers but bijection could fail! Won't risk it.\n");
                    integers_ok = 0;
                    break;
                }
                else {
                    worst *= range;
                    range  = st_info->sortvars_maxs[k] - st_info->sortvars_mins[k] + (k < (kvars_sort - 1));
                }
            }
        }
        else {
            integers_ok = 1;
        }
    }
    else integers_ok = 0;

    /*********************************************************************
     *          Relative position of targets and sort variables          *
     *********************************************************************/

    size_t strmax = sortvars_maxlen > 0? sortvars_maxlen + 1: 1;

    st_info->pos_num_sortvars = calloc(kvars_sort_num,  sizeof st_info->pos_num_sortvars);
    st_info->pos_str_sortvars = calloc(kvars_sort_str,  sizeof st_info->pos_str_sortvars);

    if ( st_info->pos_num_sortvars == NULL ) return(sf_oom_error("sf_parse_info", "st_info->pos_num_sortvars"));
    if ( st_info->pos_str_sortvars == NULL ) return(sf_oom_error("sf_parse_info", "st_info->pos_str_sortvars"));

    double pos_str_sortvars_double[kvars_sort_str];
    double pos_num_sortvars_double[kvars_sort_num];

    // pos_str_sortvars[k] gives the position in the sort variables of the kth string variable
    if ( kvars_sort_str > 0 ) {
        if ( (rc = sf_get_vector("__qsort_strpos", pos_str_sortvars_double)) ) return(rc);
        for (k = 0; k < kvars_sort_str; k++)
            st_info->pos_str_sortvars[k] = (int) pos_str_sortvars_double[k];
    }

    // pos_num_sortvars[k] gives the position in the sort variables of the kth numeric variable
    if ( kvars_sort_num > 0 ) {
        if ( (rc = sf_get_vector("__qsort_numpos", pos_num_sortvars_double)) ) return(rc);
        for (k = 0; k < kvars_sort_num; k++)
            st_info->pos_num_sortvars[k] = (int) pos_num_sortvars_double[k];
    }

    /*********************************************************************
     *                    Parse rest of the variables                    *
     *********************************************************************/

    st_info->restvars_lens = calloc(kvars_rest, sizeof st_info->restvars_lens);
    if ( st_info->restvars_lens == NULL ) return(sf_oom_error("sf_parse_info", "st_info->restvars_lens"));

    // Get lengths of rest variables
    size_t strmax_rest = 0;
    double restvars_lens_double[kvars_rest];
    if ( (rc = sf_get_vector("__qsort_restvars", restvars_lens_double)) ) return(rc);
    for (i = 0; i < kvars_rest; i++) {
        st_info->restvars_lens[i] = (int) restvars_lens_double[i];
        if ( st_info->restvars_lens[i] > strmax_rest ) strmax_rest = st_info->restvars_lens[i];
    }

    // Get count of numeric and string rest variables
    size_t kvars_rest_str = 0;
    for (i = 0; i < kvars_rest; i++) {
        kvars_rest_str += (st_info->restvars_lens[i] > 0);
    }
    size_t kvars_rest_num = kvars_rest - kvars_rest_str;

    // Position of string and numeric variables
    st_info->pos_num_restvars = calloc(kvars_rest_num,  sizeof st_info->pos_num_restvars);
    st_info->pos_str_restvars = calloc(kvars_rest_str,  sizeof st_info->pos_str_restvars);

    if ( st_info->pos_num_restvars == NULL ) return(sf_oom_error("sf_parse_info", "st_info->pos_num_restvars"));
    if ( st_info->pos_str_restvars == NULL ) return(sf_oom_error("sf_parse_info", "st_info->pos_str_restvars"));

    double pos_str_restvars_double[kvars_rest_str];
    double pos_num_restvars_double[kvars_rest_num];

    // pos_str_restvars[k] gives the position in the rest variables of the kth string variable
    if ( kvars_rest_str > 0 ) {
        if ( (rc = sf_get_vector("__qsort_strpos_rest", pos_str_restvars_double)) ) return(rc);
        for (k = 0; k < kvars_rest_str; k++)
            st_info->pos_str_restvars[k] = (int) pos_str_restvars_double[k];
    }

    // pos_num_restvars[k] gives the position in the rest variables of the kth numeric variable
    if ( kvars_rest_num > 0 ) {
        if ( (rc = sf_get_vector("__qsort_numpos_rest", pos_num_restvars_double)) ) return(rc);
        for (k = 0; k < kvars_rest_num; k++)
            st_info->pos_num_restvars[k] = (int) pos_num_restvars_double[k];
    }

    /*********************************************************************
     *                      Save info in structure                       *
     *********************************************************************/

    st_info->in1              = in1;
    st_info->in2              = in2;
    st_info->N                = N;
    st_info->kvars_rest       = kvars_rest;
    st_info->kvars_rest_start = kvars_rest_start;
    st_info->kvars_rest_num   = kvars_rest_num;
    st_info->kvars_rest_str   = kvars_rest_str;
    st_info->kvars_sort       = kvars_sort;
    st_info->kvars_sort_num   = kvars_sort_num;
    st_info->kvars_sort_str   = kvars_sort_str;
    st_info->verbose          = verbose;
    st_info->benchmark        = benchmark;
    st_info->integers_ok      = integers_ok;
    st_info->strmax           = strmax;
    st_info->strmax_rest      = strmax_rest;
    st_info->sortvars_minlen  = sortvars_minlen;
    st_info->sortvars_maxlen  = sortvars_maxlen;

    return (0);
}
