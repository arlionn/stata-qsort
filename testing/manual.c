// gcc -Wall -O2 manual.c -o manual; ./manual

#define _GNU_SOURCE
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "quicksort.c"

#define COMP(a, b) ( ( (a) > (b) ) - ( (a) < (b) ) )

#define COMPSTR(a, b) ( strcmp(a, b) )

struct MixedArray {
    char  **str_dta;
    double *num_dta;
};

struct MixedInfo {
    size_t *sort_info;
    int K;
};

int mf_compare_test  (const void *a, const void *b, void *arg);
int mf_compare  (const void *a, const void *b, void *arg);
int mf_compare1 (const void *a, const void *b, void *arg);
int mf_compare2 (const void *a, const void *b, void *arg);
int mf_comparek (const void *a, const void *b, void *arg);
int mf_comparez (const void *a, const void *b, void *arg);

size_t mf_min_unsigned(size_t x[], size_t N);
size_t mf_max_unsigned(size_t x[], size_t N);

int mf_compare_mixed (const void *a, const void *b, void *arg);

int sf_oom_error (char * step_desc, char * obj_desc);
void sf_running_timer (clock_t *timer, const char *msg);

int main()
{
    int i, k; int K = 6, N = 20000000;
    // int i, k; int K = 6, N = 20;

    /*********************************************************************
     *                          Mixed Array Foo                          *
     *********************************************************************/

    size_t kvars = 5; size_t sortvars = 3;
    struct MixedInfo st_info;
    st_info.sort_info = calloc(kvars, sizeof(*st_info.sort_info));
    if ( st_info.sort_info == NULL ) return (sf_oom_error("sf_gsort", "st_dta"));
    
    struct MixedArray *st_dta = calloc(N, sizeof(*st_dta));
    if ( st_dta == NULL ) return (sf_oom_error("sf_gsort", "st_dta"));

    st_info.sort_info[0] = 0;
    st_info.sort_info[1] = 10;
    st_info.sort_info[2] = 0;
    st_info.sort_info[3] = 23;
    st_info.sort_info[4] = 12;

    size_t kmin = mf_min_unsigned(st_info.sort_info, kvars);
    size_t kmax = mf_max_unsigned(st_info.sort_info, kvars);

    size_t inum, istr;
    size_t knum = 0;
    size_t kstr = 0;
    size_t lstr = 0;
    for (k = 0; k < kvars; k++) {
        st_info.sort_info[k]? ++kstr: ++knum;
        lstr += st_info.sort_info[k];
    }

    char *s; s = malloc(kmax * sizeof(char));

    clock_t stimer = clock();
    if ( kmax > 0 ) {
        if ( kmin == 0 ) {
            for (i = 0; i < N; i++) {
                inum = istr = 0;
                st_dta[i].num_dta = calloc(knum, sizeof(*st_dta[i].num_dta));
                st_dta[i].str_dta = calloc(kstr, sizeof(*st_dta[i].str_dta));
                for (k = 0; k < kvars; k++) {
                    if ( st_info.sort_info[k] ) {
                        st_dta[i].str_dta[istr] = malloc(st_info.sort_info[k] * sizeof(char));
                        memset (st_dta[i].str_dta[istr], '\0', st_info.sort_info[k]);
                        memset (s, '\0', kmax);
                        strcat(s, (i < (N / 2))? "sometimes": "amazing!!");
                        memcpy (st_dta[i].str_dta[istr], s, strlen(s));
                        istr++;
                    }
                    else {
                        // st_dta[i].num_dta[inum] = i + i * k % (k + 2);
                        st_dta[i].num_dta[inum] = (N - i) % 3;
                        inum++;
                    }
                }
            }
        }
    }
    sf_running_timer (&stimer, "\tMixedArray");

    clock_t mtimer = clock();
    for (k = 0; k < sortvars; k++) {
        st_info.K = k;
        quicksort(st_dta, N, sizeof(struct MixedArray), mf_compare_mixed, &st_info);
        printf("\t\t(%d): ", k); sf_running_timer (&stimer, "BSD qsort");
    }
    sf_running_timer (&mtimer, "\tSorted MixedArray");

    for (i = 0; i < N; i++) {
        for (k = 0; k < kstr; k++) {
            free(st_dta[i].str_dta[k]);
        }
        free(st_dta[i].str_dta);
        free(st_dta[i].num_dta);
    }
    free(st_dta);
    sf_running_timer (&mtimer, "\tFreed MixedArray");

    return (0);

    // for (i = 0; i < N; i++) {
    //     inum = istr = 0;
    //     for (k = 0; k < kvars; k++) {
    //         if ( st_info.sort_info[k] ) {
    //             printf("\t%s", st_dta[i].str_dta[istr++]);
    //         }
    //         else {
    //             printf("\t%.1f", st_dta[i].num_dta[inum++]);
    //         }
    //     }
    //     printf("\n");
    // }

    /*********************************************************************
     *                            Numeric Foo                            *
     *********************************************************************/

    double *to_msort = calloc(N * K, sizeof(*to_msort));
    double *to_qsort = calloc(N * K, sizeof(*to_qsort));
    for (i = 0; i < N; i++){
        for (k = 0; k < K; k++) {
            to_msort[i * K + k] = i + i * k % (k + 2);
            to_qsort[i * K + k] = i + i * k % (k + 2);
        }
    }

    clock_t  timer = clock();
    clock_t ptimer = clock();

    for (k = 1; k < K; k++) {
        quicksort(to_msort, N, K * sizeof(double), mf_comparek, &k);
        printf("\t\t(%d): ", k); sf_running_timer (&ptimer, "BSD qsort");
    }
    sf_running_timer (&timer, "\tBSD qsort");

    for (k = 0; k < K; k++) {
        qsort_r(to_qsort, N, K * sizeof(double), mf_comparek, &k);
        printf("\t\t(%d): ", k); sf_running_timer (&ptimer, "built-in qsort");
    }
    sf_running_timer (&timer, "\tbuilt-in qsort");

    int unsorted = 0;
    for (i = 1; i < N; i++) {
        if ( (unsorted = (to_msort[(i - 1) * K] > to_msort[i * K])) ) break;
    }
    printf("\t\t(%d): ", unsorted); sf_running_timer (&ptimer, "checked it it was sorted");

    for (i = 0; i < N; i++){
        for (k = 0; k < K; k++){
            if ( to_msort[i * K + k] != to_qsort[i * K + k] ) {
                printf("\tERROR (%d, %d): %.3f vs %.3f \n", i, k, to_msort[i * K + k], to_qsort[i * K + k]);
            }
        }
    }

    free (to_msort);
    free (to_qsort);
}

int mf_compare_test (const void *a, const void *b, void *arg)
{
    return (0);
}

int mf_compare (const void *a, const void *b, void *arg)
{
    return COMP(*(double *)a, *(double *)b);
}

int mf_compare1 (const void *a, const void *b, void *arg)
{
    int result;
    if ( (result = COMP(*(double *)a, *(double *)b)) )
        return (result);
    else
        return COMP(*((double *)a + 1), *((double *)b + 1));
}

int mf_compare2 (const void *a, const void *b, void *arg)
{
    int result;
    if ( (result = COMP(*(double *)a, *(double *)b)) )
        return (result);
    else {
        if ( (result = COMP(*((double *)a + 1), *((double *)b + 1))) )
            return (result);
        else
            return COMP(*((double *)a + 2), *((double *)b + 2));
    }
}

int mf_comparek (const void *a, const void *b, void *arg)
{
    int K = *(int *) arg, i = 0, result;
    for (i = 0; i < K; i++) {
        if ( (result = COMP(*((double *)a + i), *((double *)b + i))) ) return (result);
    }
    return COMP(*((double *)a + K), *((double *)b + K));
}

int mf_comparez (const void *a, const void *b, void *arg)
{
    int Z = *(int *) arg, i = 0;
    for (i = 0; i < Z; i++) {
        if ( COMP(*((double *)a + i), *((double *)b + i)) ) return (0);
    }
    return COMP(*((double *)a + Z), *((double *)b + Z));
}

int mf_compare_mixed (const void *a, const void *b, void *arg)
{
    struct MixedInfo  *st_info  = (struct MixedInfo  *)arg;
    struct MixedArray *st_row_a = (struct MixedArray *)a;
    struct MixedArray *st_row_b = (struct MixedArray *)b;
    int istr = 0, inum = 0, k, result;
    for (k = 0; k < st_info->K; k++) {
        if ( st_info->sort_info[k] ) {
            if ( (result = COMPSTR(st_row_a->str_dta[istr], st_row_b->str_dta[istr])) ) return (result);
            istr++;
        }
        else {
            if ( (result = COMP(st_row_a->num_dta[inum], st_row_b->num_dta[inum])) ) return (result);
            inum++;
        }
    }
    if ( st_info->sort_info[st_info->K] ) {
        return COMPSTR(st_row_a->str_dta[istr], st_row_b->str_dta[istr]);
    }
    else {
        return COMP(st_row_a->num_dta[inum], st_row_b->num_dta[inum]);
    }
}

/*********************************************************************
 *                         Helper functions                          *
 *********************************************************************/

void sf_running_timer (clock_t *timer, const char *msg)
{
    double diff  = (double) (clock() - *timer) / CLOCKS_PER_SEC;
    printf ("%s; %.3f seconds.\n", msg, diff);
    *timer = clock();
}

/**
 * @brief Wrapper for OOM error exit message
 */
int sf_oom_error (char * step_desc, char * obj_desc)
{
    printf ("%s: Unable to allocate memory for object '%s'.\n", step_desc, obj_desc);
    /* SF_display ("See {help gcollapse##memory:help gcollapse (Out of memory)}.\n"); */
    return (42002);
}

/**
 * @brief Minimum for unsigned integer array
 *
 * @param x vector of integers to get the min
 * @param N number of elements in @x
 * @return Smallest integer in @x
 */
size_t mf_min_unsigned(size_t x[], size_t N)
{
    size_t min = x[0]; size_t i;
    for (i = 1; i < N; ++i) {
        if (min > x[i]) min = x[i];
    }
    return (min);
}

/**
 * @brief Maximum for unsigned integer array
 *
 * @param x vector of integers to get the max
 * @param N number of elements in @x
 * @return Smallest integer in @x
 */
size_t mf_max_unsigned(size_t x[], size_t N)
{
    size_t max = x[0]; size_t i;
    for (i = 1; i < N; ++i) {
        if (max < x[i]) max = x[i];
    }
    return (max);
}
