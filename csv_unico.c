/*
 * Atividade - CSV sem identificadores duplicados
 * Implementacoes: busca sequencial iterativa/recursiva, MergeSort iterativo/recursivo
 * e busca binaria iterativa/recursiva.
 */
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define KEY_COLUMN_NAME "Identificador"
#define MAX_LINE_LEN 8192
#define MAX_KEY_LEN 64
#define INITIAL_CAPACITY 1024

typedef enum { MODE_SEQ_ITER, MODE_SEQ_REC, MODE_MERGE_ITER, MODE_MERGE_REC } Mode;
typedef struct { long long *data; size_t size; size_t capacity; } KeyVector;

void csv_trim(char *s);



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

static void usage(const char *prog) {
    fprintf(stderr,
            "Uso: %s <novas.csv> <destino.csv> [modo]\n"
            "Modos: seq-it | seq-rec | merge-it | merge-rec (padrao: merge-it)\n",
            prog);
}

static int parse_mode(const char *s, Mode *mode) {
    if (s == NULL || strcmp(s, "merge-it") == 0) { *mode = MODE_MERGE_ITER; return 1; }
    if (strcmp(s, "seq-it") == 0) { *mode = MODE_SEQ_ITER; return 1; }
    if (strcmp(s, "seq-rec") == 0) { *mode = MODE_SEQ_REC; return 1; }
    if (strcmp(s, "merge-rec") == 0) { *mode = MODE_MERGE_REC; return 1; }
    return 0;
}

static int vector_init(KeyVector *vec, size_t initial) {
    vec->data = (long long *)malloc(initial * sizeof(long long));
    if (vec->data == NULL) return 0;
    vec->size = 0;
    vec->capacity = initial;
    return 1;
}

static void vector_destroy(KeyVector *vec) {
    free(vec->data);
    vec->data = NULL;
    vec->size = vec->capacity = 0;
}

static int vector_push(KeyVector *vec, long long key) {
    if (vec->size == vec->capacity) {
        size_t next = vec->capacity * 2;
        long long *tmp = (long long *)realloc(vec->data, next * sizeof(long long));
        if (tmp == NULL) return 0;
        vec->data = tmp;
        vec->capacity = next;
    }
    vec->data[vec->size++] = key;
    return 1;
}

static int vector_insert_sorted(KeyVector *vec, long long key, size_t pos) {
    if (vec->size == vec->capacity) {
        size_t next = vec->capacity * 2;
        long long *tmp = (long long *)realloc(vec->data, next * sizeof(long long));
        if (tmp == NULL) return 0;
        vec->data = tmp;
        vec->capacity = next;
    }
    memmove(&vec->data[pos + 1], &vec->data[pos], (vec->size - pos) * sizeof(long long));
    vec->data[pos] = key;
    vec->size++;
    return 1;
}

static size_t lower_bound(const long long *v, size_t n, long long key) {
    size_t left = 0, right = n;
    while (left < right) {
        size_t mid = left + (right - left) / 2;
        if (v[mid] < key) left = mid + 1;
        else right = mid;
    }
    return left;
}

static int load_destination_keys(FILE *dest, int key_col, KeyVector *vec) {
    char line[MAX_LINE_LEN];
    long long key;
    rewind(dest);
    if (fgets(line, sizeof(line), dest) == NULL) return 0;
    while (fgets(line, sizeof(line), dest) != NULL) {
        if (line[0] == '\n' || line[0] == '\r' || line[0] == '\0') continue;
        if (!csv_parse_key(line, key_col, &key)) {
            fprintf(stderr, "Registro invalido no destino: chave ausente/invalida.\n");
            return 0;
        }
        if (!vector_push(vec, key)) return 0;
    }
    return 1;
}

static int vector_is_unique_sorted(const KeyVector *vec) {
    size_t i;
    for (i = 1; i < vec->size; i++) {
        if (vec->data[i] == vec->data[i - 1]) return 0;
    }
    return 1;
}

static int find_key_sorted(const KeyVector *vec, Mode mode, long long key, long long *comparisons) {
    if (mode == MODE_MERGE_ITER) {
        return binary_search_iterative(vec->data, vec->size, key, comparisons);
    }
    return binary_search_recursive(vec->data, 0, vec->size, key, comparisons);
}

static int process_sequential(const char *input_name, const char *dest_name, int input_key_col, int dest_key_col, Mode mode,
                              long long *read_count, long long *inserted, long long *duplicates, long long *comparisons) {
    FILE *input = fopen(input_name, "r");
    FILE *dest = NULL;
    char line[MAX_LINE_LEN];
    long long key;
    if (input == NULL) { perror("Falha ao abrir arquivo de novas entradas"); return 0; }
    dest = fopen(dest_name, "r+");
    if (dest == NULL) { perror("Falha ao abrir arquivo de destino"); fclose(input); return 0; }

    if (fgets(line, sizeof(line), input) == NULL) {
        fclose(input); fclose(dest); return 1;
    }

    while (fgets(line, sizeof(line), input) != NULL) {
        int exists;
        long long local_cmp = 0;
        if (line[0] == '\n' || line[0] == '\r' || line[0] == '\0') continue;
        (*read_count)++;
        if (!csv_parse_key(line, input_key_col, &key)) {
            fprintf(stderr, "Entrada ignorada: identificador invalido.\n");
            continue;
        }

        if (mode == MODE_SEQ_ITER) exists = sequential_search_iterative(dest, dest_key_col, key, &local_cmp);
        else exists = sequential_search_recursive(dest, dest_key_col, key, &local_cmp);
        *comparisons += local_cmp;

        if (exists) {
            (*duplicates)++;
            continue;
        }

        fseek(dest, 0, SEEK_END);
        if (!csv_append_row(dest, line)) {
            fprintf(stderr, "Falha ao inserir registro no destino.\n");
            fclose(input); fclose(dest); return 0;
        }
        (*inserted)++;
    }

    fclose(input);
    fclose(dest);
    return 1;
}

static int process_binary(const char *input_name, const char *dest_name, int input_key_col, int dest_key_col, Mode mode,
                          long long *read_count, long long *inserted, long long *duplicates, long long *comparisons,
                          size_t *initial_n) {
    FILE *input = fopen(input_name, "r");
    FILE *dest = NULL;
    char line[MAX_LINE_LEN];
    long long key;
    KeyVector vec;
    long long *aux = NULL;
    if (input == NULL) { perror("Falha ao abrir arquivo de novas entradas"); return 0; }
    dest = fopen(dest_name, "r+");
    if (dest == NULL) { perror("Falha ao abrir arquivo de destino"); fclose(input); return 0; }
    if (!vector_init(&vec, INITIAL_CAPACITY)) { fclose(input); fclose(dest); return 0; }

    if (!load_destination_keys(dest, dest_key_col, &vec)) {
        vector_destroy(&vec); fclose(input); fclose(dest); return 0;
    }
    *initial_n = vec.size;
    if (vec.size > 0) {
        aux = (long long *)malloc(vec.size * sizeof(long long));
        if (aux == NULL) { vector_destroy(&vec); fclose(input); fclose(dest); return 0; }
        if (mode == MODE_MERGE_ITER) mergesort_iterative(vec.data, aux, vec.size);
        else mergesort_recursive(vec.data, aux, 0, vec.size);
        free(aux);
        aux = NULL;
        if (!vector_is_unique_sorted(&vec)) {
            fprintf(stderr, "O arquivo de destino ja possui identificadores duplicados.\n");
            vector_destroy(&vec); fclose(input); fclose(dest); return 0;
        }
    }

    if (fgets(line, sizeof(line), input) == NULL) {
        vector_destroy(&vec); fclose(input); fclose(dest); return 1;
    }

    while (fgets(line, sizeof(line), input) != NULL) {
        int exists;
        size_t pos;
        long long local_cmp = 0;
        if (line[0] == '\n' || line[0] == '\r' || line[0] == '\0') continue;
        (*read_count)++;
        if (!csv_parse_key(line, input_key_col, &key)) {
            fprintf(stderr, "Entrada ignorada: identificador invalido.\n");
            continue;
        }

        exists = find_key_sorted(&vec, mode, key, &local_cmp);
        *comparisons += local_cmp;
        if (exists) {
            (*duplicates)++;
            continue;
        }

        pos = lower_bound(vec.data, vec.size, key);
        fseek(dest, 0, SEEK_END);
        if (!csv_append_row(dest, line) || !vector_insert_sorted(&vec, key, pos)) {
            fprintf(stderr, "Falha ao inserir registro no destino/estrutura ordenada.\n");
            vector_destroy(&vec); fclose(input); fclose(dest); return 0;
        }
        (*inserted)++;
    }

    vector_destroy(&vec);
    fclose(input);
    fclose(dest);
    return 1;
}

int main(int argc, char **argv) {
    const char *input_name;
    const char *dest_name;
    Mode mode;
    FILE *input = NULL, *dest = NULL;
    char input_header[MAX_LINE_LEN], dest_header[MAX_LINE_LEN];
    int input_key_col, dest_key_col;
    long long read_count = 0, inserted = 0, duplicates = 0, comparisons = 0;
    size_t initial_n = 0;
    int ok;

    if (argc < 3 || argc > 4) { usage(argv[0]); return EXIT_FAILURE; }
    input_name = argv[1];
    dest_name = argv[2];
    if (!parse_mode(argc == 4 ? argv[3] : NULL, &mode)) { usage(argv[0]); return EXIT_FAILURE; }

    input = fopen(input_name, "r");
    if (input == NULL) { perror("Falha ao abrir novas entradas"); return EXIT_FAILURE; }
    dest = fopen(dest_name, "r");
    if (dest == NULL) { perror("Falha ao abrir destino"); fclose(input); return EXIT_FAILURE; }

    if (fgets(input_header, sizeof(input_header), input) == NULL || fgets(dest_header, sizeof(dest_header), dest) == NULL) {
        fprintf(stderr, "Os dois CSVs precisam possuir cabecalho.\n");
        fclose(input); fclose(dest); return EXIT_FAILURE;
    }
    input_key_col = csv_find_column(input_header, KEY_COLUMN_NAME);
    dest_key_col = csv_find_column(dest_header, KEY_COLUMN_NAME);
    if (input_key_col < 0 || dest_key_col < 0) {
        fprintf(stderr, "A coluna '%s' precisa existir nos dois arquivos.\n", KEY_COLUMN_NAME);
        fclose(input); fclose(dest); return EXIT_FAILURE;
    }
    fclose(input);
    fclose(dest);

    if (!csv_ensure_trailing_newline(dest_name)) {
        fprintf(stderr, "Nao foi possivel preparar o arquivo de destino para append.\n");
        return EXIT_FAILURE;
    }

    if (mode == MODE_SEQ_ITER || mode == MODE_SEQ_REC) {
        ok = process_sequential(input_name, dest_name, input_key_col, dest_key_col, mode,
                                &read_count, &inserted, &duplicates, &comparisons);
    } else {
        ok = process_binary(input_name, dest_name, input_key_col, dest_key_col, mode,
                            &read_count, &inserted, &duplicates, &comparisons, &initial_n);
    }

    if (!ok) return EXIT_FAILURE;

    printf("Modo: %s\n", mode == MODE_SEQ_ITER ? "busca sequencial iterativa" :
           mode == MODE_SEQ_REC ? "busca sequencial recursiva" :
           mode == MODE_MERGE_ITER ? "MergeSort iterativo + busca binaria iterativa" :
           "MergeSort recursivo + busca binaria recursiva");
    printf("Registros lidos: %lld\n", read_count);
    printf("Inseridos: %lld\n", inserted);
    printf("Duplicados rejeitados: %lld\n", duplicates);
    printf("Comparacoes de chave: %lld\n", comparisons);
    if (mode == MODE_MERGE_ITER || mode == MODE_MERGE_REC) {
        printf("Registros iniciais carregados na RAM: %zu\n", initial_n);
    }
    return EXIT_SUCCESS;
}
