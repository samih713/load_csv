#define CXV_IMPLEMENTATION
#include "cxv.h"

// things to improve
/*
[] cfg improve with sensible defaults
[] reduce allocation
*/

typedef struct point_s {
  size_t x[11];
  size_t y[11];
  char* headers[2];
} point_t;

void set_point(char** values, int row, void* ctx) {
  point_t* points = (point_t*)ctx;
  if (row == 0) {
    points->headers[0] = strdup(values[0]);
    points->headers[1] = strdup(values[1]);
  }
  points->x[row] = atoi(values[0]);
  points->y[row] = atoi(values[1]);
}

int main() {
  const char* csv_path = "data.csv";

  point_t points;
  cxv_cfg_t c = {.n_cols = 2, .del = ',', .r_to_read = 10};

  if (cxv_read(csv_path, c, &points, set_point) == -1) {
    return -1;
  }

  for (size_t i = 0; i < 10; ++i) {
    if (i == 0) {
      printf("[%s, %s]\n", points.headers[0], points.headers[1]);
    } else {
      printf("[%zu, %zu]\n", points.x[i], points.y[i]);
    }
  }

  free(points.headers[0]);
  free(points.headers[1]);

  return 0;
}