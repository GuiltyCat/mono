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
  Wav *wav = malloc(sizeof(Wav) + sizeof(int16_t) * len);
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
