#include "mono_music.h"
#include <math.h>
#define M_PI 3.14159265358979323846

size_t sec_to_length(double sec, uint32_t sampling_freq) {
  return sec * sampling_freq;
}
Wav* WavInit(size_t len, size_t sampling_freq) {
  Wav* wav = (Wav*)calloc(sizeof(Wav) + sizeof(int16_t) * len,1);
  if (wav == NULL) {
    return NULL;
  }
  wav->bit_size      = 16;
  wav->nchannel      = 1;
  wav->sampling_freq = sampling_freq;
  wav->length        = len;
  return wav;
}

Wav* WavInitSec(double sec) {
  const uint32_t sampling_freq = 80000;
  size_t         len           = sec_to_length(sec, sampling_freq);
  return WavInit(len, sampling_freq);
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
  header[8]  = 'W';
  header[9]  = 'A';
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
  ret          = fwrite(wav->signal, sizeof(uint8_t), nmemb, fp);
  // printf("nmemb = %lu.\n", nmemb);
  if (ret != nmemb) {
    perror("bit_size == 8. fwrite failed.");
    return -1;
  }

  return 0;
}

size_t read_line(FILE* fp, uint8_t* line, size_t num) {
  size_t count = 0;
  int    c;
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

int note_to_num(uint8_t note, uint8_t shift) {
  /* C4 D4 E4 F4 G4 A4 B4 */
  int chromatic = 0;
  switch (shift) {
    // case '-':
    case 'b': chromatic = -1; break;
    // case '+':
    case '#': chromatic = 1; break;
    case ' ':
    default: chromatic = 0;
  }
  switch (note) {
    case 'C':
    case 'c': return -9 + chromatic;
    case 'D':
    case 'd': return -7 + chromatic;
    case 'E':
    case 'e': return -5 + chromatic;
    case 'F':
    case 'f': return -4 + chromatic;
    case 'G':
    case 'g': return -2 + chromatic;
    case 'A':
    case 'a': return 0 + chromatic;
    case 'B':
    case 'b': return 2 + chromatic;
    default: perror("Error"); exit(-1);
  }
}

double octave_shift(double freq, int8_t shift) {
  return freq * pow(2, shift);
}

double base_freq_equal_temptation(int num, double A4) {
  return A4 * pow(2, num / 12.0);
}

double note_to_freq(uint8_t note, uint8_t acc, int8_t octave, double A4) {
  printf("note=%c, acc=%c, octave=%d, A4=%f\n", note, acc, octave, A4);
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

struct List;
typedef struct List List;
struct Wave;
typedef struct Wave Wave;

struct List {
  size_t i;
  List*  next;
  Wave*  wave;
};

List* ListInit(void) {
  List* l = (List*)malloc(sizeof(List));
  l->next = NULL;
  l->wave = NULL;
  l->i    = 0;
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

List* ListAdd(List* node, Wave* wave) {
  List* next = ListInit();
  next->wave = wave;
  node->next = next;
  return next;
}

struct Wave {
  size_t period;
  size_t i;
  bool   trigger;
};

Wave* WaveInit(double freq, size_t sampling_freq) {
  Wave* w = (Wave*)malloc(sizeof(Wave));
  if (w == NULL) {
    return NULL;
  }
  w->period  = (double)sampling_freq / freq;
  w->i       = 0;
  w->trigger = false;
  return w;
}
void WaveFree(Wave* w) {
  free(w);
}
double SawtoothWave(Wave* w) {
  /* saw tooth Wave */
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
  Wav*   wav = WavInitSec(sec);
  if (wav == NULL) {
    perror("wav == NULL");
    return false;
  }

  size_t sampling_freq = 80000;
  Wave*  w             = WaveInit(440, sampling_freq);
  for (size_t i = 0; i < sec * sampling_freq; i++) {
    double next    = 10000 * WaveNext(w);
    wav->signal[i] = next;
  }

  WaveFree(w);

  WavWrite(fp, wav);

  fclose(fp);
  return true;
}

uint8_t parse_note(FILE* fp) {
  int c;
  while ((c = fgetc(fp)) != EOF) {
    switch (c) {
      case 'a':
      case 'A':
      case 'b':
      case 'B':
      case 'c':
      case 'C':
      case 'd':
      case 'D':
      case 'e':
      case 'E':
      case 'f':
      case 'F':
      case 'g':
      case 'G': return c;
      default: perror("Such value is not permitted."); break;
    }
  }
  return 0;
}

uint8_t parse_octave(FILE* fp) {
  int c;
  while ((c = fgetc(fp)) != EOF) {
    switch (c) {
      case '0': return 0;
      case '1': return 1;
      case '2': return 2;
      case '3': return 3;
      case '4': return 4;
      case '5': return 5;
      case '6': return 6;
      case '7': return 7;
      case '8': return 8;
      default: perror("Such value is not permitted.");
    }
  }
  perror("cannot reach here.");
  exit(-1);
}

List* parse_sheet(FILE* fp, uint32_t sampling_freq) {
  List* head = ListInit();
  List* next = head;
  for (;;) {
    if (feof(fp)) {
      break;
    }
    uint8_t note = parse_note(fp);
    if (note == 0) {
      break;
    }
    uint8_t oct  = parse_octave(fp);
    double  freq = note_to_freq(note, ' ', oct, 440);
    Wave*   w    = WaveInit(freq, sampling_freq);
    next         = ListAdd(next, w);
    // WaveFree(w);
    printf("freq=%f\n", freq);
  }
  return head;
}

bool ListToWav(FILE* fp, List* l) {
  uint32_t sampling_freq = 80000;
  double   sec           = 100;
  Wav*     wav           = WavInitSec(sec);
  if (wav == NULL) {
    return false;
  }
  size_t i = 0;
  for (List* next = l; next != NULL; next = next->next) {
    for (size_t j = 0; j < sampling_freq; j++) {
      // printf("%lu:%lu\n", i, j);
      if (next->wave)
        wav->signal[i++] = 10000 * WaveNext(next->wave);
    }
  }
  WavWrite(fp, wav);
  WavFree(wav);
  return true;
}
void parse_test(void) {
  FILE* fp = fopen("test_music.mono", "r");
  if (fp == NULL) {
    perror("open failed");
    return;
  }
  List* l = parse_sheet(fp, 80000);
  fclose(fp);
  fp = fopen("test.wav", "w");
  if (fp == NULL) {
    perror("write open failed.");
    exit(-1);
  }
  ListToWav(fp, l);
  fclose(fp);
  ListFree(l);
  return;
}

double Zwave(Node0* n) {
  /* saw tooth Node0 */
  return (double)n->i / (double)n->period - 0.5;
}
double Swave(Node0* n) {
  /* sine wave */
  return (double)sin(2 * M_PI * n->i / n->period);
}
double Pwave(Node0* n) {
  (void)n;
  /* pulse wave */
  // return n->i <= n->period * n->p
  return 0;
}
double Twave(Node0* n) {
  /* triangle wave */
  (void)n;
  return 0;
}

double Node0Next(Node0* n) {
  n->i %= n->period;
  double ret = n->volume * Zwave(n);
  // double ret = n->wave(n);
  n->i++;
  return ret;
}

List0* List0Insert(List0* l, Node0* node) {
  List0* next = malloc(sizeof(List0));
  if (next == NULL) {
    return NULL;
  }
  next->node = node;
  if (l == NULL) {
    next->next = NULL;
    return next;
  } else {
    next->next = l->next;
    l->next    = next;
  }
  return next;
}

#define PRINT_ERROR printf("%s:%s:%d\n", __FILE__, __func__, __LINE__)
double read_float(FILE* fp) {
  double num = 0;
  bool   f   = false;
  int    fc  = 0;
  int    c;
  while ((c = fgetc(fp)) != EOF) {
    // printf("c=%c\n", c);
    switch (c) {
      case '\n': continue;
      case '\r': continue;
      case ' ': continue;
      case '0': num += 0; break;
      case '1': num += 1; break;
      case '2': num += 2; break;
      case '3': num += 3; break;
      case '4': num += 4; break;
      case '5': num += 5; break;
      case '6': num += 6; break;
      case '7': num += 7; break;
      case '8': num += 8; break;
      case '9': num += 9; break;
      case '.': f = true; break;
      default: ungetc(c, fp); return num / pow(10.0, fc + 1);
    }
    num *= 10;
    if (f) {
      fc++;
    }
    // printf("num=%f, fc=%d\n",num,fc);
  }
  // PRINT_ERROR;
  return num / pow(10.0, fc);
}

// double read_int_with_times(FILE* fp, double times) {
//  double num = 0;
//  int    c;
//  while ((c = fgetc(fp)) != EOF) {
//	  printf("c=%c\n",c);
//    switch (c) {
//      case '\n': continue;
//      case '\r': continue;
//      case ' ': continue;
//      case '0': num += 0; break;
//      case '1': num += 1; break;
//      case '2': num += 2; break;
//      case '3': num += 3; break;
//      case '4': num += 4; break;
//      case '5': num += 5; break;
//      case '6': num += 6; break;
//      case '7': num += 7; break;
//      case '8': num += 8; break;
//      case '9': num += 9; break;
//      default: ungetc(c, fp); return num/10.0;
//    }
//    num *= times;
//  }
//  PRINT_ERROR;
//  return num;
//}
//
// double read_float(FILE* fp) {
//  double i = read_int_with_times(fp, 10);
//  printf("ri = %f\n",i);
//  int    c = fgetc(fp);
//  if (c == EOF || c != '.') {
//    return i;
//  }
//  double f = read_int_with_times(fp, 0.1);
//  printf("rf = %f\n",f);
//  return i + f;
//}

Node0* Node0Init(double s,
                 double p,
                 double l,
                 double v,
                 size_t sampling_freq,
                 size_t max_volume) {
  Node0* n = malloc(sizeof(Node0));
  if (n == NULL) {
    PRINT_ERROR;
    return false;
  }
  n->start  = s * sampling_freq;
  n->period = sampling_freq / p;
  n->length = l * sampling_freq;
  n->volume = max_volume * v;
  n->i      = 0;
  return n;
}

bool MonoMusic0Insert(MonoMusic0* mm0, double s, double p, double l, double v) {
  if (mm0 == NULL) {
    PRINT_ERROR;
    return false;
  }
  Node0* n = Node0Init(s, p, l, v, mm0->sampling_freq, mm0->max_volume);
  if (n == NULL) {
    PRINT_ERROR;
    return false;
  }
  List0* next = List0Insert(mm0->tail, n);
  mm0->tail   = next;
  if (next == NULL) {
    PRINT_ERROR;
    return false;
  }
  if (mm0->head == NULL) {
    /* in this case mm0->head == mm0->tail */
    mm0->head = next;
  }
  return true;
}

double read_chunk(FILE* fp, char key) {
  int c;
  while ((c = fgetc(fp)) != EOF) {
    switch (c) {
      case '\n': continue;
      case '\r': continue;
      case ' ': continue;
      default:
        if (c != key) {
          PRINT_ERROR;
          printf("c != key : %c != %c\n", c, key);
        }
        double f = read_float(fp);
        printf("%c=%f\n", key, f);
        return f;
    }
  }
  return -1;
}

MonoMusic0* mono_music0_parse(FILE* fp, size_t sampling_freq) {
  MonoMusic0* mm0 = malloc(sizeof(MonoMusic0));
  if (mm0 == NULL) {
    return NULL;
  }
  mm0->sampling_freq = sampling_freq;
  mm0->max_volume    = INT16_MAX;
  for (;;) {
    if (feof(fp)) {
      break;
    }
    double s = read_chunk(fp, 's');
    double p = read_chunk(fp, 'p');
    double l = read_chunk(fp, 'l');
    double v = read_chunk(fp, 'v');
    if (s != -1 && p != -1 && l != -1 && v != -1) {
      MonoMusic0Insert(mm0, s, p, l, v);
    }
    (void)read_chunk(fp, ',');
  }
  return mm0;
}

int mono_music0_wav(FILE* fp, MonoMusic0* mm0) {
  List0* tail = mm0->tail;
  List0* head = mm0->head;
  Wav* w = WavInit(tail->node->start + tail->node->length, mm0->sampling_freq);
  for (List0* l = head; l != NULL; l = l->next) {
    Node0* n = l->node;
    printf("s=%lu, p=%lu, l=%lu, v=%f\n", n->start, n->period, n->length,
           n->volume);
    for (size_t i = 0; i < n->length; i++) {
		//printf("i=%lu\n",n->i);
      WAV_AT(w, n->start + i) += Node0Next(n);
      //printf("%lu:%d\n",i, WAV_AT(w, n->start + i));
    }
  }
  putchar('\n');
  int ret = WavWrite(fp, w);
  WavFree(w);
  return ret;
}
