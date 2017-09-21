#include <sys/cdefs.h>
#include <stdlib.h>
#include <string.h>

/*********************************************************************
 *                            Definitions                            *
 *********************************************************************/

void MixedQuicksort (
    MixedUnion * start,
    size_t N,
    size_t kstart,
    size_t kend,
    size_t elsize,
    size_t *ltypes
);

void GenQuicksort (
    MixedUnion *start,
    size_t N,
    size_t elsize,
    int (*GenCompare) (MixedUnion *a, MixedUnion *b),
    size_t kstart
);

int GenCompareChar (
    MixedUnion *a,
    MixedUnion *b
);

int GenCompareNum (
    MixedUnion *a,
    MixedUnion *b
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


/*********************************************************************
 *                              Helpers                              *
 *********************************************************************/

int GenCompareChar (MixedUnion *a, MixedUnion *b)
{
    return BaseCompareChar(a->cval, b->cval);
}

int GenCompareNum (MixedUnion *a, MixedUnion *b)
{
    return BaseCompareNum(a->dval, b->dval);
}

void MixedQuicksort (
    MixedUnion *start,
    size_t N,
    size_t kstart,
    size_t kend,
    size_t elsize,
    size_t *ltypes)
{
    size_t j;
    short ischar;
    MixedUnion *i, *end;

    printf("Sorting to %lu on %lu / %lu\n", N, kstart, kend);
    sf_printf("Sorting to %lu on %lu / %lu\n", N, kstart, kend);
    GenQuicksort (
        start,
        N,
        elsize,
        ( (ischar = (ltypes[kstart] > 0)) )? GenCompareChar: GenCompareNum,
        kstart
    );

    if ( kstart >= kend )
        return;

    end = start + N * elsize;

loop:

    j = 1;
    if ( ischar ) {
        for (i = start + elsize; i < end; i += elsize) {
            if ( GenCompareChar(i - elsize + kstart, i + kstart) ) break;
            j++;
        }
    }
    else {
        for (i = start + elsize; i < end; i += elsize) {
            if ( GenCompareNum(i - elsize + kstart, i + kstart) ) break;
            j++;
        }
    }

    if ( j > 1 ) {
        MixedQuicksort (
            start,
            j,
            kstart + 1,
            kend,
            elsize,
            ltypes
        );
    }

    if ( (kstart < kend) ) {
        start = i;
        if ( start < end )
            goto loop;
    }
}
