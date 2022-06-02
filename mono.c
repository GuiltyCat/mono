#define DEBUG
#include <math.h>
//#include "mono_image.h"
#include "mono_music.h"

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

/* https://www.alsa-project.org/alsa-doc/alsa-lib/ */
#include <alsa/asoundlib.h>
int play_music(Wav* wav) {
  int               err;
  snd_pcm_t*        handle;
  snd_pcm_sframes_t frames;
  //err = snd_pcm_open(&handle, "default", SND_PCM_STREAM_PLAYBACK, SND_PCM_NONBLOCK);
  err = snd_pcm_open(&handle, "default", SND_PCM_STREAM_PLAYBACK, SND_PCM_ASYNC);
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
#include <ncurses.h>
#include <unistd.h>

int keymap(int c) {
  switch (c) {
    /* b4 */
    case 'z': return 1;
    /* c5 */
    case 'x': return 2;
    /* c5# == d5b */
    case 'd': return 3;
    /* d5 */
    case 'c': return 4;
    /* d5# == e5b */
    case 'f': return 5;
    /* e5 == f5b */
    case 'v': return 6;
    /* f5 == e5# */
    case 'b': return 7;
    /* f5# == g5b */
    case 'h': return 8;
    /* g5 */
    case 'n': return 9;
    /* g5# == a5b */
    case 'j': return 10;
    /* a5 */
    case 'm': return 11;
    /* a5# == b5b */
    case 'k': return 12;
    /* b5 */
    case ',': return 13;
    default: return -1;
  }
}

int keyboard_player(void) {
  double A4         = 440;
  size_t max_volume = INT16_MAX;

  size_t sampling_freq    = 44100;
  size_t one_sound_length = sampling_freq;
  Wav*   wav              = WavInit(one_sound_length, sampling_freq);
  if (wav == NULL) {
    PRINT_DEBUG("WavInit failed.\n");
    return -1;
  }

  /* prepare alsa */
  int               err;
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

  /* prepare ncurses */
  initscr();
  noecho();
  cbreak();
  //timeout(0);

  /* read keyboard and write to alsa */
  int c = 0;
  for (;;) {
    c = getch();

    if (c == 'q') {
      break;
    }
    c = keymap(c);
    if (c < -1) {
      continue;
    }

    double freq                 = base_freq_equal_temptation(c, A4);
    Chunk0 chunk[7]             = {0};
    chunk[key_value('s')].value = 0;
    chunk[key_value('p')].value = freq;
    chunk[key_value('l')].value = one_sound_length;
    chunk[key_value('v')].value = 0.2;
    chunk[key_value('m')].value = 0.5;
    chunk[key_value('t')].value = one_sound_length;
    chunk[key_value('w')].value = 0;

    Node0* node = Node0InitChunk0(chunk, wav->sampling_freq, max_volume);
    for (size_t i = 0; i < wav->length; i++) {
		//printf("%lu\n",i);
      wav->signal[i] = Node0Next(node, wav->sampling_freq);
    }
    free(node);

	printf("ok %lu\n",wav->length);
    frames = snd_pcm_writei(handle, wav->signal, wav->length);
	//sleep(1);
	(void)frames;
    // if (frames < 0) {
    //   frames = snd_pcm_recover(handle, frames, 0);
    // }
    // if (frames < 0) {
    //   perror("snd_pcm_writei failed.");
    //   PRINT_DEBUG("%s\n", snd_strerror(err));
	//   break;
    //   //return -1;
    // }
  }

  /* close ncurses */
  endwin();

  /* close alsa */
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

int main(int argc, char* argv[]) {
 // keyboard_player();
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
    PRINT_ERROR("player mode is disabled.\n");
  } else {
    MonoMusic0Wav(args.output, mm0);
  }
  if (args.output != stdout) {
    fclose(args.output);
  }
  MonoMusic0Free(mm0);

  return 0;
}
