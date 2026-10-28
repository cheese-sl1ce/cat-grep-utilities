#include "s21_grep.h"

pattern_list* CreatePatternList(void) {
  pattern_list* pats = (pattern_list*)calloc(1, sizeof(pattern_list));
  if (pats != NULL) {
    pats->patterns = (char**)calloc(MAX_PATTERNS, sizeof(char*));
    if (pats->patterns == NULL) {
      free(pats);
      pats = NULL;
    }
  }
  return pats;
}

void FreePatternList(pattern_list* pats) {
  if (pats == NULL) return;

  if (pats->patterns != NULL) {
    int i = 0;
    while (i < pats->count) {
      if (pats->patterns[i] != NULL) {
        free(pats->patterns[i]);
      }
      i++;
    }
    free(pats->patterns);
  }
  free(pats);
}

void AppendPatternStr(pattern_list* pats, const char* start, size_t len) {
  if (pats == NULL || start == NULL || pats->patterns == NULL) return;

  if (pats->count < MAX_PATTERNS) {
    pats->patterns[pats->count] = (char*)calloc(len + 1, sizeof(char));
    if (pats->patterns[pats->count] != NULL) {
      strncpy(pats->patterns[pats->count], start, len);
      pats->patterns[pats->count][len] = '\0';
      pats->count++;
    }
  }
}

void AddPattern(pattern_list* pats, const char* new_pat) {
  if (pats == NULL || new_pat == NULL) return;

  if (new_pat[0] == '\0') {
    AppendPatternStr(pats, "", 0);
  } else {
    const char* start = new_pat;
    while (*start != '\0' && pats->count < MAX_PATTERNS) {
      const char* end = strchr(start, '\n');
      size_t len = (end != NULL) ? (size_t)(end - start) : strlen(start);
      AppendPatternStr(pats, start, len);
      start = (end != NULL) ? end + 1 : start + len;
    }
  }
}

int ParseFlags(int argc, char* argv[], grep_flags* flags, pattern_list* pats) {
  if (argc <= 0 || argv == NULL || flags == NULL || pats == NULL) return 1;

  int is_error = 0;
  int opt = 0;

  while ((opt = getopt(argc, argv, "e:ivcln")) != -1 && is_error == 0) {
    if (opt == 'e') {
      flags->e = 1;
      AddPattern(pats, optarg);
    } else if (opt == 'i') {
      flags->i = 1;
    } else if (opt == 'v') {
      flags->v = 1;
    } else if (opt == 'c') {
      flags->c = 1;
    } else if (opt == 'l') {
      flags->l = 1;
    } else if (opt == 'n') {
      flags->n = 1;
    } else {
      is_error = 1;
    }
  }

  return is_error;
}

void PrintMatch(grep_flags flags, const char* filename, int file_count,
                int line_num, const char* line, ssize_t read_bytes) {
  if (flags.c == 0 && flags.l == 0) {
    if (file_count > 1) {
      printf("%s:", filename);
    }
    if (flags.n) {
      printf("%d:", line_num);
    }
    printf("%s", line);
    if (read_bytes > 0 && line[read_bytes - 1] != '\n') {
      printf("\n");
    }
  }
}

void PrintSummary(grep_flags flags, const char* filename, int file_count,
                  int match_count) {
  if (flags.l) {
    if (match_count > 0) {
      printf("%s\n", filename);
    }
  } else if (flags.c) {
    if (file_count > 1) {
      printf("%s:", filename);
    }
    printf("%d\n", match_count);
  }
}

int MatchLine(const regex_t* regexes, int regex_count, const char* line) {
  if (regexes == NULL || line == NULL) return 0;

  int is_match = 0;
  int i = 0;
  while (i < regex_count && is_match == 0) {
    if (regexec(&regexes[i], line, 0, NULL, 0) == 0) {
      is_match = 1;
    }
    i++;
  }
  return is_match;
}

int CompileRegexes(const pattern_list* pats, int cflags, regex_t* regexes) {
  if (pats == NULL || regexes == NULL) return -1;

  int compiled_count = 0;
  int compile_error = 0;
  int k = 0;
  while (k < pats->count && compile_error == 0) {
    if (regcomp(&regexes[k], pats->patterns[k], cflags) == 0) {
      compiled_count++;
    } else {
      compile_error = 1;
    }
    k++;
  }
  if (compile_error) {
    FreeRegexes(regexes, compiled_count);
    compiled_count = -1;
  }
  return compiled_count;
}

void FreeRegexes(regex_t* regexes, int count) {
  if (regexes == NULL) return;

  int j = 0;
  while (j < count) {
    regfree(&regexes[j]);
    j++;
  }
}

int ProcessFileLines(FILE* fp, grep_flags flags, const regex_t* regexes,
                     int compiled_count, const char* filename, int file_count) {
  if (fp == NULL || regexes == NULL) return -1;

  int file_matches = 0;
  char* line = NULL;
  size_t len = 0;
  ssize_t read_bytes = 0;
  int line_num = 0;

  while ((read_bytes = getline(&line, &len, fp)) != -1) {
    line_num++;
    int is_match = MatchLine(regexes, compiled_count, line);

    if (flags.v) {
      is_match = !is_match;
    }

    if (is_match) {
      file_matches++;
      PrintMatch(flags, filename, file_count, line_num, line, read_bytes);
    }
  }

  PrintSummary(flags, filename, file_count, file_matches);
  free(line);
  return file_matches;
}

int ProcessOpenedFile(FILE* fp, grep_flags flags, const pattern_list* pats,
                      const char* filename, int file_count) {
  if (fp == NULL || pats == NULL) return -1;

  int file_matches = -1;
  regex_t* regexes = (regex_t*)calloc(MAX_PATTERNS, sizeof(regex_t));
  if (regexes != NULL) {
    int cflags = REG_NEWLINE | (flags.i ? REG_ICASE : 0);
    int compiled_count = CompileRegexes(pats, cflags, regexes);

    if (compiled_count >= 0) {
      file_matches = ProcessFileLines(fp, flags, regexes, compiled_count,
                                      filename, file_count);
      FreeRegexes(regexes, compiled_count);
    }
    free(regexes);
  }
  return file_matches;
}

int ProcessFile(grep_flags flags, const pattern_list* pats,
                const char* filename, int file_count) {
  if (pats == NULL || filename == NULL) return -1;

  int file_matches = -1;
  FILE* fp = fopen(filename, "r");

  if (fp == NULL) {
    fprintf(stderr, "s21_grep: %s: No such file or directory\n", filename);
  } else {
    file_matches = ProcessOpenedFile(fp, flags, pats, filename, file_count);
    fclose(fp);
  }

  return file_matches;
}

int ProcessAllFiles(grep_flags flags, const pattern_list* pats, int argc,
                    char* argv[], int* total_matches) {
  if (pats == NULL || argv == NULL || total_matches == NULL) return 1;

  int is_error = 0;
  int file_count = argc - optind;
  int i = optind;

  while (i < argc) {
    int res = ProcessFile(flags, pats, argv[i], file_count);
    if (res < 0) {
      is_error = 1;
    } else {
      *total_matches += res;
    }
    i++;
  }

  return is_error;
}

int main(int argc, char* argv[]) {
  pattern_list* pats = CreatePatternList();
  if (pats == NULL) return 2;

  grep_flags flags = {0};
  int return_code = ParseFlags(argc, argv, &flags, pats);

  if (return_code == 0 && flags.e == 0) {
    if (optind < argc) {
      AddPattern(pats, argv[optind]);
      optind++;
    } else {
      return_code = 2;
    }
  }

  if (return_code == 0) {
    int total_matches = 0;
    if (ProcessAllFiles(flags, pats, argc, argv, &total_matches) != 0) {
      return_code = 2;
    } else if (total_matches == 0) {
      return_code = 1;
    }
  }

  FreePatternList(pats);
  return return_code;
}