#ifndef S21_GREP_H_
#define S21_GREP_H_

#define _POSIX_C_SOURCE 200809L
#include <getopt.h>
#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFFER_SIZE 8192
#define MAX_PATTERNS 256

typedef struct {
  int e;
  int i;
  int v;
  int c;
  int l;
  int n;
} grep_flags;

typedef struct {
  char** patterns;
  int count;
} pattern_list;

pattern_list* CreatePatternList(void);
void FreePatternList(pattern_list* pats);
void AppendPatternStr(pattern_list* pats, const char* start, size_t len);
void AddPattern(pattern_list* pats, const char* new_pat);
int ParseFlags(int argc, char* argv[], grep_flags* flags, pattern_list* pats);
void PrintMatch(grep_flags flags, const char* filename, int file_count,
                int line_num, const char* line, ssize_t read_bytes);
void PrintSummary(grep_flags flags, const char* filename, int file_count,
                  int match_count);
int MatchLine(const regex_t* regexes, int regex_count, const char* line);
int CompileRegexes(const pattern_list* pats, int cflags, regex_t* regexes);
void FreeRegexes(regex_t* regexes, int count);
int ProcessFileLines(FILE* fp, grep_flags flags, const regex_t* regexes,
                     int compiled_count, const char* filename, int file_count);
int ProcessOpenedFile(FILE* fp, grep_flags flags, const pattern_list* pats,
                      const char* filename, int file_count);
int ProcessFile(grep_flags flags, const pattern_list* pats,
                const char* filename, int file_count);
int ProcessAllFiles(grep_flags flags, const pattern_list* pats, int argc,
                    char* argv[], int* total_matches);

#endif  // S21_GREP_H_