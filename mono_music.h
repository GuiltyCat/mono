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

int16_t* test_sine(int16_t* wave,
                   size_t   len,
                   double   amp,
                   double   freq,
                   uint32_t sampling_freq);

size_t read_line(FILE* fp, uint8_t* line, size_t num);
bool   make_sound(Wav* wav, size_t start, size_t end, double amp, double freq);

double note_to_freq(uint8_t note, uint8_t acc, int8_t octave, double A4);

void test_note_freq(void);

bool parse_line(Wav* wav, size_t start, size_t end, uint8_t* line, size_t num);

bool parse_mono_music(FILE* fp);
bool play_sheet(FILE* fp);
bool WaveTest(void);
void parse_test(void);

typedef struct Node0      Node0;
typedef struct List0      List0;
typedef struct MonoMusic0 MonoMusic0;

struct Node0 {
  /* all value is converted to integer by sampling freq */
  size_t start;  /* absolute */
  size_t period; /* absolute */
  size_t length; /* absolute */
  double volume; /* absolute */
  size_t i;
  // double (*wave)(Node0* node);
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
