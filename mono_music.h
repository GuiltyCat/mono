#ifndef MONO_MUSIC_H_
#define MONO_MUSIC_H_
#include <stdbool.h>

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

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

typedef struct Node0      Node0;
typedef struct List0      List0;
typedef struct MonoMusic0 MonoMusic0;

struct Node0 {
  /* all value is converted to integer by sampling freq */
  size_t start;  /* absolute */
  size_t period; /* absolute */
  size_t length; /* absolute */
  double volume; /* absolute */
  size_t i;      /* index for next */
  size_t midway; /* used for pulse wave */
  double (*wave)(Node0* node);
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

#endif
