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
  size_t e_size;
  size_t r_to_read;
  char del;
  bool first_row;
} cxv_cfg_t;

// API
size_t cxv_read(const char* file_path, cxv_cfg_t c, void* data,
                void (*f)(char**, void*));

#ifndef CXV_MAX_ROW_SIZE
#define CXV_MAX_ROW_SIZE 1024
#endif  // MAX_ROW_SIZE

// IMPLEMENTATION
#if defined(CXV_IMPLEMENTATION)

static void cxv__free_values(size_t n_cols, char** values);
static char** cxv__process_row(char* line, size_t ncols, char del);
static FILE* cxv__load(const char* fpath);

size_t cxv_read(const char* file_path, cxv_cfg_t c, void* data,
                void (*f)(char**, void*)) {
  size_t processed_rows = 0;
  FILE* csv = cxv__load(file_path);
  if (csv == NULL) {
    return processed_rows;
  }

  char line[CXV_MAX_ROW_SIZE] = {0};

  while (fgets(line, CXV_MAX_ROW_SIZE, csv) && processed_rows < c.r_to_read) {
    if (c.first_row) {
      c.first_row = false;
      continue;
    }
    char** values = cxv__process_row(line, c.n_cols, c.del);
    f(values, (char*)data + (processed_rows * c.e_size));
    cxv__free_values(c.n_cols, values);
    processed_rows++;
  }
  return processed_rows;
}

static FILE* cxv__load(const char* fpath) {
  FILE* fp = fopen(fpath, "rb");
  if (!fp) {
    fprintf(stderr, "Failed to read file: %s", strerror(errno));
    return NULL;
  }
  return fp;
}

static char** cxv__process_row(char* line, size_t ncols, char del) {
  char** values = (char**)malloc(sizeof(char*) * ncols);
  if (!values) {
    fprintf(stderr, "Failed to malloc\n");
    return NULL;
  }

  size_t j = 0;
  size_t start = 0;
  size_t value_size = 0;

  for (size_t i = 0; line[i]; ++i) {
    if (strchr("\n\r\0", line[i]) || line[i] == del) {
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