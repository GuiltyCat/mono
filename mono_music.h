#ifndef MONO_MUSIC_H_
#define MONO_MUSIC_H_
#include <stdbool.h>
#include "debug.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

double base_freq_equal_temptation(int num, double A4);
int    key_value(char key);

typedef struct Node0 Node0;
typedef struct List0 List0;
struct Node0 {
  /* all value is converted to integer by sampling freq */
  size_t start;      /* absolute */
  size_t lcm_period; /* absolute */
  // size_t deno_period; /* absolute */
  double freq; /* absolute */
  // size_t period; /* absolute */
  size_t length; /* absolute */
  double volume; /* absolute */
  size_t i;      /* index for next */
  // size_t midway; /* used for pulse wave */
  double midway; /* used for pulse wave */
  size_t till;   /* sec */
  double (*wave)(Node0* node, size_t sampling_freq);
};

struct List0 {
  /* all node should be ordered by start */
  List0* next;
  Node0* node;
};

struct MonoMusic0 {
  size_t sampling_freq;
  size_t max_volume;
  List0* head;
  List0* tail;
};


typedef struct {
  double value;
  /* +1 means relative +
   * 0 means absolute
   * -1  means relative - */
  int sign;
} Chunk0;
Node0* Node0InitChunk0(Chunk0 chunk[], size_t sampling_freq, size_t max_volume);
double Node0Next(Node0* n, size_t sampling_freq) ;

typedef struct {
  unsigned bit_size;
  unsigned nchannel;
  uint32_t sampling_freq;
  size_t   length;
  int16_t  signal[];
} Wav;

#define WAV_AT(wav, i) wav->signal[i]

Wav* WavInit(size_t len, size_t sampling_freq);
Wav* WavInitSec(double sec);
void WavFree(Wav* wav);
int  WavWrite(FILE* fp, Wav* wav);

bool WaveTest(void);

struct MonoMusic0;
typedef struct MonoMusic0 MonoMusic0;

MonoMusic0* MonoMusic0Parse(FILE*  fp,
                            size_t sampling_freq,
                            double A4,
                            double bpm);
int         MonoMusic0Wav(FILE* fp, MonoMusic0* mm0);
Wav*        MonoMusic02Wav(MonoMusic0* mm0);
void        MonoMusic0Free(MonoMusic0* mm0);

typedef struct {
  /* n/d */
  size_t n;
  size_t d;
} Frac;

Frac float2frac(double f);
bool plot_wave(void);
#endif
