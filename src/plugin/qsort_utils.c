#include "qsort_utils.h"

/*********************************************************************
 *                           General Utils                           *
 *********************************************************************/

/**
 * @brief Minimum for signed integer array
 *
 * @param x vector of integers to get the min
 * @param N number of elements in @x
 * @return Smallest integer in @x
 */
int mf_min_signed(int x[], size_t N)
{
    int min = x[0]; size_t i;
    for (i = 1; i < N; ++i) {
        if (min > x[i]) min = x[i];
    }
    return (min);
}

/**
 * @brief Maximum for signed integer array
 *
 * @param x vector of integers to get the max
 * @param N number of elements in @x
 * @return Smallest integer in @x
 */
int mf_max_signed(int x[], size_t N)
{
    int max = x[0]; size_t i;
    for (i = 1; i < N; ++i) {
        if (max < x[i]) max = x[i];
    }
    return (max);
}

/**
 * @brief Sum for an integer array
 *
 * @return Sum of integers array
 */
int mf_sum_signed(int x[], size_t N)
{
    int sum = x[0]; size_t i;
    for (i = 1; i < N; i++)
        sum += x[i];
    return (sum);
}

/**
 * @brief Minimum for unsigned integer array
 *
 * @param x vector of integers to get the min
 * @param N number of elements in @x
 * @return Smallest integer in @x
 */
size_t mf_min_unsigned(size_t x[], size_t N)
{
    size_t min = x[0]; size_t i;
    for (i = 1; i < N; ++i) {
        if (min > x[i]) min = x[i];
    }
    return (min);
}

/**
 * @brief Maximum for unsigned integer array
 *
 * @param x vector of integers to get the max
 * @param N number of elements in @x
 * @return Smallest integer in @x
 */
size_t mf_max_unsigned(size_t x[], size_t N)
{
    size_t max = x[0]; size_t i;
    for (i = 1; i < N; ++i) {
        if (max < x[i]) max = x[i];
    }
    return (max);
}

/**
 * @brief Sum for an unsigned array
 *
 * @return Sum of unsigned array
 */
size_t mf_sum_unsigned(size_t x[], size_t N)
{
    size_t sum = x[0]; size_t i;
    for (i = 1; i < N; i++)
        sum += x[i];
    return (sum);
}

/*********************************************************************
 *                            Stata Utils                            *
 *********************************************************************/

/**
 * @brief Wrapper for OOM error exit message
 */
int sf_oom_error (char * step_desc, char * obj_desc)
{
    sf_errprintf ("%s: Unable to allocate memory for object '%s'.\n", step_desc, obj_desc);
    SF_display ("See {help gcollapse##memory:help gcollapse (Out of memory)}.\n");
    return (42002);
}

/**
 * @brief Get length of Stata vector
 *
 * @param st_matrix name of Stata vector (1xN or Nx1)
 * @return Return number of rows or cols.
 */
int sf_get_vector_length(char * st_matrix)
{
    int ncol = SF_col(st_matrix);
    int nrow = SF_row(st_matrix);
    if ( (ncol > 1) & (nrow > 1) ) {
        sf_errprintf ("tried to get the length a %d by %d matrix\n", nrow, ncol);
        return (-1);
    }
    return ( ncol > nrow? ncol: nrow );
}

/**
 * @brief Parse stata vector into C array
 *
 * @param st_matrix name of stata matrix to get
 * @param v array where to store the vector
 * @return Store min and max of @x
 */
int sf_get_vector(char * st_matrix, double v[])
{
    ST_retcode rc ;
    ST_double  z ;

    int i;
    int ncol = SF_col(st_matrix);
    int nrow = SF_row(st_matrix);
    if ( (ncol > 1) & (nrow > 1) ) {
        sf_errprintf ("tried to read a %d by %d matrix into an array\n", nrow, ncol);
        return (198);
    }
    if ( ncol > 1 ) {
        for (i = 0; i < ncol; i++) {
            if ( (rc = SF_mat_el(st_matrix, 1, i + 1, &z)) ) return(rc);
            v[i] = z;
        }
    }
    else {
        for (i = 0; i < nrow; i++) {
            if ( (rc = SF_mat_el(st_matrix, i + 1, 1, &z)) ) return(rc);
            v[i] = z;
        }
    }
    return (0);
}

/**
 * @brief Check if variable requested is integer in disguise
 *
 * @return Stores __qsort_is_int in Stata with result
 */
int sf_isint()
{
    ST_retcode rc ;
    ST_double  z ;
    size_t i;

    for (i = SF_in1(); i <= SF_in2(); i++) {
        if ( (rc = SF_vdata(1, i, &z)) ) return (rc);
        if ( ceilf(z) == z ) continue;
        else {
            if ( (rc = SF_scal_save ("__qsort_is_int", (double) 0)) ) return (rc);
            sf_printf ("(not an integer)\n");
            return (0);
        }
    }

    if ( (rc = SF_scal_save ("__qsort_is_int", (double) 1)) ) return (rc);
    sf_printf ("(an integer in disguise!)\n");
    return(0);
}

/**
 * @brief Min, max, and missing count for numeric sort vars
 *
 * @return Computes min, max, and missing count in Stata
 */
int sf_numsetup()
{
    ST_retcode rc ;
    ST_double  z ;
    int i, k;

    int kvars_sort_num = sf_get_vector_length("c_qsort_sortmiss");
    if ( kvars_sort_num < 0 ) return (198);

    double mins[kvars_sort_num], maxs[kvars_sort_num], miss[kvars_sort_num];
    int nonmiss[kvars_sort_num];
    size_t in1    = SF_in1();
    size_t in2    = SF_in2();
    size_t N      = in2 - in1 + 1;
    size_t start  = 0;

    for (k = 0; k < kvars_sort_num; k++)
        mins[k] = maxs[k] = miss[k] = nonmiss[k] = 0;

    do {
        for (k = 0; k < kvars_sort_num; k++) {
            if ( (rc = SF_vdata(k + 1, start + in1, &z)) ) return(rc);
            if ( SF_is_missing(z) ) {
                miss[k] = 1;
            }
            else {
                if ( nonmiss[k] == 0 ) {
                    nonmiss[k] = 1;
                    mins[k]    = z;
                    maxs[k]    = z;
                }
            }
        }
        ++start;
    } while ( (mf_sum_signed(nonmiss, kvars_sort_num) < kvars_sort_num) & (start < N) );

    for (i = 0; i < N; i++) {
        for (k = 0; k < kvars_sort_num; k++) {
            if ( (rc = SF_vdata(k + 1, i + in1, &z)) ) return(rc);
            if ( SF_is_missing(z) ) {
                miss[k] = 1;
            }
            else {
                if (mins[k] > z) mins[k] = z;
                if (maxs[k] < z) maxs[k] = z;
            }
        }
    }

    for (k = 0; k < kvars_sort_num; k++)
        sf_printf ("%.4f \t %.4f \t %.4f \n", mins[k], maxs[k], miss[k]);

    for (k = 0; k < kvars_sort_num; k++) {
        if ( (rc = SF_mat_store("c_qsort_sortmiss", 1, k + 1, miss[k])) ) return(rc);
        if ( (rc = SF_mat_store("c_qsort_sortmin",  1, k + 1, mins[k])) ) return(rc);
        if ( (rc = SF_mat_store("c_qsort_sortmax",  1, k + 1, maxs[k])) ) return(rc);
    }

    return(0);
}

/**
 * @brief Update a running timer and print a message to satata console
 *
 * Prints a messasge to Stata that the running timer @timer was last set
 * @diff seconds ago. It then updates the timer to the current time.
 *
 * @param timer clock object containing time since last udpate
 * @param msg message to print before # of seconds
 * @return Print time since last update to Stata console
 */
void sf_running_timer (clock_t *timer, const char *msg)
{
    double diff  = (double) (clock() - *timer) / CLOCKS_PER_SEC;
    sf_printf (msg);
    sf_printf ("; %.3f seconds.\n", diff);
    *timer = clock();
}
