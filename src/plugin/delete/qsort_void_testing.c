#include <sys/cdefs.h>
#include <stdlib.h>
#include <string.h>

/*********************************************************************
 *                            Definitions                            *
 *********************************************************************/

void MixedQuicksort (
    void *start,
    size_t N,
    size_t kstart,
    size_t kend,
    size_t elsizeSwap,
    size_t *ltypes
);

void GenQuicksort (
    void *start,
    size_t N,
    size_t elsizeSwap,
    int (*GenCompare) (void *a, void *b, size_t kstart),
    size_t kstart
);

static inline char * GenMed3(
    char *a,
    char *b,
    char *c,
    int (*GenCompare) (void *a, void *b, size_t kstart),
    size_t kstart
);

int GenCompareChar (
    void *a,
    void *b,
    size_t kstart
);

int GenCompareNum (
    void *a,
    void *b,
    size_t kstart
);

#ifndef min
#define min(a, b) (a) < (b) ? a : b
#endif

#define BaseCompareNum(a, b) ( ( (a) > (b) ) - ( (a) < (b) ) )
#define BaseCompareChar(a, b) ( strcmp(a, b) )

#define GenSwapMixed(a, b, n) { \
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

/*********************************************************************
 *                      Custom Sorting! #Speed                       *
 *********************************************************************/

void GenQuicksort (
    void *start,
    size_t N,
    size_t elsizeSwap,
    int (*GenCompare) (void *a, void *b, size_t kstart),
    size_t kstart)
{

    char *pa, *pb, *pc, *pd;
    char *pl, *pm, *pn;
    char *end;

    int swap_count;
    int cmp_result;
    size_t d, r;
    size_t cutoff = 9;
    /* size_t elsize = 1; */
    size_t elsize = elsizeSwap;

loop:
    end = (char *)start + N * elsize;

    if ( N < cutoff ) {
        for (pm = (char *)start + elsize; pm < end; pm += elsize) {
            for (pl = pm;
                 pl > (char *)start && GenCompare(pl - elsize, pl, kstart) > 0;
                 pl -= elsize) {
                 GenSwapMixed(pl, pl - elsize, elsizeSwap);
            }
        }
        return;
    }

    pm = (char *)start + (N / 2) * elsize;
    if ( N > cutoff ) {
        pl = (char *)start;
        pn = (char *)start + (N - 1) * elsize;
        if ( N > 40 ) {
            d  = ((N / 8) * elsize);
            pl = GenMed3(pl,         pl + d, pl + 2 * d, GenCompare, kstart);
            pm = GenMed3(pm - d,     pm,     pm + d,     GenCompare, kstart);
            pn = GenMed3(pn - 2 * d, pn - d, pn,         GenCompare, kstart);
        }
        pm = GenMed3(pl, pm, pn, GenCompare, kstart);
    }
    GenSwapMixed(start, pm, elsizeSwap);

    pa = pb = (char *)start + elsize;
    pc = pd = (char *)start + (N - 1) * elsize;

    swap_count = 0;
    for (;;) {
        while (pb <= pc && (cmp_result = GenCompare(pb, start, kstart)) <= 0) {
            if ( cmp_result == 0 ) {
                swap_count = 1;
                GenSwapMixed(pa, pb, elsizeSwap);
                pa += elsize;
            }
            pb += elsize;
        }

        while (pb <= pc && (cmp_result = GenCompare(pc, start, kstart)) >= 0) {
            if ( cmp_result == 0 ) {
                swap_count = 1;
                GenSwapMixed(pc, pd, elsizeSwap);
                pd -= elsize;
            }
            pc -= elsize;
        }

        if ( pb > pc )
            break;

        GenSwapMixed(pb, pc, elsizeSwap);
        swap_count = 1;
        pb += elsize;
        pc -= elsize;
    }

    if ( swap_count == 0 ) {
        for (pm = (char *)start + elsize; pm < end; pm += elsize)
            for (pl = pm;
                 pl > (char *)start && GenCompare(pl - elsize, pl, kstart) > 0;
                 pl -= elsize)
                GenSwapMixed(pl, pl - elsize, elsizeSwap);
        return;
    }

    /* TODO: Write vecswap to work here // 2017-09-20 11:43 EDT */

    pn = end;
    r  = min(pa - (char *)start, pb - pa);
    if ( r > 0 ) GenSwapMixed(start, pb - r, r);

    r = min(pd - pc, pn - pd - elsize);
    if ( r > 0 ) GenSwapMixed(pb, pn - r, r);

    if ( (r = pb - pa) > elsize )
        GenQuicksort ((char *)start, r / elsize, elsize, GenCompare, kstart);

    if ( (r = pd - pc) > elsize ) {
        start = pn - r;
        N     = r / elsize;
        goto loop;
    }
}

/*********************************************************************
 *                              Helpers                              *
 *********************************************************************/

#define BaseCompareNum(a, b) ( ( (a) > (b) ) - ( (a) < (b) ) )
#define BaseCompareChar(a, b) ( strcmp(a, b) )
int GenCompareChar (void *a, void *b, size_t kstart)
{
    return BaseCompareChar(*(char **)(a + kstart), *(char **)(b + kstart));
}

int GenCompareNum (void *a, void *b, size_t kstart)
{
    return BaseCompareNum((*((double *)a + kstart)), (*((double *)b + kstart)));
}

static inline char * GenMed3(
    char *a,
    char *b,
    char *c,
    int (*GenCompare) (void *a, void *b, size_t kstart),
    size_t kstart)
{
    return  GenCompare(a, b, kstart) < 0 ?
           (GenCompare(b, c, kstart) < 0 ? b : (GenCompare(a, c, kstart) < 0 ? c : a ))
          :(GenCompare(b, c, kstart) > 0 ? b : (GenCompare(a, c, kstart) < 0 ? a : c ));
}

void MixedQuicksort (
    void *start,
    size_t N,
    size_t kstart,
    size_t kend,
    size_t elsizeSwap,
    size_t *ltypes)
{
    size_t j, k;
    short ischar;
    void *i, *end;
    size_t kmax = 0;
    /* size_t elsize = 1; */
    size_t elsize = elsizeSwap;

    // for (k = 0; k <= kend; k++)
    //     kmax += (ltypes[kstart] > 0)? ltypes[kstart]: 8;
    kmax = kend;

    GenQuicksort (
        start,
        N,
        elsize,
        ( (ischar = (ltypes[kstart] > 0)) )? GenCompareChar: GenCompareNum,
        kstart
    );

    if ( kstart >= kmax )
        return;

    end = start + N * elsize;

loop:

    j = 1;
    if ( ischar ) {
        for (i = start + elsize; i < end; i += elsize) {
            if ( GenCompareChar(i - elsize, i, kstart) ) break;
            j++;
        }
    }
    else {
        for (i = start + elsize; i < end; i += elsize) {
            if ( GenCompareNum(i - elsize, i, kstart) ) break;
            j++;
        }
    }

    if ( j > 1 ) {
        MixedQuicksort (
            start,
            j,
            /* kstart + (ischar? ltypes[kstart]: 8), */
            kstart + 1,
            kend,
            elsize,
            ltypes
        );
    }

    if ( (kstart < kmax) ) {
        start = i;
        if ( start < end )
            goto loop;
    }
}

/*********************************************************************
 *                           Array version                           *
 *********************************************************************/

/*
void GenQuicksort (
    void *st_dtax,
    size_t start,
    size_t end,
    size_t kvars,
    size_t elsize,
    int (*GenCompare) (void *a, void *b),
    size_t kstart
);

void GenQuicksort (
    void *st_dtax,
    size_t start,
    size_t N,
    size_t kvars,
    size_t elsize,
    int (*GenCompare) (void *a, void *b),
    size_t kstart)
{

    int swap_count;
    int pa, pb, pc, pd;
    int pl, pm, pn;
    int cmp_result;
    size_t d, r, end;
    size_t cutoff = 9;

loop:
    end = start + N * kvars;

    if ( N < cutoff ) {
        for (pm = start + kvars; pm < end; pm += kvars) {
            for (pl = pm;
                 pl > start && GenCompare(st_dtax[pl - kvars + kstart], st_dtax[pl + kstart]) > 0;
                 pl -= kvars) {
                 GenSwapMixed(&st_dtax[pl], &st_dtax[pl - kvars], elsize);
            }
        }
        return;
    }

    pm = start + ((int) (N / 2)) * kvars;
    if ( N > cutoff ) {
        pl = start;
        pn = end - kvars;
        if ( N > 40 ) {
            d  = ((int) (N / 8)) * kvars;
            pl = GenMed3(st_dtax, pl,         pl + d, pl + 2 * d, GenCompare, kstart);
            pm = GenMed3(st_dtax, pm - d,     pm,     pm + d,     GenCompare, kstart);
            pn = GenMed3(st_dtax, pn - 2 * d, pn - d, pn,         GenCompare, kstart);
        }
        pm = GenMed3(st_dtax, pl, pm, pn, GenCompare, kstart);
    }
    GenSwapMixed(&st_dtax[start], &st_dtax[pm], elsize);

    pa = pb = start + kvars;
    pc = pd = end - kvars;

    swap_count = 0;
    for (;;) {
        while (pb <= pc && (cmp_result = GenCompare(st_dtax[pb + kstart], st_dtax[start + kstart])) <= 0) {
            if ( cmp_result == 0 ) {
                swap_count = 1;
                GenSwapMixed(&st_dtax[pa], &st_dtax[pb], elsize);
                pa += kvars;
            }
            pb += kvars;
        }

        while (pb <= pc && (cmp_result = GenCompare(st_dtax[pc + kstart], st_dtax[start + kstart])) >= 0) {
            if ( cmp_result == 0 ) {
                swap_count = 1;
                GenSwapMixed(&st_dtax[pc], &st_dtax[pd], elsize);
                pd -= kvars;
            }
            pc -= kvars;
        }

        if ( pb > pc )
            break;

        GenSwapMixed(&st_dtax[pb], &st_dtax[pc], elsize);
        swap_count = 1;
        pb += kvars;
        pc -= kvars;
    }

    if ( swap_count == 0 ) {
        for (pm = start + kvars; pm < end; pm += kvars)
            for (pl = pm;
                 pl > start && GenCompare(st_dtax[pl - kvars + kstart], st_dtax[pl + kstart]) > 0;
                 pl -= kvars)
                GenSwapMixed(&st_dtax[pl], &st_dtax[pl - kvars], elsize);
        return;
    }

    pn = end;
    r  = min(pa - start, pb - pa);
    if ( r > 0 ) GenSwapMixed(&st_dtax[start], &st_dtax[pb - r], r * elsize / kvars);

    r = min(pd - pc, pn - pd - kvars);
    if ( r > 0 ) GenSwapMixed(&st_dtax[pb], &st_dtax[pn - r], r * elsize / kvars);

    if ( (r = pb - pa) > kvars )
        GenQuicksort (st_dtax, start, r / kvars, kvars, elsize, GenCompare, kstart);

    if ( (r = pd - pc) > kvars) {
        start = pn - r;
        N     = r / kvars;
        goto loop;
    }
}

int GenMed3(
    void *st_dtax,
    int a,
    int b,
    int c,
    int (*GenCompare) (void *a, void *b),
    size_t kstart
);

int GenMed3(
    void *st_dtax,
    int a,
    int b,
    int c,
    int (*GenCompare) (void *a, void *b),
    size_t kstart)
{
    return  GenCompare(st_dtax[a + kstart], st_dtax[b + kstart]) < 0 ?
           (GenCompare(st_dtax[b + kstart], st_dtax[c + kstart]) < 0 ? b : (GenCompare(st_dtax[a + kstart], st_dtax[c + kstart]) < 0 ? c : a ))
          :(GenCompare(st_dtax[b + kstart], st_dtax[c + kstart]) > 0 ? b : (GenCompare(st_dtax[a + kstart], st_dtax[c + kstart]) < 0 ? a : c ));
}
*/
