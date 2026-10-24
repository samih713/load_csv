#define CXV_IMPLEMENTATION
#include "cxv.h"

typedef struct point_s {
  size_t x;
  size_t y;
} point_t;

void set_point(char** values, void* p) {
  point_t* points = (point_t*)p;
  points->x = atoi(values[0]);
  points->y = atoi(values[1]);
}

int main() {
  const char* csv_path = "tests/data/uneven_columns.csv";

  point_t points[10];
  cxv_cfg_t c = {.e_size = sizeof(point_t),
                 .n_cols = 2,
                 .first_row = true,
                 .del = ',',
                 .r_to_read = 10};

  if(cxv_read(csv_path, c, points, set_point) == -1) {
    return -1;
  }

  for (size_t i = 0; i < 10; ++i) {
    printf("[%zu, %zu]\n", points[i].x, points[i].y);
  }
  return 0;
}