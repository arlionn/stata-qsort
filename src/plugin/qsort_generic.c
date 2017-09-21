#include "qsort_generic.h"
#include "quicksortMixedUnion.c"
#include "quicksortMultiLevel.c"
#include <unistd.h>

#define GenCopyMixed(a, b, n) { \
    long i = (n) / sizeof (char);  \
    char *pi = (char *) (a);       \
    char *pj = (char *) (b);       \
    do {                           \
        *pi++ = *pj++;             \
    } while (--i > 0);             \
}

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

    // Read in the data
    // ----------------

    ST_retcode rc ;
    clock_t timer = clock();

    /*********************************************************************
     *                              Testing                              *
     *********************************************************************/

    // We do calloc because we can't always get a contiguous block of memory
    // for everything we want. So st_dtax + i is a pointer to the ith block of
    // size (allbytes + 2), which is contiguous and contains all our data!

    size_t sel;
    // void **st_dtax = calloc(N, (allbytes + 2) * sizeof(*st_dtax));
    MixedUnion *st_dtax = calloc(N * kvars, sizeof(*st_dtax));
    //  void **st_dtax = calloc(N * kvars, sizeof(*st_dtax));

    for (i = 0; i < N; i++) {
        for (k = 0; k < kvars; k++) {
            sel = i * kvars + k;
            if ( (ilen = ltypes[k]) ) {
                st_dtax[sel].cval = malloc((ilen + 1) * sizeof(char));
                memset (st_dtax[sel].cval, '\0', ilen + 1);
            }
            // else {
            //     st_dtax[sel] = malloc(sizeof(double));
            //     allbytes += sizeof(double);
            // }
        }
    }

    for (i = 0; i < N; i++) {
        for (k = 0; k < kvars; k++) {
            sel = i * kvars + k;
            if ( (ilen = ltypes[k]) ) {
                // st_dtax[sel].cval = malloc((ilen + 1) * sizeof(char));
                // memset (st_dtax[sel].cval, '\0', ilen + 1);
                // memset (s, '\0', kmax);
                // if ( (rc = SF_sdata(1 + k, i + in1, s)) ) return(rc);
                // memcpy (st_dtax[sel].cval, s, strlen(s));
                if ( (rc = SF_sdata(1 + k, i + in1, st_dtax[sel].cval)) ) return(rc);
            }
            else {
                if ( (rc = SF_vdata(1 + k, i + in1, &(st_dtax[sel].dval))) ) return(rc);
                // if ( (rc = SF_vdata(1 + k, i + in1, &z)) ) return(rc);
                // st_dtax[sel].dval = z;
                // memcpy ((double *)st_dtax[sel], &z, sizeof(double));
            }
        }
        // if (i < 12) {
        //     printf("%d", i);
        //     for (k = 0; k < kvars; k++) {
        //         sel = i * kvars + k;
        //         if ( (ilen = ltypes[k]) ) {
        //             printf("\t%d (%lu is %s)", k, sel, st_dtax[sel].cval);
        //         }
        //         else 
        //             printf("\t%d (%lu is %.4f)", k, sel, st_dtax[sel].dval);
        //     }
        //     printf("\n");
        // }
    }

    double mib_dtax = N * kvars * sizeof(*st_dtax) / 1024 / 1024;
    double mib_all  = mib_dtax + N * strbytes * sizeof(char) / 1024 / 1024;
    if ( st_info->verbose ) sf_printf("(memory overhead > %.2fMiB)\n", mib_all);
    if ( st_info->benchmark ) sf_running_timer (&timer, "\tSort (1): Read in copy of data");

    /*********************************************************************
     *                              Testing                              *
     *********************************************************************/

    // Sort the data
    // -------------

    /* MultiQuicksort (st_dtax, N, 0, ksort - 1, kvars * sizeof(*st_dtax), ltypes); */
    MixedQuicksort (st_dtax, 0, N, 0, ksort - 1, kvars, kvars * sizeof(*st_dtax), ltypes);
    if ( st_info->benchmark ) sf_running_timer (&timer, "\tSort (2): Sorted array");

        /* Sort (1): Read in copy of data; 0.367 seconds. */
        /* Sort (2): Sorted array; 1.737 seconds. */
        /* Sort (3): Wrote back sorted data; 0.695 seconds. */

    // Write the data
    // --------------

    for (i = 0; i < N; i++) {
        for (k = 0; k < kvars; k++) {
            sel = i * kvars + k;
            if ( ltypes[k] ) {
                // if ( (rc = SF_sstore(1 + k, i + in1, (char *)st_dtax[sel])) ) return(rc);
                if ( (rc = SF_sstore(1 + k, i + in1, st_dtax[sel].cval)) ) return(rc);
                // free(st_dtax[sel].cval);
            }
            else {
                // if ( (rc = SF_vstore(1 + k, i + in1, *((double *)st_dtax[sel]))) ) return(rc);
                if ( (rc = SF_vstore(1 + k, i + in1, st_dtax[sel].dval)) ) return(rc);
            }
        }
    }
    if ( st_info->benchmark ) sf_running_timer (&timer, "\tSort (3): Wrote back sorted data");

    // for (i = 0; i < N; i++)
    //     for (k = 0; k < kvars; k++)
    //         if ( ltypes[k] )
    //             free(st_dtax[i * kvars + k].cval);

    free (st_dtax);
    free (ltypes);

    return (0);
}
