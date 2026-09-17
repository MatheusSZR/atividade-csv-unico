#include "binary_search.h"
#include "csv_utils.h"
#include "mergesort.h"
#include "sequential_search.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INITIAL_CAPACITY 1024

typedef enum {
    MODE_SEQ_ITER,
    MODE_SEQ_REC,
    MODE_MERGE_ITER,
    MODE_MERGE_REC
} Mode;

typedef struct {
    long long *data;
    size_t size;
    size_t capacity;
} KeyVector;

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
