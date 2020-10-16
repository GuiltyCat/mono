#ifndef MONO_MUSIC_H_
#define MONO_MUSIC_H_
#include <stdbool.h>

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#ifdef DEBUG
#define eprintf(...) fprintf(stderr, __VA_ARGS__)
#define PRINT_ERROR(...) \
  fprintf(stderr, "error:%s:%s:%d:", __FILE__, __func__, __LINE__, s);fprintf(stderr,__VA_ARGS__)
#define PRINT_DEBUG(s) \
  fprintf(stderr, "debug:%s:%s:%d:", __FILE__, __func__, __LINE__, s);fprintf(stderr,__VA_ARGS__)
#else
#include<assert.h>
#define eprintf(...) (void)(0)
#define PRINT_ERROR(...) (void)(0)
#define PRINT_ERROR(...) (void)(0)

#endif

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

MonoMusic0* mono_music0_parse(FILE*  fp,
                              size_t sampling_freq,
                              double A4,
                              double bpm);
int         mono_music0_wav(FILE* fp, MonoMusic0* mm0);

typedef struct {
  /* n/d */
  size_t n;
  size_t d;
} Frac;

Frac float2frac(double f);
#endif
