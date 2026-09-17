#include "sequential_search.h"
#include "csv_utils.h"

#include <string.h>

int sequential_search_iterative(FILE *dest, int key_col, long long target, long long *comparisons) {
    char line[MAX_LINE_LEN];
    long long key;
    if (comparisons) *comparisons = 0;
    if (dest == NULL) return 0;
    rewind(dest);
    if (fgets(line, sizeof(line), dest) == NULL) return 0;
    while (fgets(line, sizeof(line), dest) != NULL) {
        if (line[0] == '\n' || line[0] == '\r' || line[0] == '\0') continue;
        if (!csv_parse_key(line, key_col, &key)) continue;
        if (comparisons) (*comparisons)++;
        if (key == target) return 1;
    }
    return 0;
}

static int sequential_search_recursive_impl(FILE *dest, int key_col, long long target, long long *comparisons) {
    static char line[MAX_LINE_LEN];
    long long key;
    if (fgets(line, sizeof(line), dest) == NULL) return 0;
    if (line[0] == '\n' || line[0] == '\r' || line[0] == '\0') {
        return sequential_search_recursive_impl(dest, key_col, target, comparisons);
    }
    if (csv_parse_key(line, key_col, &key)) {
        if (comparisons) (*comparisons)++;
        if (key == target) return 1;
    }
    return sequential_search_recursive_impl(dest, key_col, target, comparisons);
}

int sequential_search_recursive(FILE *dest, int key_col, long long target, long long *comparisons) {
    char header[MAX_LINE_LEN];
    if (comparisons) *comparisons = 0;
    if (dest == NULL) return 0;
    rewind(dest);
    if (fgets(header, sizeof(header), dest) == NULL) return 0;
    return sequential_search_recursive_impl(dest, key_col, target, comparisons);
}
