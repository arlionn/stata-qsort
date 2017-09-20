#include "qsort_generic.h"
// #include "quicksortMixedUnion.c"
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

    int ilen; size_t allbytes = 0;
    for (k = 0; k < ksort; k++) {
        ilen = st_info->sortvars_lens[k];
        if ( ilen > 0 ) {
            ltypes[k] = ilen;
            allbytes += (ltypes[k] * sizeof(char));
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
            allbytes += (ltypes[k + ksort] * sizeof(char));
        }
        else {
            ltypes[k + ksort] = 0;
            allbytes += sizeof(double);
        }
    }

    // Read in the data
    // ----------------

    ST_retcode rc ;
    ST_double  z ;
    clock_t timer = clock();

    size_t kmax = mf_max_unsigned(ltypes, kvars);
    char *s; s = malloc(kmax * sizeof(char));

    /*********************************************************************
     *                              Testing                              *
     *********************************************************************/

    // We do calloc because we can't always get a contiguous block of memory
    // for everything we want. So st_dtax + i is a pointer to the ith block of
    // size (allbytes + 2), which is contiguous and contains all our data!

    size_t sel;
    // void **st_dtax = calloc(N, (allbytes + 2) * sizeof(*st_dtax));
    //  MixedUnion *st_dtax = calloc(N * kvars, sizeof(*st_dtax))
    void **st_dtax = calloc(N * kvars, sizeof(*st_dtax));

    allbytes = 0;
    for (i = 0; i < N; i++) {
        for (k = 0; k < kvars; k++) {
            sel = i * kvars + k;
            if ( (ilen = ltypes[k]) ) {
                st_dtax[sel] = malloc(ilen * sizeof(char));
                allbytes += ilen * sizeof(char);
                memset (st_dtax[sel], '\0', ilen + 1);
            }
            else {
                st_dtax[sel] = malloc(sizeof(double));
                allbytes += sizeof(double);
            }
        }
    }

// printf("Hi; I'm here (%.2fMiB)\n",
//        (double) ((allbytes + N * kvars * sizeof(*st_dtax)) / 1024 / 1024));
//     sleep(6);
// printf("Hi; I'm there\n");
// return (231432);

    for (i = 0; i < N; i++) {
        for (k = 0; k < kvars; k++) {
            sel = i * kvars + k;
            if ( (ilen = ltypes[k]) ) {
                memset (s, '\0', kmax);
                if ( (rc = SF_sdata(1 + k, i + in1, s)) ) return(rc);
                memcpy (st_dtax[sel], s, strlen(s));
            }
            else {
                if ( (rc = SF_vdata(1 + k, i + in1, &z)) ) return(rc);
                memcpy ((double *)st_dtax[sel], &z, sizeof(double));
            }
        }
    }
    if ( st_info->benchmark ) sf_running_timer (&timer, "\tSort (1): Read in copy of data");

    /*********************************************************************
     *                              Testing                              *
     *********************************************************************/

    // Sort the data
    // -------------

    /* MixedQuicksort (st_dtax, N, 0, ksort - 1, kvars * sizeof(*st_dtax), ltypes); */
    if ( st_info->benchmark ) sf_running_timer (&timer, "\tSort (2): Sorted array");

    // Write the data
    // --------------

    for (i = 0; i < N; i++) {
        for (k = 0; k < kvars; k++) {
            sel = i * kvars + k;
            if ( ltypes[k] ) {
                if ( (rc = SF_sstore(1 + k, i + in1, (char *)st_dtax[sel])) ) return(rc);
            }
            else {
                if ( (rc = SF_vstore(1 + k, i + in1, *((double *)st_dtax[sel]))) ) return(rc);
            }
        }
    }
    if ( st_info->benchmark ) sf_running_timer (&timer, "\tSort (3): Wrote back sorted data");

    free (st_dtax);
    free (ltypes);

    return (0);
}
