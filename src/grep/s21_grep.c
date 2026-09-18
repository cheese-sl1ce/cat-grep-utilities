#include "s21_grep.h"

void AddPattern(char *pattern, const char *new_pat) {
  if (pattern[0] != '\0') {
    strcat(pattern, "|");
  }
  strcat(pattern, new_pat);
}

int ParseFlags(int argc, char *argv[], grep_flags *flags, char *pattern) {
  int is_error = 0;
  int opt;
  
  while ((opt = getopt(argc, argv, "e:ivcln")) != -1 && is_error == 0) {
    switch (opt) {
      case 'e':
        flags->e = 1;
        AddPattern(pattern, optarg);
        break;
      case 'i':
        flags->i = 1;
        break;
      case 'v':
        flags->v = 1;
        break;
      case 'c':
        flags->c = 1;
        break;
      case 'l':
        flags->l = 1;
        break;
      case 'n':
        flags->n = 1;
        break;
      default:
        is_error = 1;
        break;
    }
  }
  
  return is_error;
}

void PrintMatch(grep_flags flags, char *filename, int file_count, int line_num, char *line, ssize_t read_bytes) {
  if (flags.c == 0 && flags.l == 0) {
    if (file_count > 1) {
      printf("%s:", filename);
    }
    if (flags.n) {
      printf("%d:", line_num);
    }
    printf("%s", line);
    if (line[read_bytes - 1] != '\n') {
      printf("\n");
    }
  }
}

void ProcessFile(grep_flags flags, char *pattern, char *filename, int file_count) {
  FILE *fp = fopen(filename, "r");
  if (fp != NULL) {
    regex_t regex;
    int cflags = REG_EXTENDED;
    if (flags.i) {
      cflags |= REG_ICASE;
    }
    
    if (regcomp(&regex, pattern, cflags) == 0) {
      char *line = NULL;
      size_t len = 0;
      ssize_t read_bytes;
      int line_num = 0;
      int match_count = 0;

      while ((read_bytes = getline(&line, &len, fp)) != -1) {
        line_num++;
        int status = regexec(&regex, line, 0, NULL, 0);
        int is_match = (status == 0);
        
        if (flags.v) {
          is_match = !is_match;
        }
        
        if (is_match) {
          match_count++;
          PrintMatch(flags, filename, file_count, line_num, line, read_bytes);
        }
      }

      if (flags.c) {
        if (flags.l && match_count > 0) {
          match_count = 1;
        }
        if (file_count > 1) {
          printf("%s:", filename);
        }
        printf("%d\n", match_count);
      }
      
      if (flags.l && match_count > 0) {
        printf("%s\n", filename);
      }
      
      free(line);
      regfree(&regex);
    }
    fclose(fp);
  } else {
    fprintf(stderr, "grep: %s: No such file or directory\n", filename);
  }
}

int main(int argc, char *argv[]) {
  int return_code = 0;
  grep_flags flags = {0};
  char pattern[BUFFER_SIZE] = "";

  if (ParseFlags(argc, argv, &flags, pattern) != 0) {
    return_code = 1;
  }

  if (return_code == 0 && flags.e == 0) {
    if (optind < argc) {
      AddPattern(pattern, argv[optind]);
      optind++;
    } else {
      return_code = 1;
    }
  }

  if (return_code == 0) {
    int file_count = argc - optind;
    int i = optind;
    while (i < argc) {
      ProcessFile(flags, pattern, argv[i], file_count);
      i++;
    }
  }
  
  return return_code;
}