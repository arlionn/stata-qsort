#include <sys/cdefs.h>
#include <stdlib.h>
#include <string.h>
#include "delete/quicksort_bsd.c"

#define BaseCompareNum(a, b) ( ( (a) > (b) ) - ( (a) < (b) ) )
#define BaseCompareChar(a, b) ( strcmp(a, b) )

int GenCompareChar (const void *a, const void *b, void *thunk);
int GenCompareChar (const void *a, const void *b, void *thunk)
{
    int kstart = *(size_t *)thunk;
    return BaseCompareChar(*((char **)a + kstart), *((char **)b + kstart));
}

int GenCompareNum (const void *a, const void *b, void *thunk);
int GenCompareNum (const void *a, const void *b, void *thunk)
{
    int kstart = *(size_t *)thunk;
    const double aa = *((double*)*((void **)a + kstart));
    const double bb = *((double*)*((void **)b + kstart));
    return BaseCompareNum(aa, bb);
}


void MixedQuicksort (
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
        ( (ischar = (ltypes[kstart] > 0)) )? GenCompareChar: GenCompareNum,
        &kstart
    );

    if ( kstart >= kend )
        return;

    end = start + N * elsize;

loop:

    j = 1;
    if ( ischar ) {
        for (i = start + elsize; i < end; i += elsize) {
            if ( GenCompareChar(i - elsize, i, &kstart) ) break;
            j++;
        }
    }
    else {
        for (i = start + elsize; i < end; i += elsize) {
            if ( GenCompareNum(i - elsize, i, &kstart) ) break;
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
