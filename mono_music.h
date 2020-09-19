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


struct MonoMusic0;
typedef struct MonoMusic0 MonoMusic0;

MonoMusic0* mono_music0_parse(FILE* fp, size_t sampling_freq, double A4, double bpm);
int         mono_music0_wav(FILE* fp, MonoMusic0* mm0);

#endif
