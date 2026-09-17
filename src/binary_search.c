#include "binary_search.h"

int binary_search_iterative(const long long *v, size_t n, long long target, long long *comparisons) {
    size_t left = 0, right = n;
    if (comparisons) *comparisons = 0;
    while (left < right) {
        size_t mid = left + (right - left) / 2;
        if (comparisons) (*comparisons)++;
        if (v[mid] == target) return 1;
        if (v[mid] < target) left = mid + 1;
        else right = mid;
    }
    return 0;
}

int binary_search_recursive(const long long *v, size_t left, size_t right, long long target, long long *comparisons) {
    size_t mid;
    if (left >= right) return 0;
    mid = left + (right - left) / 2;
    if (comparisons) (*comparisons)++;
    if (v[mid] == target) return 1;
    if (v[mid] < target) return binary_search_recursive(v, mid + 1, right, target, comparisons);
    return binary_search_recursive(v, left, mid, target, comparisons);
}
