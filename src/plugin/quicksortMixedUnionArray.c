void GenQuicksort (
    MixedUnion *st_dtax,
    size_t start,
    size_t N,
    size_t kvars,
    size_t elsize,
    int (*GenCompare) (MixedUnion a, MixedUnion b),
    size_t kstart
);

int GenMed3(
    MixedUnion *st_dtax,
    int a,
    int b,
    int c,
    int (*GenCompare) (MixedUnion a, MixedUnion b),
    size_t kstart
);

int GenMed3(
    MixedUnion *st_dtax,
    int a,
    int b,
    int c,
    int (*GenCompare) (MixedUnion a, MixedUnion b),
    size_t kstart)
{
    return  GenCompare(st_dtax[a + kstart], st_dtax[b + kstart]) < 0 ?
           (GenCompare(st_dtax[b + kstart], st_dtax[c + kstart]) < 0 ? b : (GenCompare(st_dtax[a + kstart], st_dtax[c + kstart]) < 0 ? c : a ))
          :(GenCompare(st_dtax[b + kstart], st_dtax[c + kstart]) > 0 ? b : (GenCompare(st_dtax[a + kstart], st_dtax[c + kstart]) < 0 ? a : c ));
}

void GenQuicksort (
    MixedUnion *st_dtax,
    size_t start,
    size_t N,
    size_t kvars,
    size_t elsize,
    int (*GenCompare) (MixedUnion a, MixedUnion b),
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

