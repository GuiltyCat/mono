#define DEBUG
#include <math.h>
#include "mono_image.h"
#include "mono_music.h"

//#define eprintf( ...) fprintf(stderr,  __VA_ARGS__)

typedef struct {
  FILE* input;
  FILE* output;
  // char*  input_file_name;
  // char*  output_file_name;
  double A4;
  double bpm;
} Args;

//#define HERE_DOC(...) #__VA_ARGS__ "\n"

void print_help(void) {
  FILE* fp = fopen("README.md", "r");
  if (fp == NULL) {
    eprintf("failed to open README.md\n");
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
    char* file_name;
    switch (*s) {
      case 'o':
        file_name    = s[1] == '\0' ? argv[++i] : &s[2];
        args->output = fopen(file_name, "w");
        if (args->output == NULL) {
          perror("such output does not exist.");
        }
        break;
      case 'i':
        file_name   = s[1] == '\0' ? argv[++i] : &s[2];
        args->input = fopen(file_name, "r");
        if (args->input == NULL) {
          perror("such input does not exist.");
        }
        break;
      case 'h': print_help(); return false;
      case 'p':
        args->A4 = s[1] == '\0' ? read_float(argv[++i]) : read_float(&s[2]);
        break;
      case 'b':
        args->bpm = s[1] == '\0' ? read_float(argv[++i]) : read_float(&s[2]);
        break;
      default: eprintf("unsupported option: -%c \n", argv[i][1]); return false;
    }
  }
  // if (args->input_file_name == NULL) {
  //   eprintf("-i <input_file_name>\n");
  //   return false;
  // }
  // if (args->output_file_name == NULL) {
  //   eprintf("-o <output_file_name>\n");
  //   return false;
  // }
  return true;
}

bool plot_wave(void);
#ifdef __linux__

/* https://www.alsa-project.org/alsa-doc/alsa-lib/_2test_2pcm_min_8c-example.html */
#include <alsa/asoundlib.h>
int play_music(Wav* wav) {
  int err;

  snd_pcm_t*        handle;
  snd_pcm_sframes_t frames;
  err = snd_pcm_open(&handle, "default", SND_PCM_STREAM_PLAYBACK, 0);
  if (err < 0) {
    perror("snd_pcm_open failed.");
    PRINT_DEBUG("%s\n", snd_strerror(err));
    return -1;
  }

  int          soft_resample = 1;      /* allow resampling */
  unsigned int latency       = 500000; /* 0.5 [sec] */

  err = snd_pcm_set_params(handle, SND_PCM_FORMAT_S16_LE,
                           SND_PCM_ACCESS_RW_INTERLEAVED, wav->nchannel,
                           wav->sampling_freq, soft_resample, latency);
  if (err < -1) {
    perror("snd_pcm_set_params failed.");
    PRINT_DEBUG("%s\n", snd_strerror(err));
    return -1;
  }

  /* write */

  frames = snd_pcm_writei(handle, wav->signal, wav->length);
  if (frames < 0) {
	  frames = snd_pcm_recover(handle, frames, 0);
  }
  if (frames < 0) {
    perror("snd_pcm_writei failed.");
    PRINT_DEBUG("%s\n", snd_strerror(err));
    return -1;
  }

  err = snd_pcm_drain(handle);
  if (err < 0) {
    perror("snd_pcm_drain failed.");
    PRINT_DEBUG("%s\n", snd_strerror(err));
    return -1;
  }
  err = snd_pcm_close(handle);
  if (err < 0) {
    perror("snd_pcm_close failed.");
    PRINT_DEBUG("%s\n", snd_strerror(err));
    return -1;
  }
  return 0;
}

#endif

Wav* MonoMusic02Wav(MonoMusic0* mm0);
int  main(int argc, char* argv[]) {
  // plot_wave();
  // return 0;
  Args args = {.input = stdin, .output = stdout, .A4 = 440, .bpm = 60};
  if (parse_args(argc, argv, &args) == false) {
    return 0;
  }
  MonoMusic0* mm0 = MonoMusic0Parse(args.input, 44100, args.A4, args.bpm);
  // MonoMusic0* mm0 = MonoMusic0Parse(args.input, 80000, args.A4, args.bpm);
  if (mm0 == NULL) {
    perror("mm0 == NULL failed.");
    return 0;
  }
  if (args.input != stdin) {
    fclose(args.input);
  }
  if (args.output == stdout) {
    Wav* wav = MonoMusic02Wav(mm0);
    play_music(wav);
    WavFree(wav);
  } else {
    MonoMusic0Wav(args.output, mm0);
  }
  if (args.output != stdout) {
    fclose(args.output);
  }
  MonoMusic0Free(mm0);

  return 0;
}
