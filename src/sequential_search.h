#ifndef SEQUENTIAL_SEARCH_H
#define SEQUENTIAL_SEARCH_H

#include <stdio.h>

int sequential_search_iterative(FILE *dest, int key_col, long long target, long long *comparisons);
int sequential_search_recursive(FILE *dest, int key_col, long long target, long long *comparisons);

#endif
