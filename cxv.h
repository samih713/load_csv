#ifndef CXV_H
#define CXV_H

#include <errno.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
  size_t n_cols;
  size_t r_to_read;
  char del;
  bool first_row;
} cxv_cfg_t;

typedef void (*cxv_row_fn)(char**, int row, void*);
// API
int cxv_read(const char* file_path, cxv_cfg_t cfg, void* ctx, cxv_row_fn f);

#ifndef CXV_MAX_ROW_SIZE
#define CXV_MAX_ROW_SIZE 1024
#endif  // MAX_ROW_SIZE

// IMPLEMENTATION
#if defined(CXV_IMPLEMENTATION)

static void cxv__free_values(size_t n_cols, char** values);
static char** cxv__split(char* line, size_t n_cols, char del,
                         const size_t line_n);
static FILE* cxv__load(const char* fpath);

int cxv_read(const char* file_path, cxv_cfg_t cfg, void* ctx, cxv_row_fn f) {
  FILE* csv = cxv__load(file_path);
  if (csv == NULL) {
    return -1;
  }

  char line[CXV_MAX_ROW_SIZE] = {0};
  int processed_row_count = 0;

  while (fgets(line, CXV_MAX_ROW_SIZE, csv) &&
         processed_row_count < cfg.r_to_read) {
    if (cfg.first_row) {
      cfg.first_row = false;
      continue;
    }
    char** values = cxv__split(line, cfg.n_cols, cfg.del, processed_row_count);
    if (values == NULL) {
      fclose(csv);
      return -1;
    }
    f(values, processed_row_count, ctx);
    cxv__free_values(cfg.n_cols, values);
    processed_row_count++;
  }
  fclose(csv);
  return processed_row_count;
}

static FILE* cxv__load(const char* fpath) {
  FILE* fp = fopen(fpath, "rb");
  if (!fp) {
    fprintf(stderr, "Failed to read file: %s\n", strerror(errno));
    return NULL;
  }
  return fp;
}

static char** cxv__split(char* line, size_t n_cols, char del,
                         const size_t line_n) {
  char** values = (char**)malloc(sizeof(char*) * n_cols);
  if (!values) {
    fprintf(stderr, "Failed to malloc\n");
    return NULL;
  }

  size_t j = 0;
  size_t start = 0;
  size_t value_size = 0;

  for (size_t i = 0; line[i]; ++i) {
    if (strchr("\n\r\0", line[i]) || line[i] == del) {
      if (j >= n_cols) {
        fprintf(stderr, "line %zu: expected %zu columns, found %zu\n", line_n,
                n_cols, j + 1);
        cxv__free_values(j, values);
        return NULL;
      }
      value_size = i - start;
      values[j] = (char*)malloc((sizeof(char) * value_size + 1));

      if (!values[j]) {
        fprintf(stderr, "Failed to malloc\n");
        cxv__free_values(j, values);
        return NULL;
      }

      strncpy(values[j], &line[start], value_size);
      values[j][value_size] = 0;
      start = i + 1;
      j++;
    }
  }
  if (j != n_cols) {
    fprintf(stderr, "line %zu: expected %zu columns, found %zu\n", line_n,
            n_cols, j);
    cxv__free_values(j, values);
    return NULL;
  }
  return values;
}

static void cxv__free_values(size_t n_cols, char** values) {
  for (size_t j = 0; j < n_cols; ++j) {
    free(values[j]);
  }
  free(values);
}
#endif  // CXV__IMPLEMENTATION
#endif  // CXV_H