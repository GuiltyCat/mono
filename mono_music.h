#define __USE_MISC
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct {
  unsigned bit_size;
  unsigned nchannel;
  uint32_t sampling_freq;
  size_t length;
  int16_t signal[];
} Wav;

Wav *WavInit(size_t len) {
  Wav *wav = (Wav *)malloc(sizeof(Wav) + sizeof(int16_t) * len);
  if (wav == NULL) {
    return NULL;
  }
  wav->bit_size = 16;
  wav->nchannel = 1;
  wav->sampling_freq = 80000;
  wav->length = len;
  return wav;
}

void WavFree(Wav *wav) { free(wav); }

int WavWrite(FILE *fp, Wav *wav) {
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

int16_t *test_sine(int16_t *wave, size_t len, double amp, double freq,
                   uint32_t sampling_freq) {
  for (size_t i = 0; i < len; i++) {
    double s = amp * sin(2 * M_PI * i * freq / sampling_freq);
    wave[i] = (int16_t)s;
  }
  return wave;
}

int WavTest(void) {
  FILE *fp = fopen("test.wav", "wb");
  if (fp == NULL) {
    perror("fp == NULL");
    return -1;
  }

  size_t length = 1000000;
  Wav *wav = WavInit(length);
  if (wav == NULL) {
    perror("wav == NULL");
    return -1;
  }

  test_sine(wav->signal, wav->length, 2000.0, 230, wav->sampling_freq);

  WavWrite(fp, wav);

  fclose(fp);

  return 0;
}

size_t read_line(FILE *fp, uint8_t *line, size_t num) {
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

bool make_sound(Wav *wav, size_t start, size_t end, double amp, double freq) {
  size_t count = 0;
  for (size_t i = start; i < end; i++) {
    wav->signal[i] = amp * sin(2 * M_PI * count * freq / wav->sampling_freq);
    // printf("%zu:%u, ", i, wav->signal[i]);
    count++;
  }
  return true;
}

bool parse_line(Wav *wav, size_t start, size_t end, uint8_t *line, size_t num) {
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

bool play_sheet(FILE *fp) {

  Wav *wav = WavInit(100000 * 100);
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
  FILE *wfp = fopen("test_music.wav", "wb");
  if (wfp == NULL) {
    perror("test_music.wav open failed.");
    return false;
  }
  WavWrite(wfp, wav);
  fclose(wfp);
  return true;
}
