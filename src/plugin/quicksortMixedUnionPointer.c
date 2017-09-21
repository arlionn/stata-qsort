void GenQuicksort (
    MixedUnion *start,
    size_t N,
    size_t elsize,
    int (*GenCompare) (MixedUnion *a, MixedUnion *b),
    size_t kstart)
{

    MixedUnion *pa, *pb, *pc, *pd;
    MixedUnion *pl, *pm, *pn;
    MixedUnion *end;

    int swap_count;
    int cmp_result;
    size_t d, r;
    size_t cutoff = 9;

loop:
    end = start + N * elsize;

    if ( N < cutoff ) {
        for (pm = start + elsize; pm < end; pm += elsize) {
            for (pl = pm;
                 pl > start && GenCompare(pl - elsize + kstart, pl + kstart) > 0;
                 pl -= elsize) {
                 GenSwapMixed(pl, pl - elsize, elsize);
            }
        }
        return;
    }

    pm = start + ((int) (N / 2)) * elsize;
    if ( N > cutoff ) {
        pl = start;
        pn = end - elsize;
        if ( N > 40 ) {
            d  = ((int) (N / 8)) * elsize;
            pl = GenMed3(pl,         pl + d, pl + 2 * d, GenCompare, kstart);
            pm = GenMed3(pm - d,     pm,     pm + d,     GenCompare, kstart);
            pn = GenMed3(pn - 2 * d, pn - d, pn,         GenCompare, kstart);
        }
        pm = GenMed3(pl, pm, pn, GenCompare, kstart);
    }
    GenSwapMixed(start, pm, elsize);

    pa = pb = start + elsize;
    pc = pd = end - elsize;

    swap_count = 0;
    for (;;) {
        while (pb <= pc && (cmp_result = GenCompare(pb + kstart, start + kstart)) <= 0) {
            if ( cmp_result == 0 ) {
                swap_count = 1;
                GenSwapMixed(pa, pb, elsize);
                pa += elsize;
            }
            pb += elsize;
        }

        while (pb <= pc && (cmp_result = GenCompare(pc + kstart, start + kstart)) >= 0) {
            if ( cmp_result == 0 ) {
                swap_count = 1;
                GenSwapMixed(pc, pd, elsize);
                pd -= elsize;
            }
            pc -= elsize;
        }

        if ( pb > pc )
            break;

        GenSwapMixed(pb, pc, elsize);
        swap_count = 1;
        pb += elsize;
        pc -= elsize;
    }

    if ( swap_count == 0 ) {
        for (pm = start + elsize; pm < end; pm += elsize)
            for (pl = pm;
                 pl > start && GenCompare(pl - elsize + kstart, pl + kstart) > 0;
                 pl -= elsize)
                GenSwapMixed(pl, pl - elsize, elsize);
        return;
    }

    pn = end;
    r  = min(pa - start, pb - pa);
    if ( r > 0 ) GenSwapMixed(start, pb - r, r);

    r = min(pd - pc, pn - pd - elsize);
    if ( r > 0 ) GenSwapMixed(pb, pn - r, r);

    if ( (r = pb - pa) > elsize )
        GenQuicksort (start, r / elsize, elsize, GenCompare, kstart);

    if ( (r = pd - pc) > elsize ) {
        start = pn - r;
        N     = r / elsize;
        goto loop;
    }
}

MixedUnion * GenMed3(
    MixedUnion *a,
    MixedUnion *b,
    MixedUnion *c,
    int (*GenCompare) (MixedUnion *a, MixedUnion *b),
    size_t kstart
);

MixedUnion * GenMed3(
    MixedUnion *a,
    MixedUnion *b,
    MixedUnion *c,
    int (*GenCompare) (MixedUnion *a, MixedUnion *b),
    size_t kstart)
{
    return  GenCompare(a + kstart, b + kstart) < 0 ?
           (GenCompare(b + kstart, c + kstart) < 0 ? b : (GenCompare(a + kstart, c + kstart) < 0 ? c : a ))
          :(GenCompare(b + kstart, c + kstart) > 0 ? b : (GenCompare(a + kstart, c + kstart) < 0 ? a : c ));
}
