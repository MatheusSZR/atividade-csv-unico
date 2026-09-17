#ifndef BINARY_SEARCH_H
#define BINARY_SEARCH_H

#include <stddef.h>

int binary_search_iterative(const long long *v, size_t n, long long target, long long *comparisons);
int binary_search_recursive(const long long *v, size_t left, size_t right, long long target, long long *comparisons);

#endif
