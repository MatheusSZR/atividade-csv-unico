#ifndef CSV_UTILS_H
#define CSV_UTILS_H

#include <stddef.h>
#include <stdio.h>

#define KEY_COLUMN_NAME "Identificador"
#define MAX_LINE_LEN 8192
#define MAX_KEY_LEN 64

int csv_find_column(const char *header, const char *wanted);
int csv_get_field(const char *line, int target_col, char *out, size_t out_size);
int csv_parse_key(const char *line, int key_col, long long *key);
int csv_append_row(FILE *dest, const char *row);
int csv_ensure_trailing_newline(const char *filename);
int csv_count_columns(const char *line);
void csv_trim(char *s);

#endif
