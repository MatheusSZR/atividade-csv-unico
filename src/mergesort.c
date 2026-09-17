#include "mergesort.h"

#include <string.h>

void merge_ranges(long long *v, long long *aux, size_t left, size_t mid, size_t right) {
    size_t i = left, j = mid, k = left;
    while (i < mid && j < right) {
        if (v[i] <= v[j]) aux[k++] = v[i++];
        else aux[k++] = v[j++];
    }
    while (i < mid) aux[k++] = v[i++];
    while (j < right) aux[k++] = v[j++];
    memcpy(&v[left], &aux[left], (right - left) * sizeof(long long));
}

void mergesort_recursive(long long *v, long long *aux, size_t left, size_t right) {
    size_t mid;
    if (right - left <= 1) return;
    mid = left + (right - left) / 2;
    mergesort_recursive(v, aux, left, mid);
    mergesort_recursive(v, aux, mid, right);
    merge_ranges(v, aux, left, mid, right);
}

void mergesort_iterative(long long *v, long long *aux, size_t n) {
    size_t width;
    for (width = 1; width < n; width *= 2) {
        size_t left;
        for (left = 0; left < n; left += 2 * width) {
            size_t mid = left + width;
            size_t right = left + 2 * width;
            if (mid > n) mid = n;
            if (right > n) right = n;
            if (mid < right) merge_ranges(v, aux, left, mid, right);
        }
        if (width > n / 2) break;
    }
}
