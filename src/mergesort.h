#ifndef MERGESORT_H
#define MERGESORT_H

#include <stddef.h>

void mergesort_recursive(long long *v, long long *aux, size_t left, size_t right);
void mergesort_iterative(long long *v, long long *aux, size_t n);
void merge_ranges(long long *v, long long *aux, size_t left, size_t mid, size_t right);

#endif
