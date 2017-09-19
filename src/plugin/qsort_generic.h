#ifndef QSORT_GENERIC
#define QSORT_GENERIC

#define COMP(a, b) ( ( (a) > (b) ) - ( (a) < (b) ) )
#define COMPSTR(a, b) ( strcmp(a, b) )

struct MixedArray {
    char  **str_dta;
    double *num_dta;
};

struct MixedInfo {
    size_t *ltypes;
    size_t *lmap;
    int K;
};

typedef union {
    double dval;
    char *cval;
} MixedUnion;

int sf_msort(struct StataInfo *st_info);
int mf_compare_mixed (const void *a, const void *b, void *arg);

#endif
