#ifndef LOAD_CSV_H
#define LOAD_CSV_H

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <stdbool.h>
#include <stddef.h>

typedef struct config_s
{
    size_t n_cols;
    size_t e_size;
    char del;
    bool first_row;
} config_t;

FILE *load_csv(const char *fpath);
size_t read_csv(FILE *csv, config_t c, void *data, size_t n, void (*f)(char **, void *));

#ifndef MAX_ROW_SIZE
#define MAX_ROW_SIZE 1024
#endif // MAX_ROW_SIZE

// IMPLEMENTATION
#if defined(LOAD_CSV_H_IMPLEMENTATION)
#define LOAD_CSV_H_IMPLEMENTATION

static void free_values(size_t n_cols, char **values);
static char **process_row(char *line, size_t ncols, char del);

FILE *load_csv(const char *fpath)
{
    FILE *fp = fopen(fpath, "rb");
    if (!fp)
    {
        fprintf(stderr, "Failed to read file: %s", strerror(errno));
        return NULL;
    }
    return fp;
}

size_t read_csv(FILE *csv, config_t c, void *data, size_t n, void (*f)(char **, void *))
{
    char line[MAX_ROW_SIZE] = {0};
    size_t row = 0;

    while (fgets(line, MAX_ROW_SIZE, csv) && row < n)
    {
        if (c.first_row)
        {
            c.first_row = false;
            continue;
        }
        char **values = process_row(line, c.n_cols, c.del);
        f(values, (char *)data + (row * c.e_size));
        free_values(c.n_cols, values);
        row++;
    }
    return row;
}

static char **process_row(char *line, size_t ncols, char del)
{
    char **values = (char **)malloc(sizeof(char *) * ncols);
    if (!values)
    {
        fprintf(stderr, "Failed to malloc\n");
        return NULL;
    }

    size_t j = 0;
    size_t start = 0;
    size_t value_size = 0;

    for (size_t i = 0; line[i]; ++i)
    {
        if (strchr("\n\r\0", line[i]) || line[i] == del)
        {
            value_size = i - start;
            values[j] = (char *)malloc((sizeof(char) * value_size + 1));

            if (!values[j])
            {
                fprintf(stderr, "Failed to malloc\n");
                free_values(j, values);
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

static void free_values(size_t n_cols, char **values)
{
    for (size_t j = 0; j < n_cols; ++j)
    {
        free(values[j]);
    }
    free(values);
}
#endif // LOAD_CSV_H_IMPLEMENTATION

#endif // LOAD_CSV_H