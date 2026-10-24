#define CXV_IMPLEMENTATION
#include "cxv.h"

typedef struct point_s {
  size_t x[10];
  size_t y[10];
} point_t;

void set_point(char** values, int row, void* ctx) {
  point_t* points = (point_t*)ctx;
  points->x[row] = atoi(values[0]);
  points->y[row] = atoi(values[1]);
}

int main() {
  const char* csv_path = "data.csv";

  point_t points;
  cxv_cfg_t c = {.n_cols = 2, .first_row = true, .del = ',', .r_to_read = 10};

  if (cxv_read(csv_path, c, &points, set_point) == -1) {
    return -1;
  }

  for (size_t i = 0; i < 10; ++i) {
    printf("[%zu, %zu]\n", points.x[i], points.y[i]);
  }
  return 0;
}