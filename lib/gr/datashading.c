#include <stdio.h>
#include <stdlib.h>

#include "gr.h"

#ifndef min
#define min(a, b) (((a) < (b)) ? (a) : (b))
#endif

static void arg_min_max(int *min_index, int *max_index, int n, const double *array) {
    /*
     * Finds the index of the minimum and maximum value in arr.
     * @param int *minIndex: Reference for the minIndex
     * @param int *maxIndex: Reference for the maxIndex
     * @param int n: The number of values in arr.
     * @param const double *arr: The value array.
     * @return The index of the minimum.
     */
    int i;
    *min_index = 0;
    *max_index = 0;
    for (i = 1; i < n; i++) {
        if (array[i] < array[*min_index])
            *min_index = i;
        if (array[i] > array[*max_index])
            *max_index = i;
    }
}

void gr_reduce_points(int n, const double *x, const double *y, int points, double *x_array, double *y_array) {
    /*
     * Reduces the number of points of the x and y array.
     * @param int n: The number of points of the x, y array.
     * @param const double *x: The x value array.
     * @param const double *y: The y value array.
     * @param int points: The requested amount of points.
     * @param double *x_array: The return array for the x values.
     * @param double *y_array: The return array for the y values.
     */
    if (n < points) {
        // Copy the original array
        memcpy(x_array, x, sizeof(double) * n);
        memcpy(y_array, y, sizeof(double) * n);
        fprintf(stderr, "Not enough points provided.\n");
        return;
    }
    int append_index = 0;
    int num_intervals = points / 2;
    double exact_interval_width = (double) n / num_intervals;
    int interval_width = n / num_intervals;
    int interval;
    for (interval = 0; interval < num_intervals; interval++) {
        int index = interval * exact_interval_width;
        int min_index, max_index;
        arg_min_max(&min_index, &max_index, min(interval_width, n - index - 1), y + index);
        x_array[append_index] = x[min_index + index];
        y_array[append_index] = y[min_index + index];
        append_index++;
        x_array[append_index] = x[max_index + index];
        y_array[append_index] = y[max_index + index];
        append_index++;
    }
}