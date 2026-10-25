#ifndef CXV_H
#define CXV_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

typedef void (*cxv_row_fn)(char**, int row, void*);

typedef struct {
  size_t n_cols;
  int r_to_read;
  char del;
} cxv_cfg_t;

// API
int cxv_read(const char* file_path, cxv_cfg_t cfg, void* ctx, cxv_row_fn f);

#ifdef __cplusplus
}
#endif  // __cplusplus

#ifndef CXV_MAX_ROW_SIZE
#define CXV_MAX_ROW_SIZE 1024
#endif  // CXV_MAX_ROW_SIZE

// IMPLEMENTATION
#if defined(CXV_IMPLEMENTATION)

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
    char** values = cxv__split(line, cfg.n_cols, cfg.del, processed_row_count);
    if (values == NULL) {
      fclose(csv);
      return -1;
    }
    f(values, processed_row_count, ctx);
    free(values);
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

  for (size_t i = 0; line[i]; ++i) {
    if (strchr("\n\r\0", line[i]) || line[i] == del) {
      if (j >= n_cols) {
        fprintf(stderr, "line %zu: expected %zu columns, found %zu\n", line_n,
                n_cols, j + 1);
        free(values);
        return NULL;
      }
      line[i] = '\0';
      values[j] = &(line[start]);
      start = i + 1;
      j++;
    }
  }
  if (j != n_cols) {
    fprintf(stderr, "line %zu: expected %zu columns, found %zu\n", line_n,
            n_cols, j);
    free(values);
    return NULL;
  }
  return values;
}

#endif  // CXV_IMPLEMENTATION
#endif  // CXV_H