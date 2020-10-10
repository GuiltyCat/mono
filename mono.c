#include <math.h>
#include "mono_image.h"
#include "mono_music.h"

typedef struct {
  char*  input_file_name;
  char*  output_file_name;
  double A4;
  double bpm;
} Args;

//#define HERE_DOC(...) #__VA_ARGS__ "\n"

void print_help(void) {
  FILE* fp = fopen("README.md", "r");
  if (fp == NULL) {
    printf("failed to open README.md\n");
    return;
  }
  int c;
  while ((c = fgetc(fp)) != EOF) {
    putchar(c);
  }
  return;
}

double read_float(char* s) {
  double num = 0;
  int    f   = -1;
  while (*s != '\0') {
    if (*s == '.') {
      f = 0;
      s++;
      continue;
    }
    switch (*s) {
      case '0': num = num * 10 + 0; break;
      case '1': num = num * 10 + 1; break;
      case '2': num = num * 10 + 2; break;
      case '3': num = num * 10 + 3; break;
      case '4': num = num * 10 + 4; break;
      case '5': num = num * 10 + 5; break;
      case '6': num = num * 10 + 6; break;
      case '7': num = num * 10 + 7; break;
      case '8': num = num * 10 + 8; break;
      case '9': num = num * 10 + 9; break;
    }
    f += f == -1 ? 0 : 1;
    s++;
  }
  return f == -1 ? num : num / pow(10.0, f);
}

bool parse_args(int argc, char* argv[], Args* args) {
  for (int i = 0; i < argc; i++) {
    char* s = argv[i];
    if (*s != '-') {
      continue;
    }
    s++;
    switch (*s) {
      case 'o':
        args->output_file_name = s[1] == '\0' ? argv[++i] : &s[2];
        break;
      case 'i': args->input_file_name = s[1] == '\0' ? argv[++i] : &s[2]; break;
      case 'h': print_help(); return false;
      case 'p':
        args->A4 = s[1] == '\0' ? read_float(argv[++i]) : read_float(&s[2]);
        break;
      case 'b':
        args->bpm = s[1] == '\0' ? read_float(argv[++i]) : read_float(&s[2]);
        break;
      default: printf("unsupported option: -%c \n", argv[i][1]); return false;
    }
  }
  if (args->input_file_name == NULL) {
    printf("-i <input_file_name>\n");
    return false;
  }
  if (args->output_file_name == NULL) {
    printf("-o <output_file_name>\n");
    return false;
  }
  return true;
}

bool test_swave(void);

int main(int argc, char* argv[]) {
  // test_swave();
  // return 0;
  Args args = {NULL, NULL, 440, 60};
  if (parse_args(argc, argv, &args) == false) {
    return 0;
  }
  printf("input = %s, output = %s, A4 = %f\n", args.input_file_name,
         args.output_file_name, args.A4);
  FILE* fp = NULL;
  fp       = fopen(args.input_file_name, "r");
  if (fp == NULL) {
    perror("file open failed.");
    printf("%s\n", args.input_file_name);
    return 0;
  }
  MonoMusic0* mm0 = mono_music0_parse(fp, 80000, args.A4, args.bpm);
  if (mm0 == NULL) {
    perror("mm0 == NULL failed.");
    return 0;
  }
  fclose(fp);
  if (args.output_file_name == NULL || args.output_file_name[0] == '-') {
    fp = stdout;
  } else {
    fp = fopen(args.output_file_name, "wb");
  }
  if (fp == NULL) {
    perror("wav file open failed.");
    return 0;
  }
  mono_music0_wav(fp, mm0);
  fclose(fp);

  return 0;
}
