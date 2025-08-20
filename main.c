#define LOAD_CSV_H_IMPLEMENTATION
#include "load_csv.h"


typedef struct point_s
{
    size_t x;
    size_t y;
} point_t;

void set_point(char **values, void *p)
{
    point_t *points = (point_t *)p;
    points->x = atoi(values[0]);
    points->y = atoi(values[1]);
}

int main()
{
    point_t points[10];

    config_t c = {.e_size = sizeof(point_t), .n_cols = 2, .first_row = true, .del=','};

    const char *csv_path = "data.csv";
    FILE *csv = load_csv(csv_path);
    read_csv(csv, c, points, 10, set_point);
    for (size_t i = 0; i < 10; ++i)
    {
        printf("[%zu, %zu]\n", points[i].x, points[i].y);
    }
    return 0;
}