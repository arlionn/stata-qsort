#include <sys/cdefs.h>
#include <stdlib.h>
#include <string.h>

/*********************************************************************
 *                            Definitions                            *
 *********************************************************************/

void GenQuicksort (
    struct MixedArray *st_dta,
    size_t start,
    size_t end,
    size_t elsize,
    int (*GenCompare) (struct MixedArray a, struct MixedArray b, int l),
    size_t l
);

int GenMed3(
    struct MixedArray *st_dta,
    int a,
    int b,
    int c,
    int (*GenCompare) (struct MixedArray a, struct MixedArray b, int l),
    int l
);

int GenCompareChar (
    struct MixedArray a,
    struct MixedArray b,
    int l
);

int GenCompareNum (
    struct MixedArray a,
    struct MixedArray b,
    int l
);

int GenCompareMixed (
    struct MixedArray a,
    struct MixedArray b,
    struct MixedInfo *sort_info,
    size_t K
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
    struct MixedArray *st_dta,
    size_t start,
    size_t N,
    size_t elsize,
    int (*GenCompare) (struct MixedArray a, struct MixedArray b, int l),
    size_t l)
{

    int swap_count;
    int pa, pb, pc, pd;
    int pl, pm, pn;
    int cmp_result;
    size_t d, r, end;
    size_t cutoff = 9;

loop:
    end = start + N;

    if ( N < cutoff ) {
        for (pm = start + 1; pm < end; pm++)
            for (pl = pm;
                 pl > start && GenCompare(st_dta[pl - 1], st_dta[pl], l) > 0;
                 pl--)
                 GenSwapMixed(&st_dta[pl], &st_dta[pl - 1], elsize);
        return;
    }

    pm = (int) (start + (N / 2));
    if ( N > cutoff ) {
        pl = start;
        pn = end - 1;
        if ( N > 40 ) {
            d  = (int) (N / 8);
            pl = GenMed3(st_dta, pl,         pl + d, pl + 2 * d, GenCompare, l);
            pm = GenMed3(st_dta, pm - d,     pm,     pm + d,     GenCompare, l);
            pn = GenMed3(st_dta, pn - 2 * d, pn - d, pn,         GenCompare, l);
        }
        pm = GenMed3(st_dta, pl, pm, pn, GenCompare, l);
    }
    GenSwapMixed(&st_dta[start], &st_dta[pm], elsize);

    pa = pb = start + 1;
    pc = pd = end - 1;

    swap_count = 0;
    for (;;) {
        while (pb <= pc && (cmp_result = GenCompare(st_dta[pb], st_dta[start], l)) <= 0) {
            if ( cmp_result == 0 ) {
                swap_count = 1;
                GenSwapMixed(&st_dta[pa], &st_dta[pb], elsize);
                ++pa;
            }
            ++pb;
        }

        while (pb <= pc && (cmp_result = GenCompare(st_dta[pc], st_dta[start], l)) >= 0) {
            if ( cmp_result == 0 ) {
                swap_count = 1;
                GenSwapMixed(&st_dta[pc], &st_dta[pd], elsize);
                --pd;
            }
            --pc;
        }

        if ( pb > pc )
            break;

        GenSwapMixed(&st_dta[pb], &st_dta[pc], elsize);
        swap_count = 1;
        ++pb;
        --pc;
    }

    if ( swap_count == 0 ) {
        for (pm = start + 1; pm < end; pm++)
            for (pl = pm;
                 pl > start && GenCompare(st_dta[pl - 1], st_dta[pl], l) > 0;
                 pl--)
                GenSwapMixed(&st_dta[pl], &st_dta[pl - 1], elsize);
        return;
    }

    pn = end;
    r  = min(pa - start, pb - pa);
    if ( r > 0 ) GenSwapMixed(&st_dta[start], &st_dta[pb - r], elsize * r);

    r = min(pd - pc, pn - pd - 1);
    if ( r > 0 ) GenSwapMixed(&st_dta[pb], &st_dta[pn - r], elsize * r);

    if ( (r = pb - pa) > 1 )
        GenQuicksort (st_dta, start, r, elsize, GenCompare, l);

    if ( (r = pd - pc) > 1) {
        start = pn - r;
        N     = r;
        goto loop;
    }
}

/*********************************************************************
 *                              Helpers                              *
 *********************************************************************/

int GenCompareChar (struct MixedArray a, struct MixedArray b, int l)
{
    return BaseCompareChar(a.str_dta[l], b.str_dta[l]);
}

int GenCompareNum (struct MixedArray a, struct MixedArray b, int l)
{
    return BaseCompareNum(a.num_dta[l], b.num_dta[l]);
}

int GenCompareMixed (
    struct MixedArray a,
    struct MixedArray b,
    struct MixedInfo *sort_info,
    size_t K)
{
    int l = sort_info->lmap[K];
    if ( sort_info->ltypes[K] > 0 ) {
        return BaseCompareChar(a.str_dta[l], b.str_dta[l]);
    }
    else {
        return BaseCompareNum(a.num_dta[l], b.num_dta[l]);
    }
}

int GenMed3(
    struct MixedArray *st_dta,
    int a,
    int b,
    int c,
    int (*GenCompare) (struct MixedArray a, struct MixedArray b, int l),
    int l)
{
    return  GenCompare(st_dta[a], st_dta[b], l) < 0 ?
           (GenCompare(st_dta[b], st_dta[c], l) < 0 ? b : (GenCompare(st_dta[a], st_dta[c], l) < 0 ? c : a ))
          :(GenCompare(st_dta[b], st_dta[c], l) > 0 ? b : (GenCompare(st_dta[a], st_dta[c], l) < 0 ? a : c ));
}

void MixedQuicksort (
    struct MixedArray *st_dta,
    size_t start,
    size_t N,
    size_t kstart,
    size_t kend,
    size_t elsize,
    struct MixedInfo *sort_info)
{
    // sf_printf("Sorting obs %u to %u on %u / %u\n", start, start + N - 1, kstart, kend);

    /*
     *
    int i, j, k, l;
    short ischar;
    size_t J;

    GenQuicksort (
        st_dta,
        start,
        N,
        elsize,
        ( (ischar = (sort_info->ltypes[kstart] > 0)) )? GenCompareChar: GenCompareNum,
        (l = sort_info->lmap[kstart])
    );

    if ( kend > 1 ) {
        size_t *ix = calloc(N + 1, sizeof(*ix));
        for (k = kstart + 1; k < kend; k++) {
            J = 0;
            ix[J++] = start;
            if ( ischar ) {
                for (i = start + 1; i < start + N; i++)
                    if ( GenCompareChar(st_dta[i - 1], st_dta[i], l) )
                        ix[J++] = i;
            }
            else {
                for (i = start + 1; i < start + N; i++)
                    if ( GenCompareNum(st_dta[i - 1], st_dta[i], l) )
                        ix[J++] = i;
            }
            ix[J] = start + N;

            l = sort_info->lmap[k];
            ischar = (sort_info->ltypes[k] > 0);
            for (j = 0; j < J; j++) {
                if ( ((i = ix[j + 1] - ix[j]) > 1) )
                    GenQuicksort (
                        st_dta,
                        ix[j],
                        i,
                        elsize,
                        ischar? GenCompareChar: GenCompareNum,
                        l
                    );
            }
        }
        free (ix);
    }
     *
     */

    /*
     */
    int l;
    short ischar;
    size_t i, end;
    GenQuicksort (
        st_dta,
        start,
        N,
        elsize,
        ( (ischar = (sort_info->ltypes[kstart] > 0)) )? GenCompareChar: GenCompareNum,
        (l = sort_info->lmap[kstart])
    );

    if ( kstart >= kend )
        return;

    end = start + N;
   /*
    *
    do {
        if ( ischar ) {
            for (i = start + 1; i < end; i++)
                if ( GenCompareChar(st_dta[i - 1], st_dta[i], l) ) break;
        }
        else {
            for (i = start + 1; i < end; i++)
                if ( GenCompareNum(st_dta[i - 1], st_dta[i], l) ) break;
        }
        i -= start;

        if ( i > 1 ) {
            MixedQuicksort (
                st_dta,
                start,
                i,
                kstart + 1,
                kend,
                elsize,
                sort_info
            );
        }

        start += i;
    } while ( (kstart < kend) & (start < end) );
     *
     */

    loop:

        if ( ischar ) {
            for (i = start + 1; i < end; i++)
                if ( GenCompareChar(st_dta[i - 1], st_dta[i], l) ) break;
        }
        else {
            for (i = start + 1; i < end; i++)
                if ( GenCompareNum(st_dta[i - 1], st_dta[i], l) ) break;
        }
        i -= start;

        if ( i > 1 ) {
            MixedQuicksort (
                st_dta,
                start,
                i,
                kstart + 1,
                kend,
                elsize,
                sort_info
            );
        }

        if ( (kstart < kend) ) {
            start += i;
            if ( start < end )
                goto loop;
        }
   /*
    */
}
