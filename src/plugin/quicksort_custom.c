#include <sys/cdefs.h>
#include <stdlib.h>
#include <string.h>

typedef int          cmp_t(const void *, const void *, void *);
static inline char * med3(char *, char *, char *, cmp_t *, void *);
static inline void   swapfunc(char *, char *, int, int);

// static void CustomSwapMixed (void *x, void *y, size_t l);

void CustomQuicksort (
    struct MixedArray *st_dta,
    size_t start,
    size_t end,
    size_t elsize,
    struct MixedInfo *sort_info
);

int CustomMed3(
    struct MixedArray *st_dta, 
    int a,
    int b,
    int c,
    struct MixedInfo *sort_info
);

int CustomCompareMixed (
    struct MixedArray a,
    struct MixedArray b,
    struct MixedInfo *sort_info
);

#ifndef min
#define min(a, b) (a) < (b) ? a : b
#endif

#define swapcode(TYPE, parmi, parmj, n) { \
    long i = (n) / sizeof (TYPE);         \
    TYPE *pi = (TYPE *) (parmi);          \
    TYPE *pj = (TYPE *) (parmj);          \
    do {                                  \
        TYPE    t = *pi;                  \
        *pi++ = *pj;                      \
        *pj++ = t;                        \
        } while (--i > 0);                \
}

#define SWAPINIT(a, es) swaptype = ((char *)a - (char *)0) % sizeof(long) || \
es % sizeof(long) ? 2 : es == sizeof(long)? 0 : 1;

static inline void
swapfunc(a, b, n, swaptype)
    char *a, *b;
    int n, swaptype;
{
    if (swaptype <= 1)
        swapcode(long, a, b, n)

    else
        swapcode(char, a, b, n)
}

#define swap(a, b)                   \
    if (swaptype == 0) {             \
        long t = *(long *)(a);       \
        *(long *)(a) = *(long *)(b); \
        *(long *)(b) = t;            \
    } else                           \
        swapfunc(a, b, es, swaptype)

#define vecswap(a, b, n) if ((n) > 0) swapfunc(a, b, n, swaptype)

#define CMP(t, x, y) (cmp((x), (y), (t)))

static inline char *
med3(char *a, char *b, char *c, cmp_t *cmp, void *thunk)
{
    return  CMP(thunk, a, b) < 0 ?
           (CMP(thunk, b, c) < 0 ? b : (CMP(thunk, a, c) < 0 ? c : a ))
          :(CMP(thunk, b, c) > 0 ? b : (CMP(thunk, a, c) < 0 ? a : c ));
}

void
quicksort (void *a, size_t n, size_t es, cmp_t *cmp, void *thunk)
{
    char *pa, *pb, *pc, *pd, *pl, *pm, *pn;
    size_t d, r;
    int cmp_result;
    int swaptype, swap_cnt;

loop:
    SWAPINIT(a, es);
    swap_cnt = 0;
    if (n < 7) {
        for (pm = (char *)a + es; pm < (char *)a + n * es; pm += es)
            for (pl = pm;
                 pl > (char *)a && CMP(thunk, pl - es, pl) > 0;
                 pl -= es)
                swap(pl, pl - es);
        return;
    }

    pm = (char *)a + (n / 2) * es;
    if (n > 7) {
        pl = a;
        pn = (char *)a + (n - 1) * es;
        if (n > 40) {
            d = (n / 8) * es;
            pl = med3(pl, pl + d, pl + 2 * d, cmp, thunk);
            pm = med3(pm - d, pm, pm + d, cmp, thunk);
            pn = med3(pn - 2 * d, pn - d, pn, cmp, thunk);
        }
        pm = med3(pl, pm, pn, cmp, thunk);
    }
    swap(a, pm);

    pa = pb = (char *)a + es;
    pc = pd = (char *)a + (n - 1) * es;

    for (;;) {
        while (pb <= pc && (cmp_result = CMP(thunk, pb, a)) <= 0) {
            if (cmp_result == 0) {
                swap_cnt = 1;
                swap(pa, pb);
                pa += es;
            }
            pb += es;
        }
        while (pb <= pc && (cmp_result = CMP(thunk, pc, a)) >= 0) {
            if (cmp_result == 0) {
                swap_cnt = 1;
                swap(pc, pd);
                pd -= es;
            }
            pc -= es;
        }
        if (pb > pc)
            break;
        swap(pb, pc);
        swap_cnt = 1;
        pb += es;
        pc -= es;
    }
    if (swap_cnt == 0) {  /* Switch to insertion sort */
        for (pm = (char *)a + es; pm < (char *)a + n * es; pm += es)
            for (pl = pm;
                 pl > (char *)a && CMP(thunk, pl - es, pl) > 0;
                 pl -= es)
                swap(pl, pl - es);
        return;
    }

    pn = (char *)a + n * es;
    r = min(pa - (char *)a, pb - pa);
    vecswap(a, pb - r, r);

    r = min(pd - pc, pn - pd - es);
    vecswap(pb, pn - r, r);

    if ((r = pb - pa) > es)
        quicksort(a, r / es, es, cmp, thunk);

    if ((r = pd - pc) > es) {
        /* Iterate rather than recurse to save stack space */
        a = pn - r;
        n = r / es;
        goto loop;
    }
}

/*********************************************************************
 *                      Custom Sorting! #Speed                       *
 *********************************************************************/

#define CustomCompareNum(a, b) ( ( (a) > (b) ) - ( (a) < (b) ) )
#define CustomCompareChar(a, b) ( strcmp(a, b) )

#define CustomSwapMixed(a, b, n) { \
    long i = (n) / sizeof (char);  \
    char *pi = (char *) (a);       \
    char *pj = (char *) (b);       \
    char pt;                       \
    do {                           \
        pt = *pi;                  \
        *pi++    = *pj;            \
        *pj++    = pt;             \
    } while (--i > 0);             \
}


void CustomQuicksort (
    struct MixedArray *st_dta,
    size_t start,
    size_t N,
    size_t elsize,
    struct MixedInfo *sort_info)
{
    int swap_count;
    int pa, pb, pc, pd;
    int pl, pm, pn;
    int cmp_result;
    size_t d, r, end;

loop:
    end = start + N;
    swap_count = 0;

    if ( N <= 7 ) {
        for (pm = start + 1; pm < end; pm++)
            for (pl = pm;
                 pl > start && CustomCompareMixed(st_dta[pl - 1], st_dta[pl], sort_info) > 0;
                 pl--)
                 CustomSwapMixed(&st_dta[pl], &st_dta[pl - 1], elsize);
        return;
    }

    pm = (int) (start + (N / 2));
    if ( N > 7 ) {
        pl = start;
        pn = end - 1;
        if ( N > 40 ) {
            d  = (int) (N / 8);
            pl = CustomMed3(st_dta, pl,         pl + d, pl + 2 * d, sort_info);
            pm = CustomMed3(st_dta, pm - d,     pm,     pm + d,     sort_info);
            pn = CustomMed3(st_dta, pn - 2 * d, pn - d, pn,         sort_info);
        }
        pm = CustomMed3(st_dta, pl, pm, pn, sort_info);
    }
    CustomSwapMixed(&st_dta[start], &st_dta[pm], elsize);

    pa = pb = start + 1;
    pc = pd = end - 1;

    for (;;) {
        while (pb <= pc && (cmp_result = CustomCompareMixed(st_dta[pb], st_dta[start], sort_info)) <= 0) {
            if ( cmp_result == 0 ) {
                swap_count = 1;
                CustomSwapMixed(&st_dta[pa], &st_dta[pb], elsize);
                ++pa;
            }
            ++pb;
        }

        while (pb <= pc && (cmp_result = CustomCompareMixed(st_dta[pc], st_dta[start], sort_info)) >= 0) {
            if ( cmp_result == 0 ) {
                swap_count = 1;
                CustomSwapMixed(&st_dta[pc], &st_dta[pd], elsize);
                --pd;
            }
            --pc;
        }

        if ( pb > pc )
            break;

        CustomSwapMixed(&st_dta[pb], &st_dta[pc], elsize);
        swap_count = 1;
        ++pb;
        --pc;
    }

    if ( swap_count == 0 ) {
        for (pm = start + 1; pm < end; pm++)
            for (pl = pm;
                 pl > start && CustomCompareMixed(st_dta[pl - 1], st_dta[pl], sort_info) > 0;
                 pl--)
                CustomSwapMixed(&st_dta[pl], &st_dta[pl - 1], elsize);
        return;
    }


    pn = end;
    r  = min(pa - start, pb - pa);
    if ( r > 0 ) CustomSwapMixed(&st_dta[start], &st_dta[pb - r], elsize * r);

    r = min(pd - pc, pn - pd - 1);
    if ( r > 0 ) CustomSwapMixed(&st_dta[pb], &st_dta[pn - r], elsize * r);

    if ( (r = pb - pa) > 1 )
        CustomQuicksort (st_dta, start, r, elsize, sort_info);

    if ( (r = pd - pc) > 1) {
        start = pn - r;
        N     = r;
        goto loop;
    }
}

int CustomCompareMixed (
    struct MixedArray a,
    struct MixedArray b,
    struct MixedInfo *sort_info)
{
    int k, l, result;
    for (k = 0; k < sort_info->K; k++) {
        l = sort_info->lmap[k];
        if ( sort_info->ltypes[k] > 0 ) {
            if ( (result = CustomCompareChar(a.str_dta[l], b.str_dta[l])) ) return (result);
        }
        else {
            if ( (result = CustomCompareNum(a.num_dta[l], b.num_dta[l])) ) return (result);
        }
    }
    l = sort_info->lmap[sort_info->K];
    if ( sort_info->ltypes[sort_info->K] > 0 ) {
        return CustomCompareChar(a.str_dta[l], b.str_dta[l]);
    }
    else {
        return CustomCompareNum(a.num_dta[l], b.num_dta[l]);
    }
}

int CustomMed3(
    struct MixedArray *st_dta, 
    int a,
    int b,
    int c,
    struct MixedInfo *sort_info)
{
    return  CustomCompareMixed(st_dta[a], st_dta[b], sort_info) < 0 ?
           (CustomCompareMixed(st_dta[b], st_dta[c], sort_info) < 0 ? b : (CustomCompareMixed(st_dta[a], st_dta[c], sort_info) < 0 ? c : a ))
          :(CustomCompareMixed(st_dta[b], st_dta[c], sort_info) > 0 ? b : (CustomCompareMixed(st_dta[a], st_dta[c], sort_info) < 0 ? a : c ));
}
