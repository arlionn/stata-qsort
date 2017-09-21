#include "qsort_generic.h"
#include "quicksortMixedUnion.c"
#include "quicksortMultiLevel.c"
#include "radixSort.c"
#include <unistd.h>

int sf_msort(struct StataInfo *st_info)
{
    // Setup
    // -----

    int i, k;
    size_t N     = st_info->N;
    size_t in1   = st_info->in1;
    size_t ksort = st_info->kvars_sort;
    size_t krest = st_info->kvars_rest;
    size_t kstr  = st_info->kvars_sort_str + st_info->kvars_rest_str;
    size_t knum  = st_info->kvars_sort_num + st_info->kvars_rest_num;
    size_t kvars = knum + kstr;

    // Variable lengths; 0 are double, > 0 are string lengths
    size_t *ltypes = calloc(kvars, sizeof(*ltypes));
    if ( ltypes == NULL ) return (sf_oom_error("sf_msort", "ltypes"));

    int ilen;
    size_t allbytes = 0;
    size_t strbytes = 0;
    for (k = 0; k < ksort; k++) {
        ilen = st_info->sortvars_lens[k];
        if ( ilen > 0 ) {
            ltypes[k] = ilen;
            allbytes += ((ltypes[k] + 1) * sizeof(char));
            strbytes += ((ltypes[k] + 1) * sizeof(char));
        }
        else {
            ltypes[k] = 0;
            allbytes += sizeof(double);
        }
    }

    for (k = 0; k < krest; k++) {
        ilen = st_info->restvars_lens[k];
        if ( ilen > 0 ) {
            ltypes[k + ksort] = ilen;
            allbytes += ((ltypes[k + ksort] + 1) * sizeof(char));
            strbytes += ((ltypes[k + ksort] + 1) * sizeof(char));
        }
        else {
            ltypes[k + ksort] = 0;
            allbytes += sizeof(double);
        }
    }

    /*********************************************************************
     *                          Allocate space                           *
     *********************************************************************/

    ST_retcode rc ;
    size_t sel;
    clock_t timer = clock();
    MixedUnion *st_dtax;

    double mib_dtax = N * kvars * sizeof(MixedUnion) / 1024 / 1024;
    double mib_all  = mib_dtax + N * strbytes * sizeof(char) / 1024 / 1024;
    if ( st_info->verbose ) sf_printf("(memory overhead > %.2fMiB)\n", mib_all);

    if ( (st_info->qsort == 0) & (st_info->kvars_sort_str == 0) ) {

        // It is leaner to use a union than an array of pointers or a structure
        // void **st_dtax = calloc(N * kvars, sizeof(*st_dtax));
        st_dtax = calloc(N * krest, sizeof(*st_dtax));
        if ( st_dtax == NULL ) return (sf_oom_error("sf_msort", "st_dtax"));

        // It is faster to allocate and zero all memory first
        for (i = 0; i < N; i++) {
            for (k = 0; k < krest; k++) {
                sel = i * krest + k;
                if ( (ilen = ltypes[k + ksort]) ) {
                    st_dtax[sel].cval = malloc((ilen + 1) * sizeof(char));
                    if ( st_dtax[sel].cval == NULL ) return (sf_oom_error("sf_msort", "st_dtax[sel].cval"));
                    memset (st_dtax[sel].cval, '\0', ilen + 1);
                }
                // else {
                //     st_dtax[sel] = malloc(sizeof(double));
                //     if ( st_dtax[sel].dval == NULL ) return (sf_oom_error("sf_msort", "st_dtax[sel].dval"));
                // }
            }
        }
    }
    else {

        // It is leaner to use a union than an array of pointers or a structure
        // void **st_dtax = calloc(N * kvars, sizeof(*st_dtax));
        st_dtax = calloc(N * kvars, sizeof(*st_dtax));
        if ( st_dtax == NULL ) return (sf_oom_error("sf_msort", "st_dtax"));

        // It is faster to allocate and zero all memory first
        for (i = 0; i < N; i++) {
            for (k = 0; k < kvars; k++) {
                sel = i * kvars + k;
                if ( (ilen = ltypes[k]) ) {
                    st_dtax[sel].cval = malloc((ilen + 1) * sizeof(char));
                    if ( st_dtax[sel].cval == NULL ) return (sf_oom_error("sf_msort", "st_dtax[sel].cval"));
                    memset (st_dtax[sel].cval, '\0', ilen + 1);
                }
                // else {
                //     st_dtax[sel] = malloc(sizeof(double));
                //     if ( st_dtax[sel].dval == NULL ) return (sf_oom_error("sf_msort", "st_dtax[sel].dval"));
                // }
            }
        }
    }

    /*********************************************************************
     *                         Read in the data                          *
     *********************************************************************/

    if ( (st_info->qsort == 0) & st_info->integers_ok ) {

        // Integers are bijected to the natural numbers
        // --------------------------------------------

        size_t *st_bijection = calloc(N, sizeof(*st_bijection));
        size_t *st_index     = calloc(N, sizeof(*st_index));
        if ( st_bijection == NULL ) return (sf_oom_error("sf_msort", "st_bijection"));
        if ( st_index     == NULL ) return (sf_oom_error("sf_msort", "st_index"));

        size_t offset = 1;
        size_t offsets[ksort];
        offsets[0] = 0;
        for (k = 0; k < ksort - 1; k++) {
            offset *= (st_info->sortvars_maxs[k] - st_info->sortvars_mins[k] + 1);
            offsets[k + 1] = offset;
        }

        // Read in integer sort vars
        // -------------------------

        ST_double z;
        int *st_int = calloc(N * ksort, sizeof(*st_int));
        if ( st_int == NULL ) return (sf_oom_error("sf_msort", "st_int"));

        for (i = 0; i < N; i++) {

            // If only one variable, it will just get adjusted by its range
            sel = i * ksort;
            if ( (rc = SF_vdata(1, i + in1, &z)) ) return(rc);
            if ( SF_is_missing(z) ) z = st_info->sortvars_maxs[0];
            st_bijection[i] = z - st_info->sortvars_mins[0] + 1;
            st_int[sel] = (int) z;

            // If multiple integers, they'll get mapped recursively
            for (k = 1; k < ksort; k++) {
                sel = i * ksort + k;
                if ( (rc = SF_vdata(1 + k, i + in1, &z)) ) return(rc);
                if ( SF_is_missing(z) ) z = st_info->sortvars_maxs[0];
                st_bijection[i] += (z - st_info->sortvars_mins[k]) * offsets[k];
                st_int[sel] = (int) z;
            }
        }
        if ( st_info->benchmark ) sf_running_timer (&timer, "\tSort (1): Read in sort vars");

        // Radix Sort
        // ----------

        if ( (rc = RadixSortIndex ( st_bijection, st_index, N, 16, 0, st_info->verbose)) ) return(rc);
        if ( st_info->benchmark ) sf_running_timer (&timer, "\tSort (2): Sorted bijection");
        free (st_bijection);

        // Read in rest of data
        // --------------------

        for (i = 0; i < N; i++) {
            for (k = ksort; k < kvars; k++) {
                sel = st_index[i] * krest + k - ksort;
                if ( (ilen = ltypes[k]) ) {
                    if ( (rc = SF_sdata(1 + k, i + in1, st_dtax[sel].cval)) ) return(rc);
                }
                else {
                    if ( (rc = SF_vdata(1 + k, i + in1, &(st_dtax[sel].dval))) ) return(rc);
                }
            }
        }
        if ( st_info->benchmark ) sf_running_timer (&timer, "\tSort (1): Read in rest of data");

        // Write back the data using index
        // -------------------------------

        for (i = 0; i < N; i++) {
            for (k = 0; k < ksort; k++) {
                sel = st_index[i] * ksort + k;
                if ( (rc = SF_vstore(1 + k, i + in1, st_int[sel])) ) return(rc);
            }

            for (k = ksort; k < kvars; k++) {
                sel = i * krest + k - ksort;
                if ( ltypes[k] ) {
                    if ( (rc = SF_sstore(1 + k, i + in1, st_dtax[sel].cval)) ) return(rc);
                }
                else {
                    if ( (rc = SF_vstore(1 + k, i + in1, st_dtax[sel].dval)) ) return(rc);
                }
            }
        }
        if ( st_info->benchmark ) sf_running_timer (&timer, "\tSort (3): Wrote back sorted data");
        free (st_int);
        free (st_index);

        // Cleanup
        // -------

        for (i = 0; i < N; i++)
            for (k = 0; k < krest; k++)
                if ( ltypes[k + ksort] )
                    free(st_dtax[i * krest + k].cval);
    }
    else {

        if ( (st_info->qsort == 0) & (st_info->kvars_sort_str == 0) ) {

            // Read in double sort vars
            // ------------------------

            double *st_double = calloc(N * (ksort + 1), sizeof(*st_double));
            if ( st_double == NULL ) return (sf_oom_error("sf_msort", "st_double"));
            for (i = 0; i < N; i++) {
                for (k = 0; k < ksort; k++) {
                    sel = i * (ksort + 1) + k;
                    if ( (rc = SF_vdata(1 + k, i + in1, &(st_double[sel]))) ) return(rc);
                }
                st_double[i * (ksort + 1) + ksort] = i;
            }
            if ( st_info->benchmark ) sf_running_timer (&timer, "\tSort (1): Read in sort vars");

            MultiQuicksort2 (st_double, N, 0, ksort - 1, (ksort + 1) * sizeof(*st_double), ltypes);
            if ( st_info->benchmark ) sf_running_timer (&timer, "\tSort (2): Sorted numeric array");

            // Read in rest of data
            // --------------------

            size_t st_map;
            for (i = 0; i < N; i++) {
                st_map = (size_t) st_double[i * (ksort + 1) + ksort];
                for (k = ksort; k < kvars; k++) {
                    sel = st_map * krest + k - ksort;
                    if ( (ilen = ltypes[k]) ) {
                        if ( (rc = SF_sdata(1 + k, i + in1, st_dtax[sel].cval)) ) return(rc);
                    }
                    else {
                        if ( (rc = SF_vdata(1 + k, i + in1, &(st_dtax[sel].dval))) ) return(rc);
                    }
                }
            }
            if ( st_info->benchmark ) sf_running_timer (&timer, "\tSort (1): Read in rest of data");

            // Write the data
            // --------------

            for (i = 0; i < N; i++) {
                for (k = 0; k < ksort; k++) {
                    sel = i * (ksort + 1) + k;
                    if ( (rc = SF_vstore(1 + k, i + in1, st_double[sel])) ) return(rc);
                }

                for (k = ksort; k < kvars; k++) {
                    sel = i * krest + k - ksort;
                    if ( ltypes[k] ) {
                        if ( (rc = SF_sstore(1 + k, i + in1, st_dtax[sel].cval)) ) return(rc);
                    }
                    else {
                        if ( (rc = SF_vstore(1 + k, i + in1, st_dtax[sel].dval)) ) return(rc);
                    }
                }
            }
            if ( st_info->benchmark ) sf_running_timer (&timer, "\tSort (3): Wrote back sorted data");

            free (st_double);

            // Cleanup
            // -------

            for (i = 0; i < N; i++)
                for (k = 0; k < krest; k++)
                    if ( ltypes[k + ksort] )
                        free(st_dtax[i * krest + k].cval);
        }
        else {

            // Read in for mixed sort types
            // ----------------------------

            for (i = 0; i < N; i++) {
                for (k = 0; k < kvars; k++) {
                    sel = i * kvars + k;
                    if ( (ilen = ltypes[k]) ) {
                        if ( (rc = SF_sdata(1 + k, i + in1, st_dtax[sel].cval)) ) return(rc);
                    }
                    else {
                        if ( (rc = SF_vdata(1 + k, i + in1, &(st_dtax[sel].dval))) ) return(rc);
                    }
                }
            }
            if ( st_info->benchmark ) sf_running_timer (&timer, "\tSort (1): Read in copy of data");

            // Sort the data
            // -------------

            MultiQuicksort (st_dtax, N, 0, ksort - 1, kvars * sizeof(*st_dtax), ltypes);
            /* MixedQuicksort (st_dtax, 0, N, 0, ksort - 1, kvars, kvars * sizeof(*st_dtax), ltypes); */
            if ( st_info->benchmark ) sf_running_timer (&timer, "\tSort (2): Sorted array");

            // Write the data
            // --------------

            for (i = 0; i < N; i++) {
                for (k = 0; k < kvars; k++) {
                    sel = i * kvars + k;
                    if ( ltypes[k] ) {
                        if ( (rc = SF_sstore(1 + k, i + in1, st_dtax[sel].cval)) ) return(rc);
                    }
                    else {
                        if ( (rc = SF_vstore(1 + k, i + in1, st_dtax[sel].dval)) ) return(rc);
                    }
                }
            }
            if ( st_info->benchmark ) sf_running_timer (&timer, "\tSort (3): Wrote back sorted data");

            // Cleanup
            // -------

            for (i = 0; i < N; i++)
                for (k = 0; k < kvars; k++)
                    if ( ltypes[k] )
                        free(st_dtax[i * kvars + k].cval);
        }
    }

    free (st_dtax);
    free (ltypes);

    return (0);
}
