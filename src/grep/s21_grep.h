#ifndef S21_GREP_H
#define S21_GREP_H

#define _GNU_SOURCE
#include <getopt.h>
#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFFER_SIZE 8192

typedef struct {
  int e;
  int i;
  int v;
  int c;
  int l;
  int n;
} grep_flags;

void AddPattern(char *pattern, const char *new_pat);
int ParseFlags(int argc, char *argv[], grep_flags *flags, char *pattern);
void PrintMatch(grep_flags flags, char *filename, int file_count, int line_num, char *line, ssize_t read_bytes);
void ProcessFile(grep_flags flags, char *pattern, char *filename, int file_count);

#endif  // S21_GREP_H