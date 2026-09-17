#include "csv_utils.h"

#include <ctype.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

static void normalize_header_field(char *s) {
    csv_trim(s);
    if ((unsigned char)s[0] == 0xEF && (unsigned char)s[1] == 0xBB && (unsigned char)s[2] == 0xBF) {
        memmove(s, s + 3, strlen(s + 3) + 1);
        csv_trim(s);
    }
}

void csv_trim(char *s) {
    size_t len;
    size_t start = 0;
    if (s == NULL) return;
    len = strlen(s);
    while (len > 0 && isspace((unsigned char)s[len - 1])) {
        s[--len] = '\0';
    }
    while (s[start] != '\0' && isspace((unsigned char)s[start])) {
        start++;
    }
    if (start > 0) {
        memmove(s, s + start, strlen(s + start) + 1);
    }
}

int csv_get_field(const char *line, int target_col, char *out, size_t out_size) {
    size_t i = 0;
    int col = 0;
    size_t out_len = 0;
    int in_quotes = 0;

    if (line == NULL || target_col < 0 || out == NULL || out_size == 0) return 0;
    out[0] = '\0';

    while (1) {
        char c = line[i];
        int is_end = (c == '\0' || (!in_quotes && (c == '\n' || c == '\r')));
        if (is_end) {
            if (col == target_col) {
                out[out_len] = '\0';
                return 1;
            }
            return 0;
        }

        if (c == '"') {
            if (in_quotes && line[i + 1] == '"') {
                if (col == target_col) {
                    if (out_len + 1 >= out_size) return 0;
                    out[out_len++] = '"';
                }
                i += 2;
                continue;
            }
            in_quotes = !in_quotes;
            i++;
            continue;
        }

        if (c == ',' && !in_quotes) {
            if (col == target_col) {
                out[out_len] = '\0';
                return 1;
            }
            col++;
            out_len = 0;
            i++;
            continue;
        }

        if (col == target_col) {
            if (out_len + 1 >= out_size) return 0;
            out[out_len++] = c;
        }
        i++;
    }
}

int csv_count_columns(const char *line) {
    size_t i = 0;
    int cols = 1;
    int in_quotes = 0;
    if (line == NULL || line[0] == '\0') return 0;
    while (line[i] != '\0' && line[i] != '\n' && line[i] != '\r') {
        if (line[i] == '"') {
            if (in_quotes && line[i + 1] == '"') {
                i += 2;
                continue;
            }
            in_quotes = !in_quotes;
        } else if (line[i] == ',' && !in_quotes) {
            cols++;
        }
        i++;
    }
    return cols;
}

int csv_find_column(const char *header, const char *wanted) {
    int col = 0;
    int total;
    char field[256];
    if (header == NULL || wanted == NULL) return -1;
    total = csv_count_columns(header);
    for (col = 0; col < total; col++) {
        if (!csv_get_field(header, col, field, sizeof(field))) return -1;
        normalize_header_field(field);
        if (strcmp(field, wanted) == 0) return col;
    }
    return -1;
}

int csv_parse_key(const char *line, int key_col, long long *key) {
    char field[MAX_KEY_LEN];
    char *end;
    long long value;
    if (key == NULL || !csv_get_field(line, key_col, field, sizeof(field))) return 0;
    csv_trim(field);
    if (field[0] == '\0') return 0;

    errno = 0;
    value = strtoll(field, &end, 10);
    if (errno != 0 || end == field) return 0;
    while (*end != '\0' && isspace((unsigned char)*end)) end++;
    if (*end != '\0') return 0;
    *key = value;
    return 1;
}

int csv_append_row(FILE *dest, const char *row) {
    size_t len;
    if (dest == NULL || row == NULL) return 0;
    len = strlen(row);
    while (len > 0 && (row[len - 1] == '\n' || row[len - 1] == '\r')) len--;
    if (len > 0 && fwrite(row, 1, len, dest) != len) return 0;
    if (fputc('\n', dest) == EOF) return 0;
    return fflush(dest) == 0;
}

int csv_ensure_trailing_newline(const char *filename) {
    FILE *f;
    long size;
    int last;
    if (filename == NULL) return 0;
    f = fopen(filename, "rb");
    if (f == NULL) return 0;
    if (fseek(f, 0, SEEK_END) != 0) { fclose(f); return 0; }
    size = ftell(f);
    if (size <= 0) { fclose(f); return 1; }
    if (fseek(f, -1, SEEK_END) != 0) { fclose(f); return 0; }
    last = fgetc(f);
    fclose(f);
    if (last == '\n') return 1;

    f = fopen(filename, "ab");
    if (f == NULL) return 0;
    if (fputc('\n', f) == EOF) { fclose(f); return 0; }
    if (fclose(f) != 0) return 0;
    return 1;
}
