#include "mono_music.h"
#include <math.h>
#define M_PI 3.14159265358979323846

size_t gcd_inner(size_t d, size_t n) {
  if (n == 0) {
    return d;
  }
  return gcd_inner(n, d % n);
}

size_t gcd(size_t d, size_t n) {
  /*  search a such that m = a*x and n = a*y
   * d = n * q + r = a * x
   * thus  q = a* z and r = a * w            */
  if (d < n) {
    size_t tmp = n;
    n          = d;
    d          = tmp;
  }
  return gcd_inner(d, n);
}

void frac_reduction(Frac* frac) {
  size_t g = gcd(frac->n, frac->d);
  frac->n /= g;
  frac->d /= g;
}

Frac float2frac(double f) {
  Frac frac = {.d = 0, .n = 0};

  double ans = f;
  double i   = 0;

  /* calculate I0 */
  f           = 1.0 / modf(f, &i);
  size_t I[2] = {0};
  I[0]        = i;
  if (ans == i) {
    frac.d = 1;
    frac.n = I[0];
    return frac;
  }

  /* calculate I1 */
  f    = 1.0 / modf(f, &i);
  I[1] = i;

  size_t n[3] = {I[0], I[0] * I[1] + 1, 0};
  size_t d[3] = {1, I[1], 0};
  n[2]        = n[1];
  d[2]        = d[1];

  while ((double)n[2] / d[2] != ans) {
    // eprintf("%f -> %lu/%lu = %f\n", ans, n[2], d[2], (double)n[2] / d[2]);
    f    = 1.0 / modf(f, &i);
    n[2] = i * n[1] + n[0];
    n[0] = n[1];
    n[1] = n[2];
    d[2] = i * d[1] + d[0];
    d[0] = d[1];
    d[1] = d[2];
  }
  frac.n = n[2];
  frac.d = d[2];
  frac_reduction(&frac);
  return frac;
}

size_t sec_to_length(double sec, uint32_t sampling_freq) {
  return sec * sampling_freq;
}

Wav* WavInit(size_t len, size_t sampling_freq) {
  Wav* wav = (Wav*)calloc(sizeof(Wav) + sizeof(int16_t) * len, 1);
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
  // eprintf("header_size=%d\n", 4 + 24 + 8);
  // eprintf("data size = %lu\n", wav->length * (wav->bit_size / 8));
  // eprintf("chunk_size=%d\n", chunk_size);

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
  // eprintf("subchunk2size = %d\n", subchunk2size);

  size_t ret;
  ret = fwrite(header, sizeof(uint8_t), sizeof(header) / sizeof(uint8_t), fp);
  if (ret != sizeof(header) / sizeof(uint8_t)) {
    perror("fwrite header size is not equal.");
    return -1;
  }

  size_t nmemb = wav->length * wav->bit_size / 8;
  ret          = fwrite(wav->signal, sizeof(uint8_t), nmemb, fp);
  // eprintf("nmemb = %lu.\n", nmemb);
  if (ret != nmemb) {
    perror("bit_size == 8. fwrite failed.");
    return -1;
  }

  return 0;
}

/* version 0 */
/*
 * one sound is composed with these parameter below.
 *
 * w[0-2]            : kinds of wave.
 * m{+-}<num>        : one of wave parameter. Set middle point of pulse wave and triangle wave.
 * s{+-}<num>        : start time. Default num is 0. + starts from previous sound's start.
 *                   : - starts from -<num> of previous sound's end.
 * l<num>            : length of sound
 * v<num>            : volume of sound 0(min) to 1(max)
 * t{+-}<num>        : + continues sound <num> length. - continues sound (length - <num>) length.
 * p<num>            : frequency of sound
 * [a-gA-G][0-9][b#] : frequency of sound. b is flat # is sharp.
 * [><][0-9]         : shift <num> from previous sound frequency. < is down. > is up.
 *
 * if some parameters are omitted, previous sound parameter is used or
 * automatically completed. For example, if s is omitted, s is set as previous
 * sound's s + l. This means that s is set as the end of previous sound.
 *
 * each sound is separated by , and order of parameter is not considered.
 *
 * Sample,
 * w0s0v.1l1t-0.02m.5p440,<1,<1,<1
 *
 */

int get_next(FILE* fp) {
  for (;;) {
    int c = fgetc(fp);
    switch (c) {
      case ' ':
      case '\t':
      case '\v':
      case '\r':
      case '\n': continue;
      case '!':
        /* skip comment */
        for (;;) {
          c = fgetc(fp);
          if (c == EOF) {
            return c;
          } else if (c == '\n') {
            break;
          }
          // eprintf("skip %c\n", c);
        }
        continue;
      default:  // eprintf("get_next %c\n", c);
        return c;
    }
  }
}

/* these alphabet do not change appearance by capital */
// double Zwave(Node0* n) {
//   /* saw tooth Wave */
//   return 2.0*((double)n->i / (double)n->period) - 1.0;
// }
double Swave(Node0* n, size_t sampling_freq) {
  /* sine wave */
  return (double)sin(2 * M_PI * n->freq * n->i / sampling_freq);
}
double Pwave(Node0* n, size_t sampling_freq) {
  /* pulse wave */
  /* |--------| 1.0/n->freq = period
   * |-----|__| n->midway*period  |  (period - n->midway*period)
   * Both sides of areas should be equal to 1 * period/2 = period/2.
   * |----|____| 1 to -1 mid point is period/2.
   *
   * n->midway*period * left_height = period /2.
   * left_height = 1/(2*n->midway)
   * right_height = - 1/(2*(1-n->midway))
   */
  double period = sampling_freq / n->freq;
  return fmod(n->i, period) <= n->midway * period
             ? (double)1.0 / (2 * n->midway)
             : -1.0 / (2 * (1.0 - n->midway));
}

double Twave(Node0* n, size_t sampling_freq) {
  /* triangle wave */
  /* |------| n->length
   * |///\\\| n->midway*period  /\  (period - n->midway*period)
   * then shift phase in order to start from and end with 0.
   * Inclination of left up line is 2.0/(n->midway*period).
   * (y is -1 to 1 and x is 0 to period - n->midway*period).
   * Inclination of right down line is 2.0/((1-n->midway)*period).
   * (y is 1 to -1 and x is period - n->midway*period to period )
   * And calculate bias.
   * left lines bias is simply -1.
   * right lines bias is little bit difficult.
   * left and right lines cross at (m*period, 1).
   * This means that right line starts from (m*period, 1).
   * Thus right line can be expressed as
   * y - 1 = 2/((1-midway)*period) ( x - m*period )
   * Bias is y's value when x = 0.
   * Substitute 0 for x.
   * y - 1 = 2/((1-midway)*period) ( - m*period )
   * y = 1 + 2/((1-midway))
   * This is bias of right line.
   * */
  double period = sampling_freq / n->freq;
  double i      = fmod(n->i + n->midway * period / 2.0, period);
  return i <= n->midway * period ? 2.0 * i / (n->midway * period) - 1.0
                                 : 1.0 + 2.0 * n->midway / (1.0 - n->midway) -
                                       2.0 * i / ((1.0 - n->midway) * period);
}

double Node0Next(Node0* n, size_t sampling_freq) {
  n->i %= n->lcm_period;
  // n->i %= n->period;
  // double ret = n->volume * Zwave(n);
  double ret = n->volume * n->wave(n, sampling_freq);
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

void List0Free(List0* l) {
  if (l == NULL) {
    return;
  }
  List0* next = l->next;
  Node0* node = l->node;
  if (node != NULL) {
    free(node);
  }
  free(l);
  List0Free(next);
}

void MonoMusic0Free(MonoMusic0* mm0) {
  List0Free(mm0->head);
  free(mm0);
}

int key_value(char key) {
  /* 0-5 for chunk
   * 6-12 for scale */
  switch (key) {
    case 'S':
    case 's': return 0;
    case 'L':
    case 'l': return 1;
    case 'W':
    case 'w': return 2;
    case 'M':
    case 'm': return 3;
    case 'P':
    case 'p': return 4;
    case 'V':
    case 'v': return 5;
    case 'T':
    case 't':
      return 6;
      /* for  */
    case 'A':
    case 'a': return 15;
    case 'B':
    case 'b': return 16;
    case 'C':
    case 'c': return 10;
    case 'D':
    case 'd': return 11;
    case 'E':
    case 'e': return 12;
    case 'F':
    case 'f': return 13;
    case 'G':
    case 'g': return 14;

    case '<': return 20;
    case '>': return 21;
    default:
      PRINT_ERROR("such key is not expected.");
      // eprintf("key=%c:%d\n", key, key);
      return -1;
  }
}

int read_sign(FILE* fp) {
  int c = get_next(fp);
  switch (c) {
    case '+': return 1;
    case '-': return -1;
    case EOF: return 0;
    default: ungetc(c, fp); return 0;
  }
}
static double read_float(FILE* fp) {
  double num = 0;
  int    f   = -1;
  int    c;
  for (;;) {
    c = get_next(fp);
    if (c == '.') {
      f = 0;
      continue;
    }
    // eprintf("c=%c\n", c);
    switch (c) {
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
      default:
        if (c != EOF) {
          ungetc(c, fp);
          // eprintf("ungetc %c\n", c);
        }
        return num == 0 || f == -1 ? num : num / pow(10.0, f);
    }
    f += f == -1 ? 0 : 1;
    // eprintf("n=%f, f=%d\n", num, f);
  }
}

void read_float_test(void) {
  FILE* fp = fopen("float.txt", "r");
  if (fp == NULL) {
    eprintf("fp == NULL");
    return;
  }
  while (!feof(fp)) {
    // double num = read_float(fp);
    fgetc(fp);
    // eprintf("ln=%f\n\n", num);
  }
  return;
}

double read_fraction(FILE* fp, int* sign) {
  *sign    = read_sign(fp);
  double n = read_float(fp);
  double d = 1;
  int    c = get_next(fp);
  if (c == '/') {
    d = read_float(fp);
  } else {
    ungetc(c, fp);
  }
  return n / d;
}

int read_acc(FILE* fp) {
  int c = get_next(fp);
  switch (c) {
    case '#': return +1;
    case 'b': return -1;
    default:
      if (c != EOF) {
        ungetc(c, fp);
      }
      return 0;
  }
}
int read_octave(FILE* fp) {
  int c = get_next(fp);
  switch (c) {
    case '0': return 1;
    case '1': return 1;
    case '2': return 2;
    case '3': return 3;
    case '4': return 4;
    case '5': return 5;
    case '6': return 6;
    case '7': return 7;
    case '8': return 8;
    case '9': return 9;
    default:
      if (c != EOF) {
        ungetc(c, fp);
      }
      return 4;
  }
}

int read_scale(int num) {
  /* CDEFGAB
   *
   * A, A# == Bb
   * */
  switch (num) {
    case 10: return -9;
    case 11: return -7;
    case 12: return -5;
    case 13: return -4;
    case 14: return -2;
    case 15: return 0;
    case 16: return 2;
    default: PRINT_ERROR("num is not allowed."); return -1;
  }
}

double octave_shift(double freq, int8_t shift) {
  return freq * pow(2, shift);
}

double note_shift(double freq, int8_t shift) {
  return freq * pow(2, shift / 12.0);
}

double base_freq_equal_temptation(int num, double A4) {
  return A4 * pow(2, num / 12.0);
}

double read_note(int num, FILE* fp, double A4) {
  int note = read_scale(num);
  int oct  = read_octave(fp);
  int acc  = read_acc(fp);
  // eprintf("note = %d, oct = %d, acc = %d\n",note,oct,acc);
  /* requre A4 freq */
  double base = base_freq_equal_temptation(note + acc, A4);
  // eprintf("base = %f\n",base);
  // eprintf("oc   = %f\n", octave_shift(base, oct) );
  return octave_shift(base, oct - 4);
}

double read_shift(FILE* fp, double freq, int sign) {
  int shift = sign * 2 * read_float(fp);
  // eprintf("freq = %f, shift = %d\n", freq, shift);
  return note_shift(freq, shift);
}

bool read_chunk0(FILE* fp, Chunk0 chunk[], double A4, double spb) {
  int c = get_next(fp);
  if (c == EOF) {
    PRINT_ERROR("c == EOF");
    return false;
  }
  // char key = c;
  int i = key_value((char)c);
  if (i == -1) {
    PRINT_ERROR("i==-1");
    perror("v == -1");
    return false;
  } else if (i <= 1) {
    chunk[i].value = spb * read_fraction(fp, &chunk[i].sign);
  } else if (i <= 6) {
    /* for chunk */
    chunk[i].value = read_fraction(fp, &chunk[i].sign);
  } else if (i < 17) {
    // eprintf("c = %d\n",c);
    /* for scale */
    size_t j       = key_value('p');
    chunk[j].value = read_note(i, fp, A4);
    // eprintf("read_note = %f\n", chunk[j].value);
  } else {
    size_t j       = key_value('p');
    chunk[j].value = read_shift(fp, chunk[j].value, i == 20 ? -1 : 1);
  }

  // eprintf("%c:chunk[%d] = %f\n", key, i, chunk[i]);
  return true;
  PRINT_ERROR(" you cannot reach here");
  return false;
}

Node0* Node0InitChunk0(Chunk0 chunk[],
                       size_t sampling_freq,
                       size_t max_volume) {
  Node0* n = malloc(sizeof(Node0));
  if (n == NULL) {
    PRINT_ERROR("n == NULL");
    return NULL;
  }
  n->start = chunk[key_value('s')].value * sampling_freq;
  n->freq  = chunk[key_value('p')].value;
  /* 1/s * n = 1/f * d */
  // eprintf("samp/freq=%lu/%f = %f\n", sampling_freq, n->freq, sampling_freq /
  // n->freq);
  Frac frac = float2frac(sampling_freq / n->freq);
  // eprintf("frac = %lu/%lu\n", frac.n, frac.d);
  // n->lcm_period =  frac.n;
  n->lcm_period = sampling_freq * frac.d;
  // n->period = round(sampling_freq / chunk[key_value('p')].value);
  n->length = chunk[key_value('l')].value * sampling_freq;
  /* end of wave must be 0 really works? */
  // n->length = n->length - n->length % n->period;
  n->volume = max_volume * chunk[key_value('v')].value;
  // n->midway = n->period * chunk[key_value('m')].value;
  n->midway = chunk[key_value('m')].value;
  n->i      = 0;
  // n->till   = n->length * chunk[key_value('t')].value;
  n->till = n->length - sampling_freq * chunk[key_value('t')].value;
  // eprintf("n->till = %lu, n->period = %lu\n", n->till, n->period);
  // n->till = n->till - n->till % n->period;
  switch ((int)chunk[key_value('w')].value) {
    case 0: n->wave = Swave; break;
    case 1: n->wave = Pwave; break;
    case 2: n->wave = Twave; break;
    default:
      PRINT_ERROR("such wave does not expected.");
      PRINT_ERROR("such w does not exist.");
      // eprintf("w=%f\n", chunk[key_value('w')].value);
      break;
  }
  // eprintf("s=%lu, l=%lu, p=%lu, m=%lu, v=%f\n", n->start, n->period,
  // n->midway,
  //        n->length, n->volume);
  return n;
}

bool MonoMusic0InsertChunk0(MonoMusic0* mm0, Chunk0 chunk[]) {
  if (mm0 == NULL) {
    PRINT_ERROR("mm0 == NULL");
    return false;
  }
  Node0* n = Node0InitChunk0(chunk, mm0->sampling_freq, mm0->max_volume);
  if (n == NULL) {
    PRINT_ERROR("n == NULL");
    return false;
  }
  // eprintf("List0Insert\n");
  List0* next = List0Insert(mm0->tail, n);
  // eprintf("next = %p\n", next);
  mm0->tail = next;
  if (next == NULL) {
    PRINT_ERROR("next == NULL");
    return false;
  }
  if (mm0->head == NULL) {
    /* in this case mm0->head == mm0->tail */
    mm0->head = next;
  }
  return true;
}

MonoMusic0* MonoMusic0Parse(FILE*  fp,
                            size_t sampling_freq,
                            double A4,
                            double bpm) {
  MonoMusic0* mm0 = malloc(sizeof(MonoMusic0));
  if (mm0 == NULL) {
    return NULL;
  }
  mm0->tail          = NULL;
  mm0->head          = NULL;
  mm0->sampling_freq = sampling_freq;
  mm0->max_volume    = INT16_MAX;

  /* second per beats */
  double spb = 60.0 / bpm;
  // eprintf("spb = %f\n", spb);

  Chunk0 chunk[7]             = {0};
  chunk[key_value('s')].value = 0;
  chunk[key_value('p')].value = 0;
  chunk[key_value('l')].value = 1;
  chunk[key_value('v')].value = 0.2;
  chunk[key_value('m')].value = 0;
  chunk[key_value('t')].value = 0;
  chunk[key_value('w')].value = 0;

  double start = 0;
  double end   = 0;

  for (;;) {
    int c = get_next(fp);
    if (c == ',' || c == EOF) {
      int s = key_value('s');
      switch (chunk[s].sign) {
        case +1: chunk[s].value += start; break;
        case 0: break;
        case -1: chunk[s].value = end - chunk[s].value; break;
        default: PRINT_ERROR("such sign is not permitted."); break;
      }
      int t = key_value('t');
      if (chunk[t].sign == 1) {
        // eprintf("t: %f ->", chunk[t].value);
        chunk[t].value = chunk[key_value('l')].value - chunk[t].value;
        // eprintf(" %f\n", chunk[t].value);
      } else {
        // eprintf("sign != 1\n");
      }
      MonoMusic0InsertChunk0(mm0, chunk);
      if (c == EOF) {
        break;
      }
      start                       = chunk[key_value('s')].value;
      end                         = start + chunk[key_value('l')].value;
      chunk[key_value('s')].value = end;
      for (size_t i = 0; i < 6; i++) {
        chunk[i].sign = 0;
      }
    } else {
      ungetc(c, fp);
    }
    if (read_chunk0(fp, chunk, A4, spb) == false) {
      PRINT_ERROR("read_chunk0(fp, chunk) == false");
      perror("read_chunk0 == false");
      break;
    }
  }
  return mm0;
}
Wav* MonoMusic02Wav(MonoMusic0* mm0) {
  List0* tail = mm0->tail;
  List0* head = mm0->head;
  if (tail == NULL || head == NULL) {
    PRINT_ERROR("tail == NULL || head == NULL");
    return NULL;
  }
  size_t length = 0;
  for (List0* l = head; l != NULL; l = l->next) {
    Node0* n = l->node;
    if (n->start + n->length > length) {
      length = n->start + n->length;
    }
  }
  // eprintf("start = %lu\n", tail->node->start);
  // eprintf("length = %lu\n", tail->node->length);
  Wav* w = WavInit(length, mm0->sampling_freq);
  for (List0* l = head; l != NULL; l = l->next) {
    Node0* n = l->node;
    if (n == NULL) {
      PRINT_ERROR("l->node == NULL\n");
      break;
    }
    // eprintf("s=%lu, p=%lu, l=%lu, v=%f\n", n->start, n->period, n->length,
    // n->volume);
    // for (size_t i = 0; i < n->length; i++) {
    for (size_t i = 0; i < n->till; i++) {
      // eprintf("i=%lu\n",n->i);
      int value = Node0Next(n, mm0->sampling_freq);
      if ((int)WAV_AT(w, n->start + i) + value > INT16_MAX) {
        // eprintf("OVERFLOW OCCURED: %lu\n", i);
        goto SKIP_FOR;
      }
      WAV_AT(w, n->start + i) += value;
      // eprintf("%lu:%d\n",i, WAV_AT(w, n->start + i));
    }
  SKIP_FOR:;
  }
  return w;
}

int MonoMusic0Wav(FILE* fp, MonoMusic0* mm0) {
  Wav* wav = MonoMusic02Wav(mm0);
  if (wav == NULL) {
    perror("MonoMusic02Wav failed.");
    return -1;
  }
  int ret = WavWrite(fp, wav);
  WavFree(wav);
  return ret;
}

bool print_wave(int wave_type, double m, FILE* fp) {
  size_t sampling_freq        = 80000;
  double sec                  = 0.01;
  Chunk0 chunk[7]             = {0};
  chunk[key_value('s')].value = 0;
  chunk[key_value('p')].value = 440;
  chunk[key_value('l')].value = sec * sampling_freq;
  chunk[key_value('v')].value = 1;
  chunk[key_value('m')].value = m;
  chunk[key_value('t')].value = 0;
  chunk[key_value('w')].value = wave_type;
  Node0* n                    = Node0InitChunk0(chunk, sampling_freq, 1);
  for (size_t i = 0; i < sampling_freq * sec; i++) {
    double s0 = Node0Next(n, sampling_freq);
    fprintf(fp, "%lu %f\n", i, s0);
  }
  free(n);
  return true;
}

bool plot_wave(void) {
  FILE* pp = popen("gnuplot -p", "w");
  // FILE* pp = stdout;
  if (pp == NULL) {
    perror("gnuplot does not exist.");
    return false;
  }
  double m = 1.0;
  // FILE* tmp = pp;
  // pp = stdout;
  fprintf(pp, "plot ");
  fprintf(pp, "'-' using 1:2 with lines title 'sine wave %d',", 0);
  fprintf(pp, "'-' using 1:2 with lines title 'pulse wave %d',", 1);
  fprintf(pp, "'-' using 1:2 with lines title 'triangle wave %d: m=%f'\n", 2,
          m);

  print_wave(0, 0.5, pp);
  fprintf(pp, "e\n");
  print_wave(1, 0.5, pp);
  fprintf(pp, "e\n");
  print_wave(2, m, pp);
  fprintf(pp, "e\n");
  // pp=tmp;
  pclose(pp);
  return true;
}

/* version 1 */
/*
 * More theoretical and recursive.
 *
 * wave had three parameter
 * - a : Amplitude
 * - f : Frequency
 * - w : wave types
 * - t : Time
 *
 * Fundamental wave types are
 * - Sine wave
 * - Triangle wave
 * - Pulse wave   
 * These fundamental waves takes only constant parameter at last.
 *
 *
 * All of these are also wave.
 *
 *
 *
 */
/* rule is little bit different.
 *
 * r<num>            : reference pitch of A8.
 * w[0-2]            : kinds of wave.
 * m{+-}<num>        : one of wave parameter. Set middle point of pulse wave and triangle wave.
 * s{+-}<num>        : start time. Default num is 0. + starts from previous sound's start.
 *                   : - starts from -<num> of previous sound's end.
 * l<num>            : length of sound
 * v<num>            : volume of sound 0(min) to 1(max)
 * t{+-}<num>        : + continues sound <num> length. - continues sound (length - <num>) length.
 * p<num>            : frequency of sound
 * [a-gA-G][0-9][b#] : frequency of sound. b is flat # is sharp.
 * [><][0-9]         : shift <num> from previous sound frequency. < is down. > is up.
 *
 * This is not theoretical.
 *
 * Available Command (Basic)
 *
 * o : Oscillator and low frequency oscillator
 *   : s : sine
 *   : p : pulse with parameter m.
 *   : t : triangle wave with parameter m.
 *
 *   : d : depth (amplitude)
 *   : r : rate (frequency)
 *
 * a : Amp 
 *   : v : maximum volume
 *
 * e : envelope generator
 *   : a : attack time [pulse]. Time to reach max volume
 *   : d : decay time [pulse]. Time to reach sustain volume.
 *   : s : sustain level. 
 *   : r : release time [pulse]. Time to continue after end of pulse.
 *
 * f : Filter
 *   : c : cutoff
 *   : r : resonance
 *   
 *
 * t : Time
 *   : p : one pulse length [sec]
 *   : s : start pulse [pulse]
 *   : d : duration [pulse]. (t in version 0)
 *   : n : note value [pulse]. (l in version 0)
 *
 * p : Pitch ???
 *   : p : pitch
 *
 *
 *
 * Complex
 *
 * a
 * b
 * c
 * e
 * f
 * g
 * h
 * j
 * k
 * m
 * n 
 * o
 * p
 * q
 * r
 * t : 
 * v
 * w
* x
* y
* z
*
* Shortcut
*
* a
* b
* c
* d
* e
* f
* g
*
* 
*
*
*
*
*/

typedef struct Node1      Node1;
typedef struct Chunk1     Chunk1;
typedef struct Var1       Var1;
typedef struct List1      List1;
typedef struct MonoMusic1 MonoMusic1;

struct Chunk1 {
	/* 0 absolute
	 * +1 or -1 relative */
	int    sign;
	double value;
};

struct Var1 {
	Node1* head;
};

typedef enum { NODE, VAR1 } Node1Type;

struct Node1 {
	Node1Type type;
	union {
		Var1* var;
		struct {
			Chunk1 w;
			Chunk1 m;
			Chunk1 v;
			Chunk1 s;
			Chunk1 l;
			Chunk1 t;
			Chunk1 p;
		};
	};
};

// Var1* Var1Insert(Var1* var, Node1* node) {
//  Var1* next = malloc(sizeof(Var1));
//  if (next == NULL) {
//    return NULL;
//  }
//  next->node = node;
//  if (var == NULL) {
//    next->next = NULL;
//    return next;
//  } else {
//    next->next = var->next;
//    var->next  = next;
//  }
//  return next;
//}
//
// void Var1Free(Var1* var) {
//  if (var == NULL) {
//    return;
//  }
//  Var1* next = var->next;
//  free(var);
//  Var1Free(next);
//}

struct List1 {
	List1* next;
	Var1*  var;
};

List1* List1Insert(List1* l, Var1* var) {
	List1* next = malloc(sizeof(List1));
	if (next == NULL) {
		return NULL;
	}
	next->var = var;
	if (l == NULL) {
		next->next = NULL;
		return next;
	} else {
		next->next = l->next;
		l->next    = next;
	}
	return next;
}

void List1Free(List1* l) {
	if (l == NULL) {
		return;
	}
	List1* next = l->next;
	Var1*  var  = l->var;
	if (var != NULL) {
		// Var1Free(var);
		return;
	}
	free(l);
	List1Free(next);
}

struct MonoMusic1 {
	size_t sampling_freq;
	size_t max_volume;
	List1* head;
	List1* tail;
};

// void MonoMusic1Parse(FILE* fp, size_t sampling_freq) {
//}

void MonoMusic1Free(MonoMusic1* mm) {
	List1Free(mm->head);
	free(mm);
}
