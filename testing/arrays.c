#include <unistd.h>
#include <math.h>
#include <locale.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

typedef union {
    double dval;
    char *cval;
} MixedUnion;

int main(int argc, char *argv[])
{
    int i, k;
    size_t N      = 5000000;
    size_t kvars  = 8;

    time_t t;
    srand((unsigned) time(&t));

    size_t strlen[kvars];
    for (k = 0; k < kvars; k++)
        if ( k == 6 )
            strlen[k] = 74;
        else if ( k == 7 )
            strlen[k] = 12;
        else
            strlen[k] = 0;
        

    size_t sel;
    // void **data = calloc(N, (allbytes + 2) * sizeof(*data));
    //  void **data = calloc(N * kvars, sizeof(*data));
    MixedUnion *data = calloc(N * kvars, sizeof(*data));

    // for (i = 0; i < N; i++) {
    //     for (k = 0; k < kvars; k++) {
    //         sel = i * kvars + k;
    //         // if ( strlen[k] ) {
    //         //     data[sel].cval = malloc(strlen[k] * sizeof(char));
    //         //     memset (data[sel].cval, '\0', strlen[k]);
    //         // }
    //         // else {
    //         //     data[sel] = malloc(sizeof(double));
    //         //     allbytes += sizeof(double);
    //         // }
    //     }
    // }

    double size_data = N * kvars * sizeof(*data) / 1024 / 1024;
    printf("data should be initially allocated %.2fMiB\n", size_data); sleep(1);

    for (i = 0; i < N; i++) {
        for (k = 0; k < kvars; k++) {
            sel = i * kvars + k;
            if ( strlen[k] ) {
                if ( k == 6 )
                    data[sel].cval = strdup("this is a very long strin I don't see how mem wouldn't go through the roof");
                else
                    data[sel].cval = strdup("and shorter!");
            }
            else {
                data[sel].dval = (double) (rand() / RAND_MAX);
            }
        }
    }

    double size_all = size_data + N * 88 * sizeof(char) / 1024 / 1024;
    printf("and it should occupy %.2fMiB in memory\n", size_all); sleep(1);
    for (i = 0; i < N; i++)
        for (k = 0; k < kvars; k++)
            if ( strlen[k] )
                free (data[i * kvars + k].cval);

    free (data);

    return (0);
}

// gcc -Wall -o arrays arrays.c; ./arrays
