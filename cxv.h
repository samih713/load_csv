#ifndef CXV_H
#define CXV_H

#include <stddef.h>

#if defined(__cplusplus)
extern "C" {
#endif  // __cplusplus

typedef void (*cxv_row_fn)(char** values, int row, void* ctx);

typedef struct {
  size_t n_cols;
  int max_rows;
  char delim;
} cxv_cfg_t;

int cxv_read(const char* file_path, cxv_cfg_t cfg, void* ctx, cxv_row_fn f);

#if defined(__cplusplus)
}
#endif  // __cplusplus
#endif  // CXV_H

// IMPLEMENTATION
#if defined(CXV_IMPLEMENTATION) && !defined(CXV_IMPLEMENTATION_DONE)
#define CXV_IMPLEMENTATION_DONE

#ifndef CXV_MAX_ROW_SIZE
#define CXV_MAX_ROW_SIZE 1024
#endif  // CXV_MAX_ROW_SIZE

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int cxv__split(char* line, size_t n_cols, char delim, size_t line_n,
                      char** cxv__values);
static FILE* cxv__load(const char* fpath);

int cxv_read(const char* file_path, cxv_cfg_t cfg, void* ctx, cxv_row_fn f) {
  FILE* csv = cxv__load(file_path);
  if (csv == NULL) {
    return -1;
  }

  char** values = (char**)malloc(sizeof *values * cfg.n_cols);
  if (!values) {
    fprintf(stderr, "Failed to malloc\n");
    fclose(csv);
    return -1;
  }

  char line[CXV_MAX_ROW_SIZE] = {0};
  int processed_row_count = 0;
  while (fgets(line, CXV_MAX_ROW_SIZE, csv) &&
         processed_row_count < cfg.max_rows) {
    int status = cxv__split(line, cfg.n_cols, cfg.delim, processed_row_count,
                            values);

    if (status < 0) {
      free(values);
      fclose(csv);
      return -1;
    }
    f(values, processed_row_count, ctx);
    processed_row_count++;
  }
  free(values);
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

static int cxv__split(char* line, size_t n_cols, char delim, size_t line_n,
                      char** values) {
  size_t j = 0;
  size_t start = 0;

  for (size_t i = 0; line[i]; ++i) {
    char c = line[i];
    if (c == delim || c == '\n' || c == '\r') {
      if (j >= n_cols) {
        fprintf(stderr, "row %zu: expected %zu columns, found %zu\n", line_n,
                n_cols, j + 1);
        return -1;
      }
      line[i] = '\0';
      values[j] = line + start;
      start = i + 1;
      j++;
    }
  }
  if (j != n_cols) {
    fprintf(stderr, "row %zu: expected %zu columns, found %zu\n", line_n,
            n_cols, j);
    return -1;
  }
  return 0;
}

#endif  // CXV_IMPLEMENTATION
