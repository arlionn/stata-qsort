#include "qsort_generic.h"
// #include "quicksort_custom.c"
// #include "quicksort_testing.c"
#include "quicksort_testingx.c"

int sf_msort(struct StataInfo *st_info)
{
    // Setup
    // -----

    int i, k; // l;
    struct MixedInfo  sort_info;

    size_t N     = st_info->N;
    size_t in1   = st_info->in1;
    size_t ksort = st_info->kvars_sort;
    size_t krest = st_info->kvars_rest;
    size_t kstr  = st_info->kvars_sort_str + st_info->kvars_rest_str;
    size_t knum  = st_info->kvars_sort_num + st_info->kvars_rest_num;
    size_t kvars = knum + kstr;

    sort_info.ltypes = calloc(kvars, sizeof(*sort_info.ltypes));
    sort_info.lmap   = calloc(kvars, sizeof(*sort_info.lmap));

    if ( sort_info.ltypes == NULL ) return (sf_oom_error("sf_msort", "sort_info.ltypes"));
    if ( sort_info.lmap   == NULL ) return (sf_oom_error("sf_msort", "sort_info.lmap"));

    int ilen;
    for (k = 0; k < ksort; k++) {
        ilen = st_info->sortvars_lens[k];
        sort_info.ltypes[k] = ilen > 0? ilen: 0;
    }

    for (k = 0; k < krest; k++) {
        ilen = st_info->restvars_lens[k];
        sort_info.ltypes[k + ksort] = ilen > 0? ilen: 0;
    }

    size_t inum = 0, istr = 0;
    for (k = 0; k < kvars; k++)
        sort_info.lmap[k] = sort_info.ltypes[k]? istr++: inum++;

    // Read in the data
    // ----------------

    ST_retcode rc ;
    ST_double  z ;
    clock_t timer = clock();

    size_t kmax = mf_max_unsigned(sort_info.ltypes, kvars);
    char *s; s = malloc(kmax * sizeof(char));

    /*********************************************************************
     *                              Testing                              *
     *********************************************************************/
    
    size_t sel;
    MixedUnion *st_dtax;

    // struct {
    //     enum { is_double, is_char } type;
    //     union {
    //         float dval;
    //         char* cval;
    //     } val;
    // } *st_dtax;

    st_dtax = calloc(N * kvars, sizeof(*st_dtax));
    if ( st_dtax == NULL ) return (sf_oom_error("sf_msort", "st_dtax"));

    for (i = 0; i < N; i++) {
        for (k = 0; k < kvars; k++) {
            sel = i * kvars + k;
            if ( (ilen = sort_info.ltypes[k]) ) {
                // st_dtax[sel].type = is_char;
                st_dtax[sel].cval = malloc(ilen * sizeof(char));
                memset (st_dtax[sel].cval, '\0', ilen);
                memset (s, '\0', kmax);
                if ( (rc = SF_sdata(1 + k, i + in1, s)) ) return(rc);
                memcpy (st_dtax[sel].cval, s, strlen(s));
            }
            else {
                if ( (rc = SF_vdata(1 + k, i + in1, &z)) ) return(rc);
                // st_dtax[sel].type = is_double;
                st_dtax[sel].dval = z;
            }
        }
    }
    if ( st_info->benchmark ) sf_running_timer (&timer, "\tSort (1): Read in copy of data");

    /*********************************************************************
     *                              Testing                              *
     *********************************************************************/

    //  struct MixedArray *st_dta;
    // st_dta  = calloc(N, sizeof(*st_dta));
    // if ( st_dta == NULL ) return (sf_oom_error("sf_msort", "st_dta"));
    //
    // for (i = 0; i < N; i++) {
    //     st_dta[i].num_dta = calloc(knum, sizeof(*st_dta[i].num_dta));
    //     st_dta[i].str_dta = calloc(kstr, sizeof(*st_dta[i].str_dta));
    //     if ( st_dta[i].num_dta == NULL ) return (sf_oom_error("sf_msort", "st_dta.num_dta"));
    //     if ( st_dta[i].str_dta == NULL ) return (sf_oom_error("sf_msort", "st_dta.str_dta"));
    //
    //     for (k = 0; k < kvars; k++) {
    //         l = sort_info.lmap[k];
    //         if ( (ilen = sort_info.ltypes[k]) ) {
    //             st_dta[i].str_dta[l] = malloc(ilen * sizeof(char));
    //             memset (st_dta[i].str_dta[l], '\0', (ilen + kstr));
    //             memset (s, '\0', kmax);
    //             if ( (rc = SF_sdata(1 + k, i + in1, s)) ) return(rc);
    //             memcpy (st_dta[i].str_dta[l], s, strlen(s));
    //         }
    //         else {
    //             if ( (rc = SF_vdata(1 + k, i + in1, &z)) ) return(rc);
    //             st_dta[i].num_dta[l] = z;
    //         }
    //     }
    // }
    // if ( st_info->benchmark ) sf_running_timer (&timer, "\tSort (1): Read in copy of data");

    // Sort the data
    // -------------

    // MixedQuicksort (st_dtax, 0, N, 0, ksort - 1, sizeof(struct MixedArray), &sort_info);
    MixedQuicksort (st_dtax, 0, N, 0, ksort - 1, kvars, sizeof(MixedUnion), &sort_info);
    if ( st_info->benchmark ) sf_running_timer (&timer, "\tSort (2): Sorted array");

    // Write the data
    // --------------

    for (i = 0; i < N; i++) {
        for (k = 0; k < kvars; k++) {
            sel = i * kvars + k;
            if ( sort_info.ltypes[k] ) {
                if ( (rc = SF_sstore(1 + k, i + in1, st_dtax[sel].cval)) ) return(rc);
                free(st_dtax[sel].cval);
            }
            else {
                if ( (rc = SF_vstore(1 + k, i + in1, st_dtax[sel].dval)) ) return(rc);
            }
        }
    }
    if ( st_info->benchmark ) sf_running_timer (&timer, "\tSort (3): Wrote back sorted data");

    // for (i = 0; i < N; i++) {
    //     for (k = 0; k < kvars; k++) {
    //         l = sort_info.lmap[k];
    //         if ( sort_info.ltypes[k] ) {
    //             if ( (rc = SF_sstore(1 + k, i + in1, st_dta[i].str_dta[l])) ) return(rc);
    //         }
    //         else {
    //             if ( (rc = SF_vstore(1 + k, i + in1, st_dta[i].num_dta[l])) ) return(rc);
    //         }
    //     }
    //
    //     for (k = 0; k < kstr; k++)
    //         free (st_dta[i].str_dta[k]);
    //
    //     free (st_dta[i].num_dta);
    //     free (st_dta[i].str_dta);
    // }
    // free (st_dta);
    // if ( st_info->benchmark ) sf_running_timer (&timer, "\tSort (3): Wrote back sorted data");

    free (st_dtax);
    free (sort_info.ltypes);
    free (sort_info.lmap);

    return (0);
}

int mf_compare_mixed (const void *a, const void *b, void *arg)
{
    struct MixedInfo  *sort_info = (struct MixedInfo  *)arg;
    struct MixedArray *st_row_a  = (struct MixedArray *)a;
    struct MixedArray *st_row_b  = (struct MixedArray *)b;
    int k, l, result;
    for (k = 0; k < sort_info->K; k++) {
        l = sort_info->lmap[k];
        if ( sort_info->ltypes[k] ) {
            if ( (result = COMPSTR(st_row_a->str_dta[l], st_row_b->str_dta[l])) ) return (result);
        }
        else {
            if ( (result = COMP(st_row_a->num_dta[l], st_row_b->num_dta[l])) ) return (result);
        }
    }
    l = sort_info->lmap[sort_info->K];
    if ( sort_info->ltypes[sort_info->K] ) {
        return COMPSTR(st_row_a->str_dta[l], st_row_b->str_dta[l]);
    }
    else {
        return COMP(st_row_a->num_dta[l], st_row_b->num_dta[l]);
    }
}
