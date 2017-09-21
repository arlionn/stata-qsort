int GenCompareChar (
    MixedUnion a,
    MixedUnion b
);

int GenCompareNum (
    MixedUnion a,
    MixedUnion b
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


