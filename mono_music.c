#include "mono_music.h"
#include <math.h>
#define M_PI 3.14159265358979323846

size_t sec_to_length(double sec, uint32_t sampling_freq) {
  return sec * sampling_freq;
}

Wav* WavInit(double sec) {
  const uint32_t sampling_freq = 80000;
  size_t len = sec_to_length(sec, sampling_freq);
  Wav* wav = (Wav*)malloc(sizeof(Wav) + sizeof(int16_t) * len);
  if (wav == NULL) {
    return NULL;
  }
  wav->bit_size = 16;
  wav->nchannel = 1;
  wav->sampling_freq = sampling_freq;
  wav->length = len;
  return wav;
}

void WavFree(Wav* wav) {
  free(wav);
}

int WavWrite(FILE* fp, Wav* wav) {
  if (fp == NULL) {
    return -1;
  }

  uint8_t header[44] = {0};

  /* ChunkID */
  header[0] = 'R';
  header[1] = 'I';
  header[2] = 'F';
  header[3] = 'F';
  /* ChunkSize */
  int32_t chunk_size = 4 + 24 + 8 + wav->length * (wav->bit_size / 8);
  for (int i = 0; i < 4; i++) {
    header[4 + i] = (chunk_size >> i * 8) & 0xFF;
  }
  // printf("header_size=%d\n", 4 + 24 + 8);
  // printf("data size = %lu\n", wav->length * (wav->bit_size / 8));
  // printf("chunk_size=%d\n", chunk_size);

  /* Format */
  header[8] = 'W';
  header[9] = 'A';
  header[10] = 'V';
  header[11] = 'E';

  /* Subchunk1ID */
  header[12] = 'f';
  header[13] = 'm';
  header[14] = 't';
  header[15] = ' ';

  /* Subchunk1Size */
  uint32_t subchunk1size = 16;
  for (int i = 0; i < 4; i++) {
    header[16 + i] = subchunk1size >> i * 8 & 0xFF;
  }

  /* AudioFormat. PCM=1 */
  header[20] = 1;
  header[21] = 0;

  /* NumChannels. Moono=1, Stereo=2 */
  header[22] = wav->nchannel & 0xFF;
  header[23] = 0;

  /* Sample Rate */
  for (int i = 0; i < 4; i++) {
    header[24 + i] = wav->sampling_freq >> i * 8 & 0xFF;
  }

  /* ByteRate */
  uint32_t byte_rate = wav->sampling_freq * wav->nchannel * wav->bit_size / 8;
  for (int i = 0; i < 4; i++) {
    header[28 + i] = byte_rate >> i * 8 & 0xFF;
  }
  /* BlockAlign */
  uint16_t block_align = wav->nchannel * wav->bit_size / 8;
  for (int i = 0; i < 2; i++) {
    header[32 + i] = block_align >> i * 8 & 0xFF;
  }
  /* BitsPerSample */
  uint16_t bits_per_sample = wav->bit_size;
  if (bits_per_sample != 16) {
    perror("currently bits_per_sample can be only 16.");
  }
  for (int i = 0; i < 2; i++) {
    header[34 + i] = bits_per_sample >> i * 8 & 0xFF;
  }

  /* Subchunk2ID */
  header[36] = 'd';
  header[37] = 'a';
  header[38] = 't';
  header[39] = 'a';

  /* Subchunk2Size */
  uint32_t subchunk2size = wav->length * wav->bit_size / 8;
  for (int i = 0; i < 4; i++) {
    header[40 + i] = (uint8_t)(subchunk2size >> i * 8) & 0xFF;
  }
  // printf("subchunk2size = %d\n", subchunk2size);

  size_t ret;
  ret = fwrite(header, sizeof(uint8_t), sizeof(header) / sizeof(uint8_t), fp);
  if (ret != sizeof(header) / sizeof(uint8_t)) {
    perror("fwrite header size is not equal.");
    return -1;
  }

  size_t nmemb = wav->length * wav->bit_size / 8;
  ret = fwrite(wav->signal, sizeof(uint8_t), nmemb, fp);
  // printf("nmemb = %lu.\n", nmemb);
  if (ret != nmemb) {
    perror("bit_size == 8. fwrite failed.");
    return -1;
  }

  return 0;
}

size_t read_line(FILE* fp, uint8_t* line, size_t num) {
  size_t count = 0;
  int c;
  while ((c = fgetc(fp)) != EOF) {
    if (c == '\n') {
      break;
    }
    if (num <= count) {
      break;
    }
    printf("%c\n", c);
    line[count] = c;
    count++;
  }
  return count;
}
bool make_sound(Wav* wav, size_t start, size_t end, double amp, double freq) {
  size_t count = 0;
  for (size_t i = start; i < end; i++) {
    wav->signal[i] = amp * sin(2 * M_PI * count * freq / wav->sampling_freq);
    count++;
  }
  return true;
}

int note_to_num(uint8_t note, uint8_t shift) {
  /* C4 D4 E4 F4 G4 A4 B4 */
  int chromatic = 0;
  switch (shift) {
    // case '-':
    case 'b':
      chromatic = -1;
      break;
    // case '+':
    case '#':
      chromatic = 1;
      break;
    case ' ':
    default:
      chromatic = 0;
  }
  switch (note) {
    case 'C':
    case 'c':
      return -9 + chromatic;
    case 'D':
    case 'd':
      return -7 + chromatic;
    case 'E':
    case 'e':
      return -5 + chromatic;
    case 'F':
    case 'f':
      return -4 + chromatic;
    case 'G':
    case 'g':
      return -2 + chromatic;
    case 'A':
    case 'a':
      return 0 + chromatic;
    case 'B':
    case 'b':
      return 2 + chromatic;
    default:
      perror("Error");
      exit(-1);
  }
}

double octave_shift(double freq, int8_t shift) {
  return freq * pow(2, shift);
}

double base_freq_equal_temptation(int num, double A4) {
  return A4 * pow(2, num / 12.0);
}

double note_to_freq(uint8_t note, uint8_t acc, int8_t octave, double A4) {
  return octave_shift(base_freq_equal_temptation(note_to_num(note, acc), A4),
                      octave - 4);
}

void test_note_freq(void) {
  uint8_t* note = (uint8_t*)"cdefgab";
  for (size_t j = 0; j < 9; j++) {
    for (size_t i = 0; i < 7; i++) {
      printf("%cb%zu:%f\n", note[i], j, note_to_freq(note[i], 'b', j, 440.0));
      printf("%c %zu:%f\n", note[i], j, note_to_freq(note[i], ' ', j, 440.0));
      printf("%c#%zu:%f\n", note[i], j, note_to_freq(note[i], '#', j, 440.0));
    }
  }
}

bool parse_line(Wav* wav, size_t start, size_t end, uint8_t* line, size_t num) {
  printf("start, end = %zu, %zu\n", start, end);
  for (size_t i = 0; i < num; i++) {
    switch (line[i]) {
      case 'A':
      case 'a':
        make_sound(wav, start, end, 430, 440);
        break;
      case 'B':
      case 'b':
        make_sound(wav, start, end, 430, 493.88);
        break;
      case 'C':
      case 'c':
        make_sound(wav, start, end, 430, 523.25);
        break;
      case 'D':
      case 'd':
        make_sound(wav, start, end, 430, 587.33);
        break;
      case 'E':
      case 'e':
        make_sound(wav, start, end, 430, 659.26);
        break;
      case 'F':
      case 'f':
        make_sound(wav, start, end, 430, 698.46);
        break;
      case 'G':
      case 'g':
        make_sound(wav, start, end, 430, 783.99);
        break;
    }
  }
  return true;
}
bool play_sheet(FILE* fp) {
  Wav* wav = WavInit(10);
  uint8_t line[500];
  size_t one_beat = 100000;
  size_t i = 0;
  for (;;) {
    printf("in.\n");
    size_t num = read_line(fp, line, 500);
    if (num == 0) {
      break;
    }
    parse_line(wav, i * one_beat, (i + 1) * one_beat, line, num);
    i++;
  }
  FILE* wfp = fopen("test_music.wav", "wb");
  if (wfp == NULL) {
    perror("test_music.wav open failed.");
    return false;
  }
  WavWrite(wfp, wav);
  fclose(wfp);
  return true;
}

/*
 *
 * - Music: Single wave
 * - Wave : Recursive addition of waves
 * - Wave :
 *   - Volume(Amplitude) change
 *   - Frequency change
 *   - Shape change
 * - Change:
 *   - Wave
 *   - Linear
 *   - Exponent
 *   - Log
 *
 * Leaf of Wave means constant frequency that has constant value.
 * Parent of leaf node is basic wave like sine the frequency is leaf node.
 *
 * General form of wave is
 * wave_amp * wave( wave_freq )
 * wave_amp and wave_freq are also wave.
 *
 * And more wave should start and end.
 * start and end are occured when triggered.
 * trigger is defined by the number of period.
 * in each period, trigger is called.
 * when the pre-defined number of trigger is called.
 * wave start and end.
 */

struct List;
typedef struct List List;
struct Wave;
typedef struct Wave Wave;

struct List {
  size_t i;
  List* next;
  Wave* wave;
};

List* ListInit(void) {
  List* l = (List*)malloc(sizeof(List));
  l->next = NULL;
  l->wave = NULL;
  l->i = 0;
  return l;
}

void ListFree(List* l) {
  /* delete from head */
  List* next = l->next;
  Wave* wave = l->wave;
  free(wave);
  free(l);
  if (next != NULL) {
    ListFree(next);
  }
  return;
}

struct Wave {
  size_t period;
  size_t i;
  bool trigger;
};

Wave* WaveInit(double freq, size_t sampling_freq) {
  Wave* w = (Wave*)malloc(sizeof(Wave));
  if (w == NULL) {
    return NULL;
  }
  w->period = (double)sampling_freq / freq;
  w->i = 0;
  w->trigger = false;
  return w;
}
void WaveFree(Wave* w) {
  free(w);
}

double SawtoothWave(Wave* w) {
  return (double)w->i / (double)w->period - 0.5;
}

double WaveNext(Wave* w) {
  w->trigger = w->i == w->period;
  w->i %= w->period;
  double ret = SawtoothWave(w);
  w->i++;
  return ret;
}

bool WaveTest(void) {
  FILE* fp = fopen("test.wav", "wb");
  if (fp == NULL) {
    perror("fp == NULL");
    return -1;
  }

  double sec = 10;
  Wav* wav = WavInit(sec);
  if (wav == NULL) {
    perror("wav == NULL");
    return false;
  }

  size_t sampling_freq = 80000;
  Wave* w = WaveInit(440, sampling_freq);
  for (size_t i = 0; i < sec * sampling_freq; i++) {
    // break;
    //double next = 10000 * (1.0-WaveNext(w));
    double next = 10000 * WaveNext(w);
    wav->signal[i] = next;
    // printf("%d,",wav->signal[i]);
    //printf("%f,", next);
  }

  WaveFree(w);

  WavWrite(fp, wav);

  fclose(fp);
  return true;
}
