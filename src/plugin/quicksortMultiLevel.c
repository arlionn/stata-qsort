#include <sys/cdefs.h>
#include <stdlib.h>
#include <string.h>
#include "delete/quicksort_bsd.c"

#define BaseCompareNum(a, b) ( ( (a) > (b) ) - ( (a) < (b) ) )
#define BaseCompareChar(a, b) ( strcmp(a, b) )

int MultiCompareChar (const void *a, const void *b, void *thunk);
int MultiCompareChar (const void *a, const void *b, void *thunk)
{
    int kstart = *(size_t *)thunk;
    // return BaseCompareChar(*((char **)a + kstart), *((char **)b + kstart));
    MixedUnion *aa = (MixedUnion *)a + kstart;
    MixedUnion *bb = (MixedUnion *)b + kstart;
    return BaseCompareChar(aa->cval, bb->cval);
}

int MultiCompareNum (const void *a, const void *b, void *thunk);
int MultiCompareNum (const void *a, const void *b, void *thunk)
{
    int kstart = *(size_t *)thunk;
    MixedUnion *aa = (MixedUnion *)a + kstart;
    MixedUnion *bb = (MixedUnion *)b + kstart;
    return BaseCompareNum(aa->dval, bb->dval);
    // const double aa = *((double*)*((void **)a + kstart));
    // const double bb = *((double*)*((void **)b + kstart));
    // return BaseCompareNum(aa, bb);
}

void MultiQuicksort (
    void *start,
    size_t N,
    size_t kstart,
    size_t kend,
    size_t elsize,
    size_t *ltypes
);

void MultiQuicksort (
    void *start,
    size_t N,
    size_t kstart,
    size_t kend,
    size_t elsize,
    size_t *ltypes)
{
    size_t j;
    short ischar;
    void *i, *end;

    quicksort (
        start,
        N,
        elsize,
        ( (ischar = (ltypes[kstart] > 0)) )? MultiCompareChar: MultiCompareNum,
        &kstart
    );

    if ( kstart >= kend )
        return;

    end = start + N * elsize;

loop:

    j = 1;
    if ( ischar ) {
        for (i = start + elsize; i < end; i += elsize) {
            if ( MultiCompareChar(i - elsize, i, &kstart) ) break;
            j++;
        }
    }
    else {
        for (i = start + elsize; i < end; i += elsize) {
            if ( MultiCompareNum(i - elsize, i, &kstart) ) break;
            j++;
        }
    }

    if ( j > 1 ) {
        MultiQuicksort (
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

/*********************************************************************
 *                              Testing                              *
 *********************************************************************/

int MultiCompareNum2 (const void *a, const void *b, void *thunk);
int MultiCompareNum2 (const void *a, const void *b, void *thunk)
{
    int kstart = *(size_t *)thunk;
    double aa = *((double *)a + kstart);
    double bb = *((double *)b + kstart);
    return BaseCompareNum(aa, bb);
}

void MultiQuicksort2 (
    void *start,
    size_t N,
    size_t kstart,
    size_t kend,
    size_t elsize,
    size_t *ltypes
);

void MultiQuicksort2 (
    void *start,
    size_t N,
    size_t kstart,
    size_t kend,
    size_t elsize,
    size_t *ltypes)
{
    size_t j;
    void *i, *end;

    quicksort (
        start,
        N,
        elsize,
        MultiCompareNum,
        &kstart
    );

    if ( kstart >= kend )
        return;

    end = start + N * elsize;

loop:

    j = 1;
    for (i = start + elsize; i < end; i += elsize) {
        if ( MultiCompareNum(i - elsize, i, &kstart) ) break;
        j++;
    }

    if ( j > 1 ) {
        MultiQuicksort (
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
