#include "qsort_generic.h"
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
    size_t kstr  = st_info->kvars_sort_str;
    size_t knum  = st_info->kvars_sort_num;
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

    /*********************************************************************
     *                          Allocate space                           *
     *********************************************************************/

    ST_retcode rc ;
    ST_double z ;
    size_t sel;
    clock_t timer = clock();

    double mib_dtax = N * (ksort + 1) * sizeof(MixedUnion) / 1024 / 1024;
    double mib_all  = mib_dtax + N * strbytes * sizeof(char) / 1024 / 1024;
    if ( st_info->verbose ) sf_printf("(memory overhead > %.2fMiB)\n", mib_all);

    /*********************************************************************
     *                         Read in the data                          *
     *********************************************************************/

    if ( st_info->integers_ok ) {

        // Integers are bijected to the natural numbers
        // --------------------------------------------

        size_t *st_bijection = calloc(N, sizeof(*st_bijection));
        size_t *st_index     = calloc(N, sizeof(*st_index));
        if ( st_bijection == NULL ) return (sf_oom_error("sf_msort", "st_bijection"));
        if ( st_index     == NULL ) return (sf_oom_error("sf_msort", "st_index"));

        int l;
        size_t offset = 1;
        size_t offsets[ksort];
        offsets[0] = 0;
        for (k = 0; k < ksort - 1; k++) {
            l = ksort - (k + 1);
            offset *= (st_info->sortvars_maxs[l] - st_info->sortvars_mins[l] + 1);
            offsets[k + 1] = offset;
        }

        // Read in the sort variables, which we know are integers
        for (i = 0; i < N; i++) {

            // If only one variable, it will just get adjusted by its range
            l = ksort - (0 + 1);
            if ( (rc = SF_vdata(1 + l, i + in1, &z)) ) return(rc);
            if ( st_info->invert[l] ) {
                if ( SF_is_missing(z) )
                    st_bijection[i] = 1;
                else
                    st_bijection[i] = st_info->sortvars_maxs[l] - z + 1;
            }
            else {
                if ( SF_is_missing(z) )
                    st_bijection[i] = st_info->sortvars_maxs[l] - st_info->sortvars_mins[l] + 1;
                else
                    st_bijection[i] = z - st_info->sortvars_mins[l] + 1;
            }

            // If multiple integers, they'll get mapped recursively
            for (k = 1; k < ksort; k++) {
                l   = ksort - (k + 1);
                if ( (rc = SF_vdata(1 + l, i + in1, &z)) ) return(rc);
                if ( st_info->invert[l] ) {
                    if ( SF_is_missing(z) )
                        st_bijection[i] += 0;
                    else
                        st_bijection[i] += (st_info->sortvars_maxs[l] - z) * offsets[k];
                }
                else {
                    if ( SF_is_missing(z) )
                        st_bijection[i] += (st_info->sortvars_maxs[l] - st_info->sortvars_mins[l]) * offsets[k];
                    else
                        st_bijection[i] += (z - st_info->sortvars_mins[l]) * offsets[k];
                }
            }
        }
        if ( st_info->benchmark ) sf_running_timer (&timer, "\tSort (1): Bijected sort integers to natural numbers");

        // Radix Sort
        // ----------

        if ( (rc = RadixSortIndex ( st_bijection, st_index, N, 16, 0, st_info->verbose)) ) return(rc);
        if ( st_info->benchmark ) sf_running_timer (&timer, "\tSort (2): Sorted bijection");
        free (st_bijection);

        /**************
         *  Debugging *
         **************/
        for (i = 0; i < N; i++) {
            if ( (rc = SF_vstore(ksort + 1, i + in1, st_index[i] + 1)) ) return(rc);
        }
        if ( st_info->benchmark ) sf_running_timer (&timer, "\tSort (3): Wrote back _sortindex");
        free (st_index);
        /**************
         *  Debugging *
         **************/

        /*
         *
        // Invert sort permutation for stata _sortindex
        // --------------------------------------------

        size_t *st_sortindex = calloc(N, sizeof(*st_sortindex));
        if ( st_sortindex == NULL ) return (sf_oom_error("sf_msort", "st_sortindex"));

        if ( (rc = RadixSortIndex ( st_index, st_sortindex, N, 16, 0, st_info->verbose)) ) return(rc);
        if ( st_info->benchmark ) sf_running_timer (&timer, "\tSort (3): Computed _sortindex permutation");
        free (st_index);

        // Write back sort index and let Stata handle the shuffle
        // ------------------------------------------------------

        for (i = 0; i < N; i++) {
            if ( (rc = SF_vstore(ksort + 1, i + in1, st_sortindex[i] + 1)) ) return(rc);
        }
        if ( st_info->benchmark ) sf_running_timer (&timer, "\tSort (4): Wrote back _sortindex permutation");
        free (st_sortindex);
         *
         */
    }
    else {

        if ( st_info->kvars_sort_str == 0 ) {

            // Read in double sort vars and sort
            // ---------------------------------

            double *st_double = calloc(N * (ksort + 1), sizeof(*st_double));
            if ( st_double == NULL ) return (sf_oom_error("sf_msort", "st_double"));
            for (i = 0; i < N; i++) {
                for (k = 0; k < ksort; k++) {
                    sel = i * (ksort + 1) + k;
                    if ( (rc = SF_vdata(1 + k, i + in1, &(st_double[sel]))) ) return(rc);
                }
                st_double[i * (ksort + 1) + ksort] = i + 1;
            }
            if ( st_info->benchmark ) sf_running_timer (&timer, "\tSort (1): Read in sort vars");

            MultiQuicksort2 (st_double, N, 0, ksort - 1, (ksort + 1) * sizeof(*st_double), ltypes, st_info->invert);
            if ( st_info->benchmark ) sf_running_timer (&timer, "\tSort (2): Sorted numeric array");

            /**************
             *  Debugging *
             **************/
            for (i = 0; i < N; i++) {
                if ( (rc = SF_vstore(ksort + 1, i + in1, st_double[i * (ksort + 1) + ksort])) ) return(rc);
            }
            free (st_double);
            if ( st_info->benchmark ) sf_running_timer (&timer, "\tSort (3): Wrote back _sortindex");
            /**************
             *  Debugging *
             **************/

            /*
             *
            // Invert sort permutation for stata _sortindex
            // --------------------------------------------

            size_t *st_index     = calloc(N, sizeof(*st_index));
            size_t *st_sortindex = calloc(N, sizeof(*st_sortindex));
            if ( st_sortindex == NULL ) return (sf_oom_error("sf_msort", "st_sortindex"));
            if ( st_index     == NULL ) return (sf_oom_error("sf_msort", "st_index"));

            for (i = 0; i < N; i++) {
                st_index[i] = st_double[i * (ksort + 1) + ksort];
            }
            free (st_double);

            if ( (rc = RadixSortIndex (st_index, st_sortindex, N, 16, 0, st_info->verbose)) ) return(rc);
            if ( st_info->benchmark ) sf_running_timer (&timer, "\tSort (3): Computed _sortindex permutation");
            free (st_index);

            // Write back sort index and let Stata handle the shuffle
            // ------------------------------------------------------

            for (i = 0; i < N; i++) {
                if ( (rc = SF_vstore(ksort + 1, i + in1, st_sortindex[i] + 1)) ) return(rc);
            }
            if ( st_info->benchmark ) sf_running_timer (&timer, "\tSort (4): Wrote back _sortindex permutation");
            free (st_sortindex);
             *
             */
        }
        else {

            /*********************************************************************
             *                              Testing                              *
             *********************************************************************/

            // char *s; s = malloc(strbytes * sizeof(char));
            size_t *positions = calloc(ksort + 1, sizeof(*positions));
            positions[0] = 0;
            for (k = 1; k < ksort + 1; k++) {
                ilen = st_info->sortvars_lens[k - 1];
                if ( ilen > 0 ) {
                    positions[k] = positions[k - 1] + (ilen + 1);
                }
                else {
                    positions[k] = positions[k - 1] + sizeof(double);
                }
            }

            double j;
            size_t rowbytes = allbytes + sizeof(size_t);
            char *st_charx  = calloc(N, rowbytes * sizeof(char));
            if ( st_charx == NULL ) return (sf_oom_error("sf_msort", "st_charx"));
            for (i = 0; i < N; i++) {
                memset (st_charx + i * rowbytes, '\0', rowbytes);
                sel = i * rowbytes + positions[ksort]; j = i + 1;
                memcpy (st_charx + sel, &j, sizeof(double));
            }

            size_t selrow;
            for (i = 0; i < N; i++) {
                selrow = i * rowbytes;
                for (k = 0; k < ksort; k++) {
                    sel = selrow + positions[k];
                    if ( (ilen = ltypes[k]) ) {
                        // memset (s, '\0', strbytes);
                        if ( (rc = SF_sdata(1 + k, i + in1, st_charx + sel)) ) return(rc);
                        // memcpy (st_charx + sel, s, strlen(s));
                    }
                    else {
                        if ( (rc = SF_vdata(1 + k, i + in1, &z)) ) return(rc);
                        memcpy (st_charx + sel, &z, sizeof(double));
                    }
                }
            }
            if ( st_info->benchmark ) sf_running_timer (&timer, "\tSort (1): Read in copy of sort variables");

            MultiQuicksort3 (st_charx, N, 0, ksort - 1, rowbytes * sizeof(char), ltypes, st_info->invert, positions);
            if ( st_info->benchmark ) sf_running_timer (&timer, "\tSort (2): Sorted array");

            for (i = 0; i < N; i++) {
                sel = i * rowbytes + positions[ksort];
                if ( (rc = SF_vstore(ksort + 1, i + in1, *(double *)(st_charx + sel))) ) return(rc);
            }
            if ( st_info->benchmark ) sf_running_timer (&timer, "\tSort (3): Wrote back _sortindex");

            free(st_charx);
            free(positions);

goto finish;

            /*********************************************************************
             *                              Testing                              *
             *********************************************************************/

            // Allocate memory
            // ---------------

            // It is leaner to use a union than an array of pointers or a structure
            // void **st_dtax = calloc(N * kvars, sizeof(*st_dtax));
            MixedUnion *st_dtax = calloc(N * (ksort + 1), sizeof(*st_dtax));
            if ( st_dtax == NULL ) return (sf_oom_error("sf_msort", "st_dtax"));

            // It is faster to allocate and zero all memory first
            for (i = 0; i < N; i++) {
                for (k = 0; k < ksort; k++) {
                    sel = i * (ksort + 1) + k;
                    if ( (ilen = ltypes[k]) ) {
                        st_dtax[sel].cval = malloc((ilen + 1) * sizeof(char));
                        if ( st_dtax[sel].cval == NULL ) return (sf_oom_error("sf_msort", "st_dtax[sel].cval"));
                        memset (st_dtax[sel].cval, '\0', ilen + 1);
                    }
                }
                st_dtax[i * (ksort + 1) + ksort].dval = i + 1;
            }

            // Read in for mixed sort types and sort
            // -------------------------------------

            for (i = 0; i < N; i++) {
                for (k = 0; k < ksort; k++) {
                    sel = i * (ksort + 1) + k;
                    if ( (ilen = ltypes[k]) ) {
                        if ( (rc = SF_sdata(1 + k, i + in1, st_dtax[sel].cval)) ) return(rc);
                    }
                    else {
                        if ( (rc = SF_vdata(1 + k, i + in1, &(st_dtax[sel].dval))) ) return(rc);
                    }
                }
            }
            if ( st_info->benchmark ) sf_running_timer (&timer, "\tSort (1): Read in copy of sort variables");

            MultiQuicksort (st_dtax, N, 0, ksort - 1, (ksort + 1) * sizeof(*st_dtax), ltypes, st_info->invert);
            if ( st_info->benchmark ) sf_running_timer (&timer, "\tSort (2): Sorted array");

            /**************
             *  Debugging *
             **************/
            for (i = 0; i < N; i++) {
                if ( (rc = SF_vstore(ksort + 1, i + in1, st_dtax[i * (ksort + 1) + ksort].dval)) ) return(rc);
            }
            for (i = 0; i < N; i++)
                for (k = 0; k < ksort; k++)
                    if ( ltypes[k] )
                        free(st_dtax[i * (ksort + 1) + k].cval);
            free (st_dtax);
            if ( st_info->benchmark ) sf_running_timer (&timer, "\tSort (3): Wrote back _sortindex");
            /**************
             *  Debugging *
             **************/

            /*
             *
            // Invert sort permutation for stata _sortindex
            // --------------------------------------------

            size_t *st_index     = calloc(N, sizeof(*st_index));
            size_t *st_sortindex = calloc(N, sizeof(*st_sortindex));
            if ( st_sortindex == NULL ) return (sf_oom_error("sf_msort", "st_sortindex"));
            if ( st_index     == NULL ) return (sf_oom_error("sf_msort", "st_index"));

            for (i = 0; i < N; i++) {
                st_index[i] = st_dtax[i * (ksort + 1) + ksort].dval;
            }

            for (i = 0; i < N; i++)
                for (k = 0; k < ksort; k++)
                    if ( ltypes[k] )
                        free(st_dtax[i * (ksort + 1) + k].cval);

            free (st_dtax);

            if ( (rc = RadixSortIndex (st_index, st_sortindex, N, 16, 0, st_info->verbose)) ) return(rc);
            if ( st_info->benchmark ) sf_running_timer (&timer, "\tSort (3): Computed _sortindex permutation");
            free (st_index);

            // Write back sort index and let Stata handle the shuffle
            // ------------------------------------------------------

            for (i = 0; i < N; i++) {
                if ( (rc = SF_vstore(ksort + 1, i + in1, st_sortindex[i] + 1)) ) return(rc);
            }
            if ( st_info->benchmark ) sf_running_timer (&timer, "\tSort (4): Wrote back _sortindex permutation");
            free (st_sortindex);
             *
             */
        }
    }


finish:
    free (ltypes);

    return (0);
}
