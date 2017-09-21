#ifndef QSORT_PLUGIN
#define QSORT_PLUGIN

// Libraries
// ---------

// #include <stdint.h>
// #include <unistd.h>
// #include <inttypes.h>
// #include <sys/types.h>
#include <math.h>
#include <locale.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Container structure for Stata-provided info
struct StataInfo {
    size_t in1;
    size_t in2;
    size_t N;
    //
    int qsort;
    int verbose;
    int benchmark;
    int integers_ok;
    int strmax;
    int strmax_rest;
    int *invert;
    //
    int kvars_sort;
    int kvars_sort_num;
    int kvars_sort_str;
    int kvars_rest;
    int kvars_rest_start;
    int kvars_rest_num;
    int kvars_rest_str;
    //
    int *restvars_lens;
    int *sortvars_lens;
    int *sortvars_mins;
    int *sortvars_maxs;
    int sortvars_minlen;
    int sortvars_maxlen;
    //
    int *pos_num_sortvars;
    int *pos_str_sortvars;
    int *pos_num_restvars;
    int *pos_str_restvars;
};

int sf_parse_info (struct StataInfo *st_info);

#endif
