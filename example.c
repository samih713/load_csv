// example.c - how to use cxv
//
// Reads data.csv (a header row followed by two numeric columns) into a
// struct and prints it as a table.
//
//   make run
//
// cxv is a single header library. In exactly one .c file, define
// CXV_IMPLEMENTATION before including it; everywhere else just include it.
#define CXV_IMPLEMENTATION
#include "cxv.h"

#define MAX_ROWS 25  // header row + 24 data rows

// 1. Describe where the parsed data goes.
//    cxv doesn't allocate storage for you, it hands every row to a callback
//    together with a user supplied pointer (ctx). Here ctx is a table_t.
typedef struct {
  size_t x[MAX_ROWS - 1];
  size_t y[MAX_ROWS - 1];
  char* headers[2];
} table_t;

// 2. Write the row callback.
//    - values: the n_cols fields of this row, as strings
//    - row:    0 based index, the header is row 0 like any other line
//    - ctx:    the pointer passed to cxv_read
//    values is freed by cxv as soon as the callback returns, so copy
//    anything you want to keep (strdup for strings, atoi/strtol for numbers).
static void set_table(char** values, int row, void* ctx) {
  table_t* table = (table_t*)ctx;

  if (row == 0) {
    table->headers[0] = strdup(values[0]);
    table->headers[1] = strdup(values[1]);
    return;
  }

  table->x[row - 1] = atoi(values[0]);
  table->y[row - 1] = atoi(values[1]);
}

// Prints the table, defined at the bottom of the file.
static void print_table(const table_t* t, size_t n_rows);

int main(void) {
  // 3. Configure the read.
  //    n_cols:   columns every row must have, a mismatch fails the read
  //    delim:    field delimiter
  //    max_rows: max rows to read, the header counts as one
  cxv_cfg_t cfg = {.n_cols = 2, .delim = ',', .max_rows = MAX_ROWS};

  // 4. Read. Returns the number of rows passed to the callback, or -1 on
  //    failure (unreadable file, wrong column count, out of memory). The
  //    reason is printed to stderr.
  table_t table = {0};
  int rows_read = cxv_read("data.csv", cfg, &table, set_table);
  if (rows_read == -1) {
    return 1;
  }

  print_table(&table, rows_read);

  // The strings copied in the callback are ours to free.
  free(table.headers[0]);
  free(table.headers[1]);
  return 0;
}

// ---------------------------------------------------------------------------
// Printing, nothing here is specific to cxv.
// ---------------------------------------------------------------------------

static int digits(size_t n) {
  int d = 1;
  while (n >= 10) {
    n /= 10;
    d++;
  }
  return d;
}

// prints "<left>" + "─" * (w[0] + 2) + "<mid>" + ... + "<right>\n"
static void print_rule(const char* left, const char* mid, const char* right,
                       const int* w, size_t n_cols) {
  fputs(left, stdout);
  for (size_t c = 0; c < n_cols; ++c) {
    for (int i = 0; i < w[c] + 2; ++i) {
      fputs("─", stdout);
    }
    fputs(c + 1 < n_cols ? mid : right, stdout);
  }
  fputc('\n', stdout);
}

// n_rows counts the header, data rows are stored in x/y[0..n_rows - 2]
static void print_table(const table_t* t, size_t n_rows) {
  size_t n_data = n_rows - 1;
  int w[2] = {(int)strlen(t->headers[0]), (int)strlen(t->headers[1])};
  for (size_t i = 0; i < n_data; ++i) {
    int dx = digits(t->x[i]);
    int dy = digits(t->y[i]);
    if (dx > w[0]) w[0] = dx;
    if (dy > w[1]) w[1] = dy;
  }

  print_rule("┌", "┬", "┐", w, 2);
  printf("│ %-*s │ %-*s │\n", w[0], t->headers[0], w[1], t->headers[1]);
  print_rule("├", "┼", "┤", w, 2);
  for (size_t i = 0; i < n_data; ++i) {
    printf("│ %*zu │ %*zu │\n", w[0], t->x[i], w[1], t->y[i]);
  }
  print_rule("└", "┴", "┘", w, 2);
  printf("%zu rows\n", n_data);
}
