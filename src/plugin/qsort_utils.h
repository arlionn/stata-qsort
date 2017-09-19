#ifndef QSORT_UTILS
#define QSORT_UTILS

#include <time.h>

int sf_isint ();
int sf_numsetup ();
void sf_running_timer (clock_t *timer, const char *msg);

int sf_oom_error (char * step_desc, char * obj_desc);
int sf_get_vector_length (char * st_matrix);
int sf_get_vector (char * st_matrix, double v[]);

int mf_min_signed (int x[], size_t N);
int mf_max_signed (int x[], size_t N);
int mf_sum_signed (int x[], size_t N);

size_t mf_min_unsigned (size_t x[], size_t N);
size_t mf_max_unsigned (size_t x[], size_t N);
size_t mf_sum_unsigned (size_t x[], size_t N);

#endif
