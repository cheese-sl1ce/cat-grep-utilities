#ifndef S21_CAT_H
#define S21_CAT_H

#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct {
  int b;
  int e;
  int v;
  int n;
  int s;
  int t;
} cat_flags;

int ParseFlags(int argc, char *argv[], cat_flags *flags);
// Измененный прототип функции для передачи состояния по указателям
void ProcessFile(FILE *fp, cat_flags flags, int *line_count, int *prev_ch, int *empty_line_count);
void HandleLineNumbering(int ch, int prev_ch, int *line_count, cat_flags flags);
int HandleNonPrintable(int ch, cat_flags flags);

#endif  // S21_CAT_H
