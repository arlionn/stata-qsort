#include <sys/cdefs.h>
#include <stdlib.h>
#include <string.h>
#include "quicksortCommon.h"
#include "quicksortMixedUnionArray.c"

void MixedQuicksort (
    MixedUnion *st_dtax,
    size_t start,
    size_t N,
    size_t kstart,
    size_t kend,
    size_t kvars,
    size_t elsize,
    size_t *ltypes
);

int GenCompareChar (MixedUnion a, MixedUnion b)
{
    return BaseCompareChar(a.cval, b.cval);
}

int GenCompareNum (MixedUnion a, MixedUnion b)
{
    return BaseCompareNum(a.dval, b.dval);
}

void MixedQuicksort (
    MixedUnion *st_dtax,
    size_t start,
    size_t N,
    size_t kstart,
    size_t kend,
    size_t kvars,
    size_t elsize,
    size_t *ltypes)
{
    size_t j;
    short ischar;
    size_t i, end;

    // printf("Sorting to %lu on %lu / %lu\n", N, kstart, kend);
    // sf_printf("Sorting to %lu on %lu / %lu\n", N, kstart, kend);
    GenQuicksort (
        st_dtax,
        start,
        N,
        kvars,
        elsize,
        ( (ischar = (ltypes[kstart] > 0)) )? GenCompareChar: GenCompareNum,
        kstart
    );

    if ( kstart >= kend )
        return;

    end = start + N * kvars;

loop:

    j = 1;
    if ( ischar ) {
        for (i = start + kvars; i < end; i += kvars) {
            if ( GenCompareChar(st_dtax[i - kvars + kstart], st_dtax[i + kstart]) ) break;
            j++;
        }
    }
    else {
        for (i = start + kvars; i < end; i += kvars) {
            if ( GenCompareNum(st_dtax[i - kvars + kstart], st_dtax[i + kstart]) ) break;
            j++;
        }
    }

    if ( j > 1 ) {
        MixedQuicksort (
            st_dtax,
            start,
            j,
            kstart + 1,
            kend,
            kvars,
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
