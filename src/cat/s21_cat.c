#include "s21_cat.h"

int ParseFlags(int argc, char *argv[], cat_flags *flags) {
  int is_error = 0;
  struct option long_options[] = {
      {"number-nonblank", no_argument, NULL, 'b'},
      {"number", no_argument, NULL, 'n'},
      {"squeeze-blank", no_argument, NULL, 's'},
      {NULL, 0, NULL, 0}};

  int opt;
  while (is_error == 0 && (opt = getopt_long(argc, argv, "beEvnstT", long_options, NULL)) != -1) {
    switch (opt) {
      case 'b': flags->b = 1; break;
      case 'e': flags->e = 1; flags->v = 1; break;
      case 'E': flags->e = 1; break;
      case 'n': flags->n = 1; break;
      case 's': flags->s = 1; break;
      case 't': flags->t = 1; flags->v = 1; break;
      case 'T': flags->t = 1; break;
      case 'v': flags->v = 1; break;
      default:  is_error = 1; break;
    }
  }
  
  if (flags->b) {
    flags->n = 0;
  }
  
  return is_error;
}

void HandleLineNumbering(int ch, int prev_ch, int *line_count, cat_flags flags) {
  if (prev_ch == '\n') {
    if ((flags.b && ch != '\n') || (flags.n && flags.b == 0)) {
      (*line_count)++;
      printf("%6d\t", *line_count);
    }
  }
}

int HandleNonPrintable(int ch, cat_flags flags) {
  int char_handled = 0;
  unsigned char uc = (unsigned char)ch;
  
  if (flags.v && uc != '\n' && uc != '\t') {
    if (uc >= 128) {
      printf("M-");
      uc -= 128;
      if (uc < 32) printf("^%c", uc + 64);
      else if (uc == 127) printf("^?");
      else putchar(uc);
      char_handled = 1;
    } else if (uc < 32) {
      printf("^%c", uc + 64);
      char_handled = 1;
    } else if (uc == 127) {
      printf("^?");
      char_handled = 1;
    }
  }
  
  return char_handled;
}

void ProcessFile(FILE *fp, cat_flags flags) {
  int ch;
  int prev_ch = '\n';
  int line_count = 0;
  int empty_line_count = 0;

  while ((ch = fgetc(fp)) != EOF) {
    int skip_output = 0;
    
    if (flags.s) {
      if (prev_ch == '\n' && ch == '\n') {
        empty_line_count++;
        if (empty_line_count > 1) skip_output = 1;
      } else {
        empty_line_count = 0;
      }
    }

    if (skip_output == 0) {
      HandleLineNumbering(ch, prev_ch, &line_count, flags);

      int char_handled = 0;
      if (flags.e && ch == '\n') {
        putchar('$');
      }
      if (flags.t && ch == '\t') {
        printf("^I");
        char_handled = 1;
      }
      
      if (char_handled == 0) {
        char_handled = HandleNonPrintable(ch, flags);
      }

      if (char_handled == 0) {
        putchar(ch);
      }
      prev_ch = ch;
    }
  }
}

int main(int argc, char *argv[]) {
  int return_code = 0;
  cat_flags flags = {0};
  
  if (ParseFlags(argc, argv, &flags) != 0) {
    return_code = 1;
  }
  
  if (return_code == 0) {
    int i = optind;
    while (i < argc) {
      FILE *fp = fopen(argv[i], "r");
      if (fp != NULL) {
        ProcessFile(fp, flags);
        fclose(fp);
      } else {
        fprintf(stderr, "cat: %s: No such file or directory\n", argv[i]);
      }
      i++;
    }
  }
  
  return return_code;
}
